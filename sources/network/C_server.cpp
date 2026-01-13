/*
 * Copyright 2025 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FastEngine/network/C_server.hpp"
#include "FastEngine/C_alloca.hpp"
#include "FastEngine/manager/network_manager.hpp"
#include "private/fge_crypt.hpp"
#include "private/fge_debug.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>

using namespace fge::priv;

namespace fge::net
{

//ReturnPacketHandler
std::optional<Error> ReturnPacketHandler::handleReturnPacket(ClientSharedPtr const& refClient,
                                                             ClientContext& clientContext,
                                                             ReceivedPacketPtr& packet) const
{
    //Check if the packet is a return packet, and if so, handle it
    if (packet->retrieveHeaderId().value() != NET_INTERNAL_ID_RETURN_PACKET)
    {
        return std::nullopt;
    }

    //TODO: verify the eventSize variable
    //TODO: make fge::net::rules more friendly to use

    constexpr char const* const func = __func__;

    using namespace fge::net::rules;
    auto const err = RValid<uint16_t>({packet->packet()})
                             .and_for_each([&](auto const& chain, [[maybe_unused]] auto& index) {
        ReturnEvents event;
        chain.packet() >> event;
        if (!chain.packet().isValid())
        {
            return chain.stop("received bad event", func);
        }

        uint16_t eventSize = 0;
        chain.packet() >> eventSize;
        if (!chain.packet().isValid())
        {
            return chain.stop("received bad event size", func);
        }

        switch (event)
        {
        case ReturnEvents::REVT_SIMPLE:
        {
            uint16_t id;
            chain.packet() >> id;
            if (!chain.packet().isValid() || eventSize != sizeof(uint16_t))
            {
                return chain.stop("received bad id / size", func);
            }

            this->_onClientSimpleReturnEvent.call(refClient, packet->getIdentity(), id);
        }
        break;
        case ReturnEvents::REVT_OBJECT:
        {
            uint16_t commandIndex;
            ObjectSid parentSid, targetSid;
            chain.packet() >> commandIndex >> parentSid >> targetSid;
            if (!chain.packet().isValid())
            {
                return chain.stop("received bad id", func);
            }

            this->_onClientObjectReturnEvent.call(refClient, packet->getIdentity(), commandIndex, parentSid, targetSid,
                                                  packet);
        }
        break;
        case ReturnEvents::REVT_ASK_FULL_UPDATE:
            if (!chain.packet().isValid() || eventSize != 0)
            {
                return chain.stop("received bad id / size", func);
            }

            this->_onClientAskFullUpdate.call(refClient, packet->getIdentity());
            break;
        case ReturnEvents::REVT_CUSTOM:
            if (!chain.packet().isValid())
            {
                return chain.stop("received bad id", func);
            }

            this->_onClientReturnEvent.call(refClient, packet->getIdentity(), packet);
            break;
        }

        return chain.skip();
    }).end();

    if (err)
    {
        FGE_DEBUG_PRINT("Return packet error");
    }

    //TODO: be sure that we are on the correct read position on the packet
    //We unpack and compute the latency with the LatencyPlanner helper class
    refClient->_latencyPlanner.unpack(packet.get(), *refClient);
    if (auto latency = refClient->_latencyPlanner.getLatency())
    {
        refClient->setSTOCLatency_ms(latency.value());
    }
    if (auto latency = refClient->_latencyPlanner.getOtherSideLatency())
    {
        refClient->setCTOSLatency_ms(latency.value());
    }

    //Extract acknowledged packets
    static std::vector<PacketCache::Label> acknowledgedPackets;
    SizeType acknowledgedPacketsSize = 0;
    packet->packet() >> acknowledgedPacketsSize;
    acknowledgedPackets.resize(acknowledgedPacketsSize);
    for (SizeType i = 0; i < acknowledgedPacketsSize; ++i)
    {
        PacketCache::Label label;
        packet->packet() >> label._counter >> label._realm;
        if (!packet->isValid())
        {
            FGE_DEBUG_PRINT("received bad label");
            return Error(Error::Types::ERR_DATA, packet->getReadPos(), "received bad acknowledged label data", func);
        }
        acknowledgedPackets[i] = label;
    }

    clientContext._cache.acknowledgeReception(acknowledgedPackets);

    this->_onClientReturnPacket.call(refClient, packet->getIdentity(), packet);

    packet.reset();
    return err;
}

//ServerFluxUdp
NetFluxUdp::NetFluxUdp(bool defaultFlux) :
        g_isDefaultFlux(defaultFlux)
{}
NetFluxUdp::~NetFluxUdp()
{
    this->clearPackets();
}

void NetFluxUdp::clearPackets()
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.clear();
    this->_g_remainingPackets = 0;
}
bool NetFluxUdp::pushPacket(ReceivedPacketPtr&& fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    if (this->_g_packets.size() >= this->g_maxPackets)
    {
        return false;
    }
    this->_g_packets.push_back(std::move(fluxPck));
    return true;
}
void NetFluxUdp::forcePushPacket(ReceivedPacketPtr fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.push_back(std::move(fluxPck));
}
void NetFluxUdp::forcePushPacketFront(ReceivedPacketPtr fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.push_front(std::move(fluxPck));
}

FluxProcessResults NetFluxUdp::processReorder(PacketReorderer& reorderer,
                                              ReceivedPacketPtr& packet,
                                              ProtocolPacket::CounterType peerCounter,
                                              ProtocolPacket::CounterType peerReorderedCounter,
                                              ProtocolPacket::RealmType realm,
                                              bool ignoreRealm)
{
    //At this point, the packet must not have the DO_NOT_REORDER flag

    auto currentRealm = ignoreRealm ? packet->retrieveRealm().value() : realm;

    if (reorderer.isEmpty())
    {
        auto const stat = PacketReorderer::checkStat(packet, peerCounter, peerReorderedCounter, currentRealm);

        if (stat == PacketReorderer::Stats::RETRIEVABLE)
        {
            packet->markAsLocallyReordered();
            return FluxProcessResults::USER_RETRIEVABLE;
        }

        reorderer.push(std::move(packet));
        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    //We push the packet in the reorderer
    reorderer.push(std::move(packet));

    //At this point we are sure that the reorderer contains at least 2 packets

    bool forced = reorderer.isForced();
    auto const stat = reorderer.checkStat(peerCounter, peerReorderedCounter, currentRealm).value();
    if (!forced && stat != PacketReorderer::Stats::RETRIEVABLE)
    {
        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    packet = reorderer.pop();
    packet->markAsLocallyReordered();
    peerCounter = packet->retrieveCounter().value();
    peerReorderedCounter = packet->retrieveReorderedCounter().value();
    currentRealm = packet->retrieveRealm().value();

    auto const packerReordererMaxSize = reorderer.getMaximumSize();
    std::size_t containerInvertedSize = 0;
    auto* containerInverted = FGE_ALLOCA_T(ReceivedPacketPtr, packerReordererMaxSize);
    FGE_PLACE_CONSTRUCT(ReceivedPacketPtr, packerReordererMaxSize, containerInverted);

    while (auto const stat = reorderer.checkStat(peerCounter, peerReorderedCounter, currentRealm))
    {
        if (!(stat == PacketReorderer::Stats::RETRIEVABLE || forced))
        {
            break;
        }

        auto reorderedPacket = reorderer.pop();

        //Mark them as reordered
        reorderedPacket->markAsLocallyReordered();

        peerCounter = reorderedPacket->retrieveCounter().value();
        peerReorderedCounter = reorderedPacket->retrieveReorderedCounter().value();
        currentRealm = reorderedPacket->retrieveRealm().value();

        //Add it to the container (we are going to push in front of the flux queue, so we need to inverse the order)
        containerInverted[containerInvertedSize++] = std::move(reorderedPacket);
    }

    //Now we push the packets (until the last one) in the correct order in the flux queue
    for (std::size_t i = containerInvertedSize; i != 0; --i)
    {
        this->forcePushPacketFront(std::move(containerInverted[i - 1]));
        ++this->_g_remainingPackets;
    }

    FGE_PLACE_DESTRUCT(ReceivedPacketPtr, packerReordererMaxSize, containerInverted);

    return FluxProcessResults::USER_RETRIEVABLE;
}

ReceivedPacketPtr NetFluxUdp::popNextPacket()
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    if (!this->_g_packets.empty())
    {
        ReceivedPacketPtr tmpPck = std::move(this->_g_packets.front());
        this->_g_packets.pop_front();
        return tmpPck;
    }
    return nullptr;
}
std::size_t NetFluxUdp::getPacketsSize() const
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    return this->_g_packets.size();
}
bool NetFluxUdp::isEmpty() const
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    return this->_g_packets.empty();
}

void NetFluxUdp::setMaxPackets(std::size_t n)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->g_maxPackets = n;
}
std::size_t NetFluxUdp::getMaxPackets() const
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    return this->g_maxPackets;
}

bool NetFluxUdp::isDefaultFlux() const
{
    return this->g_isDefaultFlux;
}

//ServerNetFluxUdp
ServerNetFluxUdp::ServerNetFluxUdp(ServerSideNetUdp& server, bool defaultFlux) :
        NetFluxUdp(defaultFlux),
        g_server(&server)
{}

void ServerNetFluxUdp::processClients()
{
    auto const now = std::chrono::steady_clock::now();
    this->g_commandsUpdateTick +=
            std::max(std::chrono::milliseconds{1},
                     std::chrono::duration_cast<std::chrono::milliseconds>(now - this->g_lastCommandUpdateTimePoint));

    auto clientsLock = this->_clients.acquireLock();

    for (auto it = this->_clients.begin(clientsLock); it != this->_clients.end(clientsLock);)
    {
        auto& client = it->second;

        //Handle bad clients
        auto const& status = client->getStatus();
        if (status.getNetworkStatus() == ClientStatus::NetworkStatus::DISCONNECTED)
        {
            //Remove the client only if all pending packets are sent
            if (!client->isPendingPacketsEmpty())
            {
                ++it;
                continue;
            }

            auto tmpIdentity = it->first;
            auto tmpClient = std::move(it->second);
            it = this->_clients.remove(it, clientsLock);
            this->_onClientDisconnected.call(tmpClient, tmpIdentity);
            continue;
        }

        //Handle timeout
        if (status.getNetworkStatus() == ClientStatus::NetworkStatus::TIMEOUT)
        {
            ++it;
            continue;
        }
        if (client->getStatus().isTimeout())
        {
            client->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::TIMEOUT);
            client->clearPackets();

            auto tmpIdentity = it->first;
            auto tmpClient = std::move(it->second);
            it = this->_clients.remove(it, clientsLock);
            this->_onClientTimeout.call(tmpClient, tmpIdentity);
            continue;
        }

        //Handle commands
        if (this->g_commandsUpdateTick >= FGE_NET_CMD_UPDATE_TICK_MS)
        { //TODO: move it to the transmit thread ?
            auto& commands = it->second->_context._commands;
            if (!commands.empty())
            {
                TransmitPacketPtr possiblePacket;
                auto const type = commands.front()->getType();
                auto result = commands.front()->update(possiblePacket, this->g_server->getAddressType(), *client,
                                                       this->g_commandsUpdateTick);
                if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
                {
                    FGE_DEBUG_PRINT(result == NetCommandResults::SUCCESS ? "SUCCESS" : "FAILURE");
                    commands.pop_front();
                }

                if (possiblePacket)
                {
                    //Pushing the packet
                    client->pushPacket(std::move(possiblePacket));
                }

                if (result == NetCommandResults::SUCCESS && type == NetCommandTypes::CONNECT_HANDLER)
                {
                    it->second->_context._cache.enable(true); //User can opt to disable cache inside the callback
                    this->_onClientConnected.call(client, it->first);
                }
            }
        }

        ++it;
    }

    if (this->g_commandsUpdateTick >= FGE_NET_CMD_UPDATE_TICK_MS)
    {
        this->g_lastCommandUpdateTimePoint = now;
        this->g_commandsUpdateTick = std::chrono::milliseconds::zero();
    }
}

void ServerNetFluxUdp::disconnectAllClients(std::chrono::milliseconds delay) const
{
    this->_clients.sendToAll(CreateDisconnectPacket());
    std::this_thread::sleep_for(delay);
}

FluxProcessResults ServerNetFluxUdp::process(ClientSharedPtr& refClient, ReceivedPacketPtr& packet)
{
    this->processClients();

    refClient.reset();
    packet.reset();

    if (this->_g_remainingPackets == 0)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NONE_AVAILABLE;
    }

    //Popping the next packet
    packet = this->popNextPacket();
    if (!packet)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NONE_AVAILABLE;
    }
    --this->_g_remainingPackets;

    //Verify if the client is known
    refClient = this->_clients.get(packet->getIdentity());

    if (refClient == nullptr)
    { //Unknown client
        if (this->isDefaultFlux())
        { //Only allow unknown clients on the default flux
            return this->processUnknownClient(refClient, packet);
        }

        this->g_server->repushPacket(std::move(packet));
        return FluxProcessResults::INTERNALLY_HANDLED;
    }
    auto const& status = refClient->getStatus();

    //Ignore packets if the client is disconnected
    if (status.isDisconnected())
    {
        return FluxProcessResults::INTERNALLY_DISCARDED;
    }

    //Checking commands
    {
        auto& commands = refClient->_context._commands;
        if (!commands.empty())
        {
            commands.front()->onReceive(packet, this->g_server->getAddressType(), *refClient);

            //Commands can drop the packet
            if (!packet)
            {
                return FluxProcessResults::INTERNALLY_HANDLED;
            }
        }
    }

    //Check if the client is in an acknowledged state
    if (status.getNetworkStatus() == ClientStatus::NetworkStatus::ACKNOWLEDGED)
    {
        //We can't allow an unknown packet at this point
        //TODO: error handler class
        return FluxProcessResults::INTERNALLY_DISCARDED;
    }

    //Check if the packet is a fragment
    if (packet->isFragmented())
    {
        auto const identity = packet->getIdentity();
        auto const result = refClient->_context._defragmentation.process(std::move(packet));
        if (result._result == PacketDefragmentation::Results::RETRIEVABLE)
        {
            packet = refClient->_context._defragmentation.retrieve(result._id, identity);
        }
        else
        {
            return FluxProcessResults::INTERNALLY_HANDLED;
        }

        if (!packet->haveCorrectHeader())
        {
            return FluxProcessResults::INTERNALLY_DISCARDED;
        }
    }

    //Check if the client is in an MTU discovered state
    if (status.getNetworkStatus() == ClientStatus::NetworkStatus::MTU_DISCOVERED)
    {
        //We can't allow an unknown packet at this point
        //TODO: error handler class
        return FluxProcessResults::INTERNALLY_DISCARDED;
    }

    auto const stat = PacketReorderer::checkStat(packet, refClient->getPacketCounter(Client::Targets::PEER),
                                                 refClient->getReorderedPacketCounter(Client::Targets::PEER),
                                                 refClient->getCurrentRealm());

    bool const doNotDiscard = packet->checkFlags(FGE_NET_HEADER_DO_NOT_DISCARD_FLAG);
    bool const doNotReorder = packet->checkFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG);

    if (!doNotDiscard)
    {
        if (stat == PacketReorderer::Stats::OLD_COUNTER)
        {
#ifdef FGE_DEF_DEBUG
            auto const packetRealm = packet->retrieveRealm().value();
            auto const packetCounter = packet->retrieveCounter().value();
            auto const packetReorderedCounter = packet->retrieveReorderedCounter().value();
            auto const peerCounter = refClient->getPacketCounter(Client::Targets::PEER);
            auto const peerReorderedCounter = refClient->getReorderedPacketCounter(Client::Targets::PEER);
#endif
            FGE_DEBUG_PRINT("Packet is old, discarding it. Packet realm: {}, counter: {}, reorderedCounter: {}. Peer "
                            "counter: {}, reorderedCounter: {}",
                            packetRealm, packetCounter, packetReorderedCounter, peerCounter, peerReorderedCounter);
            refClient->advanceLostPacketCount();
            return FluxProcessResults::INTERNALLY_DISCARDED;
        }
    }

    if (!doNotReorder && !packet->isMarkedAsLocallyReordered())
    {
        auto reorderResult = this->processReorder(
                refClient->_context._reorderer, packet, refClient->getPacketCounter(Client::Targets::PEER),
                refClient->getReorderedPacketCounter(Client::Targets::PEER), refClient->getCurrentRealm(), true);
        if (reorderResult != FluxProcessResults::USER_RETRIEVABLE)
        {
            return reorderResult;
        }
    }

    //Verify the realm
    if (!this->verifyRealm(refClient, packet))
    {
        FGE_DEBUG_PRINT("Packet bad realm, discarding it");
        return FluxProcessResults::INTERNALLY_DISCARDED;
    }

    if (stat == PacketReorderer::Stats::WAITING_NEXT_REALM || stat == PacketReorderer::Stats::WAITING_NEXT_COUNTER)
    {
#ifdef FGE_DEF_DEBUG
        auto const packetRealm = packet->retrieveRealm().value();
        auto const packetCounter = packet->retrieveCounter().value();
        auto const packetReorderedCounter = packet->retrieveReorderedCounter().value();
        auto const peerCounter = refClient->getPacketCounter(Client::Targets::PEER);
        auto const peerReorderedCounter = refClient->getReorderedPacketCounter(Client::Targets::PEER);
#endif
        FGE_DEBUG_PRINT("We lose a packet. Packet realm: {}, counter: {}, reorderedCounter: {}. Peer counter: {}, "
                        "reorderedCounter: {}",
                        packetRealm, packetCounter, packetReorderedCounter, peerCounter, peerReorderedCounter);
        refClient->advanceLostPacketCount(); //We are missing a packet
    }

    auto const peerRealm = packet->retrieveRealm().value();
    refClient->setCurrentRealm(peerRealm);
    auto const peerCounter = packet->retrieveCounter().value();
    refClient->setPacketCounter(Client::Targets::PEER, peerCounter);
    auto const peerReorderedCounter = packet->retrieveReorderedCounter().value();
    refClient->setReorderedPacketCounter(Client::Targets::PEER, peerReorderedCounter);

    //Check if the packet is a return packet, and if so, handle it
    if (this->handleReturnPacket(refClient, refClient->_context, packet).has_value() || packet == nullptr)
    {
        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    //Check for a disconnected client
    if (packet->retrieveHeaderId().value() == NET_INTERNAL_ID_DISCONNECT)
    {
        refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
        refClient->clearPackets();
        this->_clients.remove(packet->getIdentity());
        this->_onClientDisconnected.call(refClient, packet->getIdentity());
        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    return FluxProcessResults::USER_RETRIEVABLE;
}

FluxProcessResults ServerNetFluxUdp::processUnknownClient(ClientSharedPtr& refClient, ReceivedPacketPtr& packet)
{
    //Check if the packet is fragmented
    if (packet->isFragmented())
    {
        //We can't (disallow) process fragmented packets from unknown clients
        return FluxProcessResults::INTERNALLY_DISCARDED;
    }

    //Check if the packet is a handshake
    if (packet->retrieveHeaderId().value() == NET_INTERNAL_ID_FGE_HANDSHAKE)
    {
        FGE_DEBUG_PRINT("Handshake received");

        using namespace fge::net::rules;
        std::string handshakeString;
        std::string versioningString;
        auto const err = RStringMustEqual(sizeof(FGE_NET_HANDSHAKE_STRING) - 1, packet->packet(), &handshakeString)
                                 .and_then([&](auto& chain) {
            return RStringRange(0, FGE_NET_MAX_VERSIONING_STRING_SIZE, chain, &versioningString);
        }).final();
        if (err || handshakeString != FGE_NET_HANDSHAKE_STRING ||
            versioningString != this->g_server->getVersioningString())
        {
            FGE_DEBUG_PRINT("Handshake failed");
            return FluxProcessResults::INTERNALLY_DISCARDED;
        }

        FGE_DEBUG_PRINT("Handshake accepted");
        //Handshake accepted, we create the client
        refClient = std::make_shared<Client>();
        refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::ACKNOWLEDGED);
        refClient->getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);

        //Create crypt context
        if (!CryptServerCreate(this->g_server->getCryptContext(), refClient->getCryptInfo()))
        {
            FGE_DEBUG_PRINT("CryptServerCreate failed");
            return FluxProcessResults::INTERNALLY_DISCARDED;
        }

        if (!this->g_server->announceNewClient(packet->getIdentity(), refClient))
        {
            FGE_DEBUG_PRINT("announceNewClient failed, identity in use");
            return FluxProcessResults::INTERNALLY_DISCARDED;
        }
        this->_clients.add(packet->getIdentity(), refClient);

        //Respond to the handshake
        auto responsePacket = CreatePacket(NET_INTERNAL_ID_FGE_HANDSHAKE);
        responsePacket->doNotReorder().doNotFragment().doNotDiscard();
        responsePacket->packet() << FGE_NET_HANDSHAKE_STRING;
        refClient->pushPacket(std::move(responsePacket));
        this->g_server->notifyTransmission();

        this->_onClientAcknowledged.call(refClient, packet->getIdentity());

        //Add connect handler command as this is required for establishing the connection
        auto command = std::make_unique<NetConnectHandlerCommand>(&refClient->_context._commands);
        //At this point commands should be empty
        refClient->_context._commands.push_front(std::move(command));

        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    //We can't process packets further for unknown clients
    return FluxProcessResults::INTERNALLY_DISCARDED;
}

bool ServerNetFluxUdp::verifyRealm(ClientSharedPtr const& refClient, ReceivedPacketPtr const& packet)
{
    auto const headerFlags = packet->retrieveFlags().value();
    auto const clientProvidedRealm = packet->retrieveRealm().value();

    if (clientProvidedRealm != refClient->getCurrentRealm() &&
        refClient->getLastRealmChangeElapsedTime() >= FGE_SERVER_MAX_TIME_DIFFERENCE_REALM)
    {
        this->_onClientBadRealm.call(refClient);
        return (headerFlags & FGE_NET_HEADER_DO_NOT_DISCARD_FLAG) > 0;
    }
    return true;
}

} // namespace fge::net

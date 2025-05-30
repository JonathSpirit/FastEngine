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

//ServerFluxUdp
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

FluxProcessResults NetFluxUdp::processReorder(Client& client,
                                              ReceivedPacketPtr& packet,
                                              ProtocolPacket::CounterType currentCounter,
                                              bool ignoreRealm)
{
    auto currentRealm = ignoreRealm ? packet->retrieveRealm().value() : client.getCurrentRealm();

    if (client.getPacketReorderer().isEmpty())
    {
        auto const stat = PacketReorderer::checkStat(packet, currentCounter, currentRealm);

        if (stat == PacketReorderer::Stats::RETRIEVABLE)
        {
            packet->markAsLocallyReordered();
            return FluxProcessResults::USER_RETRIEVABLE;
        }

        client.getPacketReorderer().push(std::move(packet));
        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    //We push the packet in the reorderer
    client.getPacketReorderer().push(std::move(packet));

    //At this point we are sure that the reorderer contains at least 2 packets

    bool forced = client.getPacketReorderer().isForced();
    auto const stat = client.getPacketReorderer().checkStat(currentCounter, currentRealm).value();
    if (!forced && stat != PacketReorderer::Stats::RETRIEVABLE)
    {
        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    packet = client.getPacketReorderer().pop();
    packet->markAsLocallyReordered();
    currentCounter = packet->retrieveCounter().value();
    currentRealm = packet->retrieveRealm().value();

    auto const packerReordererMaxSize = client.getPacketReorderer().getMaximumSize();
    std::size_t containerInversedSize = 0;
    auto* containerInversed = FGE_ALLOCA_T(ReceivedPacketPtr, packerReordererMaxSize);
    FGE_PLACE_CONSTRUCT(ReceivedPacketPtr, packerReordererMaxSize, containerInversed);

    while (auto const stat = client.getPacketReorderer().checkStat(currentCounter, currentRealm))
    {
        if (!(stat == PacketReorderer::Stats::RETRIEVABLE || forced))
        {
            break;
        }

        auto reorderedPacket = client.getPacketReorderer().pop();

        //Mark them as reordered
        reorderedPacket->markAsLocallyReordered();

        currentCounter = reorderedPacket->retrieveCounter().value();
        currentRealm = reorderedPacket->retrieveRealm().value();

        //Add it to the container (we are going to push in front of the flux queue, so we need to inverse the order)
        containerInversed[containerInversedSize++] = std::move(reorderedPacket);
    }

    //Now we push the packets (until the last one) in the correct order in the flux queue
    for (std::size_t i = containerInversedSize; i != 0; --i)
    {
        this->forcePushPacketFront(std::move(containerInversed[i - 1]));
        ++this->_g_remainingPackets;
    }

    FGE_PLACE_DESTRUCT(ReceivedPacketPtr, packerReordererMaxSize, containerInversed);

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

//ServerNetFluxUdp
void ServerNetFluxUdp::processClients()
{
    auto const now = std::chrono::steady_clock::now();
    this->g_commandsUpdateTick +=
            std::max(std::chrono::milliseconds{1},
                     std::chrono::duration_cast<std::chrono::milliseconds>(now - this->g_lastCommandUpdateTimePoint));

    auto clientsLock = this->_clients.acquireLock();

    for (auto it = this->_clients.begin(clientsLock); it != this->_clients.end(clientsLock);)
    {
        auto& client = it->second._client;

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
            auto tmpClient = std::move(it->second._client);
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
            auto tmpClient = std::move(it->second._client);
            it = this->_clients.remove(it, clientsLock);
            this->_onClientTimeout.call(tmpClient, tmpIdentity);
            continue;
        }

        //Handle commands
        if (this->g_commandsUpdateTick >= FGE_NET_CMD_UPDATE_TICK_MS)
        { //TODO: move it to the transmit thread ?
            auto& commands = it->second._commands;
            if (!commands.empty())
            {
                TransmitPacketPtr possiblePacket;
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

FluxProcessResults
ServerNetFluxUdp::process(ClientSharedPtr& refClient, ReceivedPacketPtr& packet, bool allowUnknownClient)
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
    auto* refClientData = this->_clients.getData(packet->getIdentity());

    if (refClientData == nullptr)
    { //Unknown client
        if (allowUnknownClient)
        {
            return this->processUnknownClient(refClient, packet);
        }

        this->g_server->repushPacket(std::move(packet));
        return FluxProcessResults::INTERNALLY_HANDLED;
    }
    refClient = refClientData->_client;
    auto const& status = refClient->getStatus();

    //Ignore packets if the client is disconnected
    if (status.isDisconnected())
    {
        return FluxProcessResults::INTERNALLY_DISCARDED;
    }

    //Check if the client is in an acknowledged state
    if (status.getNetworkStatus() == ClientStatus::NetworkStatus::ACKNOWLEDGED)
    {
        return this->processAcknowledgedClient(*refClientData, packet);
    }

    //Check if the packet is a fragment
    if (packet->isFragmented())
    {
        auto const identity = packet->getIdentity();
        auto const result = refClientData->_defragmentation.process(std::move(packet));
        if (result._result == PacketDefragmentation::Results::RETRIEVABLE)
        {
            packet = refClientData->_defragmentation.retrieve(result._id, identity);
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
        return this->processMTUDiscoveredClient(*refClientData, packet);
    }

    auto const stat =
            PacketReorderer::checkStat(packet, refClient->getClientPacketCounter(), refClient->getCurrentRealm());

    if (!packet->checkFlags(FGE_NET_HEADER_DO_NOT_DISCARD_FLAG))
    {
        if (stat == PacketReorderer::Stats::OLD_COUNTER)
        {
#ifdef FGE_DEF_DEBUG
            auto const packetCounter = packet->retrieveCounter().value();
            auto const packetRealm = packet->retrieveRealm().value();
            auto const currentCounter = refClient->getClientPacketCounter();
#endif
            FGE_DEBUG_PRINT("Packet is old, discarding it packetCounter: {}, packetRealm: {}, currentCounter: {}",
                            packetCounter, packetRealm, currentCounter);
            refClient->advanceLostPacketCount();
            return FluxProcessResults::INTERNALLY_DISCARDED;
        }
    }

    bool const doNotReorder = packet->checkFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG);
    if (!doNotReorder && !packet->isMarkedAsLocallyReordered())
    {
        auto reorderResult = this->processReorder(*refClient, packet, refClient->getClientPacketCounter(), true);
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
        auto const packetCounter = packet->retrieveCounter().value();
        auto const packetRealm = packet->retrieveRealm().value();
        auto const currentCounter = refClient->getClientPacketCounter();
#endif
        FGE_DEBUG_PRINT("We lose a packet packetCounter: {}, packetRealm: {}, currentCounter: {}", packetCounter,
                        packetRealm, currentCounter);
        refClient->advanceLostPacketCount(); //We are missing a packet
    }

    if (!doNotReorder)
    {
        auto const countId = packet->retrieveCounter().value();
        refClient->setClientPacketCounter(countId);
    }

    //Check if the packet is a return packet, and if so, handle it
    if (packet->retrieveHeaderId().value() == NET_INTERNAL_ID_RETURN_PACKET)
    {
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

                this->_onClientObjectReturnEvent.call(refClient, packet->getIdentity(), commandIndex, parentSid,
                                                      targetSid, packet);
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
                return FluxProcessResults::INTERNALLY_DISCARDED;
            }
            acknowledgedPackets[i] = label;
        }

        {
            auto const packetCache = refClient->getPacketCache();
            packetCache.first->acknowledgeReception(acknowledgedPackets);
        }

        this->_onClientReturnPacket.call(refClient, packet->getIdentity(), packet);

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
        this->_clients.add(packet->getIdentity(), refClient);
        this->g_server->notifyNewClient(packet->getIdentity(), refClient);

        //Add MTU command as this is required for the next state
        auto* clientData = this->_clients.getData(packet->getIdentity());
        auto command = std::make_unique<NetMTUCommand>(&clientData->_commands);
        clientData->_mtuFuture = command->get_future();
        //At this point commands should be empty
        clientData->_commands.push_front(std::move(command));

        //Respond to the handshake
        auto responsePacket = CreatePacket(NET_INTERNAL_ID_FGE_HANDSHAKE);
        responsePacket->doNotReorder().doNotFragment().doNotDiscard();
        responsePacket->packet() << FGE_NET_HANDSHAKE_STRING;
        refClient->pushPacket(std::move(responsePacket));
        this->g_server->notifyTransmission();

        this->_onClientAcknowledged.call(refClient, packet->getIdentity());

        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    //We can't process packets further for unknown clients
    return FluxProcessResults::INTERNALLY_DISCARDED;
}
FluxProcessResults ServerNetFluxUdp::processAcknowledgedClient(ClientList::Data& refClientData,
                                                               ReceivedPacketPtr& packet)
{
    //At this point, the client still need to do the MTU discovery
    switch (packet->retrieveHeaderId().value())
    {
    case NET_INTERNAL_ID_MTU_TEST:
    {
        auto response = CreatePacket(NET_INTERNAL_ID_MTU_TEST_RESPONSE);
        response->doNotDiscard().doNotReorder();
        refClientData._client->pushPacket(std::move(response));
        refClientData._client->getStatus().resetTimeout();
        FGE_DEBUG_PRINT("received MTU test");
        break;
    }
    case NET_INTERNAL_ID_MTU_ASK:
    {
        auto response = CreatePacket(NET_INTERNAL_ID_MTU_ASK_RESPONSE);
        response->doNotDiscard().doNotReorder()
                << SocketUdp::retrieveAdapterMTUForDestination(packet->getIdentity()._ip).value_or(0);
        refClientData._client->pushPacket(std::move(response));
        refClientData._client->getStatus().resetTimeout();
        FGE_DEBUG_PRINT("received MTU ask");
        break;
    }
    case NET_INTERNAL_ID_MTU_FINAL:
        FGE_DEBUG_PRINT("received MTU final");
        refClientData._client->_mtuFinalizedFlag = true;
        refClientData._client->getStatus().resetTimeout();
        //Client have finished the MTU discovery, but we have to check if the server have finished too
        if (refClientData._client->getMTU() != 0)
        {
            if (!CryptServerCreate(this->g_server->getCryptContext(), *refClientData._client))
            {
                FGE_DEBUG_PRINT("CryptServerCreate failed");
                //Discard the packet and the client
                this->_clients.remove(packet->getIdentity());
                this->_onClientDropped.call(refClientData._client, packet->getIdentity());
                return FluxProcessResults::INTERNALLY_DISCARDED;
            }
            FGE_DEBUG_PRINT("CryptServerCreate success, starting crypt exchange");
            refClientData._client->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::MTU_DISCOVERED);
            refClientData._client->getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
            this->_onClientMTUDiscovered.call(refClientData._client, packet->getIdentity());
        }
        break;
    default:
    {
        auto const result = this->checkCommands(refClientData._client, refClientData._commands, packet);
        if (result == NetCommandResults::FAILURE)
        {
            FGE_DEBUG_PRINT("Command failed");
            //Discard the packet and the client
            this->_clients.remove(packet->getIdentity());
            this->_onClientDropped.call(refClientData._client, packet->getIdentity());
            return FluxProcessResults::INTERNALLY_DISCARDED;
        }

        if (result == NetCommandResults::SUCCESS)
        {
            FGE_DEBUG_PRINT("Command success");
            refClientData._client->setMTU(refClientData._mtuFuture.get());

            auto response = CreatePacket(NET_INTERNAL_ID_MTU_FINAL);
            response->doNotDiscard().doNotReorder();
            refClientData._client->pushPacket(std::move(response));
            refClientData._client->getStatus().resetTimeout();

            //Server have finished the MTU discovery, but we have to check if the client have finished too
            if (refClientData._client->_mtuFinalizedFlag)
            {
                FGE_DEBUG_PRINT("mtu finalized");
                if (!CryptServerCreate(this->g_server->getCryptContext(), *refClientData._client))
                {
                    FGE_DEBUG_PRINT("CryptServerCreate failed");
                    //Discard the packet and the client
                    this->_clients.remove(packet->getIdentity());
                    this->_onClientDropped.call(refClientData._client, packet->getIdentity());
                    return FluxProcessResults::INTERNALLY_DISCARDED;
                }

                FGE_DEBUG_PRINT("CryptServerCreate success, starting crypt exchange");
                refClientData._client->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::MTU_DISCOVERED);
                refClientData._client->getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
                this->_onClientMTUDiscovered.call(refClientData._client, packet->getIdentity());
            }
        }
    }
    break;
    }
    //TODO: add a timeout for the MTU discovery
    return FluxProcessResults::INTERNALLY_HANDLED;
}
FluxProcessResults ServerNetFluxUdp::processMTUDiscoveredClient(ClientList::Data& refClientData,
                                                                ReceivedPacketPtr& packet)
{
    //Check if the packet is a crypt handshake
    if (packet->retrieveHeaderId().value() != NET_INTERNAL_ID_CRYPT_HANDSHAKE)
    {
        return FluxProcessResults::INTERNALLY_DISCARDED;
    }

    auto& refClient = refClientData._client;

    FGE_DEBUG_PRINT("receiving crypt handshake");
    auto& info = refClient->getCryptInfo();

    refClient->getStatus().resetTimeout();

    auto const readPos = packet->getReadPos();
    BIO_write(static_cast<BIO*>(info._rbio), packet->getData() + readPos, packet->getDataSize() - readPos);

    auto const result = SSL_do_handshake(static_cast<SSL*>(info._ssl));
    if (result <= 0)
    {
        auto const err = SSL_get_error(static_cast<SSL*>(info._ssl), result);

        if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
        {
            ERR_print_errors_fp(stderr);
            refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
            this->_clients.remove(packet->getIdentity());
            this->_onClientDropped.call(refClient, packet->getIdentity());
            return FluxProcessResults::INTERNALLY_DISCARDED;
        }
    }

    FGE_DEBUG_PRINT("check for transmit crypt");
    // Check if OpenSSL has produced encrypted handshake data
    auto const pendingSize = BIO_ctrl_pending(static_cast<BIO*>(info._wbio));
    if (pendingSize == 0)
    {
        FGE_DEBUG_PRINT("no crypt handshake to transmit");
        return FluxProcessResults::INTERNALLY_DISCARDED;
    }
    FGE_DEBUG_PRINT("transmitting crypt");
    auto response = CreatePacket(NET_INTERNAL_ID_CRYPT_HANDSHAKE);
    auto const packetStartDataPosition = response->doNotDiscard().getDataSize();
    response->append(pendingSize);
    auto const finalSize =
            BIO_read(static_cast<BIO*>(info._wbio), response->getData() + packetStartDataPosition, pendingSize);
    if (finalSize <= 0 || static_cast<std::size_t>(finalSize) != pendingSize)
    {
        FGE_DEBUG_PRINT("failed crypt");
        refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
        this->_clients.remove(packet->getIdentity());
        this->_onClientDropped.call(refClient, packet->getIdentity());
        return FluxProcessResults::INTERNALLY_DISCARDED;
    }
    refClient->pushPacket(std::move(response));
    this->g_server->notifyTransmission();

    if (SSL_is_init_finished(static_cast<SSL*>(info._ssl)) == 1)
    {
        FGE_DEBUG_PRINT("CONNECTED");
        refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::CONNECTED);
        refClient->getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_CONNECTED_TIMEOUT);
        refClient->setClientPacketCounter(0);
        refClient->setCurrentPacketCounter(0);
        this->_onClientConnected.call(refClient, packet->getIdentity());
        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    return FluxProcessResults::INTERNALLY_HANDLED;
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
NetCommandResults
ServerNetFluxUdp::checkCommands(ClientSharedPtr const& refClient, CommandQueue& commands, ReceivedPacketPtr& packet)
{
    if (commands.empty())
    {
        return NetCommandResults::FAILURE;
    }

    auto const result = commands.front()->onReceive(packet, this->g_server->getAddressType(), *refClient);
    if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
    {
        commands.pop_front();
    }
    return result;
}

//ServerUdp
ServerSideNetUdp::ServerSideNetUdp(IpAddress::Types type) :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_defaultFlux(*this),
        g_socket(type),
        g_running(false)
{}
ServerSideNetUdp::~ServerSideNetUdp()
{
    this->stop();
}

void ServerSideNetUdp::setVersioningString(std::string_view versioningString)
{
    std::scoped_lock const lock{this->g_mutexServer};
    this->g_versioningString = versioningString;
}
std::string const& ServerSideNetUdp::getVersioningString() const
{
    return this->g_versioningString;
}

bool ServerSideNetUdp::start(Port bindPort, IpAddress const& bindIp, IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }

    this->g_socket.setAddressType(addressType);
    if (this->g_socket.bind(bindPort, bindIp) == Socket::Errors::ERR_NOERROR)
    {
        if (!CryptServerInit(this->g_crypt_ctx))
        {
            this->g_socket.close();
            return false;
        }

        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ServerSideNetUdp::threadReception, this);
        this->g_threadTransmission = std::make_unique<std::thread>(&ServerSideNetUdp::threadTransmission, this);

        return true;
    }
    return false;
}
bool ServerSideNetUdp::start(IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }
    this->g_socket.setAddressType(addressType);
    if (this->g_socket.isValid())
    {
        if (!CryptServerInit(this->g_crypt_ctx))
        {
            this->g_socket.close();
            return false;
        }

        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ServerSideNetUdp::threadReception, this);
        this->g_threadTransmission = std::make_unique<std::thread>(&ServerSideNetUdp::threadTransmission, this);

        return true;
    }
    return false;
}
void ServerSideNetUdp::stop()
{
    if (this->g_running)
    {
        this->g_running = false;

        this->g_threadReception->join();
        this->g_threadTransmission->join();

        this->g_threadReception = nullptr;
        this->g_threadTransmission = nullptr;

        this->g_socket.close();

        //Clear the flux
        std::scoped_lock const lock(this->g_mutexServer);
        for (auto& flux: this->g_fluxes)
        {
            flux->clearPackets();
        }
        this->g_defaultFlux.clearPackets();
        decltype(this->g_transmissionQueue)().swap(this->g_transmissionQueue);

        CryptUninit(this->g_crypt_ctx);
    }
}

ServerNetFluxUdp* ServerSideNetUdp::newFlux()
{
    std::scoped_lock const lock(this->g_mutexServer);

    this->g_fluxes.push_back(std::make_unique<ServerNetFluxUdp>(*this));
    return this->g_fluxes.back().get();
}
ServerNetFluxUdp* ServerSideNetUdp::getFlux(std::size_t index)
{
    std::scoped_lock const lock(this->g_mutexServer);

    if (index >= this->g_fluxes.size())
    {
        return nullptr;
    }
    return this->g_fluxes[index].get();
}
ServerNetFluxUdp* ServerSideNetUdp::getDefaultFlux()
{
    return &this->g_defaultFlux;
}
std::size_t ServerSideNetUdp::getFluxSize() const
{
    return this->g_fluxes.size();
}
IpAddress::Types ServerSideNetUdp::getAddressType() const
{
    return this->g_socket.getAddressType();
}
void ServerSideNetUdp::closeFlux(NetFluxUdp* flux)
{
    std::scoped_lock const lock(this->g_mutexServer);

    for (std::size_t i = 0; i < this->g_fluxes.size(); ++i)
    {
        if (this->g_fluxes[i].get() == flux)
        {
            this->g_fluxes.erase(this->g_fluxes.begin() + i);
            break;
        }
    }
}
void ServerSideNetUdp::closeAllFlux()
{
    std::scoped_lock const lock(this->g_mutexServer);
    this->g_fluxes.clear();
}

void ServerSideNetUdp::repushPacket(ReceivedPacketPtr&& packet)
{
    if (!packet->checkFluxLifetime(this->g_fluxes.size()))
    {
        this->g_defaultFlux.pushPacket(std::move(packet));
        return;
    }
    this->g_fluxes[packet->getFluxIndex()]->forcePushPacket(std::move(packet));
}

void ServerSideNetUdp::notifyTransmission()
{
    this->g_transmissionNotifier.notify_one();
}

bool ServerSideNetUdp::isRunning() const
{
    return this->g_running;
}

void ServerSideNetUdp::notifyNewClient(Identity const& identity, ClientSharedPtr const& client)
{
    std::scoped_lock const lock(this->g_mutexServer);
    this->g_clientsMap[identity] = client;
}

void ServerSideNetUdp::sendTo(TransmitPacketPtr& pck, Client const& client, Identity const& id)
{
    pck->applyOptions(client);
    pck->doNotReorder();

    {
        std::scoped_lock const lock(this->g_mutexServer);
        this->g_transmissionQueue.emplace(std::move(pck), id);
    }
    this->g_transmissionNotifier.notify_one();
}
void ServerSideNetUdp::sendTo(TransmitPacketPtr& pck, Identity const& id)
{
    pck->applyOptions();
    pck->doNotReorder();

    {
        std::scoped_lock const lock(this->g_mutexServer);
        this->g_transmissionQueue.emplace(std::move(pck), id);
    }
    this->g_transmissionNotifier.notify_one();
}

void* ServerSideNetUdp::getCryptContext() const
{
    return this->g_crypt_ctx;
}

void ServerSideNetUdp::threadReception()
{
    Identity idReceive;
    Packet pckReceive;
    std::size_t pushingIndex = 0;
    auto gcClientsMap = std::chrono::steady_clock::now();

    CompressorLZ4 compressor;

    while (this->g_running)
    {
        if (this->g_socket.select(true, FGE_SERVER_PACKET_RECEPTION_TIMEOUT_MS) == Socket::Errors::ERR_NOERROR)
        {
            if (this->g_socket.receiveFrom(pckReceive, idReceive._ip, idReceive._port) == Socket::Errors::ERR_NOERROR)
            {
#ifdef FGE_ENABLE_SERVER_NETWORK_RANDOM_LOST
                if (fge::_random.range(0, 1000) <= 10)
                {
                    continue;
                }
#endif

                auto packet = std::make_unique<ProtocolPacket>(std::move(pckReceive), idReceive);
                packet->setTimestamp(Client::getTimestamp_ms());

                std::scoped_lock const lck(this->g_mutexServer);

                auto itClient = this->g_clientsMap.find(idReceive);
                if (itClient != this->g_clientsMap.end())
                {
                    auto client = itClient->second.lock();
                    if (!client)
                    { //bad client
                        this->g_clientsMap.erase(itClient);
                    }
                    else
                    {
                        //Check if the packet is encrypted
                        if (client->getStatus().isInEncryptedState())
                        {
                            if (!CryptDecrypt(*client, *packet))
                            {
                                continue;
                            }
                        }
                    }
                }

                //Here we consider that the packet is not encrypted
                if (!packet->haveCorrectHeader())
                {
                    continue;
                }
                //Skip the header for reading
                packet->skip(ProtocolPacket::HeaderSize);

                //Decompress the packet if needed
                if (!packet->decompress(compressor))
                {
                    continue;
                }

                //Realm and countId is verified by the flux

                if (this->g_fluxes.empty())
                {
                    this->g_defaultFlux.pushPacket(std::move(packet));
                    continue;
                }

                //Try to push packet in a flux
                for (std::size_t i = 0; i < this->g_fluxes.size(); ++i)
                {
                    pushingIndex = packet->bumpFluxIndex(this->g_fluxes.size());
                    if (this->g_fluxes[pushingIndex]->pushPacket(std::move(packet)))
                    { //Packet is correctly pushed
                        break;
                    }
                }
                //If every flux is busy, the new packet is dismissed
            }
        }

        //"Garbage collection" of the clients map
        auto const now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - gcClientsMap) >=
            std::chrono::milliseconds{5000})
        {
            gcClientsMap = now;

            for (auto it = this->g_clientsMap.begin(); it != this->g_clientsMap.end();)
            {
                if (it->second.expired())
                {
                    it = this->g_clientsMap.erase(it);
                    continue;
                }
                ++it;
            }
        }
    }
}
void ServerSideNetUdp::threadTransmission()
{
    std::unique_lock lckServer(this->g_mutexServer);
    CompressorLZ4 compressor;
    std::chrono::steady_clock::time_point timePoint;

    while (this->g_running)
    {
        this->g_transmissionNotifier.wait_for(lckServer, std::chrono::milliseconds(10));

        //Checking fluxes
        for (std::size_t i = 0; i < this->g_fluxes.size() + 1; ++i)
        {
            ClientList* clients{nullptr};
            if (i == this->g_fluxes.size())
            { //Doing the default flux
                clients = &this->g_defaultFlux._clients;
            }
            else
            {
                clients = &this->g_fluxes[i]->_clients;
            }

            timePoint = std::chrono::steady_clock::now();
            auto clientLock = clients->acquireLock();

            for (auto itClient = clients->begin(clientLock); itClient != clients->end(clientLock); ++itClient)
            {
                auto& client = itClient->second._client;

                //check cache
                {
                    auto const packetCache = client->getPacketCache();

                    auto const clientLatency =
                            client->getPacketReturnRate() * FGE_NET_PACKET_CACHE_DELAY_FACTOR +
                            std::chrono::milliseconds(client->_latencyPlanner.getRoundTripTime().value_or(1));

                    while (packetCache.first->check(
                            timePoint, std::chrono::duration_cast<std::chrono::milliseconds>(clientLatency)))
                    {
                        FGE_DEBUG_PRINT("re-transmit packet as client didn't acknowledge it");
                        client->pushForcedFrontPacket(packetCache.first->pop());
                    }
                }

                if (client->isPendingPacketsEmpty())
                {
                    continue;
                }

                if (client->getLastPacketLatency() < client->getSTOCLatency_ms())
                {
                    continue;
                }

                auto transmissionPacket = client->popPacket();

                if (!transmissionPacket->isMarkedAsCached())
                {
                    //Compression and applying options
                    if (!transmissionPacket->isFragmented() && client->getStatus().isInEncryptedState())
                    {
                        transmissionPacket->applyOptions(*client);
                        if (!transmissionPacket->compress(compressor))
                        {
                            continue;
                        }
                        auto const packetCache = client->getPacketCache();
                        packetCache.first->push(transmissionPacket);
                    }
                    else
                    {
                        transmissionPacket->applyOptions(*client);
                    }
                }

                //MTU check
                if (!transmissionPacket->isFragmented() &&
                    !transmissionPacket->checkFlags(FGE_NET_HEADER_DO_NOT_FRAGMENT_FLAG))
                {
                    auto const mtu = client->getMTU();

                    //Packet is not fragmented, we have to check is size
                    if (mtu == 0)
                    { //We don't know the MTU yet
                        goto mtu_check_skip;
                    }

                    auto fragments = transmissionPacket->fragment(mtu);
                    for (std::size_t iFragment = 0; iFragment < fragments.size(); ++iFragment)
                    {
                        if (iFragment == 0)
                        {
                            transmissionPacket = std::move(fragments[iFragment]);
                        }
                        else
                        {
                            client->pushForcedFrontPacket(std::move(fragments[iFragment]));
                        }
                    }
                }
            mtu_check_skip:

                if (!transmissionPacket->packet() || !transmissionPacket->haveCorrectHeaderSize())
                { //Last verification of the packet
                    continue;
                }

                //Check if the packet must be encrypted
                if (transmissionPacket->isMarkedForEncryption())
                {
                    if (!CryptEncrypt(*client, *transmissionPacket))
                    {
                        continue;
                    }
                }

                //Sending the packet
                this->g_socket.sendTo(transmissionPacket->packet(), itClient->first._ip, itClient->first._port);
                client->resetLastPacketTimePoint();
            }
        }

        //Checking isolated transmission queue TODO: maybe remove all that
        while (!this->g_transmissionQueue.empty())
        {
            auto data = std::move(this->g_transmissionQueue.front());
            this->g_transmissionQueue.pop();

            if (!data.first->packet() || !data.first->haveCorrectHeaderSize())
            { //Last verification of the packet
                continue;
            }

            //Sending the packet
            this->g_socket.sendTo(data.first->packet(), data.second._ip, data.second._port);
        }
    }
}

//ServerClientSideUdp
ClientSideNetUdp::ClientSideNetUdp(IpAddress::Types addressType) :
        g_socket(addressType)
{
    this->_client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
}

ClientSideNetUdp::~ClientSideNetUdp()
{
    this->stop();
}

bool ClientSideNetUdp::start(Port bindPort,
                             IpAddress const& bindIp,
                             Port connectRemotePort,
                             IpAddress const& connectRemoteAddress,
                             IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }

    this->resetReturnPacket();

    this->g_socket.setAddressType(addressType);
    if (addressType == IpAddress::Types::Ipv6)
    {
        this->g_socket.setIpv6Only(false);
    }
    else
    {
        this->g_socket.setDontFragment(true);
    }

    this->_client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);

    if (this->g_socket.bind(bindPort, bindIp) == Socket::Errors::ERR_NOERROR)
    {
        if (this->g_socket.connect(connectRemoteAddress, connectRemotePort) != Socket::Errors::ERR_NOERROR)
        {
            this->g_socket.close();
            return false;
        }

        if (!CryptClientInit(this->g_crypt_ctx))
        {
            this->g_socket.close();
            return false;
        }
        if (!CryptClientCreate(this->g_crypt_ctx, this->_client))
        {
            this->g_socket.close();
            return false;
        }

        this->g_clientIdentity._ip = connectRemoteAddress;
        this->g_clientIdentity._port = connectRemotePort;

        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ClientSideNetUdp::threadReception, this);
        this->g_threadTransmission = std::make_unique<std::thread>(&ClientSideNetUdp::threadTransmission, this);

        return true;
    }
    this->g_socket.close();
    return false;
}
void ClientSideNetUdp::stop()
{
    if (this->g_running)
    {
        this->disconnect().wait();

        this->g_running = false;

        this->g_threadReception->join();
        this->g_threadTransmission->join();

        this->g_threadReception = nullptr;
        this->g_threadTransmission = nullptr;

        this->g_socket.close();

        //Clear the flux
        this->clearPackets();
        //Clear client
        this->_client.clearPackets();
        this->_client.clearLostPacketCount();
        this->_client.setClientPacketCounter(0);
        this->_client.setCurrentRealm(FGE_NET_DEFAULT_REALM);
        this->_client.setCurrentPacketCounter(0);

        CryptClientDestroy(this->_client);
        CryptUninit(this->g_crypt_ctx);
    }
}

void ClientSideNetUdp::notifyTransmission()
{
    this->g_transmissionNotifier.notify_one();
}

IpAddress::Types ClientSideNetUdp::getAddressType() const
{
    return this->g_socket.getAddressType();
}

bool ClientSideNetUdp::isRunning() const
{
    return this->g_running;
}

std::future<uint16_t> ClientSideNetUdp::retrieveMTU()
{
    if (!this->g_running)
    {
        throw Exception("Cannot retrieve MTU without a running client");
    }

    auto command = std::make_unique<NetMTUCommand>(&this->g_commands);
    auto future = command->get_future();

    std::scoped_lock const lock(this->g_mutexCommands);
    this->g_commands.push_back(std::move(command));

    return future;
}
std::future<bool> ClientSideNetUdp::connect(std::string_view versioningString)
{
    if (!this->g_running)
    {
        throw Exception("Cannot connect without a running client");
    }

    auto command = std::make_unique<NetConnectCommand>(&this->g_commands);
    auto future = command->get_future();
    command->setVersioningString(versioningString);

    std::scoped_lock const lock(this->g_mutexCommands);
    this->g_commands.push_back(std::move(command));

    return future;
}
std::future<void> ClientSideNetUdp::disconnect()
{
    if (!this->g_running)
    {
        std::promise<void> promise;
        auto future = promise.get_future();
        promise.set_value();
        return future;
    }

    this->enableReturnPacket(false);

    auto command = std::make_unique<NetDisconnectCommand>(&this->g_commands);
    auto future = command->get_future();

    std::scoped_lock const lock(this->g_mutexCommands);
    this->g_commands.push_back(std::move(command));

    return future;
}

Identity const& ClientSideNetUdp::getClientIdentity() const
{
    return this->g_clientIdentity;
}

FluxProcessResults ClientSideNetUdp::process(ReceivedPacketPtr& packet)
{
    ///TODO: no lock ?
    packet.reset();

    if (this->_client.getStatus().isDisconnected())
    {
        this->_g_remainingPackets = 0;
        return FluxProcessResults::NONE_AVAILABLE;
    }

    //Checking timeout
    if (this->_client.getStatus().isTimeout())
    {
        this->_client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::TIMEOUT);
        this->_g_remainingPackets = 0;
        this->clearPackets();
        this->_onClientTimeout.call(*this);
        return FluxProcessResults::NONE_AVAILABLE;
    }

    //Checking return packet
    if (this->isReturnPacketEnabled())
    {
        auto const now = std::chrono::steady_clock::now();
        if (now - this->g_returnPacketTimePoint >= this->_client.getPacketReturnRate())
        {
            this->g_returnPacketTimePoint = now;
            auto returnPacket = this->prepareAndRetrieveReturnPacket();
            this->_onTransmitReturnPacket.call(*this, returnPacket);
            this->_client.pushPacket(std::move(returnPacket));
        }
    }

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

    if (!packet->isMarkedAsLocallyReordered())
    {
        this->_client.acknowledgeReception(packet);
    }

    auto const stat = PacketReorderer::checkStat(packet, this->_client.getCurrentPacketCounter(),
                                                 this->_client.getCurrentRealm());

    if (!packet->checkFlags(FGE_NET_HEADER_DO_NOT_DISCARD_FLAG))
    {
        if (stat == PacketReorderer::Stats::OLD_REALM || stat == PacketReorderer::Stats::OLD_COUNTER)
        {
#ifdef FGE_DEF_DEBUG
            auto const packetCounter = packet->retrieveCounter().value();
            auto const packetRealm = packet->retrieveRealm().value();
            auto const currentCounter = this->_client.getCurrentPacketCounter();
#endif
            FGE_DEBUG_PRINT("Packet is old, discarding it packetCounter: {}, packetRealm: {}, currentCounter: {}",
                            packetCounter, packetRealm, currentCounter);
            this->_client.advanceLostPacketCount();
            return FluxProcessResults::INTERNALLY_DISCARDED;
        }
    }

    bool const doNotReorder = packet->checkFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG);
    if (!doNotReorder && !packet->isMarkedAsLocallyReordered())
    {
        auto reorderResult =
                this->processReorder(this->_client, packet, this->_client.getCurrentPacketCounter(), false);
        if (reorderResult != FluxProcessResults::USER_RETRIEVABLE)
        {
            return reorderResult;
        }
    }

    if (packet->retrieveHeaderId().value() == NET_INTERNAL_ID_DISCONNECT)
    {
        this->_client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
        this->_g_remainingPackets = 0;
        this->clearPackets();
        this->_onClientDisconnected.call(*this);
        return FluxProcessResults::NONE_AVAILABLE;
    }

    if (stat == PacketReorderer::Stats::WAITING_NEXT_REALM || stat == PacketReorderer::Stats::WAITING_NEXT_COUNTER)
    {
#ifdef FGE_DEF_DEBUG
        auto const packetCounter = packet->retrieveCounter().value();
        auto const packetRealm = packet->retrieveRealm().value();
        auto const currentCounter = this->_client.getCurrentPacketCounter();
#endif
        FGE_DEBUG_PRINT("We lose a packet packetCounter: {}, packetRealm: {}, currentCounter: {}", packetCounter,
                        packetRealm, currentCounter);
        this->_client.advanceLostPacketCount(); //We are missing a packet
    }

    if (!doNotReorder)
    {
        auto const serverCountId = packet->retrieveCounter().value();
        this->_client.setCurrentPacketCounter(serverCountId);
    }
    auto const serverRealm = packet->retrieveRealm().value();
    this->_client.setCurrentRealm(serverRealm);

    return FluxProcessResults::USER_RETRIEVABLE;
}

void ClientSideNetUdp::resetReturnPacket()
{
    this->g_returnPacket = CreatePacket(NET_INTERNAL_ID_RETURN_PACKET);
    this->g_returnPacket->append(sizeof(this->g_returnPacketEventCount));

    this->g_returnPacketEventCount = 0;
    this->g_isAskingFullUpdate = false;
    this->g_returnPacketEventStarted = false;
    this->g_returnPacketTimePoint = std::chrono::steady_clock::now();
}

TransmitPacketPtr& ClientSideNetUdp::startReturnEvent(ReturnEvents event)
{
    if (this->g_returnPacketEventStarted)
    {
        throw Exception("Cannot start a new return event without ending the previous one");
    }
    this->g_returnPacketEventStarted = true;
    ++this->g_returnPacketEventCount;
    *this->g_returnPacket << event;
    this->g_returnPacketStartPosition = this->g_returnPacket->getDataSize();
    this->g_returnPacket->append(sizeof(uint16_t));
    return this->g_returnPacket;
}

TransmitPacketPtr&
ClientSideNetUdp::startObjectReturnEvent(uint16_t commandIndex, ObjectSid parentSid, ObjectSid targetSid)
{
    this->startReturnEvent(ReturnEvents::REVT_OBJECT);
    *this->g_returnPacket << commandIndex << parentSid << targetSid;
    return this->g_returnPacket;
}

void ClientSideNetUdp::endReturnEvent()
{
    if (!this->g_returnPacketEventStarted)
    {
        throw Exception("Cannot end a return event without starting one");
    }
    this->g_returnPacketEventStarted = false;

    //Rewrite event data size
    uint16_t eventSize = this->g_returnPacket->getDataSize() - this->g_returnPacketStartPosition - sizeof(uint16_t);
    this->g_returnPacket->pack(this->g_returnPacketStartPosition, &eventSize, sizeof(uint16_t));
}

void ClientSideNetUdp::simpleReturnEvent(uint16_t id)
{
    this->startReturnEvent(ReturnEvents::REVT_SIMPLE);
    *this->g_returnPacket << id;
    this->endReturnEvent();
}

void ClientSideNetUdp::askFullUpdateReturnEvent()
{
    if (this->g_isAskingFullUpdate)
    {
        return;
    }
    this->g_isAskingFullUpdate = true;
    this->startReturnEvent(ReturnEvents::REVT_ASK_FULL_UPDATE);
    this->endReturnEvent();
}

void ClientSideNetUdp::enableReturnPacket(bool enable)
{
    this->g_returnPacketEnabled = enable;
    if (enable)
    {
        this->resetReturnPacket();
    }
}
bool ClientSideNetUdp::isReturnPacketEnabled() const
{
    return this->g_returnPacketEnabled;
}

TransmitPacketPtr ClientSideNetUdp::prepareAndRetrieveReturnPacket()
{
    if (this->g_returnPacketEventStarted)
    {
        throw Exception("Cannot retrieve a return packet without ending the current command");
    }

    auto returnPacket = std::move(this->g_returnPacket);

    //Rewrite events count
    returnPacket->packet().pack(ProtocolPacket::HeaderSize, &this->g_returnPacketEventCount,
                                sizeof(this->g_returnPacketEventCount));

    //After return events, the packet is mostly composed of timestamp and latency information to limit bandwidth of packets.
    //The LatencyPlanner class will do all the work for that.
    this->_client._latencyPlanner.pack(returnPacket);

    //Pack acknowledged packets
    auto const& acknowledgedPackets = this->_client.getAcknowledgedList();
    SizeType const size = acknowledgedPackets.size();
    returnPacket->packet() << size;
    for (auto const& acknowledgedPacket: acknowledgedPackets)
    {
        returnPacket->packet() << acknowledgedPacket._counter << acknowledgedPacket._realm;
    }
    this->_client.clearAcknowledgedList();

    //Prepare the new returnPacket
    this->resetReturnPacket();

    return returnPacket;
}

std::size_t ClientSideNetUdp::waitForPackets(std::chrono::milliseconds time_ms)
{
    std::unique_lock lock(this->_g_mutexFlux);
    auto packetSize = this->_g_packets.size();
    if (packetSize > 0)
    {
        return packetSize;
    }

    this->g_receptionNotifier.wait_for(lock, time_ms);
    return this->_g_packets.size();
}

void ClientSideNetUdp::threadReception()
{
    Packet pckReceive;
    CompressorLZ4 compressor;

    while (this->g_running)
    {
        if (this->g_socket.select(true, FGE_SERVER_PACKET_RECEPTION_TIMEOUT_MS) == Socket::Errors::ERR_NOERROR)
        {
            if (this->g_socket.receive(pckReceive) != Socket::Errors::ERR_NOERROR)
            {
                continue;
            }

#ifdef FGE_ENABLE_CLIENT_NETWORK_RANDOM_LOST
            if (fge::_random.range(0, 1000) <= 10)
            {
                continue;
            }
#endif

            //Decrypting the packet if needed
            if (this->_client.getStatus().isInEncryptedState())
            {
                if (!CryptDecrypt(this->_client, pckReceive))
                {
                    FGE_DEBUG_PRINT("CryptDecrypt failed");
                    continue;
                }
            }

            auto packet = std::make_unique<ProtocolPacket>(std::move(pckReceive), this->g_clientIdentity);
            packet->setTimestamp(Client::getTimestamp_ms());

            //Here we consider that the packet is not encrypted
            if (!packet->haveCorrectHeader())
            {
                continue;
            }
            //Skip the header for reading
            packet->skip(ProtocolPacket::HeaderSize);

            //Check if the packet is a fragment
            if (packet->isFragmented())
            {
                auto const result = this->g_defragmentation.process(std::move(packet));
                if (result._result == PacketDefragmentation::Results::RETRIEVABLE)
                {
                    packet = this->g_defragmentation.retrieve(result._id, this->g_clientIdentity);
                }
                else
                {
                    continue;
                }
            }

            //Decompress the packet if needed
            if (!packet->decompress(compressor))
            {
                FGE_DEBUG_PRINT("decompress failed");
                continue;
            }

            //Check client status and reset timeout
            auto const networkStatus = this->_client.getStatus().getNetworkStatus();
            if (networkStatus != ClientStatus::NetworkStatus::TIMEOUT)
            {
                //TODO : check if we need to reset the timeout
                this->_client.getStatus().resetTimeout();
            }

            auto const headerId = packet->retrieveFullHeaderId().value();

            //MTU handling
            if (networkStatus == ClientStatus::NetworkStatus::ACKNOWLEDGED)
            {
                switch (headerId & ~FGE_NET_HEADER_FLAGS_MASK)
                {
                case NET_INTERNAL_ID_MTU_TEST:
                {
                    auto response = CreatePacket(NET_INTERNAL_ID_MTU_TEST_RESPONSE);
                    response->doNotDiscard().doNotReorder();
                    this->_client.pushPacket(std::move(response));
                    this->_client.getStatus().resetTimeout();
                    FGE_DEBUG_PRINT("received MTU test");
                    continue;
                }
                case NET_INTERNAL_ID_MTU_ASK:
                {
                    auto response = CreatePacket(NET_INTERNAL_ID_MTU_ASK_RESPONSE);
                    response->doNotDiscard().doNotReorder() << this->g_socket.retrieveCurrentAdapterMTU().value_or(0);
                    this->_client.pushPacket(std::move(response));
                    this->_client.getStatus().resetTimeout();
                    FGE_DEBUG_PRINT("received MTU ask");
                    continue;
                }
                case NET_INTERNAL_ID_MTU_FINAL:
                    FGE_DEBUG_PRINT("received MTU final");
                    this->_client._mtuFinalizedFlag = true;
                    this->_client.getStatus().resetTimeout();
                    continue;
                }
            }

            //Checking commands
            {
                std::scoped_lock const commandLock(this->g_mutexCommands);
                if (!this->g_commands.empty())
                {

                    auto const result =
                            this->g_commands.front()->onReceive(packet, this->g_socket.getAddressType(), this->_client);
                    if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
                    {
                        this->g_commands.pop_front();
                    }

                    //Commands can drop the packet
                    if (!packet)
                    {
                        continue;
                    }
                }
            }

            this->pushPacket(std::move(packet));
            this->g_receptionNotifier.notify_all();
        }
    }
}
void ClientSideNetUdp::threadTransmission()
{
    auto lastTimePoint = std::chrono::steady_clock::now();
    std::chrono::milliseconds commandsTime{0};
    CompressorLZ4 compressor;

    std::unique_lock lckServer(this->_g_mutexFlux);

    while (this->g_running)
    {
        this->g_transmissionNotifier.wait_for(lckServer, std::chrono::milliseconds(10));

        auto const now = std::chrono::steady_clock::now();
        auto const deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTimePoint);
        lastTimePoint = now;

        //Checking commands
        commandsTime += std::max(std::chrono::milliseconds{1}, deltaTime);
        if (commandsTime >= FGE_NET_CMD_UPDATE_TICK_MS)
        {
            std::scoped_lock const commandLock(this->g_mutexCommands);
            if (!this->g_commands.empty())
            {
                TransmitPacketPtr possiblePacket;
                auto const result = this->g_commands.front()->update(possiblePacket, this->g_socket.getAddressType(),
                                                                     this->_client, commandsTime);
                if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
                {
                    this->g_commands.pop_front();
                }

                if (possiblePacket)
                {
                    //Pushing the packet to the client queue
                    this->_client.pushPacket(std::move(possiblePacket));
                }
            }

            commandsTime = std::chrono::milliseconds::zero();
        }

        //Flux
        if (this->_client.isPendingPacketsEmpty())
        {
            continue;
        }

        if (this->_client.getLastPacketLatency() >= this->_client.getCTOSLatency_ms())
        { //Ready to send !
            auto transmissionPacket = this->_client.popPacket();

            //Compression and applying options
            if (!transmissionPacket->isFragmented() && this->_client.getStatus().isInEncryptedState())
            {
                transmissionPacket->applyOptions(this->_client);
                if (!transmissionPacket->compress(compressor))
                {
                    continue;
                }
            }
            else
            {
                transmissionPacket->applyOptions(this->_client);
            }

            //MTU check
            if (!transmissionPacket->isFragmented() &&
                !transmissionPacket->checkFlags(FGE_NET_HEADER_DO_NOT_FRAGMENT_FLAG))
            {
                //Packet is not fragmented, we have to check is size
                if (this->_client.getMTU() == 0)
                { //We don't know the MTU yet
                    goto mtu_check_skip;
                }

                auto fragments = transmissionPacket->fragment(this->_client.getMTU());
                for (std::size_t i = 0; i < fragments.size(); ++i)
                {
                    if (i == 0)
                    {
                        transmissionPacket = std::move(fragments[i]);
                    }
                    else
                    {
                        this->_client.pushForcedFrontPacket(std::move(fragments[i]));
                    }
                }
            }
        mtu_check_skip:

            if (!transmissionPacket->packet() || !transmissionPacket->haveCorrectHeaderSize())
            { //Last verification of the packet
                continue;
            }

            //Check if the packet is a disconnection packet
            if (transmissionPacket->retrieveHeaderId().value() == NET_INTERNAL_ID_DISCONNECT)
            {
                this->_client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
                this->_client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
                this->_client.clearPackets();
            }

            //Check if the packet must be encrypted
            if (transmissionPacket->isMarkedForEncryption())
            {
                if (!CryptEncrypt(this->_client, *transmissionPacket))
                {
                    continue;
                }
            }

            //Sending the packet
            this->g_socket.send(transmissionPacket->packet());
            this->_client.resetLastPacketTimePoint();
        }
    }
}

} // namespace fge::net

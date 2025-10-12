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

} // namespace fge::net

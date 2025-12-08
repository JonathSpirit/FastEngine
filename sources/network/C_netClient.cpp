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

#include "FastEngine/manager/network_manager.hpp"
#include "FastEngine/network/C_server.hpp"
#include "private/fge_crypt.hpp"
#include "private/fge_debug.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>

using namespace fge::priv;

namespace fge::net
{

//ClientSideNetUdp
ClientSideNetUdp::ClientSideNetUdp(IpAddress::Types addressType) :
        NetFluxUdp(false),
        g_socket(addressType)
{
    this->_client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
    this->resetReturnPacket();
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
        if (!CryptClientCreate(this->g_crypt_ctx, this->_client.getCryptInfo()))
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

        CryptClientDestroy(this->_client.getCryptInfo());
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

    auto command = std::make_unique<NetMTUCommand>(&this->g_clientContext._commands);
    auto future = command->get_future();

    std::scoped_lock const lock(this->g_mutexCommands);
    this->g_clientContext._commands.push_back(std::move(command));

    return future;
}
std::future<bool> ClientSideNetUdp::connect(std::string_view versioningString)
{
    if (!this->g_running)
    {
        throw Exception("Cannot connect without a running client");
    }

    auto command = std::make_unique<NetConnectCommand>(&this->g_clientContext._commands);
    auto future = command->get_future();
    command->setVersioningString(versioningString);

    std::scoped_lock const lock(this->g_mutexCommands);
    this->g_clientContext._commands.push_back(std::move(command));

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

    auto command = std::make_unique<NetDisconnectCommand>(&this->g_clientContext._commands);
    auto future = command->get_future();

    std::scoped_lock const lock(this->g_mutexCommands);
    this->g_clientContext._commands.push_back(std::move(command));

    return future;
}

Identity const& ClientSideNetUdp::getClientIdentity() const
{
    return this->g_clientIdentity;
}
ClientContext const& ClientSideNetUdp::getClientContext() const
{
    return this->g_clientContext;
}
ClientContext& ClientSideNetUdp::getClientContext()
{
    return this->g_clientContext;
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
                this->processReorder(this->g_clientContext._reorderer, packet, this->_client.getCurrentPacketCounter(),
                                     this->_client.getCurrentRealm(), false);
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

bool ClientSideNetUdp::askFullUpdateReturnEvent()
{
    if (this->g_isAskingFullUpdate)
    {
        return false;
    }
    this->g_isAskingFullUpdate = true;
    this->startReturnEvent(ReturnEvents::REVT_ASK_FULL_UPDATE);
    this->endReturnEvent();
    return true;
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
                auto const result = this->g_clientContext._defragmentation.process(std::move(packet));
                if (result._result == PacketDefragmentation::Results::RETRIEVABLE)
                {
                    packet = this->g_clientContext._defragmentation.retrieve(result._id, this->g_clientIdentity);
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
                if (!this->g_clientContext._commands.empty())
                {
                    this->g_clientContext._commands.front()->onReceive(packet, this->g_socket.getAddressType(),
                                                                       this->_client);

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
            if (!this->g_clientContext._commands.empty())
            {
                TransmitPacketPtr possiblePacket;
                auto const result = this->g_clientContext._commands.front()->update(
                        possiblePacket, this->g_socket.getAddressType(), this->_client, commandsTime);
                if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
                {
                    this->g_clientContext._commands.pop_front();
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

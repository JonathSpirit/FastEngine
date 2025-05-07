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

#include "FastEngine/network/C_netCommand.hpp"
#include "FastEngine/manager/network_manager.hpp"
#include "FastEngine/network/C_client.hpp"
#include "FastEngine/network/C_socket.hpp"
#include "private/fge_debug.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace fge::net
{

//NetCommand
NetCommandResults NetCommand::update(TransmitPacketPtr& buffPacket,
                                     IpAddress::Types addressType,
                                     Client& client,
                                     std::chrono::milliseconds deltaTime)
{
    this->g_timeout += deltaTime;
    if (this->g_timeout >= this->getTimeoutTarget())
    {
        switch (this->timeout(client))
        {
        case NetCommandResults::SUCCESS:
            return NetCommandResults::SUCCESS;
        case NetCommandResults::FAILURE:
            FGE_DEBUG_PRINT("Command timeout");
            return NetCommandResults::FAILURE;
        default:
            //Check if the timeout have been reset, if so we can continue
            if (this->g_timeout != std::chrono::milliseconds::zero())
            {
                FGE_DEBUG_PRINT("Command timeout");
                return NetCommandResults::FAILURE;
            }
            break;
        }
    }

    return this->internalUpdate(buffPacket, addressType, client);
}

std::chrono::milliseconds NetCommand::getTimeoutTarget() const
{
    return std::chrono::milliseconds(FGE_NET_COMMAND_TIMEOUT_MS);
}

NetCommandResults NetCommand::timeout([[maybe_unused]] Client& client)
{
    return NetCommandResults::FAILURE;
}
void NetCommand::resetTimeout()
{
    this->g_timeout = std::chrono::milliseconds::zero();
}

//NetMTUCommand
NetCommandResults NetMTUCommand::internalUpdate(TransmitPacketPtr& buffPacket,
                                                IpAddress::Types addressType,
                                                [[maybe_unused]] Client& client)
{
    switch (this->g_state)
    {
    case States::ASKING:
        FGE_DEBUG_PRINT("MTU: asking");
        buffPacket = CreatePacket(NET_INTERNAL_ID_MTU_ASK);
        buffPacket->doNotDiscard().doNotReorder().doNotFragment();
        this->g_state = States::WAITING_RESPONSE;
        break;
    case States::DISCOVER:
    {
        //Transmit the new target MTU
        buffPacket = CreatePacket(NET_INTERNAL_ID_MTU_TEST);
        auto const currentSize = buffPacket->doNotDiscard().doNotReorder().doNotFragment().packet().getDataSize();

        auto const extraHeader =
                (addressType == IpAddress::Types::Ipv4 ? FGE_SOCKET_IPV4_HEADER_SIZE : FGE_SOCKET_IPV6_HEADER_SIZE) +
                FGE_SOCKET_UDP_HEADER_SIZE;

        FGE_DEBUG_PRINT("MTU: discover: current try count: {}", this->g_tryCount);

        --this->g_tryCount;
        if (this->g_tryCount == 0 && this->g_currentMTU == 0)
        {
            FGE_DEBUG_PRINT("MTU: discover: last try");
            //Last try
            this->g_targetMTU =
                    addressType == IpAddress::Types::Ipv4 ? FGE_SOCKET_IPV4_MIN_MTU : FGE_SOCKET_IPV6_MIN_MTU;
        }
        buffPacket->append(this->g_targetMTU - currentSize - extraHeader);

#ifdef FGE_DEF_DEBUG
        auto const packetSize = buffPacket->getDataSize();
#endif
        FGE_DEBUG_PRINT("MTU: discover: current packet size: {}", packetSize);

        this->resetTimeout();
        client.getStatus().resetTimeout();
        this->g_state = States::WAITING;
        break;
    }
    default:
        break;
    }

    return NetCommandResults::WORKING;
}
NetCommandResults NetMTUCommand::onReceive(std::unique_ptr<ProtocolPacket>& packet,
                                           IpAddress::Types addressType,
                                           [[maybe_unused]] Client& client)
{
    switch (this->g_state)
    {
    case States::WAITING_RESPONSE:
        if (packet->retrieveHeaderId().value() == NET_INTERNAL_ID_MTU_ASK_RESPONSE)
        {
            std::unique_ptr packetOwned{std::move(packet)};

            //Extract the target MTU
            uint16_t targetMTU;
            if (rules::RValid<uint16_t>({packetOwned->packet(), &targetMTU}).end() || !packetOwned->endReached())
            {
                //Invalid packet
                FGE_DEBUG_PRINT("MTU: Invalid packet");
                this->g_promise.set_value(0);
                return NetCommandResults::FAILURE;
            }

            FGE_DEBUG_PRINT("MTU: targetMTU: {}", targetMTU);

            auto const ourCurrentMTU = static_cast<uint16_t>(FGE_SOCKET_FULL_DATAGRAM_SIZE);
            //socket.retrieveCurrentAdapterMTU().value_or(FGE_SOCKET_FULL_DATAGRAM_SIZE); TODO
            if (targetMTU == 0)
            {
                //We have to figure it ourselves
                this->g_maximumMTU = ourCurrentMTU;
            }
            else
            {
                this->g_maximumMTU = std::min(targetMTU, ourCurrentMTU);
            }

            FGE_DEBUG_PRINT("MTU: maximumMTU: {}", this->g_maximumMTU);

            this->g_currentMTU =
                    addressType == IpAddress::Types::Ipv4 ? FGE_SOCKET_IPV4_MIN_MTU : FGE_SOCKET_IPV6_MIN_MTU;
            if (this->g_currentMTU == this->g_maximumMTU)
            {
                FGE_DEBUG_PRINT("MTU: currentMTU == maximumMTU");
                this->g_promise.set_value(this->g_currentMTU);
                return NetCommandResults::SUCCESS;
            }

            //Compute a new target MTU
            this->g_targetMTU = this->g_maximumMTU;

            if (this->g_maximumMTU < this->g_currentMTU)
            { //Handle the case where the max MTU is smaller than standard IPV4/IPV6 min MTU
                this->g_tryCount = 1;
                this->g_currentMTU = this->g_maximumMTU;
            }
            else
            {
                std::size_t const diff = this->g_maximumMTU - this->g_currentMTU;
                if (diff < FGE_NET_MTU_MIN_INTERVAL)
                {
                    this->g_tryCount = 1;
                }
                else
                {
                    this->g_tryCount = FGE_NET_MTU_TRY_COUNT;
                    this->g_intervalMTU = diff / 2;
                }
            }

            FGE_DEBUG_PRINT("MTU: currentMTU: {}, interval: {}", this->g_currentMTU, this->g_intervalMTU);

            this->resetTimeout();
            this->g_state = States::DISCOVER;
            return NetCommandResults::WORKING;
        }
        break;
    case States::DISCOVER:
    case States::WAITING:
        if (packet->retrieveHeaderId().value() == NET_INTERNAL_ID_MTU_TEST_RESPONSE)
        {
            packet.reset();

            this->g_currentMTU = this->g_targetMTU;

            if (this->g_tryCount == 0 || this->g_currentMTU == this->g_maximumMTU)
            {
                FGE_DEBUG_PRINT(this->g_currentMTU == 0 ? "MTU: discovery failed" : "MTU: discovery ok");
                this->g_promise.set_value(this->g_currentMTU);
                return this->g_currentMTU == 0 ? NetCommandResults::FAILURE : NetCommandResults::SUCCESS;
            }

            this->g_targetMTU += this->g_intervalMTU;
            this->g_intervalMTU = std::max<uint16_t>(FGE_NET_MTU_MIN_INTERVAL, this->g_intervalMTU / 2);
            if (this->g_targetMTU > this->g_maximumMTU)
            {
                this->g_targetMTU = this->g_maximumMTU;
                this->g_tryCount = 0;
            }

            this->resetTimeout();
            this->g_state = States::DISCOVER;
        }
        break;
    default:
        break;
    }

    return NetCommandResults::WORKING;
}
NetCommandResults NetMTUCommand::timeout([[maybe_unused]] Client& client)
{
    if (this->g_state == States::WAITING)
    {
        if (this->g_tryCount == 0)
        {
            FGE_DEBUG_PRINT(this->g_currentMTU == 0 ? "MTU: discovery failed" : "MTU: discovery ok");
            this->g_promise.set_value(this->g_currentMTU);
            return this->g_currentMTU == 0 ? NetCommandResults::FAILURE : NetCommandResults::SUCCESS;
        }

        FGE_DEBUG_PRINT("MTU: packet timeout");

        this->g_targetMTU -= this->g_intervalMTU;
        this->g_intervalMTU = std::max<uint16_t>(FGE_NET_MTU_MIN_INTERVAL, this->g_intervalMTU / 2);

        this->resetTimeout();
        this->g_state = States::DISCOVER;
        return NetCommandResults::WORKING;
    }

    FGE_DEBUG_PRINT("MTU: timeout");
    this->g_promise.set_value(0);
    return NetCommandResults::FAILURE;
}

//NetConnectCommand

void NetConnectCommand::setVersioningString(std::string_view versioningString)
{
    if (versioningString.size() > FGE_NET_MAX_VERSIONING_STRING_SIZE)
    {
        throw Exception("Versioning string too long");
    }

    this->g_versioningString = versioningString;
}
std::string const& NetConnectCommand::getVersioningString() const
{
    return this->g_versioningString;
}

NetCommandResults NetConnectCommand::internalUpdate(TransmitPacketPtr& buffPacket,
                                                    [[maybe_unused]] IpAddress::Types addressType,
                                                    Client& client)
{
    switch (this->g_state)
    {
    case States::TRANSMIT_FGE_HANDSHAKE:
        FGE_DEBUG_PRINT("transmitting handshake");
        client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::UNKNOWN);
        client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
        client._mtuFinalizedFlag = false;

        buffPacket = CreatePacket(NET_INTERNAL_ID_FGE_HANDSHAKE);
        buffPacket->doNotDiscard().doNotReorder().doNotFragment()
                << FGE_NET_HANDSHAKE_STRING << this->g_versioningString;
        this->resetTimeout();
        this->g_state = States::WAITING_FGE_HANDSHAKE;
        break;
    case States::DEALING_WITH_MTU:
        if (this->g_mtuTested)
        {
            auto const mtu = this->g_mtuFuture.valid() ? this->g_mtuFuture.get() : 0;

            if (mtu == 0)
            {
                FGE_DEBUG_PRINT("MTU discovery failed");
                //MTU discovery failed
                client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
                this->g_promise.set_value(false);
                return NetCommandResults::FAILURE;
            }
            client.setMTU(mtu);
            FGE_DEBUG_PRINT("MTU discovery ok, now waiting for server to finish");

            buffPacket = CreatePacket(NET_INTERNAL_ID_MTU_FINAL);
            buffPacket->doNotDiscard().doNotReorder().doNotFragment();

            this->resetTimeout();
            this->g_state = States::WAITING_SERVER_FINAL_MTU;
        }
        else
        {
            FGE_DEBUG_PRINT("testing MTU");
            this->g_mtuTested = true;
            client.setMTU(0);
            auto command = std::make_unique<NetMTUCommand>(this->_g_commandQueue);
            this->g_mtuFuture = command->get_future();
            this->_g_commandQueue->push_front(std::move(command));
            this->resetTimeout();
        }
        break;
    case States::WAITING_SERVER_FINAL_MTU:
    {
        if (!client._mtuFinalizedFlag)
        {
            return NetCommandResults::WORKING;
        }
        FGE_DEBUG_PRINT("MTU finalized");
        client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::MTU_DISCOVERED);
        client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
        this->resetTimeout();
        this->g_state = States::CRYPT_HANDSHAKE;
    }
    break;
    case States::CRYPT_HANDSHAKE:
    {
        auto& info = client.getCryptInfo();

        if (SSL_is_init_finished(static_cast<SSL*>(info._ssl)) == 1)
        {
            FGE_DEBUG_PRINT("TX CONNECTED");
            client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::CONNECTED);
            client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_CONNECTED_TIMEOUT);
            client.setClientPacketCounter(0);
            client.setCurrentPacketCounter(0);
            this->g_promise.set_value(true);
            this->g_state = States::CONNECTED;
            return NetCommandResults::SUCCESS;
        }

        auto const result = SSL_do_handshake(static_cast<SSL*>(info._ssl));
        if (result <= 0)
        {
            auto const err = SSL_get_error(static_cast<SSL*>(info._ssl), result);

            if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
            {
                ERR_print_errors_fp(stderr);
                client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
                this->g_promise.set_value(false);
                return NetCommandResults::FAILURE;
            }
        }

        FGE_DEBUG_PRINT("check for transmit crypt");

        this->resetTimeout();

        // Check if OpenSSL has produced encrypted handshake data
        auto const pendingSize = BIO_ctrl_pending(static_cast<BIO*>(info._wbio));
        if (pendingSize == 0)
        {
            FGE_DEBUG_PRINT("no crypt handshake to transmit");
            return NetCommandResults::WORKING;
        }

        FGE_DEBUG_PRINT("transmitting crypt");
        buffPacket = CreatePacket(NET_INTERNAL_ID_CRYPT_HANDSHAKE);

        auto const packetStartDataPosition = buffPacket->doNotDiscard().getDataSize();
        buffPacket->append(pendingSize);

        auto const finalSize =
                BIO_read(static_cast<BIO*>(info._wbio), buffPacket->getData() + packetStartDataPosition, pendingSize);
        if (finalSize <= 0 || static_cast<std::size_t>(finalSize) != pendingSize)
        {
            FGE_DEBUG_PRINT("failed crypt");
            client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
            this->g_promise.set_value(false);
            return NetCommandResults::FAILURE;
        }

        FGE_DEBUG_PRINT("waiting response");
        this->g_state = States::CRYPT_WAITING;
    }
    break;
    case States::CRYPT_WAITING:
        if (SSL_is_init_finished(static_cast<SSL*>(client.getCryptInfo()._ssl)) == 1)
        {
            FGE_DEBUG_PRINT("CONNECTED");
            client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::CONNECTED);
            client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_CONNECTED_TIMEOUT);
            client.setClientPacketCounter(0);
            client.setCurrentPacketCounter(0);
            this->g_promise.set_value(true);
            return NetCommandResults::SUCCESS;
        }
        break;
    case States::CONNECTED:
        return NetCommandResults::SUCCESS;
    default:
        break;
    }

    return NetCommandResults::WORKING;
}

NetCommandResults NetConnectCommand::onReceive(std::unique_ptr<ProtocolPacket>& packet,
                                               [[maybe_unused]] IpAddress::Types addressType,
                                               Client& client)
{
    switch (this->g_state)
    {
    case States::WAITING_FGE_HANDSHAKE:
    {
        if (packet->retrieveHeaderId().value() != NET_INTERNAL_ID_FGE_HANDSHAKE)
        {
            return NetCommandResults::WORKING;
        }

        std::unique_ptr packetOwned{std::move(packet)};

        FGE_DEBUG_PRINT("receiving handshake response");

        std::string handshake;
        if (rules::RValid<std::string>({packetOwned->packet(), &handshake}).end() || !packetOwned->endReached())
        {
            FGE_DEBUG_PRINT("handshake failed");
            client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
            this->g_promise.set_value(false);
            return NetCommandResults::FAILURE;
        }

        if (handshake != FGE_NET_HANDSHAKE_STRING)
        {
            FGE_DEBUG_PRINT("handshake failed");
            client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
            this->g_promise.set_value(false);
            return NetCommandResults::FAILURE;
        }

        FGE_DEBUG_PRINT("RX ACKNOWLEDGED");
        client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::ACKNOWLEDGED);
        client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
        this->resetTimeout();

        this->g_state = States::DEALING_WITH_MTU;
    }
    break;
    case States::CRYPT_WAITING:
    {
        if (packet->retrieveHeaderId() != NET_INTERNAL_ID_CRYPT_HANDSHAKE)
        {
            return NetCommandResults::WORKING;
        }

        std::unique_ptr packetOwned{std::move(packet)};

        auto& info = client.getCryptInfo();

        auto const readPos = packetOwned->getReadPos();
        BIO_write(static_cast<BIO*>(info._rbio), packetOwned->getData() + readPos,
                  packetOwned->getDataSize() - readPos);

        FGE_DEBUG_PRINT("Crypt: received some data");

        this->resetTimeout();
        this->g_state = States::CRYPT_HANDSHAKE;
    }
    break;
    case States::CONNECTED:
        return NetCommandResults::SUCCESS;
    default:
        break;
    }

    return NetCommandResults::WORKING;
}

NetCommandResults NetConnectCommand::timeout(Client& client)
{
    FGE_DEBUG_PRINT("connect: timeout");
    client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
    this->g_promise.set_value(false);
    return NetCommandResults::FAILURE;
}

//NetDisconnectCommand

NetCommandResults NetDisconnectCommand::internalUpdate(TransmitPacketPtr& buffPacket,
                                                       [[maybe_unused]] IpAddress::Types addressType,
                                                       Client& client)
{
    if (client.getStatus().getNetworkStatus() == ClientStatus::NetworkStatus::DISCONNECTED)
    {
        this->g_promise.set_value();
        return NetCommandResults::SUCCESS;
    }

    if (this->g_transmitted)
    {
        return NetCommandResults::WORKING;
    }

    client.clearPackets();

    buffPacket = CreatePacket(NET_INTERNAL_ID_DISCONNECT);
    buffPacket->doNotDiscard().doNotReorder().doNotFragment();
    this->g_transmitted = true;

    return NetCommandResults::SUCCESS;
}

NetCommandResults NetDisconnectCommand::onReceive([[maybe_unused]] std::unique_ptr<ProtocolPacket>& packet,
                                                  [[maybe_unused]] IpAddress::Types addressType,
                                                  [[maybe_unused]] Client& client)
{
    return NetCommandResults::WORKING;
}

NetCommandResults NetDisconnectCommand::timeout([[maybe_unused]] Client& client)
{
    this->g_promise.set_value();
    return NetCommandResults::FAILURE;
}

} // namespace fge::net

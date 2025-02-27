/*
 * Copyright 2024 Guillaume Guillet
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
#include "FastEngine/network/C_socket.hpp"
#include <iostream>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace fge::net
{

//NetCommands

NetCommandResults NetMTUCommand::update(TransmitPacketPtr& buffPacket,
                                        IpAddress::Types addressType,
                                        [[maybe_unused]] Client& client,
                                        std::chrono::milliseconds deltaTime)
{
    this->_g_timeout += deltaTime;

    switch (this->g_state)
    {
    case States::ASKING:
        std::cout << "MTU: asking" << std::endl;
        buffPacket = CreatePacket(NET_INTERNAL_ID_MTU_ASK);
        buffPacket->doNotDiscard().doNotReorder().doNotFragment();
        this->g_state = States::WAITING_RESPONSE;
        break;
    case States::WAITING_RESPONSE:
        if (this->_g_timeout >= FGE_NET_MTU_TIMEOUT_MS)
        {
            std::cout << "MTU: timeout" << std::endl;
            this->g_promise.set_value(0);
            return NetCommandResults::FAILURE;
        }
        break;
    case States::DISCOVER:
    {
        //Transmit the new target MTU
        buffPacket = CreatePacket(NET_INTERNAL_ID_MTU_TEST);
        auto const currentSize = buffPacket->doNotDiscard().doNotReorder().doNotFragment().packet().getDataSize();

        auto const extraHeader =
                (addressType == IpAddress::Types::Ipv4 ? FGE_SOCKET_IPV4_HEADER_SIZE : FGE_SOCKET_IPV6_HEADER_SIZE) +
                FGE_SOCKET_UDP_HEADER_SIZE;

        std::cout << "MTU: discover: currentSize: " << currentSize << std::endl;

        --this->g_tryCount;
        if (this->g_tryCount == 0 && this->g_currentMTU == 0)
        {
            std::cout << "MTU: discover: last try" << std::endl;
            //Last try
            this->g_targetMTU =
                    addressType == IpAddress::Types::Ipv4 ? FGE_SOCKET_IPV4_MIN_MTU : FGE_SOCKET_IPV6_MIN_MTU;
        }
        buffPacket->packet().append(this->g_targetMTU - currentSize - extraHeader);
        this->g_state = States::WAITING;
        break;
    }
    case States::WAITING:
        if (this->_g_timeout >= FGE_NET_MTU_TIMEOUT_MS)
        {
            if (this->g_tryCount == 0)
            {
                std::cout << ((this->g_currentMTU == 0) ? "MTU: discovery failed" : "MTU: discovery ok") << std::endl;
                this->g_promise.set_value(this->g_currentMTU);
                return this->g_currentMTU == 0 ? NetCommandResults::FAILURE : NetCommandResults::SUCCESS;
            }

            std::cout << "MTU: packet timeout" << std::endl;

            this->g_targetMTU -= this->g_intervalMTU;
            this->g_intervalMTU = std::max<uint16_t>(FGE_NET_MTU_MIN_INTERVAL, this->g_intervalMTU / 2);

            this->_g_timeout = std::chrono::milliseconds::zero();
            this->g_state = States::DISCOVER;
            return NetCommandResults::WORKING;
        }
        break;
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
            //Extract the target MTU
            uint16_t targetMTU;
            if (rules::RValid<uint16_t>({packet->packet(), &targetMTU}).end() || !packet->endReached())
            {
                //Invalid packet
                std::cout << "MTU: Invalid packet" << std::endl;
                this->g_promise.set_value(0);
                return NetCommandResults::FAILURE;
            }

            std::cout << "MTU: targetMTU: " << targetMTU << std::endl;

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

            std::cout << "MTU: maximumMTU: " << this->g_maximumMTU << std::endl;

            this->g_currentMTU =
                    addressType == IpAddress::Types::Ipv4 ? FGE_SOCKET_IPV4_MIN_MTU : FGE_SOCKET_IPV6_MIN_MTU;
            if (this->g_currentMTU == this->g_maximumMTU)
            {
                std::cout << "MTU: currentMTU == maximumMTU" << std::endl;
                this->g_promise.set_value(this->g_currentMTU);
                return NetCommandResults::SUCCESS;
            }

            //Compute a new target MTU
            this->g_targetMTU = this->g_maximumMTU;

            std::size_t const diff = this->g_maximumMTU - this->g_currentMTU;
            if (diff < FGE_NET_MTU_MIN_INTERVAL)
            {
                this->g_tryCount = 0;
            }
            else
            {
                this->g_tryCount = FGE_NET_MTU_TRY_COUNT;
                this->g_intervalMTU = diff / 2;
            }

            std::cout << "MTU: currentMTU: " << this->g_currentMTU << std::endl;

            this->_g_timeout = std::chrono::milliseconds::zero();
            this->g_state = States::DISCOVER;
            return NetCommandResults::WORKING;
        }
        break;
    case States::DISCOVER:
    case States::WAITING:
        if (packet->retrieveHeaderId().value() == NET_INTERNAL_ID_MTU_TEST_RESPONSE)
        {
            this->g_currentMTU = this->g_targetMTU;

            if (this->g_tryCount == 0 || this->g_currentMTU == this->g_maximumMTU)
            {
                std::cout << (this->g_currentMTU == 0 ? "MTU: discovery failed" : "MTU: discovery ok") << std::endl;
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

            this->_g_timeout = std::chrono::milliseconds::zero();
            this->g_state = States::DISCOVER;
            break;
        }
        break;
    default:
        break;
    }

    return NetCommandResults::WORKING;
}

NetCommandResults NetConnectCommand::update(TransmitPacketPtr& buffPacket,
                                            [[maybe_unused]] IpAddress::Types addressType,
                                            Client& client,
                                            std::chrono::milliseconds deltaTime)
{
    this->_g_timeout += deltaTime;

    switch (this->g_state)
    {
    case States::TRANSMIT_FGE_HANDSHAKE:
        std::cout << "transmitting handshake" << std::endl;
        client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::UNKNOWN);
        client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
        client._mtuFinalizedFlag = false;

        buffPacket = CreatePacket(NET_INTERNAL_ID_FGE_HANDSHAKE);
        buffPacket->doNotDiscard().doNotReorder().doNotFragment() << FGE_NET_HANDSHAKE_STRING;
        this->_g_timeout = std::chrono::milliseconds::zero();
        this->g_state = States::WAITING_FGE_HANDSHAKE;
        break;
    case States::WAITING_FGE_HANDSHAKE:
        if (this->_g_timeout >= FGE_NET_CONNECT_TIMEOUT_MS)
        {
            std::cout << "timeout" << std::endl;
            client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
            this->g_promise.set_value(false);
            return NetCommandResults::FAILURE;
        }
        break;
    case States::DEALING_WITH_MTU:
        if (this->g_mtuTested)
        {
            auto const mtu = this->g_mtuFuture.valid() ? this->g_mtuFuture.get() : 0;

            if (mtu == 0)
            {
                std::cout << "MTU discovery failed" << std::endl;
                //MTU discovery failed
                client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
                this->g_promise.set_value(false);
                return NetCommandResults::FAILURE;
            }
            client.setMTU(mtu);
            std::cout << "MTU discovery ok, now waiting for server to finish" << std::endl;

            buffPacket = CreatePacket(NET_INTERNAL_ID_MTU_FINAL);
            buffPacket->doNotDiscard().doNotReorder().doNotFragment();

            this->_g_timeout = std::chrono::milliseconds::zero();
            this->g_state = States::WAITING_SERVER_FINAL_MTU;
        }
        else
        {
            std::cout << "testing MTU" << std::endl;
            this->g_mtuTested = true;
            client.setMTU(0);
            auto command = std::make_unique<NetMTUCommand>(this->_g_commandQueue);
            this->g_mtuFuture = command->get_future();
            this->_g_commandQueue->push_front(std::move(command));
            this->_g_timeout = std::chrono::milliseconds::zero();
        }
        break;
    case States::WAITING_SERVER_FINAL_MTU:
    {
        if (!client._mtuFinalizedFlag)
        {
            return NetCommandResults::WORKING;
        }
        std::cout << "MTU finalized" << std::endl;
        client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::MTU_DISCOVERED);
        client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
        this->_g_timeout = std::chrono::milliseconds::zero();
        this->g_state = States::CRYPT_HANDSHAKE;
    }
    break;
    case States::CRYPT_HANDSHAKE:
    {
        auto& info = client.getCryptInfo();

        if (SSL_is_init_finished(static_cast<SSL*>(info._ssl)) == 1)
        {
            std::cout << "TX CONNECTED" << std::endl;
            client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::CONNECTED);
            client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_CONNECTED_TIMEOUT);
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

        std::cout << "check for transmit crypt" << std::endl;

        this->_g_timeout = std::chrono::milliseconds::zero();

        // Check if OpenSSL has produced encrypted handshake data
        auto const pendingSize = BIO_ctrl_pending(static_cast<BIO*>(info._wbio));
        if (pendingSize == 0)
        {
            std::cout << "NONE" << std::endl;
            return NetCommandResults::WORKING;
        }

        std::cout << "transmitting crypt" << std::endl;
        buffPacket = CreatePacket(NET_INTERNAL_ID_CRYPT_HANDSHAKE);

        auto const packetStartDataPosition = buffPacket->doNotDiscard().getDataSize();
        buffPacket->append(pendingSize);

        auto const finalSize =
                BIO_read(static_cast<BIO*>(info._wbio), buffPacket->getData() + packetStartDataPosition, pendingSize);
        if (finalSize <= 0 || static_cast<std::size_t>(finalSize) != pendingSize)
        {
            std::cout << "failed crypt" << std::endl;
            client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
            this->g_promise.set_value(false);
            return NetCommandResults::FAILURE;
        }

        std::cout << "waiting response" << std::endl;
        this->g_state = States::CRYPT_WAITING;
    }
    break;
    case States::CRYPT_WAITING:
        if (SSL_is_init_finished(static_cast<SSL*>(client.getCryptInfo()._ssl)) == 1)
        {
            std::cout << "CONNECTED" << std::endl;
            client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::CONNECTED);
            client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_CONNECTED_TIMEOUT);
            this->g_promise.set_value(true);
            return NetCommandResults::SUCCESS;
        }

        if (this->_g_timeout >= FGE_NET_CONNECT_TIMEOUT_MS)
        {
            client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
            this->g_promise.set_value(false);
            return NetCommandResults::FAILURE;
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

        std::cout << "receiving handshake response" << std::endl;

        std::string handshake;
        if (rules::RValid<std::string>({packet->packet(), &handshake}).end() || !packet->endReached())
        {
            std::cout << "handshake failed" << std::endl;
            return NetCommandResults::FAILURE;
        }

        if (handshake != FGE_NET_HANDSHAKE_STRING)
        {
            std::cout << "handshake failed" << std::endl;
            return NetCommandResults::FAILURE;
        }

        std::cout << "RX ACKNOWLEDGED" << std::endl;
        client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::ACKNOWLEDGED);
        client.getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
        this->_g_timeout = std::chrono::milliseconds::zero();

        this->g_state = States::DEALING_WITH_MTU;
    }
    break;
    case States::CRYPT_WAITING:
    {
        if (packet->retrieveHeaderId() != NET_INTERNAL_ID_CRYPT_HANDSHAKE)
        {
            return NetCommandResults::WORKING;
        }

        auto& info = client.getCryptInfo();

        auto const readPos = packet->getReadPos();
        BIO_write(static_cast<BIO*>(info._rbio), packet->getData() + readPos, packet->getDataSize() - readPos);

        std::cout << "Crypt: received some data" << std::endl;

        this->_g_timeout = std::chrono::milliseconds::zero();
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

} // namespace fge::net

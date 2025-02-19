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

#ifndef _FGE_C_NETCOMMAND_HPP_INCLUDED
#define _FGE_C_NETCOMMAND_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/network/C_protocol.hpp"
#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <thread>

#define FGE_NET_MTU_TIMEOUT_MS                                                                                         \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        200                                                                                                            \
    }
#define FGE_NET_MTU_TRY_COUNT 12
#define FGE_NET_MTU_MIN_INTERVAL 16

#define FGE_NET_CONNECT_TIMEOUT_MS                                                                                     \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        1000                                                                                                           \
    }

namespace fge::net
{

enum class NetCommandTypes
{
    DISCOVER_MTU,
    CONNECT
};
enum class NetCommandResults
{
    SUCCESS,
    WORKING,
    FAILURE
};

class NetCommand;
using CommandQueue = std::deque<std::unique_ptr<NetCommand>>;

class NetCommand
{
public:
    inline explicit NetCommand(CommandQueue* commandQueue) :
            _g_commandQueue(commandQueue)
    {}
    virtual ~NetCommand() = default;

    [[nodiscard]] virtual NetCommandTypes getType() const = 0;
    [[nodiscard]] virtual NetCommandResults
    transmit(TransmitPacketPtr& buffPacket, IpAddress::Types addressType, Client& client) = 0;
    [[nodiscard]] virtual NetCommandResults receive(std::unique_ptr<ProtocolPacket>& packet,
                                                    IpAddress::Types addressType,
                                                    std::chrono::milliseconds deltaTime,
                                                    Client* client) = 0;

protected:
    CommandQueue* _g_commandQueue{nullptr};
};

class FGE_API NetMTUCommand : public NetCommand
{
public:
    using NetCommand::NetCommand;
    ~NetMTUCommand() final = default;

    [[nodiscard]] NetCommandTypes getType() const override { return NetCommandTypes::DISCOVER_MTU; }

    [[nodiscard]] NetCommandResults
    transmit(TransmitPacketPtr& buffPacket, IpAddress::Types addressType, Client& client) override;
    [[nodiscard]] NetCommandResults receive(std::unique_ptr<ProtocolPacket>& packet,
                                            IpAddress::Types addressType,
                                            std::chrono::milliseconds deltaTime,
                                            Client* client) override;

    [[nodiscard]] inline std::future<uint16_t> get_future() { return this->g_promise.get_future(); }

private:
    std::promise<uint16_t> g_promise;
    uint16_t g_currentMTU{0};
    uint16_t g_targetMTU{0};
    uint16_t g_maximumMTU{0};
    uint16_t g_intervalMTU{0};
    std::size_t g_tryCount{0};
    enum class States
    {
        ASKING,
        WAITING_RESPONSE,

        DISCOVER,
        WAITING
    } g_state{States::ASKING};
    std::chrono::milliseconds g_receiveTimeout{0};
};

class FGE_API NetConnectCommand : public NetCommand
{
public:
    using NetCommand::NetCommand;
    ~NetConnectCommand() final = default;

    [[nodiscard]] NetCommandTypes getType() const override { return NetCommandTypes::CONNECT; }

    [[nodiscard]] NetCommandResults
    transmit(TransmitPacketPtr& buffPacket, IpAddress::Types addressType, Client& client) override;
    [[nodiscard]] NetCommandResults receive(std::unique_ptr<ProtocolPacket>& packet,
                                            IpAddress::Types addressType,
                                            std::chrono::milliseconds deltaTime,
                                            Client* client) override;

    [[nodiscard]] inline std::future<bool> get_future() { return this->g_promise.get_future(); }

private:
    std::promise<bool> g_promise;
    enum class States
    {
        TRANSMIT_FGE_HANDSHAKE,
        WAITING_FGE_HANDSHAKE,

        DEALING_WITH_MTU,
        WAITING_SERVER_FINAL_MTU,

        CRYPT_HANDSHAKE,
        CRYPT_WAITING,

        CONNECTED
    } g_state{States::TRANSMIT_FGE_HANDSHAKE};
    bool g_mtuTested{false};
    std::future<uint16_t> g_mtuFuture;
    std::chrono::milliseconds g_timeout{0};
};

} // namespace fge::net

#endif // _FGE_C_NETCOMMAND_HPP_INCLUDED

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

#ifndef _FGE_C_NETCOMMAND_HPP_INCLUDED
#define _FGE_C_NETCOMMAND_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/network/C_protocol.hpp"
#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <thread>

#define FGE_NET_CMD_UPDATE_TICK_MS                                                                                     \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        30                                                                                                             \
    }

#define FGE_NET_MTU_TIMEOUT_MS                                                                                         \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        400                                                                                                            \
    }
#define FGE_NET_MTU_TRY_COUNT 12
#define FGE_NET_MTU_MIN_INTERVAL 16

#define FGE_NET_CONNECT_TIMEOUT_MS                                                                                     \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        1000                                                                                                           \
    }

#define FGE_NET_DISCONNECT_TIMEOUT_MS                                                                                  \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        1000                                                                                                           \
    }

#define FGE_NET_COMMAND_TIMEOUT_MS                                                                                     \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        500                                                                                                            \
    }

#define FGE_NET_MAX_VERSIONING_STRING_SIZE 32

namespace fge::net
{

enum class NetCommandTypes
{
    DISCOVER_MTU,
    CONNECT,
    DISCONNECT
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
    [[nodiscard]] NetCommandResults update(TransmitPacketPtr& buffPacket,
                                           IpAddress::Types addressType,
                                           Client& client,
                                           std::chrono::milliseconds deltaTime);
    [[nodiscard]] virtual NetCommandResults
    onReceive(std::unique_ptr<ProtocolPacket>& packet, IpAddress::Types addressType, Client& client) = 0;

    [[nodiscard]] virtual std::chrono::milliseconds getTimeoutTarget() const;

protected:
    [[nodiscard]] virtual NetCommandResults internalUpdate(TransmitPacketPtr& buffPacket,
                                                           IpAddress::Types addressType,
                                                           Client& client,
                                                           std::chrono::milliseconds deltaTime) = 0;
    [[nodiscard]] virtual NetCommandResults timeout(Client& client);
    void resetTimeout();

    CommandQueue* _g_commandQueue{nullptr};

private:
    std::chrono::milliseconds g_timeout{0};
};

class FGE_API NetMTUCommand final : public NetCommand
{
public:
    using NetCommand::NetCommand;
    ~NetMTUCommand() override = default;

    [[nodiscard]] NetCommandTypes getType() const override { return NetCommandTypes::DISCOVER_MTU; }

    [[nodiscard]] NetCommandResults internalUpdate(TransmitPacketPtr& buffPacket,
                                                   IpAddress::Types addressType,
                                                   Client& client,
                                                   std::chrono::milliseconds deltaTime) override;
    [[nodiscard]] NetCommandResults
    onReceive(std::unique_ptr<ProtocolPacket>& packet, IpAddress::Types addressType, Client& client) override;

    [[nodiscard]] inline std::future<uint16_t> get_future() { return this->g_promise.get_future(); }

    [[nodiscard]] inline std::chrono::milliseconds getTimeoutTarget() const override { return FGE_NET_MTU_TIMEOUT_MS; }

private:
    [[nodiscard]] NetCommandResults timeout(Client& client) override;

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
};

class FGE_API NetConnectCommand final : public NetCommand
{
public:
    using NetCommand::NetCommand;
    ~NetConnectCommand() override = default;

    void setVersioningString(std::string_view versioningString);
    [[nodiscard]] std::string const& getVersioningString() const;

    [[nodiscard]] NetCommandTypes getType() const override { return NetCommandTypes::CONNECT; }

    [[nodiscard]] NetCommandResults internalUpdate(TransmitPacketPtr& buffPacket,
                                                   IpAddress::Types addressType,
                                                   Client& client,
                                                   std::chrono::milliseconds deltaTime) override;
    [[nodiscard]] NetCommandResults
    onReceive(std::unique_ptr<ProtocolPacket>& packet, IpAddress::Types addressType, Client& client) override;

    [[nodiscard]] inline std::future<bool> get_future() { return this->g_promise.get_future(); }

    [[nodiscard]] inline std::chrono::milliseconds getTimeoutTarget() const override
    {
        return FGE_NET_CONNECT_TIMEOUT_MS;
    }

private:
    [[nodiscard]] NetCommandResults timeout(Client& client) override;

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
    std::string g_versioningString;
};

class FGE_API NetConnectHandlerCommand final : public NetCommand
{
public:
    using NetCommand::NetCommand;
    ~NetConnectHandlerCommand() override = default;

    [[nodiscard]] NetCommandTypes getType() const override { return NetCommandTypes::CONNECT; }

    [[nodiscard]] NetCommandResults internalUpdate(TransmitPacketPtr& buffPacket,
                                                   IpAddress::Types addressType,
                                                   Client& client,
                                                   std::chrono::milliseconds deltaTime) override;
    [[nodiscard]] NetCommandResults
    onReceive(std::unique_ptr<ProtocolPacket>& packet, IpAddress::Types addressType, Client& client) override;

    [[nodiscard]] inline std::future<bool> get_future() { return this->g_promise.get_future(); }

    [[nodiscard]] inline std::chrono::milliseconds getTimeoutTarget() const override
    {
        return FGE_NET_CONNECT_TIMEOUT_MS;
    }

private:
    [[nodiscard]] NetCommandResults timeout(Client& client) override;

    std::promise<bool> g_promise;
    enum class States
    {
        LOOKUP_MTU,

        DEALING_WITH_MTU,
        WAITING_CLIENT_FINAL_MTU,

        CRYPT_HANDSHAKE,
        CRYPT_WAITING,

        CONNECTED
    } g_state{States::LOOKUP_MTU};

    std::future<uint16_t> g_mtuFuture;
    NetMTUCommand g_mtuCommand{this->_g_commandQueue};
};

class FGE_API NetDisconnectCommand final : public NetCommand
{
public:
    using NetCommand::NetCommand;
    ~NetDisconnectCommand() override = default;

    [[nodiscard]] NetCommandTypes getType() const override { return NetCommandTypes::CONNECT; }

    [[nodiscard]] NetCommandResults internalUpdate(TransmitPacketPtr& buffPacket,
                                                   IpAddress::Types addressType,
                                                   Client& client,
                                                   std::chrono::milliseconds deltaTime) override;
    [[nodiscard]] NetCommandResults
    onReceive(std::unique_ptr<ProtocolPacket>& packet, IpAddress::Types addressType, Client& client) override;

    [[nodiscard]] inline std::future<void> get_future() { return this->g_promise.get_future(); }

    [[nodiscard]] inline std::chrono::milliseconds getTimeoutTarget() const override
    {
        return FGE_NET_DISCONNECT_TIMEOUT_MS;
    }

private:
    [[nodiscard]] NetCommandResults timeout(Client& client) override;

    std::promise<void> g_promise;
    bool g_transmitted{false};
};

} // namespace fge::net

#endif // _FGE_C_NETCOMMAND_HPP_INCLUDED

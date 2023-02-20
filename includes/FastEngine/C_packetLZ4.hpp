/*
 * Copyright 2023 Guillaume Guillet
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

#ifndef _FGE_C_PACKETLZ4_HPP_INCLUDED
#define _FGE_C_PACKETLZ4_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_packet.hpp"

/*
 * This file is using the library :
 * LZ4 - Fast LZ compression algorithm
 * Copyright (C) 2011-present, Yann Collet.
 * BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
 */

#define FGE_PACKETLZ4_DEFAULT_MAXUNCOMPRESSEDRECEIVEDSIZE 65536
#define FGE_PACKETLZ4HC_DEFAULT_MAXUNCOMPRESSEDRECEIVEDSIZE 65536
#define FGE_PACKETLZ4_VERSION "1.9.4"

namespace fge
{
namespace net
{

class FGE_API PacketLZ4 : public fge::net::Packet
{
public:
    PacketLZ4();
    PacketLZ4(fge::net::PacketLZ4&& pck) noexcept;
    PacketLZ4(fge::net::PacketLZ4& pck) = default;
    PacketLZ4(const fge::net::PacketLZ4& pck) = default;
    ~PacketLZ4() override = default;

    static uint32_t _maxUncompressedReceivedSize;

    [[nodiscard]] std::size_t getLastCompressionSize() const;

protected:
    void onSend(std::vector<uint8_t>& buffer, std::size_t offset) override;
    void onReceive(void* data, std::size_t dsize) override;

private:
    std::vector<char> g_buffer;
    std::size_t g_lastCompressionSize;
};

class FGE_API PacketLZ4HC : public fge::net::Packet
{
public:
    PacketLZ4HC();
    PacketLZ4HC(fge::net::PacketLZ4HC&& pck) noexcept;
    PacketLZ4HC(fge::net::PacketLZ4HC& pck) = default;
    PacketLZ4HC(const fge::net::PacketLZ4HC& pck) = default;
    ~PacketLZ4HC() override = default;

    static uint32_t _maxUncompressedReceivedSize;

    void setCompressionLevel(int value);
    [[nodiscard]] int getCompressionLevel() const;

    [[nodiscard]] std::size_t getLastCompressionSize() const;

protected:
    void onSend(std::vector<uint8_t>& buffer, std::size_t offset) override;
    void onReceive(void* data, std::size_t dsize) override;

private:
    std::vector<char> g_buffer;
    int g_compressionLevel;
    std::size_t g_lastCompressionSize;
};

} // namespace net
} // namespace fge

#endif // _FGE_C_PACKETLZ4_HPP_INCLUDED

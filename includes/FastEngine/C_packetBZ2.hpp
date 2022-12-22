/*
 * Copyright 2022 Guillaume Guillet
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

#ifndef _FGE_C_PACKETBZ2_HPP_INCLUDED
#define _FGE_C_PACKETBZ2_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_packet.hpp>

/*
 * This file is using the library :
 * libbzip2
 * copyright (C) 1996-2019 Julian R Seward.
 */

#define FGE_PACKETBZ2_DEFAULT_WORKFACTOR 0
#define FGE_PACKETBZ2_DEFAULT_BLOCKSIZE 4

#define FGE_PACKETBZ2_DEFAULT_MAXUNCOMPRESSEDRECEIVEDSIZE 65536
#define FGE_PACKETBZ2_VERSION "1.0.8"

namespace fge
{
namespace net
{

class FGE_API PacketBZ2 : public fge::net::Packet
{
public:
    PacketBZ2();
    PacketBZ2(fge::net::PacketBZ2&& pck) noexcept;
    PacketBZ2(fge::net::PacketBZ2& pck) = default;
    PacketBZ2(const fge::net::PacketBZ2& pck) = default;
    ~PacketBZ2() override = default;

    static uint32_t _maxUncompressedReceivedSize;

    void setBlockSize(int blockSize);
    [[nodiscard]] int getBlockSize() const;
    void setWorkFactor(int factor);
    [[nodiscard]] int getWorkFactor() const;

    [[nodiscard]] std::size_t getLastCompressionSize() const;

protected:
    void onSend(std::vector<uint8_t>& buffer, std::size_t offset) override;
    void onReceive(void* data, std::size_t dsize) override;

private:
    int g_blockSize;
    int g_workfactor;

    std::vector<char> g_buffer;
    std::size_t g_lastCompressionSize;
};

} // namespace net
} // namespace fge

#endif // _FGE_C_PACKETBZ2_HPP_INCLUDED

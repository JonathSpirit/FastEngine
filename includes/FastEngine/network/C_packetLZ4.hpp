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

#ifndef _FGE_C_PACKETLZ4_HPP_INCLUDED
#define _FGE_C_PACKETLZ4_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_packet.hpp"
#include "FastEngine/C_compressorLZ4.hpp"
#include <atomic>

#define FGE_NET_LZ4HC_DEFAULT_PACKET_MAX_SIZE FGE_COMPRESSOR_LZ4HC_DEFAULT_MAX_SIZE
#define FGE_NET_LZ4_DEFAULT_PACKET_MAX_SIZE FGE_COMPRESSOR_LZ4_DEFAULT_MAX_SIZE

namespace fge::net
{

class FGE_API PacketLZ4 : public Packet
{
public:
    PacketLZ4() = default;
    PacketLZ4(PacketLZ4&& pck) noexcept;
    PacketLZ4(Packet&& pck) noexcept;
    PacketLZ4(PacketLZ4 const& pck);
    PacketLZ4(Packet const& pck);
    ~PacketLZ4() override = default;

    [[nodiscard]] CompressorLZ4 const& getCompressor() const;

    static std::atomic_uint32_t gMaxUncompressedSize;

protected:
    bool onSend(std::vector<uint8_t>& buffer, std::size_t offset) override;
    void onReceive(void* data, std::size_t dsize) override;

private:
    CompressorLZ4 g_compressor;
};

class FGE_API PacketLZ4HC : public Packet
{
public:
    PacketLZ4HC() = default;
    PacketLZ4HC(PacketLZ4HC&& pck) noexcept;
    PacketLZ4HC(Packet&& pck) noexcept;
    PacketLZ4HC(PacketLZ4HC const& pck);
    PacketLZ4HC(Packet const& pck);
    ~PacketLZ4HC() override = default;

    [[nodiscard]] CompressorLZ4HC const& getCompressor() const;

    static std::atomic_uint32_t gMaxUncompressedSize;
    static std::atomic_int gCompressionLevel;

protected:
    bool onSend(std::vector<uint8_t>& buffer, std::size_t offset) override;
    void onReceive(void* data, std::size_t dsize) override;

private:
    CompressorLZ4HC g_compressor;
};

} // namespace fge::net

#endif // _FGE_C_PACKETLZ4_HPP_INCLUDED

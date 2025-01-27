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
#include "C_compressor.hpp"
#include "C_packet.hpp"
#include <atomic>
#include <limits>

/*
 * This file is using the library :
 * LZ4 - Fast LZ compression algorithm
 * Copyright (C) 2011-present, Yann Collet.
 * BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
 */

#define FGE_NET_LZ4_EXTRA_BYTES 10
#define FGE_NET_LZ4_DEFAULT_MAX_SIZE std::numeric_limits<uint16_t>::max()
#define FGE_NET_LZ4_DEFAULT_PACKET_MAX_SIZE FGE_NET_LZ4_DEFAULT_MAX_SIZE
#define FGE_NET_LZ4HC_DEFAULT_MAX_SIZE std::numeric_limits<uint16_t>::max()
#define FGE_NET_LZ4HC_DEFAULT_PACKET_MAX_SIZE FGE_NET_LZ4HC_DEFAULT_MAX_SIZE
#define FGE_NET_LZ4HC_DEFAULT_COMPRESSION_LEVEL 9
#define FGE_NET_LZ4_VERSION "1.10.0"

namespace fge::net
{

class FGE_API CompressorLZ4 : public Compressor
{
public:
    using Compressor::Compressor;

    [[nodiscard]] std::optional<Error> compress(std::span<uint8_t const> rawData) override;
    [[nodiscard]] std::optional<Error> uncompress(std::span<uint8_t const> data) override;

    void setMaxUncompressedSize(uint32_t value);
    [[nodiscard]] uint32_t getMaxUncompressedSize() const;

private:
    uint32_t g_maxUncompressedSize{FGE_NET_LZ4_DEFAULT_MAX_SIZE};
};

class FGE_API CompressorLZ4HC : public Compressor
{
public:
    using Compressor::Compressor;

    [[nodiscard]] std::optional<Error> compress(std::span<uint8_t const> rawData) override;
    [[nodiscard]] std::optional<Error> uncompress(std::span<uint8_t const> data) override;

    void setMaxUncompressedSize(uint32_t value);
    [[nodiscard]] uint32_t getMaxUncompressedSize() const;

    void setCompressionLevel(int value);
    [[nodiscard]] int getCompressionLevel() const;

private:
    uint32_t g_maxUncompressedSize{FGE_NET_LZ4HC_DEFAULT_MAX_SIZE};
    int g_compressionLevel{FGE_NET_LZ4HC_DEFAULT_COMPRESSION_LEVEL};
};

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
    void onSend(std::vector<uint8_t>& buffer, std::size_t offset) override;
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
    void onSend(std::vector<uint8_t>& buffer, std::size_t offset) override;
    void onReceive(void* data, std::size_t dsize) override;

private:
    CompressorLZ4HC g_compressor;
};

} // namespace fge::net

#endif // _FGE_C_PACKETLZ4_HPP_INCLUDED

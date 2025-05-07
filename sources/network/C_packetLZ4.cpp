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

#include "FastEngine/network/C_packetLZ4.hpp"

namespace fge::net
{

//PacketLZ4

std::atomic_uint32_t PacketLZ4::gMaxUncompressedSize{FGE_NET_LZ4_DEFAULT_PACKET_MAX_SIZE};

PacketLZ4::PacketLZ4(PacketLZ4&& pck) noexcept :
        Packet(std::move(static_cast<Packet&&>(pck))),
        g_compressor(std::move(pck.g_compressor))
{}
PacketLZ4::PacketLZ4(Packet&& pck) noexcept :
        Packet(std::move(pck))
{}
PacketLZ4::PacketLZ4(PacketLZ4 const& pck) :
        Packet(pck),
        g_compressor(pck.g_compressor)
{}
PacketLZ4::PacketLZ4(Packet const& pck) :
        Packet(pck)
{}

CompressorLZ4 const& PacketLZ4::getCompressor() const
{
    return this->g_compressor;
}

bool PacketLZ4::onSend(std::size_t offset)
{
    auto const err = this->g_compressor.compress({this->getData(), this->getDataSize()});
    if (err)
    {
        this->_g_transmitCacheValid = false;
        return false;
    }

    this->_g_transmitCache.resize(this->g_compressor.getLastCompressionSize() + offset);
    std::memcpy(this->_g_transmitCache.data() + offset, this->g_compressor.getBuffer().data(),
                this->g_compressor.getLastCompressionSize());
    this->_g_transmitCacheValid = true;
    return true;
}

void PacketLZ4::onReceive(std::span<uint8_t const> const& data)
{
    if (data.data() == nullptr || data.empty())
    {
        this->invalidate();
        return;
    }

    this->g_compressor.setMaxUncompressedSize(gMaxUncompressedSize);
    auto const err = this->g_compressor.uncompress(data);
    if (err)
    {
        this->invalidate();
        return;
    }

    this->append(this->g_compressor.getBuffer().data(), this->g_compressor.getBuffer().size());
}

//PacketLZ4HC

std::atomic_uint32_t PacketLZ4HC::gMaxUncompressedSize{FGE_NET_LZ4HC_DEFAULT_PACKET_MAX_SIZE};
std::atomic_int PacketLZ4HC::gCompressionLevel{FGE_COMPRESSOR_LZ4HC_DEFAULT_COMPRESSION_LEVEL};

PacketLZ4HC::PacketLZ4HC(PacketLZ4HC&& pck) noexcept :
        Packet(std::move(static_cast<Packet&&>(pck))),
        g_compressor(std::move(pck.g_compressor))
{}
PacketLZ4HC::PacketLZ4HC(Packet&& pck) noexcept :
        Packet(std::move(pck))
{}
PacketLZ4HC::PacketLZ4HC(PacketLZ4HC const& pck) :
        Packet(pck),
        g_compressor(pck.g_compressor)
{}
PacketLZ4HC::PacketLZ4HC(Packet const& pck) :
        Packet(pck)
{}

CompressorLZ4HC const& PacketLZ4HC::getCompressor() const
{
    return this->g_compressor;
}

bool PacketLZ4HC::onSend(std::size_t offset)
{
    this->g_compressor.setCompressionLevel(gCompressionLevel);
    auto const err = this->g_compressor.compress({this->getData(), this->getDataSize()});
    if (err)
    {
        this->_g_transmitCacheValid = false;
        return false;
    }

    this->_g_transmitCache.resize(this->g_compressor.getLastCompressionSize() + offset);
    std::memcpy(this->_g_transmitCache.data() + offset, this->g_compressor.getBuffer().data(),
                this->g_compressor.getLastCompressionSize());
    this->_g_transmitCacheValid = true;
    return true;
}

void PacketLZ4HC::onReceive(std::span<uint8_t const> const& data)
{
    if (data.data() == nullptr || data.empty())
    {
        this->invalidate();
        return;
    }

    this->g_compressor.setMaxUncompressedSize(gMaxUncompressedSize);
    auto const err = this->g_compressor.uncompress(data);
    if (err)
    {
        this->invalidate();
        return;
    }

    this->append(this->g_compressor.getBuffer().data(), this->g_compressor.getBuffer().size());
}

} // namespace fge::net

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

#include "FastEngine/network/C_protocol.hpp"
#include "FastEngine/C_compressor.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/network/C_server.hpp"
#include "private/fge_debug.hpp"

namespace fge::net
{

//ProtocolPacket

bool ProtocolPacket::compress(Compressor& compressor)
{
    if (!this->haveCorrectHeaderSize())
    {
        return false;
    }

    if (this->checkFlags(FGE_NET_HEADER_COMPRESSED_FLAG))
    {
        return true; //Already compressed
    }

    auto const payloadSize = this->getDataSize() - HeaderSize;
    if (payloadSize == 0)
    {
        return true; //Nothing to compress
    }

    if (compressor.compress({this->getData() + HeaderSize, payloadSize}))
    {
        return false; //Compression failed
    }

    this->shrink(payloadSize);
    this->append(compressor.getBuffer().data(), compressor.getBuffer().size());

    this->addFlags(FGE_NET_HEADER_COMPRESSED_FLAG);

    return true;
}
bool ProtocolPacket::decompress(Compressor& compressor)
{
    if (!this->haveCorrectHeaderSize())
    {
        return false;
    }

    if (!this->checkFlags(FGE_NET_HEADER_COMPRESSED_FLAG))
    {
        return true; //Already decompressed
    }

    auto const payloadSize = this->getDataSize() - HeaderSize;
    if (payloadSize == 0)
    {
        return false; //Abnormal size
    }

    if (compressor.uncompress({this->getData() + HeaderSize, payloadSize}))
    {
        return false; //Decompression failed
    }

    this->shrink(payloadSize);
    this->append(compressor.getBuffer().data(), compressor.getBuffer().size());

    this->removeFlags(FGE_NET_HEADER_COMPRESSED_FLAG);

    return true;
}

void ProtocolPacket::applyOptions(Client const& client)
{
    for (auto const& option: this->g_options)
    {
        if (option._option == Options::UPDATE_TIMESTAMP)
        {
            Timestamp updatedTimestamp = Client::getTimestamp_ms();
            this->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
        }
        else if (option._option == Options::UPDATE_FULL_TIMESTAMP)
        {
            FullTimestamp updatedTimestamp = Client::getFullTimestamp_ms();
            this->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
        }
        else if (option._option == Options::UPDATE_CORRECTION_LATENCY)
        {
            Latency_ms correctorLatency = client.getCorrectorLatency().value_or(FGE_NET_BAD_LATENCY);
            this->pack(option._argument, &correctorLatency, sizeof(correctorLatency));
        }
    }
}
void ProtocolPacket::applyOptions()
{
    for (auto const& option: this->g_options)
    {
        if (option._option == Options::UPDATE_TIMESTAMP)
        {
            Timestamp updatedTimestamp = Client::getTimestamp_ms();
            this->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
        }
        else if (option._option == Options::UPDATE_FULL_TIMESTAMP)
        {
            FullTimestamp updatedTimestamp = Client::getFullTimestamp_ms();
            this->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
        }
        else if (option._option == Options::UPDATE_CORRECTION_LATENCY)
        {
            throw Exception("Cannot apply correction latency without a client");
        }
    }
}

std::vector<std::unique_ptr<ProtocolPacket>> ProtocolPacket::fragment(uint16_t mtu) const
{
    if (this->getDataSize() < mtu)
    {
        std::vector<std::unique_ptr<ProtocolPacket>> fragments(1);
        fragments.back() = std::make_unique<ProtocolPacket>(*this);
        return fragments;
    }

    //We have to fragment the packet
    auto const packetSize = this->getDataSize();
    auto const maxFragmentSize = mtu - HeaderSize - sizeof(InternalFragmentedPacketData);
    auto const fragmentCount = packetSize / maxFragmentSize + (packetSize % maxFragmentSize > 0 ? 1 : 0);

    auto const fragmentRealm = this->retrieveCounter().value();

    InternalFragmentedPacketData fragmentData{};
    fragmentData._fragmentTotal = fragmentCount;

    std::vector<std::unique_ptr<ProtocolPacket>> fragments(fragmentCount);
    for (std::size_t i = 0; i < fragmentCount; ++i)
    {
        auto fragmentedPacket = std::make_unique<ProtocolPacket>(NET_INTERNAL_ID_FRAGMENTED_PACKET, fragmentRealm, i);

        fragmentedPacket->pack(&fragmentData, sizeof(fragmentData));
        fragmentedPacket->append(this->getData() + i * maxFragmentSize,
                                 i == fragmentCount - 1 ? packetSize - i * maxFragmentSize : maxFragmentSize);

        fragmentedPacket->doNotFragment().doNotReorder();
        if (this->isMarkedForEncryption())
        {
            fragmentedPacket->markForEncryption();
        }

        fragments[i] = std::move(fragmentedPacket);
    }
    return fragments;
}

//PacketDefragmentation

void PacketDefragmentation::clear()
{
    this->g_data.clear();
}

PacketDefragmentation::Result PacketDefragmentation::process(ReceivedPacketPtr&& packet)
{
    auto const id = packet->retrieveRealm().value();
    auto const counter = packet->retrieveCounter().value();

    for (auto itData = this->g_data.begin(); itData != this->g_data.end(); ++itData)
    {
        auto& data = *itData;
        if (data._id != id)
        {
            continue;
        }

        if (counter >= data._fragments.size())
        {
            //Invalid fragment counter, overflows the total
            this->g_data.erase(itData);
            return {Results::DISCARDED, id};
        }

        auto& fragment = data._fragments[counter];
        if (fragment != nullptr)
        {
            //Already received, some duplicate, discarded
            this->g_data.erase(itData);
            return {Results::DISCARDED, id};
        }

        //Insert the new fragment
        fragment = std::move(packet);
        ++data._count;

        //Check if we have all the fragments
        if (data._fragments.size() == data._count)
        {
            return {Results::RETRIEVABLE, id};
        }
        return {Results::WAITING, id};
    }

    //New fragment
    InternalFragmentedPacketData fragmentedData{};
    packet->packet().unpack(ProtocolPacket::HeaderSize, &fragmentedData, sizeof(fragmentedData));

    this->g_data.emplace_back(id, fragmentedData._fragmentTotal)._fragments[counter] = std::move(packet);
    ///TODO: remove the oldest data if the cache is full
    return {Results::WAITING, id};
}
ReceivedPacketPtr PacketDefragmentation::retrieve(ProtocolPacket::RealmType id, Identity const& client)
{
    for (auto itData = this->g_data.begin(); itData != this->g_data.end(); ++itData)
    {
        if (itData->_id == id)
        {
            Packet unfragmentedPacket;

            for (auto const& fragment: itData->_fragments)
            {
                unfragmentedPacket.append(fragment->packet().getData() + ProtocolPacket::HeaderSize +
                                                  sizeof(InternalFragmentedPacketData),
                                          fragment->packet().getDataSize() - ProtocolPacket::HeaderSize -
                                                  sizeof(InternalFragmentedPacketData));
            }

            unfragmentedPacket.skip(ProtocolPacket::HeaderSize);
            auto unfragmentedPacketPtr = std::make_unique<ProtocolPacket>(std::move(unfragmentedPacket), client);
            unfragmentedPacketPtr->setTimestamp(Client::getTimestamp_ms());

            this->g_data.erase(itData);
            return unfragmentedPacketPtr;
        }
    }
    return nullptr;
}

//PacketReorderer

void PacketReorderer::clear()
{
    this->g_forceRetrieve = false;
    decltype(this->g_cache)().swap(this->g_cache);
}

void PacketReorderer::push(ReceivedPacketPtr&& packet)
{
    this->g_cache.emplace(std::move(packet));
    if (this->g_cache.size() >= this->g_cacheSize)
    {
        //We can't wait anymore for the correct packet to arrive
        //as the cache is full, the waiting packet is considered lost
        //and we can move on. (if he is still missing)
        this->g_forceRetrieve = true;
        FGE_DEBUG_PRINT("PacketReorderer: Cache is full, we can't wait anymore for the correct packet to arrive");
    }
}
PacketReorderer::Stats PacketReorderer::checkStat(ReceivedPacketPtr const& packet,
                                                  ProtocolPacket::CounterType currentCounter,
                                                  ProtocolPacket::RealmType currentRealm)
{
    auto const lastCounter = packet->retrieveLastCounter().value();
    auto counter = packet->retrieveCounter().value();

    counter = (counter == lastCounter) ? counter : (lastCounter + 1);

    auto const realm = packet->retrieveRealm().value();

    if (realm < currentRealm && currentRealm + 1 != 0)
    {
        return Stats::OLD_REALM;
    }

    if (currentRealm != realm && counter != 0)
    { //Different realm, we can switch to the new realm only if the counter is 0 (first packet of the new realm)
        return Stats::WAITING_NEXT_REALM;
    }

    if (counter == currentCounter + 1)
    { //Same realm, we can switch to the next counter
        return Stats::RETRIEVABLE;
    }

    if (counter < currentCounter)
    { //We are missing a packet
        return Stats::OLD_COUNTER;
    }

    //We can't switch to the next counter, we wait for the correct packet to arrive
    return Stats::WAITING_NEXT_COUNTER;
}
bool PacketReorderer::isForced() const
{
    return this->g_forceRetrieve;
}
std::optional<PacketReorderer::Stats> PacketReorderer::checkStat(ProtocolPacket::CounterType currentCounter,
                                                                 ProtocolPacket::RealmType currentRealm) const
{
    if (this->g_cache.empty())
    {
        return std::nullopt;
    }

    return this->g_cache.top().checkStat(currentCounter, currentRealm);
}
ReceivedPacketPtr PacketReorderer::pop()
{
    if (this->g_cache.empty())
    {
        return nullptr;
    }

    auto packet = std::move(const_cast<ReceivedPacketPtr&>(this->g_cache.top()._packet));
    this->g_cache.pop();

    if (this->g_cache.empty())
    {
        this->g_forceRetrieve = false;
    }

    return packet;
}

bool PacketReorderer::isEmpty() const
{
    return this->g_cache.empty();
}

void PacketReorderer::setMaximumSize(std::size_t size)
{
    this->g_cacheSize = size;
    this->clear();
}
std::size_t PacketReorderer::getMaximumSize() const
{
    return this->g_cacheSize;
}

PacketReorderer::Data::Data(ReceivedPacketPtr&& packet) :
        _packet(std::move(packet)),
        _counter(_packet->retrieveCounter().value()),
        _lastCounter(_packet->retrieveLastCounter().value()),
        _realm(_packet->retrieveRealm().value())
{}
PacketReorderer::Data::Data(Data&& r) noexcept :
        _packet(std::move(r._packet)),
        _counter(r._counter),
        _lastCounter(r._lastCounter),
        _realm(r._realm)
{}
PacketReorderer::Data::~Data() = default;

PacketReorderer::Data& PacketReorderer::Data::operator=(Data&& r) noexcept
{
    this->_packet = std::move(r._packet);
    this->_counter = r._counter;
    this->_lastCounter = r._lastCounter;
    this->_realm = r._realm;
    return *this;
}

PacketReorderer::Stats PacketReorderer::Data::checkStat(ProtocolPacket::CounterType currentCounter,
                                                        ProtocolPacket::RealmType currentRealm) const
{
    FGE_DEBUG_PRINT("PacketReorderer: currentCounter {}, currentRealm {}, packetCounter {}->{}, packetRealm {}",
                    currentCounter, currentRealm, this->_lastCounter, this->_counter, this->_realm);

    if (this->_realm < currentRealm && currentRealm + 1 != 0)
    {
        FGE_DEBUG_PRINT("\tPacketReorderer: Old realm");
        return Stats::OLD_REALM;
    }

    if (currentRealm != this->_realm && this->_counter != 0)
    { //Different realm, we can switch to the new realm only if the counter is 0 (first packet of the new realm)
        FGE_DEBUG_PRINT("\tPacketReorderer: Different realm, we can switch to the new realm only if the counter is 0 "
                        "(first packet of the new realm)");
        return Stats::WAITING_NEXT_REALM;
    }

    if (this->_lastCounter == currentCounter)
    { //Same realm, we can switch to the next counter
        FGE_DEBUG_PRINT("\tPacketReorderer: Same realm, correct counter, retrievable");
        return Stats::RETRIEVABLE;
    }

    if (this->_lastCounter < currentCounter)
    { //We are missing a packet
        FGE_DEBUG_PRINT("\tPacketReorderer: We are missing a packet");
        return Stats::OLD_COUNTER;
    }

    FGE_DEBUG_PRINT("\tPacketReorderer: We can't switch to the next counter, we wait for the correct packet to arrive");
    //We can't switch to the next counter, we wait for the correct packet to arrive
    return Stats::WAITING_NEXT_COUNTER;
}

//PacketCache

void PacketCache::clear()
{
    this->g_cache.clear();
    this->g_cache.resize(FGE_NET_PACKET_CACHE_MAX);
    this->g_start = 0;
    this->g_end = 0;
}

bool PacketCache::isEmpty() const
{
    return this->g_start == this->g_end;
}
bool PacketCache::isEnabled() const
{
    return this->g_enable;
}
void PacketCache::enable(bool enable)
{
    this->g_enable = enable;
}

void PacketCache::push(TransmitPacketPtr const& packet)
{
    if (!this->g_enable)
    {
        return;
    }

    auto const next = (this->g_end + 1) % FGE_NET_PACKET_CACHE_MAX;

    if (next == this->g_start)
    {
        //Cache is full, we need to replace the oldest packet
        FGE_DEBUG_PRINT("PacketCache: Cache is full, replacing the oldest packet {}",
                        this->g_cache[this->g_start]._label._counter);
        this->g_cache[this->g_start] = std::make_unique<ProtocolPacket>(*packet);
        this->g_cache[this->g_start]._packet->markAsCached();
        this->g_start = (this->g_start + 1) % FGE_NET_PACKET_CACHE_MAX;
        this->g_end = next;
        return;
    }

    this->g_cache[next] = std::make_unique<ProtocolPacket>(*packet);
    this->g_cache[next]._packet->markAsCached();
    this->g_end = next;
}

void PacketCache::acknowledgeReception(std::span<Label> labels)
{
    if (this->isEmpty())
    {
        return;
    }

    for (auto const& label: labels)
    {
        auto index = this->g_start;

        do {
            auto& data = this->g_cache[index];
            index = (index + 1) % FGE_NET_PACKET_CACHE_MAX;

            if (!data._packet)
            { //Already acknowledged
                continue;
            }

            if (data._label == label)
            {                           //We found the packet
                data._packet = nullptr; //We acknowledge the packet by setting it to nullptr
                break;
            }
        } while (index != this->g_end);
    }
}

bool PacketCache::check(std::chrono::steady_clock::time_point const& timePoint, std::chrono::milliseconds clientDelay)
{
    if (this->isEmpty())
    {
        return false;
    }

    if (clientDelay.count() < FGE_NET_PACKET_CACHE_MIN_LATENCY_MS)
    {
        clientDelay = std::chrono::milliseconds(FGE_NET_PACKET_CACHE_MIN_LATENCY_MS);
    }

    auto index = this->g_start;
    do {
        auto& data = this->g_cache[index];
        index = (index + 1) % FGE_NET_PACKET_CACHE_MAX;

        if (!data._packet)
        { //Packet is acknowledged, let's remove it
            this->g_start = index;
            continue;
        }

        if (timePoint - data._time >= clientDelay)
        { //Packet has expired, advise the code to resend it
            return true;
        }
        return false;
    } while (index != this->g_end);

    //If we are here, it means that every packet that was in the cache has been acknowledged, so it's good
    return false;
}

TransmitPacketPtr PacketCache::pop()
{
    if (this->isEmpty())
    {
        return nullptr;
    }

    //check() must be called before pop(), so we can remove index verification
    //else it will be undefined behavior

    auto const start = this->g_start;
    this->g_start = (this->g_start + 1) % FGE_NET_PACKET_CACHE_MAX;
    return std::move(this->g_cache[start]._packet);
}

PacketCache::Data::Data(TransmitPacketPtr&& packet) :
        _packet(std::move(packet)),
        _label(this->_packet->retrieveCounter().value(), this->_packet->retrieveRealm().value()),
        _time(std::chrono::steady_clock::now())
{}

PacketCache::Data& PacketCache::Data::operator=(TransmitPacketPtr&& packet)
{
    this->_packet = std::move(packet);
    this->_label._counter = this->_packet->retrieveCounter().value();
    this->_label._realm = this->_packet->retrieveRealm().value();
    this->_time = std::chrono::steady_clock::now();
    return *this;
}

} // namespace fge::net

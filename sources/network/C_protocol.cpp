/*
 * Copyright 2026 Guillaume Guillet
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

#include "FastEngine/C_alloca.hpp"
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
#ifdef FGE_ENABLE_PACKET_DEBUG_VERBOSE
    FGE_DEBUG_PRINT("Defragmentation: Processing fragment for id {}, counter {}.", id, counter);
#endif

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

bool PacketReorderer::process(Client& client, NetFluxUdp& flux, bool ignoreRealm)
{
    auto statOpt = this->checkStat(client, ignoreRealm);
    if (!statOpt)
    {
        return false; //Empty
    }

    if (statOpt.value() == Stats::OLD_COUNTER || statOpt.value() == Stats::OLD_REALM)
    {
#ifdef FGE_DEF_DEBUG
        auto packet = this->pop();
        auto const packetRealm = packet->retrieveRealm().value();
        auto const packetCounter = packet->retrieveCounter().value();
        auto const packetReorderedCounter = packet->retrieveReorderedCounter().value();
        FGE_DEBUG_PRINT("Reorderer: Packet is old, discarding it. Packet realm: {}, counter: {}, reorderedCounter: {}.",
                        packetRealm, packetCounter, packetReorderedCounter);
#else
        (void) this->pop();
#endif
        client.advanceLostPacketCount();
        return false; //We can't process old packets
    }

    if (!this->g_forceRetrieve && statOpt.value() != Stats::RETRIEVABLE)
    {
        return false; //We can't process the packet now
    }

    ProtocolPacket::CounterType peerCounter{0};
    ProtocolPacket::CounterType peerReorderedCounter{0};
    ProtocolPacket::RealmType currentRealm{0};

    auto const reordererMaxSize = this->getMaximumSize();
    std::size_t containerInvertedSize = 0;
    auto* containerInverted = FGE_ALLOCA_T(ReceivedPacketPtr, reordererMaxSize);
    FGE_PLACE_CONSTRUCT(ReceivedPacketPtr, reordererMaxSize, containerInverted);

    do {
        auto packet = this->pop();
        peerCounter = packet->retrieveCounter().value();
        peerReorderedCounter = packet->retrieveReorderedCounter().value();
        currentRealm = packet->retrieveRealm().value();

        //Add it to the container (we are going to push in front of the queue, so we need to invert the order)
        containerInverted[containerInvertedSize++] = std::move(packet);

        statOpt = this->checkStat(peerCounter, peerReorderedCounter, currentRealm, ignoreRealm);
    } while (statOpt.has_value() && (statOpt.value() == Stats::RETRIEVABLE || this->g_forceRetrieve));

    //Now we push the packets (until the last one) in the correct order in the flux queue
    for (std::size_t i = containerInvertedSize; i != 0; --i)
    {
        flux.forcePushPacketFront(std::move(containerInverted[i - 1]));
    }

    FGE_PLACE_DESTRUCT(ReceivedPacketPtr, reordererMaxSize, containerInverted);

    this->g_forceRetrieve = false;
    return true;
}

void PacketReorderer::push(ReceivedPacketPtr&& packet)
{
    if (packet->checkFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG))
    {
        FGE_DEBUG_PRINT("Reorderer: Packet has DO_NOT_REORDER flag, cannot be pushed to reorderer");
        return;
    }

    packet->markAsLocallyReordered();
    this->g_cache.emplace(std::move(packet));
    if (this->g_cache.size() >= this->g_cacheSize)
    {
        //We can't wait anymore for the correct packet to arrive
        //as the cache is full, the waiting packet is considered lost
        //and we can move on. (if he is still missing)
        this->g_forceRetrieve = true;
        FGE_DEBUG_PRINT("Reorderer: Cache is full, we can't wait anymore for the correct packet to arrive");
    }
}

PacketReorderer::Stats
PacketReorderer::checkStat(ReceivedPacketPtr const& packet, Client const& client, bool ignoreRealm)
{
    return PacketReorderer::checkStat(packet, client.getPacketCounter(Client::Targets::PEER),
                                      client.getReorderedPacketCounter(Client::Targets::PEER), client.getCurrentRealm(),
                                      ignoreRealm);
}
PacketReorderer::Stats PacketReorderer::checkStat(ReceivedPacketPtr const& packet,
                                                  ProtocolPacket::CounterType peerCounter,
                                                  ProtocolPacket::CounterType peerReorderedCounter,
                                                  ProtocolPacket::RealmType peerRealm,
                                                  bool ignoreRealm)
{
    auto const packetCounter = packet->retrieveCounter().value();
    auto const packetReorderedCounter = packet->retrieveReorderedCounter().value();
    auto const packetRealm = packet->retrieveRealm().value();

    auto const currentRealm = ignoreRealm ? packetRealm : peerRealm;

    auto const doNotReorderFlag = packet->checkFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG);

    if (packetRealm < currentRealm && currentRealm + 1 != 0)
    {
        return Stats::OLD_REALM;
    }

    if (currentRealm != packetRealm && packetCounter != 0)
    { //Different realm, we can switch to the new realm only if the counter is 0 (first packet of the new realm)
        return Stats::WAITING_NEXT_REALM;
    }

    if (doNotReorderFlag)
    {
        if (packetCounter < peerCounter)
        { //We are missing a packet
            //TODO: we do not handle case where there is a overflow
            return Stats::OLD_COUNTER;
        }
        return Stats::RETRIEVABLE;
    }

    if (packetReorderedCounter == peerReorderedCounter + 1)
    { //Same realm, we can switch to the next counter
        return Stats::RETRIEVABLE;
    }

    if (packetReorderedCounter < peerReorderedCounter)
    { //We are missing a packet
        return Stats::OLD_COUNTER;
    }

    //We can't switch to the next counter, we wait for the correct packet to arrive
    return Stats::WAITING_NEXT_COUNTER;
}
std::optional<PacketReorderer::Stats> PacketReorderer::checkStat(Client const& client, bool ignoreRealm) const
{
    return this->checkStat(client.getPacketCounter(Client::Targets::PEER),
                           client.getReorderedPacketCounter(Client::Targets::PEER), client.getCurrentRealm(),
                           ignoreRealm);
}
std::optional<PacketReorderer::Stats> PacketReorderer::checkStat(ProtocolPacket::CounterType peerCounter,
                                                                 ProtocolPacket::CounterType peerReorderedCounter,
                                                                 ProtocolPacket::RealmType peerRealm,
                                                                 bool ignoreRealm) const
{
    if (this->g_cache.empty())
    {
        return std::nullopt;
    }

    return this->g_cache.top().checkStat(peerCounter, peerReorderedCounter, peerRealm, ignoreRealm);
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
bool PacketReorderer::isForced() const
{
    return this->g_forceRetrieve;
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
        _reorderedCounter(_packet->retrieveReorderedCounter().value()),
        _realm(_packet->retrieveRealm().value())
{}
PacketReorderer::Data::Data(Data&& r) noexcept :
        _packet(std::move(r._packet)),
        _counter(r._counter),
        _reorderedCounter(r._reorderedCounter),
        _realm(r._realm)
{}
PacketReorderer::Data::~Data() = default;

PacketReorderer::Data& PacketReorderer::Data::operator=(Data&& r) noexcept
{
    this->_packet = std::move(r._packet);
    this->_counter = r._counter;
    this->_reorderedCounter = r._reorderedCounter;
    this->_realm = r._realm;
    return *this;
}

PacketReorderer::Stats PacketReorderer::Data::checkStat(ProtocolPacket::CounterType peerCounter,
                                                        ProtocolPacket::CounterType peerReorderedCounter,
                                                        ProtocolPacket::RealmType peerRealm,
                                                        bool ignoreRealm) const
{
#ifdef FGE_ENABLE_PACKET_DEBUG_VERBOSE
    FGE_DEBUG_PRINT("Reorderer::Data: Peer counters[{}/{}], realm {}. Packet counters[{}/{}], realm {}", peerCounter,
                    peerReorderedCounter, peerRealm, this->_counter, this->_reorderedCounter, this->_realm);
#endif

#ifndef FGE_ENABLE_PACKET_DEBUG_VERBOSE
    return PacketReorderer::checkStat(this->_packet, peerCounter, peerReorderedCounter, peerRealm, ignoreRealm);
#else
    auto const stat =
            PacketReorderer::checkStat(this->_packet, peerCounter, peerReorderedCounter, peerRealm, ignoreRealm);

    switch (stat)
    {
    case Stats::OLD_REALM:
        FGE_DEBUG_PRINT("\tReorderer::Data: Old realm");
        break;
    case Stats::WAITING_NEXT_REALM:
        FGE_DEBUG_PRINT("\tReorderer::Data: Different realm, we can switch to the new realm only if the counter is 0 "
                        "(first packet of the new realm)");
        break;
    case Stats::OLD_COUNTER:
        FGE_DEBUG_PRINT("\tReorderer::Data: We are missing a packet");
        break;
    case Stats::WAITING_NEXT_COUNTER:
        FGE_DEBUG_PRINT(
                "\tReorderer::Data: We can't switch to the next counter, we wait for the correct packet to arrive");
        break;
    case Stats::RETRIEVABLE:
        FGE_DEBUG_PRINT("\tReorderer::Data: Same realm, correct counter, retrievable");
        break;
    default:
        FGE_DEBUG_PRINT("\tReorderer::Data: Result is UNKNOWN");
        break;
    }
    return stat;
#endif
}

//PacketCache

PacketCache::PacketCache()
{
    this->g_cache.reserve(FGE_NET_PACKET_CACHE_MAX);
}
PacketCache::PacketCache(PacketCache&& r) noexcept :
        PacketCache()
{
    std::scoped_lock const lockThis(this->g_mutex);
    std::scoped_lock const lockR(r.g_mutex);

    this->g_cache = std::move(r.g_cache);
    this->g_alarm = r.g_enable;
    this->g_enable = r.g_enable;

    r.g_cache.clear();
    r.g_cache.reserve(FGE_NET_PACKET_CACHE_MAX);
    r.g_alarm = false;
}

PacketCache& PacketCache::operator=(PacketCache&& r) noexcept
{
    if (this != &r)
    {
        std::scoped_lock const lockThis(this->g_mutex);
        std::scoped_lock const lockR(r.g_mutex);

        this->g_cache = std::move(r.g_cache);
        this->g_alarm = r.g_enable;
        this->g_enable = r.g_enable;

        r.g_cache.clear();
        r.g_cache.reserve(FGE_NET_PACKET_CACHE_MAX);
        r.g_alarm = false;
    }
    return *this;
}

void PacketCache::clear()
{
    std::scoped_lock const lock(this->g_mutex);
    this->g_cache.clear();
    this->g_cache.reserve(FGE_NET_PACKET_CACHE_MAX);
    this->g_alarm = false;
}

bool PacketCache::isEmpty() const
{
    std::scoped_lock const lock(this->g_mutex);
    return this->g_cache.empty();
}
bool PacketCache::isEnabled() const
{
    std::scoped_lock const lock(this->g_mutex);
    return this->g_enable;
}
bool PacketCache::isAlarmed() const
{
    std::scoped_lock const lock(this->g_mutex);
    return this->g_alarm;
}
void PacketCache::enable(bool enable)
{
    std::scoped_lock const lock(this->g_mutex);
    this->g_enable = enable;
}

void PacketCache::push(TransmitPacketPtr const& packet)
{
    std::scoped_lock const lock(this->g_mutex);
    if (!this->g_enable)
    {
        return;
    }

    this->g_cache.emplace_back(std::make_unique<ProtocolPacket>(*packet))._packet->markAsCached();
    if (this->g_cache.size() >= FGE_NET_PACKET_CACHE_MAX)
    {
        FGE_DEBUG_PRINT("PacketCache: Cache reached maximum size");
        this->g_alarm = true;
    }
}

void PacketCache::acknowledgeReception(std::span<Label> labels)
{
    std::scoped_lock const lock(this->g_mutex);

    if (this->g_cache.empty())
    {
        this->g_alarm = false;
        return;
    }

    for (auto const& label: labels)
    {
        for (auto it = this->g_cache.begin(); it != this->g_cache.end(); ++it)
        {
            if (it->_label == label)
            {
                this->g_cache.erase(it);
                break;
            }
        }
    }

    if (this->g_cache.empty())
    {
        this->g_alarm = false;
    }
}

bool PacketCache::process(std::chrono::steady_clock::time_point const& timePoint, Client& client)
{
    std::scoped_lock const lock(this->g_mutex);

    if (this->g_cache.empty())
    {
        client.allowMorePendingPackets(!this->g_alarm);
        return false;
    }

    auto clientDelay =
            std::chrono::milliseconds(static_cast<std::chrono::milliseconds::rep>(
                    static_cast<float>(client.getPacketReturnRate().count()) * FGE_NET_PACKET_CACHE_DELAY_FACTOR)) +
            std::chrono::milliseconds(client._latencyPlanner.getRoundTripTime().value_or(1));

    if (clientDelay.count() < FGE_NET_PACKET_CACHE_MIN_LATENCY_MS)
    {
        clientDelay = std::chrono::milliseconds(FGE_NET_PACKET_CACHE_MIN_LATENCY_MS);
    }

    bool needSetAlarm = false;
    for (auto it = this->g_cache.begin(); it != this->g_cache.end();)
    {
        if (timePoint - it->_time >= clientDelay)
        { //Packet has expired, advise the code to resend it
            if (it->_tryCount++ == 3)
            { //We loose this packet
#ifdef FGE_DEF_DEBUG
                auto const counter = it->_packet->retrieveCounter().value();
                auto const reorderedCounter = it->_packet->retrieveReorderedCounter().value();
                FGE_DEBUG_PRINT("PacketCache: Packet [{}/{}] lost after all tries", counter, reorderedCounter);
#endif
                it = this->g_cache.erase(it);
                client.advanceLostPacketCount();
                continue;
            }

            it->_time = timePoint;

#ifdef FGE_DEF_DEBUG
            auto const counter = it->_packet->retrieveCounter().value();
            auto const reorderedCounter = it->_packet->retrieveReorderedCounter().value();
            FGE_DEBUG_PRINT("re-transmit packet [{}/{}] try {}, as client didn't acknowledge it", counter,
                            reorderedCounter, it->_tryCount);
#endif

            client.pushForcedFrontPacket(std::make_unique<ProtocolPacket>(*it->_packet));

            needSetAlarm = true;
        }
        else if (it->_tryCount > 0)
        {
            //We are in a current retransmission, set the alarm again
            needSetAlarm = true;
        }

        ++it;
    }

    client.allowMorePendingPackets(!needSetAlarm);
    this->g_alarm = needSetAlarm;

    return needSetAlarm;
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

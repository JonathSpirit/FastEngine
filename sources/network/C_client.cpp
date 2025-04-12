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

#include "FastEngine/network/C_client.hpp"
#include "FastEngine/C_random.hpp"
#include "FastEngine/network/C_server.hpp"
#include "private/fge_crypt.hpp"
#include <limits>

namespace fge::net
{

//ClientStatus

ClientStatus::ClientStatus(std::string_view status, NetworkStatus networkStatus) :
        g_status(status),
        g_networkStatus(networkStatus)
{}

bool ClientStatus::isInEncryptedState() const
{
    return this->g_networkStatus == NetworkStatus::AUTHENTICATED || this->g_networkStatus == NetworkStatus::CONNECTED;
}

std::string const& ClientStatus::getStatus() const
{
    return this->g_status;
}
ClientStatus::NetworkStatus ClientStatus::getNetworkStatus() const
{
    return this->g_networkStatus;
}
std::chrono::milliseconds ClientStatus::getTimeout() const
{
    return this->g_timeout;
}
std::chrono::milliseconds ClientStatus::getRemainingTimeout() const
{
    auto diff = std::chrono::steady_clock::now() - this->g_currentTimeout;
    return diff >= this->g_timeout ? std::chrono::milliseconds(0)
                                   : std::chrono::duration_cast<std::chrono::milliseconds>(this->g_timeout - diff);
}

void ClientStatus::set(std::string_view status, NetworkStatus networkStatus)
{
    this->g_status = status;
    this->g_networkStatus = networkStatus;
}
void ClientStatus::setStatus(std::string_view status)
{
    this->g_status = status;
}
void ClientStatus::setNetworkStatus(NetworkStatus networkStatus)
{
    this->g_networkStatus = networkStatus;
}

void ClientStatus::setTimeout(std::chrono::milliseconds timeout)
{
    this->g_timeout = timeout;
    this->g_currentTimeout = std::chrono::steady_clock::now();
}
void ClientStatus::resetTimeout()
{
    this->g_currentTimeout = std::chrono::steady_clock::now();
}
bool ClientStatus::isTimeout() const
{
    if (std::chrono::steady_clock::now() - this->g_currentTimeout >= this->g_timeout)
    {
        return true;
    }
    return false;
}

//Client

Client::Client() :
        g_correctorTimestamp(std::nullopt),
        g_CTOSLatency_ms(FGE_NET_DEFAULT_LATENCY),
        g_STOCLatency_ms(FGE_NET_DEFAULT_LATENCY),
        g_lastPacketTimePoint(std::chrono::steady_clock::now()),
        g_skey(FGE_NET_BAD_SKEY),
        g_lastRealmChangeTimePoint(std::chrono::steady_clock::now())
{}
Client::~Client()
{
    priv::CryptClientDestroy(*this);
}
Client::Client(Latency_ms CTOSLatency, Latency_ms STOCLatency) :
        g_correctorTimestamp(std::nullopt),
        g_CTOSLatency_ms(CTOSLatency),
        g_STOCLatency_ms(STOCLatency),
        g_lastPacketTimePoint(std::chrono::steady_clock::now()),
        g_skey(FGE_NET_BAD_SKEY),
        g_lastRealmChangeTimePoint(std::chrono::steady_clock::now())
{}

Skey Client::GenerateSkey()
{
    return _random.range<Skey>(1, std::numeric_limits<Skey>::max());
}
void Client::setSkey(Skey key)
{
    this->g_skey = key;
}
Skey Client::getSkey() const
{
    return this->g_skey;
}

void Client::setCTOSLatency_ms(Latency_ms latency)
{
    this->g_CTOSLatency_ms = latency;
}
void Client::setSTOCLatency_ms(Latency_ms latency)
{
    this->g_STOCLatency_ms = latency;
}
Latency_ms Client::getCTOSLatency_ms() const
{
    return this->g_CTOSLatency_ms;
}
Latency_ms Client::getSTOCLatency_ms() const
{
    return this->g_STOCLatency_ms;
}
Latency_ms Client::getPing_ms() const
{
    return this->g_CTOSLatency_ms + this->g_STOCLatency_ms;
}

void Client::setCorrectorTimestamp(Timestamp timestamp)
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_correctorTimestamp = timestamp;
}
std::optional<Timestamp> Client::getCorrectorTimestamp() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_correctorTimestamp;
}
std::optional<Latency_ms> Client::getCorrectorLatency() const
{
    std::scoped_lock const lck(this->g_mutex);
    if (this->g_correctorTimestamp.has_value())
    {
        auto latency = Client::computeLatency_ms(this->g_correctorTimestamp.value(), Client::getTimestamp_ms());
        this->g_correctorTimestamp = std::nullopt;
        return latency;
    }
    return std::nullopt;
}

void Client::resetLastPacketTimePoint()
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_lastPacketTimePoint = std::chrono::steady_clock::now();
}
std::chrono::milliseconds Client::getLastPacketElapsedTime() const
{
    std::scoped_lock const lck(this->g_mutex);

    auto const now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - this->g_lastPacketTimePoint);
}
Latency_ms Client::getLastPacketLatency() const
{
    std::scoped_lock const lck(this->g_mutex);

    auto const now = std::chrono::steady_clock::now();
    auto const t = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->g_lastPacketTimePoint).count();
    return t >= std::numeric_limits<Latency_ms>::max() ? std::numeric_limits<Latency_ms>::max()
                                                       : static_cast<Latency_ms>(t);
}

Timestamp Client::getTimestamp_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
                   .count() %
           FGE_NET_CLIENT_TIMESTAMP_MODULO;
}
Timestamp Client::getTimestamp_ms(FullTimestamp fullTimestamp)
{
    return fullTimestamp % FGE_NET_CLIENT_TIMESTAMP_MODULO;
}
FullTimestamp Client::getFullTimestamp_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count();
}

Latency_ms Client::computeLatency_ms(Timestamp const& sentTimestamp, Timestamp const& receivedTimestamp)
{
    int32_t t = static_cast<int32_t>(receivedTimestamp) - static_cast<int32_t>(sentTimestamp);
    if (t < 0)
    {
        t += FGE_NET_CLIENT_TIMESTAMP_MODULO;
    }
    return static_cast<Latency_ms>(t);
}

void Client::clearPackets()
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_pendingTransmitPackets.clear();
}
void Client::pushPacket(TransmitPacketPtr pck)
{
    std::scoped_lock const lck(this->g_mutex);

#ifdef FGE_DEF_SERVER
    pck->setCounter(this->advanceCurrentPacketCounter());
#else
    pck->setCounter(this->advanceClientPacketCounter());
#endif

    pck->setRealm(this->getCurrentRealm());

    pck->setLastReorderedPacketCounter(this->getLastReorderedPacketCounter());

    if (!pck->checkFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG))
    {
        this->resetLastReorderedPacketCounter();
    }

    if (this->g_status.isInEncryptedState())
    {
        pck->markForEncryption();
    }
    this->g_pendingTransmitPackets.push_back(std::move(pck));
}
void Client::pushForcedFrontPacket(TransmitPacketPtr pck)
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_pendingTransmitPackets.push_front(std::move(pck));
}
TransmitPacketPtr Client::popPacket()
{
    std::scoped_lock const lck(this->g_mutex);

    if (this->g_pendingTransmitPackets.empty())
    {
        return nullptr;
    }
    auto packet = std::move(this->g_pendingTransmitPackets.front());
    this->g_pendingTransmitPackets.pop_front();
    return packet;
}
bool Client::isPendingPacketsEmpty() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_pendingTransmitPackets.empty();
}

ProtocolPacket::RealmType Client::getCurrentRealm() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_currentRealm;
}
std::chrono::milliseconds Client::getLastRealmChangeElapsedTime() const
{
    std::scoped_lock const lck(this->g_mutex);
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                 this->g_lastRealmChangeTimePoint);
}
void Client::setCurrentRealm(ProtocolPacket::RealmType realm)
{
    std::scoped_lock const lck(this->g_mutex);
    if (this->g_currentRealm != realm)
    {
        this->g_currentPacketCounter = 0;
        this->g_lastRealmChangeTimePoint = std::chrono::steady_clock::now();
        this->g_currentRealm = realm;
    }
}
ProtocolPacket::RealmType Client::advanceCurrentRealm()
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_currentPacketCounter = 0;
    this->g_lastRealmChangeTimePoint = std::chrono::steady_clock::now();
    return ++this->g_currentRealm;
}

ProtocolPacket::CounterType Client::getCurrentPacketCounter() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_currentPacketCounter;
}
ProtocolPacket::CounterType Client::advanceCurrentPacketCounter()
{
    std::scoped_lock const lck(this->g_mutex);
    return ++this->g_currentPacketCounter;
}
void Client::setCurrentPacketCounter(ProtocolPacket::CounterType counter)
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_currentPacketCounter = counter;
#ifdef FGE_DEF_SERVER
    this->g_lastReorderedPacketCounter = counter;
#endif
}

ProtocolPacket::CounterType Client::getClientPacketCounter() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_clientPacketCounter;
}
ProtocolPacket::CounterType Client::advanceClientPacketCounter()
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_clientPacketCounter++;
}
void Client::setClientPacketCounter(ProtocolPacket::CounterType countId)
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_clientPacketCounter = countId;
#ifndef FGE_DEF_SERVER
    this->g_lastReorderedPacketCounter = countId;
#endif
}

void Client::resetLastReorderedPacketCounter()
{
    std::scoped_lock const lck(this->g_mutex);
#ifdef FGE_DEF_SERVER
    this->g_lastReorderedPacketCounter = this->g_currentPacketCounter;
#else
    this->g_lastReorderedPacketCounter = this->g_clientPacketCounter;
#endif
}
ProtocolPacket::CounterType Client::getLastReorderedPacketCounter() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_lastReorderedPacketCounter;
}

PacketReorderer& Client::getPacketReorderer()
{
    return this->g_packetReorderer;
}
PacketReorderer const& Client::getPacketReorderer() const
{
    return this->g_packetReorderer;
}

void Client::clearLostPacketCount()
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_lostPacketCount = 0;
}
uint32_t Client::advanceLostPacketCount()
{
    std::scoped_lock const lck(this->g_mutex);
    ++this->g_lostPacketCount;
    if (this->g_lostPacketCount != 0 && this->g_lostPacketCount % this->g_lostPacketThreshold == 0)
    {
        this->_onThresholdLostPacket.call(*this);
    }
    return this->g_lostPacketCount;
}
void Client::setLostPacketThreshold(uint32_t threshold)
{
    if (threshold == 0)
    {
        return;
    }
    std::scoped_lock const lck(this->g_mutex);
    this->g_lostPacketThreshold = threshold;
}
uint32_t Client::getLostPacketThreshold() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_lostPacketThreshold;
}
uint32_t Client::getLostPacketCount() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_lostPacketCount;
}

ClientStatus const& Client::getStatus() const
{
    return this->g_status;
}
ClientStatus& Client::getStatus()
{
    return this->g_status;
}

Client::CryptInfo const& Client::getCryptInfo() const
{
    return this->g_cryptInfo;
}
Client::CryptInfo& Client::getCryptInfo()
{
    return this->g_cryptInfo;
}

uint16_t Client::getMTU() const
{
    return this->g_mtu;
}
void Client::setMTU(uint16_t mtu)
{
    this->g_mtu = mtu;
}

//OneWayLatencyPlanner

void OneWayLatencyPlanner::pack(TransmitPacketPtr& tPacket)
{
    //Append timestamp
    auto const myTimestampPos = tPacket->packet().getDataSize();
    tPacket->packet().append(sizeof(Timestamp));
    tPacket->options().emplace_back(ProtocolPacket::Options::UPDATE_TIMESTAMP, myTimestampPos);

    //Append latency corrector
    auto const myLatencyCorrectorPos = tPacket->packet().getDataSize();
    tPacket->packet().append(sizeof(Latency_ms));

    //Pack computed latency
    auto const myComputedLatency = this->g_latency.value_or(FGE_NET_BAD_LATENCY);
    tPacket->packet().pack(&myComputedLatency, sizeof(myComputedLatency));

    //Append full timestamp
    std::size_t const myFullTimestampPos = tPacket->packet().getDataSize();
    tPacket->packet().append(sizeof(FullTimestamp));
    tPacket->options().emplace_back(ProtocolPacket::Options::UPDATE_FULL_TIMESTAMP, myFullTimestampPos);

    //Pack sync stat
    tPacket->packet().pack(&this->g_syncStat, sizeof(this->g_syncStat));

    //Pack external stored timestamp (if exist)
    if ((this->g_syncStat & HAVE_EXTERNAL_TIMESTAMP) > 0)
    {
        tPacket->packet().pack(&this->g_externalStoredTimestamp, sizeof(this->g_externalStoredTimestamp));
        tPacket->options().emplace_back(ProtocolPacket::Options::UPDATE_CORRECTION_LATENCY, myLatencyCorrectorPos);
        this->g_syncStat &= ~HAVE_EXTERNAL_TIMESTAMP;
    }
}
void OneWayLatencyPlanner::unpack(ProtocolPacket* packet, Client& client)
{
    bool const finishedToSendLastPacket = !client.getCorrectorTimestamp().has_value();

    if (finishedToSendLastPacket)
    {
        packet->unpack(&this->g_externalStoredTimestamp, sizeof(Timestamp));
        this->g_syncStat |= HAVE_EXTERNAL_TIMESTAMP;
    }
    else
    {
        packet->skip(sizeof(Timestamp));
    }

    //Retrieve external latency corrector
    Latency_ms latencyCorrector;
    packet->unpack(&latencyCorrector, sizeof(latencyCorrector));

    //Retrieve the latency computed at the other side
    Latency_ms otherSideLatency;
    packet->unpack(&otherSideLatency, sizeof(otherSideLatency));
    if (otherSideLatency != FGE_NET_BAD_LATENCY)
    {
        this->g_otherSideLatency = otherSideLatency;
    }

    //Retrieve full server timestamp
    FullTimestamp fullTimestamp;
    packet->unpack(&fullTimestamp, sizeof(fullTimestamp));

    //Retrieve external sync stat
    Stats externalSyncStat;
    packet->unpack(&externalSyncStat, sizeof(externalSyncStat));

    //Does he have our timestamp ?
    if ((externalSyncStat & HAVE_EXTERNAL_TIMESTAMP) > 0)
    {
        //Retrieve our timestamp
        Timestamp firstTimestamp;
        packet->unpack(&firstTimestamp, sizeof(firstTimestamp));

        //We didn't finish the last packet yet
        if (!finishedToSendLastPacket)
        {
            return;
        }

        client.setCorrectorTimestamp(packet->getTimeStamp());

        //Do nothing as we don't have a latency corrector
        if (latencyCorrector == FGE_NET_BAD_LATENCY)
        {
            return;
        }

        //Compute RTT
        this->g_roundTripTime = Client::computeLatency_ms(firstTimestamp, packet->getTimeStamp());

        //Compute new latency
        this->g_latency = (this->g_roundTripTime.value() < latencyCorrector)
                                  ? FGE_NET_DEFAULT_LATENCY
                                  : ((this->g_roundTripTime.value() - latencyCorrector) / 2);

        //Compute time offset
        FullTimestampOffset const clockOffset = static_cast<FullTimestampOffset>(Client::getFullTimestamp_ms()) -
                                                static_cast<FullTimestampOffset>(fullTimestamp) +
                                                this->g_latency.value();
        if (this->g_clockOffsetCount == this->g_clockOffsets.max_size())
        {
            //Shift the array by 1
            for (std::size_t i = 0; i < this->g_clockOffsets.max_size() - 1; ++i)
            {
                this->g_clockOffsets[i] = this->g_clockOffsets[i + 1];
            }
            //Insert new offset
            this->g_clockOffsets.back() = clockOffset;
        }
        else
        {
            //Insert new offset
            this->g_clockOffsets[this->g_clockOffsetCount++] = clockOffset;
        }

        //Compute new offset
        FullTimestamp result = 0;
        for (std::size_t i = 0; i < this->g_clockOffsetCount; ++i)
        {
            result += this->g_clockOffsets[i];
        }
        this->g_meanClockOffset = result / this->g_clockOffsetCount;
    }
}

std::optional<FullTimestampOffset> OneWayLatencyPlanner::getClockOffset() const
{
    return this->g_meanClockOffset;
}
std::optional<Latency_ms> OneWayLatencyPlanner::getLatency() const
{
    return this->g_latency;
}
std::optional<Latency_ms> OneWayLatencyPlanner::getOtherSideLatency() const
{
    return this->g_otherSideLatency;
}
std::optional<Latency_ms> OneWayLatencyPlanner::getRoundTripTime() const
{
    return this->g_roundTripTime;
}

} // namespace fge::net

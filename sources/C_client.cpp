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

#include "FastEngine/C_client.hpp"
#include "FastEngine/C_random.hpp"
#include "FastEngine/C_server.hpp"
#include <limits>

namespace fge::net
{

//TransmissionPacket

void TransmissionPacket::applyOptions(fge::net::Client const& client)
{
    for (auto const& option: this->g_options)
    {
        if (option._option == fge::net::TransmissionPacket::Options::UPDATE_TIMESTAMP)
        {
            fge::net::Timestamp updatedTimestamp = fge::net::Client::getTimestamp_ms();
            this->g_packet->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
        }
        else if (option._option == fge::net::TransmissionPacket::Options::UPDATE_FULL_TIMESTAMP)
        {
            fge::net::FullTimestamp updatedTimestamp = fge::net::Client::getFullTimestamp_ms();
            this->g_packet->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
        }
        else if (option._option == fge::net::TransmissionPacket::Options::UPDATE_CORRECTION_LATENCY)
        {
            fge::net::Latency_ms correctorLatency = client.getCorrectorLatency().value_or(FGE_NET_BAD_LATENCY);
            this->g_packet->pack(option._argument, &correctorLatency, sizeof(correctorLatency));
        }
    }
}
void TransmissionPacket::applyOptions()
{
    for (auto const& option: this->g_options)
    {
        if (option._option == fge::net::TransmissionPacket::Options::UPDATE_TIMESTAMP)
        {
            fge::net::Timestamp updatedTimestamp = fge::net::Client::getTimestamp_ms();
            this->g_packet->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
        }
        else if (option._option == fge::net::TransmissionPacket::Options::UPDATE_FULL_TIMESTAMP)
        {
            fge::net::FullTimestamp updatedTimestamp = fge::net::Client::getFullTimestamp_ms();
            this->g_packet->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
        }
        else if (option._option == fge::net::TransmissionPacket::Options::UPDATE_CORRECTION_LATENCY)
        {
            throw fge::Exception("Cannot apply correction latency without a client");
        }
    }
}

//Client

Client::Client() :
        g_correctorTimestamp(std::nullopt),
        g_CTOSLatency_ms(FGE_NET_DEFAULT_LATENCY),
        g_STOCLatency_ms(FGE_NET_DEFAULT_LATENCY),
        g_lastPacketTimePoint(std::chrono::steady_clock::now()),
        g_skey(FGE_NET_BAD_SKEY)
{}
Client::Client(fge::net::Latency_ms CTOSLatency, fge::net::Latency_ms STOCLatency) :
        g_correctorTimestamp(std::nullopt),
        g_CTOSLatency_ms(CTOSLatency),
        g_STOCLatency_ms(STOCLatency),
        g_lastPacketTimePoint(std::chrono::steady_clock::now()),
        g_skey(FGE_NET_BAD_SKEY)
{}

fge::net::Skey Client::GenerateSkey()
{
    return fge::_random.range<fge::net::Skey>(1, std::numeric_limits<fge::net::Skey>::max());
}
void Client::setSkey(fge::net::Skey key)
{
    this->g_skey = key;
}
fge::net::Skey Client::getSkey() const
{
    return this->g_skey;
}

void Client::setCTOSLatency_ms(fge::net::Latency_ms latency)
{
    this->g_CTOSLatency_ms = latency;
}
void Client::setSTOCLatency_ms(fge::net::Latency_ms latency)
{
    this->g_STOCLatency_ms = latency;
}
fge::net::Latency_ms Client::getCTOSLatency_ms() const
{
    return this->g_CTOSLatency_ms;
}
fge::net::Latency_ms Client::getSTOCLatency_ms() const
{
    return this->g_STOCLatency_ms;
}
fge::net::Latency_ms Client::getPing_ms() const
{
    return this->g_CTOSLatency_ms + this->g_STOCLatency_ms;
}

void Client::setCorrectorTimestamp(fge::net::Timestamp timestamp)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    this->g_correctorTimestamp = timestamp;
}
std::optional<fge::net::Timestamp> Client::getCorrectorTimestamp() const
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    return this->g_correctorTimestamp;
}
std::optional<fge::net::Latency_ms> Client::getCorrectorLatency() const
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    if (this->g_correctorTimestamp.has_value())
    {
        auto latency = fge::net::Client::computeLatency_ms(this->g_correctorTimestamp.value(),
                                                           fge::net::Client::getTimestamp_ms());
        this->g_correctorTimestamp = std::nullopt;
        return latency;
    }
    return std::nullopt;
}

void Client::resetLastPacketTimePoint()
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    this->g_lastPacketTimePoint = std::chrono::steady_clock::now();
}
fge::net::Latency_ms Client::getLastPacketElapsedTime()
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);

    std::chrono::steady_clock::time_point timeNow = std::chrono::steady_clock::now();
    uint64_t t = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - this->g_lastPacketTimePoint).count();
    return (t >= std::numeric_limits<fge::net::Latency_ms>::max()) ? std::numeric_limits<fge::net::Latency_ms>::max()
                                                                   : static_cast<fge::net::Latency_ms>(t);
}

fge::net::Timestamp Client::getTimestamp_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
                   .count() %
           FGE_NET_CLIENT_TIMESTAMP_MODULO;
}
fge::net::Timestamp Client::getTimestamp_ms(fge::net::FullTimestamp fullTimestamp)
{
    return fullTimestamp % FGE_NET_CLIENT_TIMESTAMP_MODULO;
}
fge::net::FullTimestamp Client::getFullTimestamp_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count();
}

fge::net::Latency_ms Client::computeLatency_ms(fge::net::Timestamp const& sentTimestamp,
                                               fge::net::Timestamp const& receivedTimestamp)
{
    int32_t t = static_cast<int32_t>(receivedTimestamp) - static_cast<int32_t>(sentTimestamp);
    if (t < 0)
    {
        t += FGE_NET_CLIENT_TIMESTAMP_MODULO;
    }
    return static_cast<fge::net::Latency_ms>(t);
}

void Client::clearPackets()
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);

    for (std::size_t i = 0; i < this->g_pendingTransmitPackets.size(); ++i)
    {
        this->g_pendingTransmitPackets.pop();
    }
}
void Client::pushPacket(fge::net::TransmissionPacketPtr pck)
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    this->g_pendingTransmitPackets.push(std::move(pck));
}
fge::net::TransmissionPacketPtr Client::popPacket()
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);

    if (this->g_pendingTransmitPackets.empty())
    {
        return nullptr;
    }
    auto packet = std::move(this->g_pendingTransmitPackets.front());
    this->g_pendingTransmitPackets.pop();
    return packet;
}
bool Client::isPendingPacketsEmpty()
{
    std::scoped_lock<std::recursive_mutex> const lck(this->g_mutex);
    return this->g_pendingTransmitPackets.empty();
}

//OneWayLatencyPlanner

void OneWayLatencyPlanner::pack(fge::net::TransmissionPacketPtr& tPacket)
{
    //Append timestamp
    auto const myTimestampPos = tPacket->packet().getDataSize();
    tPacket->packet().append(sizeof(fge::net::Timestamp));
    tPacket->options().emplace_back(fge::net::TransmissionPacket::Options::UPDATE_TIMESTAMP, myTimestampPos);

    //Append latency corrector
    auto const myLatencyCorrectorPos = tPacket->packet().getDataSize();
    tPacket->packet().append(sizeof(fge::net::Latency_ms));

    //Pack computed latency
    auto const myComputedLatency = this->g_latency.value_or(FGE_NET_BAD_LATENCY);
    tPacket->packet().pack(&myComputedLatency, sizeof(myComputedLatency));

    //Append full (only server) timestamp
    std::size_t myFullTimestampPos = tPacket->packet().getDataSize();
    tPacket->packet().append(sizeof(fge::net::FullTimestamp));
    tPacket->options().emplace_back(fge::net::TransmissionPacket::Options::UPDATE_FULL_TIMESTAMP, myFullTimestampPos);

    //Pack sync stat
    tPacket->packet().pack(&this->g_syncStat, sizeof(this->g_syncStat));

    //Pack external stored timestamp (if exist)
    if ((this->g_syncStat & Stats::HAVE_EXTERNAL_TIMESTAMP) > 0)
    {
        tPacket->packet().pack(&this->g_externalStoredTimestamp, sizeof(this->g_externalStoredTimestamp));
        tPacket->options().emplace_back(fge::net::TransmissionPacket::Options::UPDATE_CORRECTION_LATENCY,
                                        myLatencyCorrectorPos);
        this->g_syncStat &= ~Stats::HAVE_EXTERNAL_TIMESTAMP;
    }
}
void OneWayLatencyPlanner::unpack(fge::net::FluxPacket* packet, fge::net::Client& client)
{
    bool const finishedToSendLastPacket = !client.getCorrectorTimestamp().has_value();

    if (finishedToSendLastPacket)
    {
        packet->_packet.unpack(&this->g_externalStoredTimestamp, sizeof(fge::net::Timestamp));
        this->g_syncStat |= Stats::HAVE_EXTERNAL_TIMESTAMP;
    }
    else
    {
        packet->_packet.skip(sizeof(fge::net::Timestamp));
    }

    //Retrieve external latency corrector
    fge::net::Latency_ms latencyCorrector;
    packet->_packet.unpack(&latencyCorrector, sizeof(latencyCorrector));

    //Retrieve the latency computed at the other side (client or server)
    fge::net::Latency_ms otherSideLatency;
    packet->_packet.unpack(&otherSideLatency, sizeof(otherSideLatency));
    if (otherSideLatency != FGE_NET_BAD_LATENCY)
    {
        this->g_otherSideLatency = otherSideLatency;
    }

    //Retrieve full server timestamp (only client)
    fge::net::FullTimestamp fullTimestamp;
    packet->_packet.unpack(&fullTimestamp, sizeof(fullTimestamp));

    //Retrieve external sync stat
    std::underlying_type_t<Stats> externalSyncStat;
    packet->_packet.unpack(&externalSyncStat, sizeof(externalSyncStat));

    //Does he have our timestamp ?
    if ((externalSyncStat & Stats::HAVE_EXTERNAL_TIMESTAMP) > 0)
    {
        //Retrieve our timestamp
        fge::net::Timestamp firstTimestamp;
        packet->_packet.unpack(&firstTimestamp, sizeof(firstTimestamp));

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
        this->g_roundTripTime = fge::net::Client::computeLatency_ms(firstTimestamp, packet->getTimeStamp());

        //Compute new latency
        this->g_latency = (this->g_roundTripTime.value() - latencyCorrector) / 2;

        //Compute time offset
        fge::net::FullTimestampOffset const clockOffset =
                static_cast<fge::net::FullTimestampOffset>(fge::net::Client::getFullTimestamp_ms()) -
                static_cast<fge::net::FullTimestampOffset>(fullTimestamp) + this->g_latency.value();
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
        fge::net::FullTimestamp result = 0;
        for (std::size_t i = 0; i < this->g_clockOffsetCount; ++i)
        {
            result += this->g_clockOffsets[i];
        }
        this->g_meanClockOffset = result / this->g_clockOffsetCount;
    }
}

std::optional<fge::net::FullTimestampOffset> OneWayLatencyPlanner::getClockOffset() const
{
    return this->g_meanClockOffset;
}
std::optional<fge::net::Latency_ms> OneWayLatencyPlanner::getLatency() const
{
    return this->g_latency;
}
std::optional<fge::net::Latency_ms> OneWayLatencyPlanner::getOtherSideLatency() const
{
    return this->g_otherSideLatency;
}
std::optional<fge::net::Latency_ms> OneWayLatencyPlanner::getRoundTripTime() const
{
    return this->g_roundTripTime;
}

} // namespace fge::net

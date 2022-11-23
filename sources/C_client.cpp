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

#include "FastEngine/C_client.hpp"
#include "FastEngine/C_random.hpp"
#include <limits>

namespace fge::net
{

Client::Client() :
    g_CTOSLatency_ms(FGE_NET_DEFAULT_LATENCY),
    g_STOCLatency_ms(FGE_NET_DEFAULT_LATENCY),
    g_lastPacketTimePoint( std::chrono::steady_clock::now() ),
    g_skey(FGE_NET_BAD_SKEY)
{
}
Client::Client(fge::net::Client::Latency_ms CTOSLatency, fge::net::Client::Latency_ms STOCLatency) :
    g_CTOSLatency_ms(CTOSLatency),
    g_STOCLatency_ms(STOCLatency),
    g_lastPacketTimePoint( std::chrono::steady_clock::now() ),
    g_skey(FGE_NET_BAD_SKEY)
{
}

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

void Client::setCTOSLatency_ms(fge::net::Client::Latency_ms latency)
{
    this->g_CTOSLatency_ms = latency;
}
void Client::setSTOCLatency_ms(fge::net::Client::Latency_ms latency)
{
    this->g_STOCLatency_ms = latency;
}
fge::net::Client::Latency_ms Client::getCTOSLatency_ms() const
{
    return this->g_CTOSLatency_ms;
}
fge::net::Client::Latency_ms Client::getSTOCLatency_ms() const
{
    return this->g_STOCLatency_ms;
}
fge::net::Client::Latency_ms Client::getPing_ms() const
{
    return this->g_CTOSLatency_ms + this->g_STOCLatency_ms;
}

void Client::resetLastPacketTimePoint()
{
    std::lock_guard<std::recursive_mutex> lck(this->g_mutex);
    this->g_lastPacketTimePoint = std::chrono::steady_clock::now();
}
fge::net::Client::Latency_ms Client::getLastPacketElapsedTime()
{
    std::lock_guard<std::recursive_mutex> lck(this->g_mutex);

    std::chrono::steady_clock::time_point timeNow = std::chrono::steady_clock::now();
    uint64_t t = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - this->g_lastPacketTimePoint).count();
    return (t >= std::numeric_limits<fge::net::Client::Latency_ms>::max()) ? std::numeric_limits<fge::net::Client::Latency_ms>::max() : static_cast<fge::net::Client::Latency_ms>(t);
}

fge::net::Client::Timestamp Client::getTimestamp_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() % FGE_NET_CLIENT_TIMESTAMP_MODULO;
}

fge::net::Client::Latency_ms Client::computeLatency_ms(const fge::net::Client::Timestamp& sentTimestamp,
                                                       const fge::net::Client::Timestamp& receivedTimestamp)
{
    int32_t t = static_cast<int32_t>(receivedTimestamp) - static_cast<int32_t>(sentTimestamp);
    if (t<0)
    {
        t += FGE_NET_CLIENT_TIMESTAMP_MODULO;
    }
    return static_cast<fge::net::Client::Latency_ms>(t);
}

void Client::clearPackets()
{
    std::lock_guard<std::recursive_mutex> lck(this->g_mutex);

    for (std::size_t i=0; i<this->g_pendingTransmitPackets.size(); ++i)
    {
        this->g_pendingTransmitPackets.pop();
    }
}
void Client::pushPacket(const fge::net::ClientSendQueuePacket& pck)
{
    std::lock_guard<std::recursive_mutex> lck(this->g_mutex);

    this->g_pendingTransmitPackets.push(pck);
}
fge::net::ClientSendQueuePacket Client::popPacket()
{
    std::lock_guard<std::recursive_mutex> lck(this->g_mutex);

    if (this->g_pendingTransmitPackets.empty())
    {
        return {nullptr};
    }
    fge::net::ClientSendQueuePacket tmp = this->g_pendingTransmitPackets.front();
    this->g_pendingTransmitPackets.pop();
    return tmp;
}
bool Client::isPendingPacketsEmpty()
{
    std::lock_guard<std::recursive_mutex> lck(this->g_mutex);
    return this->g_pendingTransmitPackets.empty();
}

}//end fge::net

#include "FastEngine/C_client.hpp"
#include "FastEngine/C_random.hpp"
#include <limits>

#define _FGE_NET_CLIENT_TIMESTAMP_MODULO 65536

namespace fge
{
namespace net
{

Client::Client() :
    g_latency_ms(FGE_NET_DEFAULT_LATENCY),
    g_lastPacketTimePoint( std::chrono::steady_clock::now() ),
    g_skey(FGE_NET_BAD_SKEY)
{
}
Client::Client(fge::net::Client::Latency_ms latency) :
    g_latency_ms(latency),
    g_lastPacketTimePoint( std::chrono::steady_clock::now() ),
    g_skey(FGE_NET_BAD_SKEY)
{
}

fge::net::Skey Client::GenerateSkey()
{
    return fge::__random.range<fge::net::Skey>(1, std::numeric_limits<fge::net::Skey>::max());
}
void Client::setSkey(fge::net::Skey key)
{
    this->g_skey = key;
}
fge::net::Skey Client::getSkey() const
{
    return this->g_skey;
}

void Client::setLatency_ms(fge::net::Client::Latency_ms t)
{
    this->g_latency_ms = t;
}
fge::net::Client::Latency_ms Client::getLatency_ms() const
{
    return this->g_latency_ms;
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
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() % _FGE_NET_CLIENT_TIMESTAMP_MODULO;
}
fge::net::Client::Latency_ms Client::computeLatency_ms(const fge::net::Client::Timestamp& startedTime,
                                                                const fge::net::Client::Timestamp& returnedTime )
{
    int32_t t = static_cast<int32_t>(returnedTime) - static_cast<int32_t>(startedTime);
    if (t<0)
        t += _FGE_NET_CLIENT_TIMESTAMP_MODULO;

    return static_cast<fge::net::Client::Latency_ms>(t);
}
fge::net::Client::Latency_ms Client::computePing_ms(const fge::net::Client::Timestamp& startedTime)
{
    fge::net::Client::Timestamp now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() % _FGE_NET_CLIENT_TIMESTAMP_MODULO;
    int32_t t = static_cast<int32_t>(now) - static_cast<int32_t>(startedTime);
    if (t<0)
        t += _FGE_NET_CLIENT_TIMESTAMP_MODULO;

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

}//end net
}//end fge

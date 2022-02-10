#ifndef _FGE_C_CLIENT_HPP_INCLUDED
#define _FGE_C_CLIENT_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_identity.hpp>
#include <FastEngine/C_valueList.hpp>
#include <FastEngine/C_event.hpp>
#include <queue>
#include <chrono>
#include <mutex>
#include <memory>

#define FGE_NET_BAD_SKEY 0
#define FGE_NET_DEFAULT_LATENCY 80

namespace fge
{
namespace net
{

using Skey = uint32_t;

class FGE_API Client
{
public:
    using Timestamp = uint16_t;
    using Latency_ms = uint16_t;

    Client();
    Client(fge::net::Client::Latency_ms latency);

    static fge::net::Skey GenerateSkey();
    void setSkey(fge::net::Skey key);
    fge::net::Skey getSkey() const;

    void setLatency_ms(fge::net::Client::Latency_ms t);
    fge::net::Client::Latency_ms getLatency_ms() const;

    void resetLastPacketTimePoint();
    fge::net::Client::Latency_ms getLastPacketElapsedTime();

    static fge::net::Client::Timestamp getTimestamp_ms();
    static fge::net::Client::Latency_ms computeLatency_ms(const fge::net::Client::Timestamp& startedTime,
                                                          const fge::net::Client::Timestamp& returnedTime );
    static fge::net::Client::Latency_ms computePing_ms(const fge::net::Client::Timestamp& startedTime);

    void clearPackets();
    void pushPacket(const std::shared_ptr<fge::net::Packet>& pck);
    std::shared_ptr<fge::net::Packet> popPacket();
    bool isPendingPacketsEmpty();

    fge::Event _event;
    fge::ValueList _data;

private:
    fge::net::Client::Latency_ms g_latency_ms;
    std::chrono::steady_clock::time_point g_lastPacketTimePoint;

    std::queue<std::shared_ptr<fge::net::Packet> > g_pendingTransmitPackets;
    std::recursive_mutex g_mutex;

    fge::net::Skey g_skey;
};

}//end net
}//end fge

#endif // _FGE_C_CLIENT_HPP_INCLUDED

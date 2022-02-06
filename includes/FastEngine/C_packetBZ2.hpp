/*
    Using the libbzip2 :

    This program, "bzip2", the associated library "libbzip2", and all
    documentation, are copyright (C) 1996-2019 Julian R Seward.  All
    rights reserved.
*/

#ifndef _FGE_C_PACKETBZ2_HPP_INCLUDED
#define _FGE_C_PACKETBZ2_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_packet.hpp>

#define FGE_PACKETBZ2_DEFAULT_WORKFACTOR 0
#define FGE_PACKETBZ2_DEFAULT_BLOCKSIZE 4

#define FGE_PACKETBZ2_DEFAULT_MAXUNCOMPRESSEDRECEIVEDSIZE 65536
#define FGE_PACKETBZ2_VERSION "1.0.8"

namespace fge
{
namespace net
{

class FGE_API PacketBZ2 : public fge::net::Packet
{
public:
    PacketBZ2();
    PacketBZ2(fge::net::PacketBZ2&& pck) noexcept;
    PacketBZ2(fge::net::PacketBZ2& pck) = default;
    PacketBZ2(const fge::net::PacketBZ2& pck) = default;
    ~PacketBZ2() override = default;

    static uint32_t _maxUncompressedReceivedSize;

    void setBlockSize(int blockSize);
    [[nodiscard]] int getBlockSize() const;
    void setWorkFactor(int factor);
    [[nodiscard]] int getWorkFactor() const;

    [[nodiscard]] std::size_t getLastCompressionSize() const;

protected:
    void onSend(std::vector<uint8_t>& buffer, std::size_t offset) override;
    void onReceive(void* data, std::size_t dsize) override;

private:
    int g_blockSize;
    int g_workfactor;

    std::vector<char> g_buffer;
    std::size_t g_lastCompressionSize;
};

}//end net
}//end fge

#endif // _FGE_C_PACKETBZ2_HPP_INCLUDED

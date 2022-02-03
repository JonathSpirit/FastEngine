namespace fge
{
namespace net
{

fge::net::Packet& SetHeader(fge::net::Packet& pck, fge::net::PacketHeader header)
{
    pck.clear();
    pck << header;
    return pck;
}
fge::net::PacketHeader GetHeader(fge::net::Packet& pck)
{
    fge::net::PacketHeader header = FGE_NET_BAD_HEADER;
    pck >> header;
    return header;
}

bool CheckSkey(fge::net::Packet& pck, fge::net::Skey skey)
{
    fge::net::Skey buff;
    if ( pck >> buff)
    {
        return buff == skey;
    }
    return false;
}
fge::net::Skey GetSkey(fge::net::Packet& pck)
{
    fge::net::Skey buff;
    if ( pck >> buff)
    {
        return buff;
    }
    return FGE_NET_BAD_SKEY;
}

}//end net
}//end fge

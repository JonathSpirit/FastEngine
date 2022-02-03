#ifndef _FGE_C_BITBANK_HPP_INCLUDED
#define _FGE_C_BITBANK_HPP_INCLUDED

#include <FastEngine/C_packet.hpp>

namespace fge
{

template<unsigned int TNbytes>
class BitBank
{
    static_assert(TNbytes>0, "TNbytes must be greater than 0");
public:
    BitBank();
    ~BitBank() = default;

    void clear();

    void set(unsigned int index, bool flag);
    bool get(unsigned int index) const;
    uint8_t getByte(unsigned int index) const;

    unsigned int getSize() const;

    void pack(fge::net::Packet& pck);
    void unpack(fge::net::Packet& pck);

private:
    uint8_t g_data[TNbytes];
};

}//end fge

#include <FastEngine/C_bitBank.inl>

#endif // _FGE_C_BITBANK_HPP_INCLUDED

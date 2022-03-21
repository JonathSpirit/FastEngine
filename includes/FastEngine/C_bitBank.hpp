#ifndef _FGE_C_BITBANK_HPP_INCLUDED
#define _FGE_C_BITBANK_HPP_INCLUDED

#include <FastEngine/C_packet.hpp>

namespace fge
{

template<std::size_t TNbytes>
class BitBank
{
    static_assert(TNbytes>0, "TNbytes must be greater than 0");
public:
    BitBank() = default;
    ~BitBank() = default;

    void clear();

    void set(std::size_t index, bool flag);
    [[nodiscard]] bool get(std::size_t index) const;
    [[nodiscard]] uint8_t getByte(std::size_t index) const;

    [[nodiscard]] std::size_t getSize() const;

    void pack(fge::net::Packet& pck);
    void unpack(fge::net::Packet& pck);

private:
    uint8_t g_data[TNbytes]{0};
};

}//end fge

#include <FastEngine/C_bitBank.inl>

#endif // _FGE_C_BITBANK_HPP_INCLUDED

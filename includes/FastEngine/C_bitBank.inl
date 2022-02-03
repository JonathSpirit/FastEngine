namespace fge
{

template<unsigned int TNbytes>
BitBank<TNbytes>::BitBank() :
    g_data{0}
{
}

template<unsigned int TNbytes>
void BitBank<TNbytes>::clear()
{
    for (unsigned int i=0; i<TNbytes; ++i)
    {
        this->g_data[i] = 0;
    }
}

template<unsigned int TNbytes>
void BitBank<TNbytes>::set(unsigned int index, bool flag)
{
    if (index < TNbytes*8)
    {
        this->g_data[index/8] = static_cast<uint8_t>(flag) << (index%8);
    }
}
template<unsigned int TNbytes>
bool BitBank<TNbytes>::get(unsigned int index) const
{
    if (index < TNbytes*8)
    {
        return this->g_data[index/8] & (0x01 << (index%8));
    }
    return false;
}
template<unsigned int TNbytes>
uint8_t BitBank<TNbytes>::getByte(unsigned int index) const
{
    if (index < TNbytes)
    {
        return this->g_data[index];
    }
    return 0;
}

template<unsigned int TNbytes>
unsigned int BitBank<TNbytes>::getSize() const
{
    return TNbytes;
}

template<unsigned int TNbytes>
void BitBank<TNbytes>::pack(fge::net::Packet& pck)
{
    pck.append(&this->g_data, TNbytes);
}
template<unsigned int TNbytes>
void BitBank<TNbytes>::unpack(fge::net::Packet& pck)
{
    pck.read(&this->g_data, TNbytes);
}

}//end fge

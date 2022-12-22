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

namespace fge
{

template<std::size_t TNbytes>
void BitBank<TNbytes>::clear()
{
    for (std::size_t i = 0; i < TNbytes; ++i)
    {
        this->g_data[i] = 0;
    }
}

template<std::size_t TNbytes>
void BitBank<TNbytes>::set(std::size_t index, bool flag)
{
    if (index < TNbytes * 8)
    {
        this->g_data[index / 8] = static_cast<uint8_t>(flag) << (index % 8);
    }
}
template<std::size_t TNbytes>
bool BitBank<TNbytes>::get(std::size_t index) const
{
    if (index < TNbytes * 8)
    {
        return this->g_data[index / 8] & (0x01 << (index % 8));
    }
    return false;
}
template<std::size_t TNbytes>
uint8_t BitBank<TNbytes>::getByte(std::size_t index) const
{
    if (index < TNbytes)
    {
        return this->g_data[index];
    }
    return 0;
}

template<std::size_t TNbytes>
std::size_t BitBank<TNbytes>::getSize() const
{
    return TNbytes;
}

template<std::size_t TNbytes>
void BitBank<TNbytes>::pack(fge::net::Packet& pck)
{
    pck.append(&this->g_data, TNbytes);
}
template<std::size_t TNbytes>
void BitBank<TNbytes>::unpack(fge::net::Packet& pck)
{
    pck.read(&this->g_data, TNbytes);
}

} // namespace fge

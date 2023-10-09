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

namespace fge::net
{

template<class TPacket>
inline std::shared_ptr<TransmissionPacket> TransmissionPacket::create()
{
    return std::shared_ptr<TransmissionPacket>{new TransmissionPacket(std::make_unique<TPacket>())};
}
template<class TPacket>
inline std::shared_ptr<TransmissionPacket> TransmissionPacket::create(std::initializer_list<Option> options)
{
    return std::shared_ptr<TransmissionPacket>{new TransmissionPacket(std::make_unique<TPacket>(), options)};
}

inline TransmissionPacket::TransmissionPacket(std::unique_ptr<Packet>&& packet) :
        g_packet(std::move(packet)),
        g_options()
{}
inline TransmissionPacket::TransmissionPacket(std::unique_ptr<Packet>&& packet, std::initializer_list<Option> options) :
        g_packet(std::move(packet)),
        g_options(options)
{}

inline fge::net::Packet const& TransmissionPacket::packet() const
{
    return *this->g_packet;
}
inline fge::net::Packet& TransmissionPacket::packet()
{
    return *this->g_packet;
}
inline std::vector<TransmissionPacket::Option> const& TransmissionPacket::options() const
{
    return this->g_options;
}
inline std::vector<TransmissionPacket::Option>& TransmissionPacket::options()
{
    return this->g_options;
}

} // namespace fge::net

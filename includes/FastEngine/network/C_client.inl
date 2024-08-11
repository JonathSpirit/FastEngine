/*
 * Copyright 2024 Guillaume Guillet
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

inline std::shared_ptr<TransmissionPacket> TransmissionPacket::create(ProtocolPacket::Header header)
{
    return std::shared_ptr<TransmissionPacket>{new TransmissionPacket(header, 0, 0)};
}
inline std::shared_ptr<TransmissionPacket> TransmissionPacket::create(Packet&& packet)
{
    return std::shared_ptr<TransmissionPacket>{new TransmissionPacket(std::move(packet))};
}

inline TransmissionPacket::TransmissionPacket(ProtocolPacket::Header header,
                                              ProtocolPacket::Realm realmId,
                                              ProtocolPacket::CountId countId) :
        g_packet(header, realmId, countId)
{}
inline TransmissionPacket::TransmissionPacket(ProtocolPacket&& packet) :
        g_packet(std::move(packet))
{}

inline ProtocolPacket const& TransmissionPacket::packet() const
{
    return this->g_packet;
}
inline ProtocolPacket& TransmissionPacket::packet()
{
    return this->g_packet;
}
inline std::vector<TransmissionPacket::Option> const& TransmissionPacket::options() const
{
    return this->g_options;
}
inline std::vector<TransmissionPacket::Option>& TransmissionPacket::options()
{
    return this->g_options;
}

inline TransmissionPacket& TransmissionPacket::doNotDiscard()
{
    this->g_packet.addHeaderFlags(FGE_NET_HEADER_DO_NOT_DISCARD_FLAG);
    return *this;
}
inline TransmissionPacket& TransmissionPacket::doNotReorder()
{
    this->g_packet.addHeaderFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG);
    return *this;
}

} // namespace fge::net

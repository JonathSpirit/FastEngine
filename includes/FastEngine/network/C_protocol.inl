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

inline ProtocolPacket::ProtocolPacket(HeaderId headerId, Realm realmId, CountId countId)
{
    this->operator<<(headerId) << realmId << countId;
}

inline bool ProtocolPacket::haveCorrectHeaderSize() const
{
    return this->getDataSize() >= HeaderSize;
}
inline std::optional<ProtocolPacket::HeaderId> ProtocolPacket::retrieveHeaderId() const
{
    if (this->haveCorrectHeaderSize())
    {
        HeaderId headerId;
        this->unpack(HeaderIdPosition, &headerId, sizeof(HeaderId));
        return headerId;
    }
    return std::nullopt;
}
inline std::optional<ProtocolPacket::Realm> ProtocolPacket::retrieveRealm() const
{
    if (this->haveCorrectHeaderSize())
    {
        Realm realm;
        this->unpack(RealmPosition, &realm, sizeof(Realm));
        return realm;
    }
    return std::nullopt;
}
inline std::optional<ProtocolPacket::CountId> ProtocolPacket::retrieveCountId() const
{
    if (this->haveCorrectHeaderSize())
    {
        CountId countId;
        this->unpack(CountIdPosition, &countId, sizeof(CountId));
        return countId;
    }
    return std::nullopt;
}

inline void ProtocolPacket::setHeaderId(HeaderId headerId)
{
    this->pack(HeaderIdPosition, &headerId, sizeof(HeaderId));
}
inline void ProtocolPacket::setHeaderFlags(HeaderId headerFlags)
{
    if (this->haveCorrectHeaderSize())
    {
        HeaderId headerId;
        this->unpack(HeaderIdPosition, &headerId, sizeof(HeaderId));
        headerId = (headerId & ~FGE_NET_HEADERID_FLAGS_MASK) | (headerFlags & FGE_NET_HEADERID_FLAGS_MASK);
        this->pack(HeaderIdPosition, &headerId, sizeof(HeaderId));
    }
}
inline void ProtocolPacket::addHeaderFlags(HeaderId headerFlags)
{
    if (this->haveCorrectHeaderSize())
    {
        HeaderId headerId;
        this->unpack(HeaderIdPosition, &headerId, sizeof(HeaderId));
        headerId |= headerFlags & FGE_NET_HEADERID_FLAGS_MASK;
        this->pack(HeaderIdPosition, &headerId, sizeof(HeaderId));
    }
}
inline void ProtocolPacket::removeHeaderFlags(HeaderId headerFlags)
{
    if (this->haveCorrectHeaderSize())
    {
        HeaderId headerId;
        this->unpack(HeaderIdPosition, &headerId, sizeof(HeaderId));
        headerId &= ~headerFlags & FGE_NET_HEADERID_FLAGS_MASK;
        this->pack(HeaderIdPosition, &headerId, sizeof(HeaderId));
    }
}
inline void ProtocolPacket::setRealm(Realm realmId)
{
    this->pack(RealmPosition, &realmId, sizeof(Realm));
}
inline void ProtocolPacket::setCountId(CountId countId)
{
    this->pack(CountIdPosition, &countId, sizeof(CountId));
}

} // namespace fge::net

/*
 * Copyright 2025 Guillaume Guillet
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
inline ProtocolPacket::ProtocolPacket(Packet const& pck,
                                      Identity const& id,
                                      std::size_t fluxIndex,
                                      std::size_t fluxLifetime) :
        Packet(pck),

        g_identity(id),

        g_fluxIndex(fluxIndex),
        g_fluxLifetime(fluxLifetime)
{}
inline ProtocolPacket::ProtocolPacket(Packet&& pck,
                                      Identity const& id,
                                      std::size_t fluxIndex,
                                      std::size_t fluxLifetime) :
        Packet(std::move(pck)),

        g_identity(id),

        g_fluxIndex(fluxIndex),
        g_fluxLifetime(fluxLifetime)
{}
inline ProtocolPacket::ProtocolPacket(IdType header, RealmType realmId, CounterType countId, CounterType lastCountId)
{
    this->operator<<(header) << realmId << countId << lastCountId << SessionId{FGE_NET_DEFAULT_SESSION};
}

inline ProtocolPacket::ProtocolPacket(Packet const& r) :
        Packet(r)
{}
inline ProtocolPacket::ProtocolPacket(Packet&& r) noexcept :
        Packet(std::move(r))
{}

inline ProtocolPacket::ProtocolPacket(ProtocolPacket const& r) :
        Packet(r),
        std::enable_shared_from_this<ProtocolPacket>(r),

        g_identity(r.g_identity),
        g_timestamp(r.g_timestamp),

        g_fluxIndex(r.g_fluxIndex),
        g_fluxLifetime(r.g_fluxLifetime),

        g_markedForEncryption(r.g_markedForEncryption),
        g_markedAsLocallyReordered(r.g_markedAsLocallyReordered),
        g_markedAsCached(r.g_markedAsCached),

        g_options(r.g_options)
{}
inline ProtocolPacket::ProtocolPacket(ProtocolPacket&& r) noexcept :
        Packet(std::move(r)),
        std::enable_shared_from_this<ProtocolPacket>(std::move(r)),

        g_identity(r.g_identity),
        g_timestamp(r.g_timestamp),

        g_fluxIndex(r.g_fluxIndex),
        g_fluxLifetime(r.g_fluxLifetime),

        g_markedForEncryption(r.g_markedForEncryption),
        g_markedAsLocallyReordered(r.g_markedAsLocallyReordered),
        g_markedAsCached(r.g_markedAsCached),

        g_options(std::move(r.g_options))
{}

inline bool ProtocolPacket::haveCorrectHeader() const
{
    if (this->haveCorrectHeaderSize())
    {
        IdType headerId;
        this->unpack(IdPosition, &headerId, sizeof(IdType));

        if ((headerId & ~FGE_NET_HEADER_FLAGS_MASK) == FGE_NET_BAD_ID)
        {
            return false;
        }
        return true;
    }
    return false;
}
inline bool ProtocolPacket::haveCorrectHeaderSize() const
{
    return this->getDataSize() >= HeaderSize;
}
inline std::optional<ProtocolPacket::IdType> ProtocolPacket::retrieveHeaderId() const
{
    if (this->haveCorrectHeaderSize())
    {
        IdType headerId;
        this->unpack(IdPosition, &headerId, sizeof(IdType));
        return headerId & ~FGE_NET_HEADER_FLAGS_MASK;
    }
    return std::nullopt;
}
inline std::optional<ProtocolPacket::IdType> ProtocolPacket::retrieveFlags() const
{
    if (this->haveCorrectHeaderSize())
    {
        IdType headerFlags;
        this->unpack(IdPosition, &headerFlags, sizeof(IdType));
        return headerFlags & FGE_NET_HEADER_FLAGS_MASK;
    }
    return std::nullopt;
}
inline std::optional<ProtocolPacket::IdType> ProtocolPacket::retrieveFullHeaderId() const
{
    if (this->haveCorrectHeaderSize())
    {
        IdType headerId;
        this->unpack(IdPosition, &headerId, sizeof(IdType));
        return headerId;
    }
    return std::nullopt;
}
inline std::optional<ProtocolPacket::RealmType> ProtocolPacket::retrieveRealm() const
{
    if (this->haveCorrectHeaderSize())
    {
        RealmType realm;
        this->unpack(RealmPosition, &realm, sizeof(RealmType));
        return realm;
    }
    return std::nullopt;
}
inline std::optional<ProtocolPacket::CounterType> ProtocolPacket::retrieveCounter() const
{
    if (this->haveCorrectHeaderSize())
    {
        CounterType counter;
        this->unpack(CounterPosition, &counter, sizeof(CounterType));
        return counter;
    }
    return std::nullopt;
}
inline std::optional<ProtocolPacket::CounterType> ProtocolPacket::retrieveLastCounter() const
{
    if (this->haveCorrectHeaderSize())
    {
        CounterType counter;
        this->unpack(LastCounterPosition, &counter, sizeof(CounterType));
        return counter;
    }
    return std::nullopt;
}
inline std::optional<ProtocolPacket::SessionId> ProtocolPacket::retrieveSessionId() const
{
    if (this->haveCorrectHeaderSize())
    {
        SessionId sessionId;
        this->unpack(SessionIdPosition, &sessionId, sizeof(SessionId));
        return sessionId;
    }
    return std::nullopt;
}
inline std::optional<ProtocolPacket::Header> ProtocolPacket::retrieveHeader() const
{
    if (this->haveCorrectHeaderSize())
    {
        Header header{};
        this->unpack(IdPosition, &header._id, sizeof(IdType));
        this->unpack(RealmPosition, &header._realm, sizeof(RealmType));
        this->unpack(CounterPosition, &header._counter, sizeof(CounterType));
        this->unpack(LastCounterPosition, &header._lastCounter, sizeof(CounterType));
        return header;
    }
    return std::nullopt;
}
inline bool ProtocolPacket::isFragmented() const
{
    return this->retrieveHeaderId().value_or(FGE_NET_BAD_ID) == NET_INTERNAL_ID_FRAGMENTED_PACKET;
}

inline ProtocolPacket& ProtocolPacket::setHeader(Header const& header)
{
    if (!this->haveCorrectHeaderSize())
    {
        this->append(HeaderSize - this->getDataSize());
    }

    this->pack(IdPosition, &header._id, sizeof(IdType));
    this->pack(RealmPosition, &header._realm, sizeof(RealmType));
    this->pack(CounterPosition, &header._counter, sizeof(CounterType));
    this->pack(LastCounterPosition, &header._lastCounter, sizeof(CounterType));
    return *this;
}
inline ProtocolPacket& ProtocolPacket::setHeaderId(IdType id)
{
    if (this->haveCorrectHeaderSize())
    {
        IdType headerFlags;
        this->unpack(IdPosition, &headerFlags, sizeof(IdType));
        id = (headerFlags & FGE_NET_HEADER_FLAGS_MASK) | (id & ~FGE_NET_HEADER_FLAGS_MASK);
        this->pack(IdPosition, &id, sizeof(IdType));
    }
    return *this;
}

inline ProtocolPacket& ProtocolPacket::setSessionId(SessionId sessionId)
{
    if (this->haveCorrectHeaderSize())
    {
        this->pack(SessionIdPosition, &sessionId, sizeof(SessionId));
    }
    return *this;
}

inline ProtocolPacket& ProtocolPacket::setFlags(IdType flags)
{
    if (this->haveCorrectHeaderSize())
    {
        IdType headerId;
        this->unpack(IdPosition, &headerId, sizeof(IdType));
        headerId = (headerId & ~FGE_NET_HEADER_FLAGS_MASK) | (flags & FGE_NET_HEADER_FLAGS_MASK);
        this->pack(IdPosition, &headerId, sizeof(IdType));
    }
    return *this;
}
inline ProtocolPacket& ProtocolPacket::addFlags(IdType flags)
{
    if (this->haveCorrectHeaderSize())
    {
        IdType headerId;
        this->unpack(IdPosition, &headerId, sizeof(IdType));
        headerId |= flags & FGE_NET_HEADER_FLAGS_MASK;
        this->pack(IdPosition, &headerId, sizeof(IdType));
    }
    return *this;
}
inline ProtocolPacket& ProtocolPacket::removeFlags(IdType flags)
{
    if (this->haveCorrectHeaderSize())
    {
        IdType headerId;
        this->unpack(IdPosition, &headerId, sizeof(IdType));
        headerId &= ~(flags & FGE_NET_HEADER_FLAGS_MASK);
        this->pack(IdPosition, &headerId, sizeof(IdType));
    }
    return *this;
}
inline bool ProtocolPacket::checkFlags(IdType flags) const
{
    if (this->haveCorrectHeaderSize())
    {
        IdType headerId;
        this->unpack(IdPosition, &headerId, sizeof(IdType));
        return ((headerId & FGE_NET_HEADER_FLAGS_MASK) & (flags & FGE_NET_HEADER_FLAGS_MASK)) > 0;
    }
    return false;
}
inline ProtocolPacket& ProtocolPacket::doNotDiscard()
{
    return this->addFlags(FGE_NET_HEADER_DO_NOT_DISCARD_FLAG);
}
inline ProtocolPacket& ProtocolPacket::doNotReorder()
{
    return this->addFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG);
}
inline ProtocolPacket& ProtocolPacket::doNotFragment()
{
    return this->addFlags(FGE_NET_HEADER_DO_NOT_FRAGMENT_FLAG);
}

inline ProtocolPacket& ProtocolPacket::setRealm(RealmType realm)
{
    this->pack(RealmPosition, &realm, sizeof(RealmType));
    return *this;
}
inline ProtocolPacket& ProtocolPacket::setCounter(CounterType counter)
{
    this->pack(CounterPosition, &counter, sizeof(CounterType));
    return *this;
}
inline ProtocolPacket& ProtocolPacket::setLastReorderedPacketCounter(CounterType counter)
{
    this->pack(LastCounterPosition, &counter, sizeof(CounterType));
    return *this;
}

inline void ProtocolPacket::setTimestamp(Timestamp timestamp)
{
    this->g_timestamp = timestamp;
}
inline Timestamp ProtocolPacket::getTimeStamp() const
{
    return this->g_timestamp;
}
inline Identity const& ProtocolPacket::getIdentity() const
{
    return this->g_identity;
}

inline std::vector<ProtocolPacket::Option> const& ProtocolPacket::options() const
{
    return this->g_options;
}
inline std::vector<ProtocolPacket::Option>& ProtocolPacket::options()
{
    return this->g_options;
}

inline void ProtocolPacket::markForEncryption()
{
    this->g_markedForEncryption = true;
}
inline void ProtocolPacket::unmarkForEncryption()
{
    this->g_markedForEncryption = false;
}
inline bool ProtocolPacket::isMarkedForEncryption() const
{
    return this->g_markedForEncryption;
}

inline void ProtocolPacket::markAsLocallyReordered()
{
    this->g_markedAsLocallyReordered = true;
}
inline void ProtocolPacket::unmarkAsLocallyReordered()
{
    this->g_markedAsLocallyReordered = false;
}
inline bool ProtocolPacket::isMarkedAsLocallyReordered() const
{
    return this->g_markedAsLocallyReordered;
}

inline void ProtocolPacket::markAsCached()
{
    this->g_markedAsCached = true;
}
inline void ProtocolPacket::unmarkAsCached()
{
    this->g_markedAsCached = false;
}
inline bool ProtocolPacket::isMarkedAsCached() const
{
    return this->g_markedAsCached;
}

inline bool ProtocolPacket::checkFluxLifetime(std::size_t fluxSize)
{
    if (++this->g_fluxLifetime >= fluxSize)
    {
        return false;
    }
    this->g_fluxIndex = (this->g_fluxIndex + 1) % fluxSize;
    return true;
}
inline std::size_t ProtocolPacket::getFluxIndex() const
{
    return this->g_fluxIndex;
}
inline std::size_t ProtocolPacket::bumpFluxIndex(std::size_t fluxSize)
{
    this->g_fluxIndex = (this->g_fluxIndex + 1) % fluxSize;
    return this->g_fluxIndex;
}

} // namespace fge::net

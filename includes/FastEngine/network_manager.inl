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

namespace fge::net
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

namespace rules
{

template<class TValue>
TValue& Extract(fge::net::rules::ChainedArguments<TValue>& args)
{
    if (args._value.has_value())
    {
        return args._value.value();
    }
    *args._pck >> args._value.emplace();
    return args._value.value();
}

template<class TValue, bool TInvertResult>
fge::net::rules::ChainedArguments<TValue> RRange(const TValue& min, const TValue& max, fge::net::rules::ChainedArguments<TValue> args)
{
    if (args._pck->isValid())
    {
        auto& val = Extract(args);

        if ( (val >= min && val <= max) ^ TInvertResult )
        {
            return std::move(args);
        }
        args._pck->setValidity(false);
    }
    return std::move(args);
}

template<class TValue, bool TInvertResult>
fge::net::rules::ChainedArguments<TValue> RMustEqual(const TValue& a, fge::net::rules::ChainedArguments<TValue> args)
{
    if (args._pck->isValid())
    {
        auto& val = Extract(args);

        if ( (val == a) ^ TInvertResult )
        {
            return std::move(args);
        }
        args._pck->setValidity(false);
    }
    return std::move(args);
}

}//end rules

}//end fge::net

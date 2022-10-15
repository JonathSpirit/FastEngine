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
bool fge::net::rules::ChainedArguments<TValue>::checkExtract()
{
    if (this->_value.has_value())
    {
        return this->_pck->isValid();
    }
    *this->_pck >> this->_value.emplace();
    return this->_pck->isValid();
}
template<class TValue>
TValue& fge::net::rules::ChainedArguments<TValue>::extract()
{
    if (this->_value.has_value())
    {
        return this->_value.value();
    }
    *this->_pck >> this->_value.emplace();
    return this->_value.value();
}
template<class TValue>
template<class TPeek>
TPeek fge::net::rules::ChainedArguments<TValue>::peek()
{
    auto pos = this->_pck->getReadPos();
    TPeek value;
    *this->_pck >> value;
    this->_pck->setReadPos(pos);
    return std::move(value);
}

template<class TValue, bool TInvertResult>
fge::net::rules::ChainedArguments<TValue> RRange(const TValue& min, const TValue& max, fge::net::rules::ChainedArguments<TValue> args)
{
    if (args._pck->isValid())
    {
        auto& val = args.extract();
        if (args._pck->isValid())
        {
            if ( !((val >= min && val <= max) ^ TInvertResult) )
            {
                args._pck->invalidate();
            }
        }
    }
    return std::move(args);
}

template<class TValue, bool TInvertResult>
fge::net::rules::ChainedArguments<TValue> RMustEqual(const TValue& a, fge::net::rules::ChainedArguments<TValue> args)
{
    if (args._pck->isValid())
    {
        auto& val = args.extract();
        if (args._pck->isValid())
        {
            if ( !((val == a) ^ TInvertResult) )
            {
                args._pck->invalidate();
            }
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
fge::net::rules::ChainedArguments<TValue> RStrictLess(TValue less, fge::net::rules::ChainedArguments<TValue> args)
{
    if (args._pck->isValid())
    {
        auto& val = args.extract();
        if (args._pck->isValid())
        {
            if ( !((val < less) ^ TInvertResult) )
            {
                args._pck->invalidate();
            }
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
fge::net::rules::ChainedArguments<TValue> RLess(TValue less, fge::net::rules::ChainedArguments<TValue> args)
{
    if (args._pck->isValid())
    {
        auto& val = args.extract();
        if (args._pck->isValid())
        {
            if ( !((val <= less) ^ TInvertResult) )
            {
                args._pck->invalidate();
            }
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
fge::net::rules::ChainedArguments<TValue> RSizeRange(fge::net::SizeType min, fge::net::SizeType max, fge::net::rules::ChainedArguments<TValue> args)
{
    if (args._pck->isValid())
    {
        fge::net::SizeType val = args.template peek<fge::net::SizeType>();
        if (args._pck->isValid())
        {
            if ( !((val >= min && val <= max) ^ TInvertResult) )
            {
                args._pck->invalidate();
            }
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
fge::net::rules::ChainedArguments<TValue> RSizeMustEqual(fge::net::SizeType a, fge::net::rules::ChainedArguments<TValue> args)
{
    if (args._pck->isValid())
    {
        fge::net::SizeType val = args.template peek<fge::net::SizeType>();
        if (args._pck->isValid())
        {
            if ( !((val == a) ^ TInvertResult) )
            {
                args._pck->invalidate();
            }
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
fge::net::rules::ChainedArguments<TValue> RMustValidUtf8(fge::net::rules::ChainedArguments<TValue> args)
{
    if (args._pck->isValid())
    {
        auto& val = args.extract();
        if (args._pck->isValid())
        {
            if ( !(fge::string::IsValidUtf8String(val) ^ TInvertResult) )
            {
                args._pck->invalidate();
            }
        }
    }
    return args;
}

}//end rules

}//end fge::net

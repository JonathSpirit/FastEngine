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
    if (pck >> buff)
    {
        return buff == skey;
    }
    return false;
}
fge::net::Skey GetSkey(fge::net::Packet& pck)
{
    fge::net::Skey buff;
    if (pck >> buff)
    {
        return buff;
    }
    return FGE_NET_BAD_SKEY;
}

namespace rules
{

template<class TValue>
constexpr ChainedArguments<TValue>::ChainedArguments(fge::net::Packet const& pck, TValue* existingValue) :
        g_pck(&pck),
        g_value(existingValue == nullptr ? Value{std::in_place_type<TValue>} : Value{existingValue}),
        g_error(pck.isValid() ? Error{} : "already invalid packet")
{}
template<class TValue>
constexpr ChainedArguments<TValue>::ChainedArguments(fge::net::Packet const& pck, Error&& err, TValue* existingValue) :
        g_pck(&pck),
        g_value(existingValue == nullptr ? Value{std::in_place_type<TValue>} : Value{existingValue}),
        g_error(std::move(err))
{}

template<class TValue>
constexpr TValue* ChainedArguments<TValue>::extract()
{
    auto* value = std::holds_alternative<TValue>(this->g_value) ? &std::get<TValue>(this->g_value)
                                                                : std::get<TValue*>(this->g_value);
    *this->g_pck >> *value;
    if (this->g_pck->isValid())
    {
        return value;
    }
    this->g_error = "extraction failed at position " + std::to_string(this->g_pck->getReadPos());
    return nullptr;
}
template<class TValue>
template<class TPeek>
constexpr std::optional<TPeek> ChainedArguments<TValue>::peek()
{
    auto startReadPos = this->g_pck->getReadPos();

    TPeek value{};
    *this->g_pck >> value;

    if (this->g_pck->isValid())
    {
        this->g_pck->setReadPos(startReadPos);
        return std::move(value);
    }
    this->g_error = "peek failed at position " + std::to_string(this->g_pck->getReadPos());
    return std::nullopt;
}

template<class TValue>
constexpr fge::net::Packet const& ChainedArguments<TValue>::packet() const
{
    return *this->g_pck;
}
template<class TValue>
constexpr TValue const& ChainedArguments<TValue>::value() const
{
    return std::holds_alternative<TValue>(this->g_value) ? std::get<TValue>(this->g_value)
                                                         : *std::get<TValue*>(this->g_value);
}
template<class TValue>
constexpr TValue& ChainedArguments<TValue>::value()
{
    return std::holds_alternative<TValue>(this->g_value) ? std::get<TValue>(this->g_value)
                                                         : *std::get<TValue*>(this->g_value);
}

template<class TValue>
template<class TInvokable>
constexpr typename std::invoke_result_t<TInvokable, ChainedArguments<TValue>&>
ChainedArguments<TValue>::and_then(TInvokable&& f)
{
    if (this->g_pck->isValid())
    {
        return std::invoke(std::forward<TInvokable>(f), *this);
    }
    return {*this->g_pck, std::move(this->g_error), nullptr};
}
template<class TValue>
template<class TInvokable, class TIndex>
constexpr ChainedArguments<TValue>&
ChainedArguments<TValue>::and_for_each(TIndex iStart, TIndex iEnd, TIndex iIncrement, TInvokable&& f)
{
    if (!this->g_pck->isValid())
    {
        return *this;
    }

    for (iStart; iStart != iEnd; iStart += iIncrement)
    {
        std::optional<Error> err =
                std::invoke(std::forward<TInvokable>(f), const_cast<ChainedArguments<TValue> const&>(*this), iStart);

        if (err)
        {
            this->invalidate(std::move(err.value()));
            return *this;
        }
    }
    return *this;
}
template<class TValue>
template<class TInvokable, class TIndex>
constexpr ChainedArguments<TValue>&
ChainedArguments<TValue>::and_for_each(TIndex iStart, TIndex iIncrement, TInvokable&& f)
{
    if (!this->g_pck->isValid())
    {
        return *this;
    }

    auto& value = this->value();

    for (; iStart != value; iStart += iIncrement)
    {
        std::optional<Error> err =
                std::invoke(std::forward<TInvokable>(f), const_cast<ChainedArguments<TValue> const&>(*this), iStart);

        if (err)
        {
            this->invalidate(std::move(err.value()));
            return *this;
        }
    }
    return *this;
}
template<class TValue>
template<class TInvokable>
inline std::optional<Error> ChainedArguments<TValue>::on_error(TInvokable&& f)
{
    if (!this->g_pck->isValid())
    {
        std::invoke(std::forward<TInvokable>(f), *this);
        return this->g_error;
    }
    return std::nullopt;
}
template<class TValue>
inline std::optional<Error> ChainedArguments<TValue>::end()
{
    return this->g_pck->isValid() ? std::nullopt : std::optional<Error>{std::move(this->g_error)};
}
template<class TValue>
inline std::optional<Error> ChainedArguments<TValue>::end(std::nullopt_t nullopt) const
{
    return std::optional<Error>{nullopt};
}
template<class TValue>
inline std::optional<Error> ChainedArguments<TValue>::end(Error&& err) const
{
    return std::optional<Error>{std::move(err)};
}

template<class TValue>
constexpr ChainedArguments<TValue>& ChainedArguments<TValue>::apply(TValue& value)
{
    if (this->g_pck->isValid())
    {
        auto* extractedValue = std::holds_alternative<TValue>(this->g_value) ? &std::get<TValue>(this->g_value)
                                                                             : std::get<TValue*>(this->g_value);
        if constexpr (std::is_move_assignable_v<TValue>)
        {
            value = std::move(*extractedValue);
        }
        else
        {
            value = *extractedValue;
        }
    }
    return *this;
}
template<class TValue>
template<class TInvokable>
constexpr ChainedArguments<TValue>& ChainedArguments<TValue>::apply(TInvokable&& f)
{
    if (this->g_pck->isValid())
    {
        auto* extractedValue = std::holds_alternative<TValue>(this->g_value) ? &std::get<TValue>(this->g_value)
                                                                             : std::get<TValue*>(this->g_value);
        std::invoke(std::forward<TInvokable>(f), const_cast<TValue&>(*extractedValue));
    }
    return *this;
}

template<class TValue>
template<class TNewValue>
constexpr ChainedArguments<TNewValue> ChainedArguments<TValue>::newChain(TNewValue* existingValue)
{
    return ChainedArguments<TNewValue>{*this->g_pck, std::move(this->g_error), existingValue};
}
template<class TValue>
template<class TNewValue>
constexpr ChainedArguments<TNewValue> ChainedArguments<TValue>::newChain(TNewValue* existingValue) const
{
    return ChainedArguments<TNewValue>{*this->g_pck, existingValue};
}

template<class TValue>
constexpr ChainedArguments<TValue>& ChainedArguments<TValue>::setError(Error&& err)
{
    this->g_error = std::move(err);
    return *this;
}
template<class TValue>
constexpr ChainedArguments<TValue>& ChainedArguments<TValue>::invalidate(Error&& err)
{
    this->g_pck->invalidate();
    this->g_error = std::move(err);
    return *this;
}

template<class TValue, bool TInvertResult>
constexpr ChainedArguments<TValue> RRange(TValue const& min, TValue const& max, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val != nullptr)
        {
            if (!((*val >= min && *val <= max) ^ TInvertResult))
            {
                args.invalidate(__func__ + std::string{" failed"});
            }
        }
    }
    return args;
}

template<class TValue>
constexpr ChainedArguments<TValue> RValid(ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val == nullptr)
        {
            args.invalidate(__func__ + std::string{" failed"});
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
constexpr ChainedArguments<TValue> RMustEqual(TValue const& a, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val != nullptr)
        {
            if (!((*val == a) ^ TInvertResult))
            {
                args.invalidate(__func__ + std::string{" failed"});
            }
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
constexpr ChainedArguments<TValue> RStrictLess(TValue less, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val != nullptr)
        {
            if (!((*val < less) ^ TInvertResult))
            {
                args.invalidate(__func__ + std::string{" failed"});
            }
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
constexpr ChainedArguments<TValue> RLess(TValue less, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val != nullptr)
        {
            if (!((*val <= less) ^ TInvertResult))
            {
                args.invalidate(__func__ + std::string{" failed"});
            }
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
constexpr ChainedArguments<TValue>
RSizeRange(fge::net::SizeType min, fge::net::SizeType max, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto size = args.template peek<fge::net::SizeType>();
        if (size)
        {
            if (!((size >= min && size <= max) ^ TInvertResult))
            {
                args.invalidate(__func__ + std::string{" failed"});
            }
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
constexpr ChainedArguments<TValue> RSizeMustEqual(fge::net::SizeType a, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto size = args.template peek<fge::net::SizeType>();
        if (size)
        {
            if (!((size == a) ^ TInvertResult))
            {
                args.invalidate(__func__ + std::string{" failed"});
            }
        }
    }
    return args;
}

template<class TValue, bool TInvertResult>
constexpr ChainedArguments<TValue> RMustValidUtf8(ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val != nullptr)
        {
            if (!(fge::string::IsValidUtf8String(val) ^ TInvertResult))
            {
                args.invalidate(__func__ + std::string{" failed"});
            }
        }
    }
    return args;
}

} // namespace rules

} // namespace fge::net

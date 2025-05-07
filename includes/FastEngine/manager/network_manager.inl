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

namespace rules
{
template<class TValue>
constexpr ChainedArguments<TValue>::ChainedArguments(Packet const& pck, TValue* existingValue) :
        g_pck(&pck),
        g_value(existingValue == nullptr ? Value{std::in_place_type<TValue>} : Value{existingValue}),
        g_error(pck.isValid() ? Error{}
                              : Error{Error::Types::ERR_ALREADY_INVALID, pck.getReadPos(), "already invalid packet",
                                      __func__})
{}
template<class TValue>
constexpr ChainedArguments<TValue>::ChainedArguments(Packet const& pck, Error&& err, TValue* existingValue) :
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
    this->g_error =
            net::Error{net::Error::Types::ERR_EXTRACT, this->g_pck->getReadPos(), "extraction failed", __func__};
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
        return value;
    }

    this->g_error = net::Error{net::Error::Types::ERR_EXTRACT, this->g_pck->getReadPos(), "peek failed", __func__};
    return std::nullopt;
}

template<class TValue>
constexpr ChainedArguments<TValue>::operator Packet const&() const
{
    return *this->g_pck;
}
template<class TValue>
constexpr Packet const& ChainedArguments<TValue>::packet() const
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

    for (; iStart != iEnd; iStart += iIncrement)
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
constexpr ChainedArguments<TValue>& ChainedArguments<TValue>::and_for_each(TInvokable&& f)
{
    if (!this->g_pck->isValid())
    {
        return *this;
    }

    auto& value = this->value();

    for (TValue iIndex = 0; iIndex != value; ++iIndex)
    {
        std::optional<Error> err =
                std::invoke(std::forward<TInvokable>(f), const_cast<ChainedArguments<TValue> const&>(*this), iIndex);

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
constexpr std::optional<Error> ChainedArguments<TValue>::on_error(TInvokable&& f)
{
    if (!this->g_pck->isValid())
    {
        std::invoke(std::forward<TInvokable>(f), *this);
        return this->g_error;
    }
    return std::nullopt;
}
template<class TValue>
constexpr std::optional<Error> ChainedArguments<TValue>::end()
{
    return this->g_pck->isValid() ? std::nullopt : std::optional<Error>{std::move(this->g_error)};
}
template<class TValue>
constexpr std::optional<Error> ChainedArguments<TValue>::final()
{
    return this->g_pck->isValid() && this->g_pck->endReached() ? std::nullopt
                                                               : std::optional<Error>{std::move(this->g_error)};
}
template<class TValue>
constexpr std::optional<Error> ChainedArguments<TValue>::skip() const
{
    return std::nullopt;
}
template<class TValue>
constexpr std::optional<Error> ChainedArguments<TValue>::stop(char const* error, char const* function) const
{
    return Error{Error::Types::ERR_EXTRACT, this->g_pck->getReadPos(), error, function};
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
template<class TValue>
constexpr ChainedArguments<TValue>& ChainedArguments<TValue>::invalidate(char const* error, char const* function)
{
    this->g_pck->invalidate();
    this->g_error = Error{Error::Types::ERR_EXTRACT, this->g_pck->getReadPos(), error, function};
    return *this;
}

template<class TValue, ROutputs TOutput>
constexpr ChainedArguments<TValue> RRange(TValue const& min, TValue const& max, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val != nullptr)
        {
            if (!((*val >= min && *val <= max) ^ static_cast<bool>(TOutput)))
            {
                args.invalidate(Error{Error::Types::ERR_RULE, args.packet().getReadPos(), "rule failed", __func__});
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
            args.invalidate(Error{Error::Types::ERR_RULE, args.packet().getReadPos(), "rule failed", __func__});
        }
    }
    return args;
}

template<class TValue, ROutputs TOutput>
constexpr ChainedArguments<TValue> RMustEqual(TValue const& a, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val != nullptr)
        {
            if (!((*val == a) ^ static_cast<bool>(TOutput)))
            {
                args.invalidate(Error{Error::Types::ERR_RULE, args.packet().getReadPos(), "rule failed", __func__});
            }
        }
    }
    return args;
}

template<class TValue, ROutputs TOutput>
constexpr ChainedArguments<TValue> RStrictLess(TValue less, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val != nullptr)
        {
            if (!((*val < less) ^ static_cast<bool>(TOutput)))
            {
                args.invalidate(Error{Error::Types::ERR_RULE, args.packet().getReadPos(), "rule failed", __func__});
            }
        }
    }
    return args;
}

template<class TValue, ROutputs TOutput>
constexpr ChainedArguments<TValue> RLess(TValue less, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val != nullptr)
        {
            if (!((*val <= less) ^ static_cast<bool>(TOutput)))
            {
                args.invalidate(Error{Error::Types::ERR_RULE, args.packet().getReadPos(), "rule failed", __func__});
            }
        }
    }
    return args;
}

template<class TValue, ROutputs TOutput>
constexpr ChainedArguments<TValue> RSizeRange(SizeType min, SizeType max, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto size = args.template peek<SizeType>();
        if (size)
        {
            if (!((size >= min && size <= max) ^ static_cast<bool>(TOutput)))
            {
                args.invalidate(Error{Error::Types::ERR_RULE, args.packet().getReadPos(), "rule failed", __func__});
            }
        }
    }
    return args;
}

template<class TValue, ROutputs TOutput>
constexpr ChainedArguments<TValue> RSizeMustEqual(SizeType a, ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto size = args.template peek<SizeType>();
        if (size)
        {
            if (!((size == a) ^ static_cast<bool>(TOutput)))
            {
                args.invalidate(Error{Error::Types::ERR_RULE, args.packet().getReadPos(), "rule failed", __func__});
            }
        }
    }
    return args;
}

template<class TValue, ROutputs TOutput>
constexpr ChainedArguments<TValue> RMustValidUtf8(ChainedArguments<TValue>&& args)
{
    if (args.packet().isValid())
    {
        auto* val = args.extract();
        if (val != nullptr)
        {
            if (!(fge::string::IsValidUtf8String(val) ^ static_cast<bool>(TOutput)))
            {
                args.invalidate(Error{Error::Types::ERR_RULE, args.packet().getReadPos(), "rule failed", __func__});
            }
        }
    }
    return args;
}

} // namespace rules

} // namespace fge::net

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

#ifndef _FGE_NETWORK_MANAGER_HPP_INCLUDED
#define _FGE_NETWORK_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/network/C_client.hpp"
#include "FastEngine/network/C_packet.hpp"
#include "FastEngine/object/C_object.hpp"
#include <optional>
#include <variant>

namespace fge::net
{

/**
 * \ingroup network
 * @{
 */

/**
 * \brief Get a basic scene checksum
 *
 * The checksum is computed with every ObjectSid.
 *
 * \param scene The scene
 * \return A 32bit checksum value
 */
FGE_API uint32_t GetSceneChecksum(fge::Scene& scene);

/**
 * \brief Utility function to write packet data into a file
 *
 * \param pck The Packet
 * \param file The file path that will contain the data
 * \return \b true if no error, \b false otherwise
 */
FGE_API bool WritePacketDataToFile(fge::net::Packet& pck, std::string const& file);

/**
 * \brief Shortcut function that will extract and compare the provided skey
 *
 * \param pck The Packet
 * \param skey An skey that will be compared
 * \return \b true if the skey is the same and the packet is valid, \b false otherwise
 */
inline bool CheckSkey(fge::net::Packet& pck, fge::net::Skey skey);
/**
 * \brief Shortcut function that will extract the skey
 *
 * \param pck The Packet
 * \return The extracted skey
 */
inline fge::net::Skey GetSkey(fge::net::Packet& pck);

/**
 * @}
 */

namespace rules
{

/**
 * \struct ChainedArguments
 * \brief This is a wrapper around a Packet and a value for safe extraction
 * \ingroup networkRules
 *
 * A ChainedArguments is the argument for all network extraction rules.
 * The goal is to chain up multiple rules and at the end output an optional error.
 *
 * When the packet is invalid, the chain will stop and the error will be returned.
 *
 * \tparam TValue The type of the value that will be extracted
 */
template<class TValue>
class ChainedArguments
{
public:
    constexpr ChainedArguments(fge::net::Packet const& pck, TValue* existingValue = nullptr);
    constexpr ChainedArguments(fge::net::Packet const& pck, Error&& err, TValue* existingValue = nullptr);
    constexpr ChainedArguments(ChainedArguments const& r) = default;
    constexpr ChainedArguments(ChainedArguments&& r) noexcept = default;

    constexpr ChainedArguments& operator=(ChainedArguments const& r) = default;
    constexpr ChainedArguments& operator=(ChainedArguments&& r) noexcept = default;

    /**
     * \brief Extract and verify the value from the packet
     *
     * If the user as provided an existing value, it will be used instead of creating a new one.
     *
     * \return The extracted value or \b nullptr if the Packet is/become invalid
     */
    [[nodiscard]] constexpr TValue* extract();
    /**
     * \brief Peek without changing the read position a copy of value
     *
     * \tparam TPeek The type of the value that will be peeked
     * \return A copy of the peeked value
     */
    template<class TPeek>
    [[nodiscard]] constexpr std::optional<TPeek> peek();

    [[nodiscard]] constexpr fge::net::Packet const& packet() const;
    [[nodiscard]] constexpr TValue const& value() const;
    [[nodiscard]] constexpr TValue& value();

    /**
     * \brief Chain up some code after a successful extraction
     *
     * If the packet is invalid, the chain will stop and the invokable argument will not be called.
     *
     * \tparam TInvokable The type of the invokable argument
     * \param f The invokable argument
     * \return A reference to the same ChainedArguments or a new one with a different value type
     */
    template<class TInvokable>
    [[nodiscard]] constexpr typename std::invoke_result_t<TInvokable, ChainedArguments<TValue>&>
    and_then(TInvokable&& f);
    /**
     * \brief Chain up some code in a for loop after a successful extraction
     *
     * If the packet is invalid, the chain will stop and the invokable argument will not be called.
     *
     * \tparam TInvokable The type of the invokable argument
     * \tparam TIndex The type of the index
     * \param iStart The starting value of the index
     * \param iEnd The ending value of the index
     * \param iIncrement The increment value of the index
     * \param f The invokable argument
     * \return A reference to the same ChainedArguments or a new one with a different value type
     */
    template<class TInvokable, class TIndex>
    [[nodiscard]] constexpr ChainedArguments<TValue>&
    and_for_each(TIndex iStart, TIndex iEnd, TIndex iIncrement, TInvokable&& f);
    /**
     * \brief Chain up some code in a for loop after a successful extraction
     *
     * This is the same as and_for_each(TIndex iStart, TIndex iEnd, TIndex iIncrement, TInvokable&& f)
     * but you don't have to provide the end value of the index as it will be got from the last
     * chain result.
     *
     * \tparam TInvokable The type of the invokable argument
     * \tparam TIndex The type of the index
     * \param iStart The starting value of the index
     * \param iIncrement The increment value of the index
     * \param f The invokable argument
     * \return A reference to the same ChainedArguments or a new one with a different value type
     */
    template<class TInvokable, class TIndex>
    [[nodiscard]] constexpr ChainedArguments<TValue>& and_for_each(TIndex iStart, TIndex iIncrement, TInvokable&& f);
    /**
     * \brief Chain up some code in a for loop after a successful extraction
     *
     * This is the same as and_for_each(TIndex iStart, TIndex iEnd, TIndex iIncrement, TInvokable&& f)
     * but you don't have to provide any index arguments as the end value of the index as it will be got from the last
     * chain result and the start is 0.
     *
     * \tparam TInvokable The type of the invokable argument
     * \param f The invokable argument
     * \return A reference to the same ChainedArguments or a new one with a different value type
     */
    template<class TInvokable>
    [[nodiscard]] constexpr ChainedArguments<TValue>& and_for_each(TInvokable&& f);
    /**
     * \brief Chain up some code after a unsuccessful extraction
     *
     * This must be the last method called in the chain as this
     * return an optional error.
     *
     * \tparam TInvokable The type of the invokable argument
     * \param f The invokable argument
     * \return An optional error
     */
    template<class TInvokable>
    constexpr std::optional<Error> on_error(TInvokable&& f);
    /**
     * \brief End the chain by doing a last validity check on the packet
     *
     * \return An optional error
     */
    [[nodiscard]] constexpr std::optional<Error> end();
    /**
     * \brief End the chain without returning an error
     *
     * This is useful when you want to skip in a and_for_each loop.
     *
     * \param nullopt A nullopt_t
     * \return An optional error (construct with nullopt)
     */
    [[nodiscard]] constexpr std::optional<Error> end(std::nullopt_t nullopt) const;
    /**
     * \brief End the chain by returning a custom error$
     *
     * \param err The error
     * \return An optional error (construct with err)
     */
    [[nodiscard]] constexpr std::optional<Error> end(Error&& err) const;

    /**
     * \brief Apply the extracted value to the provided reference
     *
     * If the packet is invalid, the value will not be applied.
     * When applied, the internal value is (if possible) moved to the provided reference.
     * So value() must not be called after this.
     *
     * \param value The reference that will be applied
     * \return A reference to the same ChainedArguments
     */
    constexpr ChainedArguments<TValue>& apply(TValue& value);
    template<class TInvokable>
    /**
     * \brief Apply the extracted value to the provided invokable argument
     *
     * \tparam TInvokable The type of the invokable argument
     * \param f The invokable argument
     * \return A reference to the same ChainedArguments
     */
    constexpr ChainedArguments<TValue>& apply(TInvokable&& f);

    /**
     * \brief Create a new chain with a different value type
     *
     * The packet and the error will be copied/moved to the new chain.
     *
     * \tparam TNewValue The type of the new value
     * \param existingValue An optional existing value
     * \return A new ChainedArguments with a different value type
     */
    template<class TNewValue>
    constexpr ChainedArguments<TNewValue> newChain(TNewValue* existingValue = nullptr);
    /**
     * \brief Create a new chain with a different value type
     *
     * This const version is generally used in a and_for_each loop.
     * The error isn't forwarded to the new chain.
     *
     * \tparam TNewValue The type of the new value
     * \param existingValue An optional existing value
     * \return A new ChainedArguments with a different value type
     */
    template<class TNewValue>
    constexpr ChainedArguments<TNewValue> newChain(TNewValue* existingValue = nullptr) const;

    /**
     * \brief Set the error
     *
     * \param err The error
     * \return A reference to the same ChainedArguments
     */
    constexpr ChainedArguments<TValue>& setError(Error&& err);
    /**
     * \brief Invalidate the packet and set the error
     *
     * \param err The error
     * \return A reference to the same ChainedArguments
     */
    constexpr ChainedArguments<TValue>& invalidate(Error&& err);

private:
    using Value = std::variant<TValue, TValue*>;

    fge::net::Packet const* g_pck;
    Value g_value;
    Error g_error;
};

/**
 * \ingroup networkRules
 * @{
 */

enum class ROutputs : bool
{
    R_NORMAL = false,
    R_INVERTED = true
};

/**
 * \brief Range rule, check if the value is in the min/max range
 *
 * The comparison is not strict for min/max.
 *
 * \tparam TValue The type of the value
 * \tparam TOutput Control how the rule will output
 * \param min The minimum range
 * \param max The maximum range
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, ROutputs TOutput = ROutputs::R_NORMAL>
constexpr ChainedArguments<TValue> RRange(TValue const& min, TValue const& max, ChainedArguments<TValue>&& args);

/**
 * \brief Valid rule, check if the value is correctly extracted
 *
 * \tparam TValue The type of the value
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue>
constexpr ChainedArguments<TValue> RValid(ChainedArguments<TValue>&& args);

/**
 * \brief Must equal rule, check if the value is equal to the provided one
 *
 * \tparam TValue The type of the value
 * \tparam TOutput Control how the rule will output
 * \param a The value that will be compared
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, ROutputs TOutput = ROutputs::R_NORMAL>
constexpr ChainedArguments<TValue> RMustEqual(TValue const& a, ChainedArguments<TValue>&& args);

/**
 * \brief Strict less rule, check if the value is strictly lesser than the provided one
 *
 * \tparam TValue The type of the value
 * \tparam TOutput Control how the rule will output
 * \param less The value that will be compared
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, ROutputs TOutput = ROutputs::R_NORMAL>
constexpr ChainedArguments<TValue> RStrictLess(TValue less, ChainedArguments<TValue>&& args);

/**
 * \brief Less rule, check if the value is lesser than the provided one
 *
 * \tparam TValue The type of the value
 * \tparam TOutput Control how the rule will output
 * \param less The value that will be compared
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, ROutputs TOutput = ROutputs::R_NORMAL>
constexpr ChainedArguments<TValue> RLess(TValue less, ChainedArguments<TValue>&& args);

/**
 * \brief Size range rule, check if the size is in the min/max range
 *
 * This function will peek the current fge::net::SizeType at read pos.
 * It is useful to apply rules on the size of something (string, container, ...) before
 * extracting it.
 *
 * The actual value will not be extracted here.
 *
 * \tparam TValue The type of the value
 * \tparam TOutput Control how the rule will output
 * \param min The minimum range
 * \param max The maximum range
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, ROutputs TOutput = ROutputs::R_NORMAL>
constexpr ChainedArguments<TValue> RSizeRange(SizeType min, SizeType max, ChainedArguments<TValue>&& args);

/**
 * \brief Size must equal rule, check if the size is equal to the provided one
 *
 * \see RSizeRange
 *
 * \tparam TValue The type of the value
 * \tparam TOutput Control how the rule will output
 * \param a The value that will be compared
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, ROutputs TOutput = ROutputs::R_NORMAL>
constexpr ChainedArguments<TValue> RSizeMustEqual(fge::net::SizeType a, ChainedArguments<TValue>&& args);

/**
 * \brief Check if the extracted string is a valid UTF8 string
 *
 * This function will extract the string, make sure that you apply size rules
 * before in order to avoid bad extraction.
 *
 * \tparam TValue The type of the value
 * \tparam TOutput Control how the rule will output
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, ROutputs TOutput = ROutputs::R_NORMAL>
constexpr ChainedArguments<TValue> RMustValidUtf8(ChainedArguments<TValue>&& args);

/**
 * @}
 */

} // namespace rules
} // namespace fge::net

#include "network_manager.inl"

#endif // _FGE_NETWORK_MANAGER_HPP_INCLUDED

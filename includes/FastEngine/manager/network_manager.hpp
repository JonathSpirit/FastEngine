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

#ifndef _FGE_NETWORK_MANAGER_HPP_INCLUDED
#define _FGE_NETWORK_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_client.hpp"
#include "FastEngine/C_packet.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/object/C_object.hpp"
#include <optional>
#include <variant>

#define FGE_NET_BAD_HEADER 0

namespace fge::net
{

/**
 * \ingroup network
 * @{
 */

/**
 * \typedef PacketHeader
 * \brief Simple aliased type for packets header, useful to differentiate multiple network actions
 */
using PacketHeader = uint16_t;

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
 * \brief Shortcut function that will clear the Packet and write an header to it
 *
 * \param pck The Packet
 * \param header The header that will be written
 * \return A reference to the same Packet
 */
inline fge::net::Packet& SetHeader(fge::net::Packet& pck, fge::net::PacketHeader header);
/**
 * \brief Shortcut function that will retrieve the received Packet header
 *
 * \param pck The Packet
 * \return A packet header
 */
inline fge::net::PacketHeader GetHeader(fge::net::Packet& pck);

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
 * \brief This is a wrapper around a Packet with an optional value
 * \ingroup networkRules
 *
 * This structure go through a rules chain and avoid extracting multiple times
 * the required value.
 *
 * When a rule is invalid, it will invalidate the Packet.
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
     * \return The extracted value or \b nullptr if the Packet is/become invalid
     */
    [[nodiscard]] constexpr TValue* extract();
    /**
     * \brief Peek without changing the read position a certain value
     *
     * \tparam TPeek The type of the value that will be peeked
     * \return A copy of the peeked value
     */
    template<class TPeek>
    [[nodiscard]] constexpr std::optional<TPeek> peek();

    [[nodiscard]] constexpr fge::net::Packet const& packet() const;
    [[nodiscard]] constexpr TValue const& value() const;
    [[nodiscard]] constexpr TValue& value();

    template<class TInvokable>
    [[nodiscard]] constexpr typename std::invoke_result_t<TInvokable, ChainedArguments<TValue>&>
    and_then(TInvokable&& f);
    template<class TInvokable, class TIndex>
    [[nodiscard]] constexpr ChainedArguments<TValue>&
    and_for_each(TIndex iStart, TIndex iEnd, TIndex iIncrement, TInvokable&& f);
    template<class TInvokable, class TIndex>
    [[nodiscard]] constexpr ChainedArguments<TValue>& and_for_each(TIndex iStart, TIndex iIncrement, TInvokable&& f);
    template<class TInvokable>
    constexpr std::optional<Error> on_error(TInvokable&& f);
    [[nodiscard]] constexpr std::optional<Error> end();
    [[nodiscard]] constexpr std::optional<Error> end(std::nullopt_t nullopt) const;
    [[nodiscard]] constexpr std::optional<Error> end(Error&& err) const;

    constexpr ChainedArguments<TValue>& apply(TValue& value);
    template<class TInvokable>
    constexpr ChainedArguments<TValue>& apply(TInvokable&& f);

    template<class TNewValue>
    constexpr ChainedArguments<TNewValue> newChain(TNewValue* existingValue = nullptr);
    template<class TNewValue>
    constexpr ChainedArguments<TNewValue> newChain(TNewValue* existingValue = nullptr) const;

    constexpr ChainedArguments<TValue>& setError(Error&& err);
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

/**
 * \brief Range rule, check if the value is in the min/max range
 *
 * The comparison is not strict for min/max.
 *
 * \tparam TValue The type of the value
 * \tparam TInvertResult Invert the result of the rule
 * \param min The minimum range
 * \param max The maximum range
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, bool TInvertResult = false>
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
 * \tparam TInvertResult Invert the result of the rule
 * \param a The value that will be compared
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, bool TInvertResult = false>
constexpr ChainedArguments<TValue> RMustEqual(TValue const& a, ChainedArguments<TValue>&& args);

/**
 * \brief Strict less rule, check if the value is strictly lesser than the provided one
 *
 * \tparam TValue The type of the value
 * \tparam TInvertResult Invert the result of the rule
 * \param less The value that will be compared
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, bool TInvertResult = false>
constexpr ChainedArguments<TValue> RStrictLess(TValue less, ChainedArguments<TValue>&& args);

/**
 * \brief Less rule, check if the value is lesser than the provided one
 *
 * \tparam TValue The type of the value
 * \tparam TInvertResult Invert the result of the rule
 * \param less The value that will be compared
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, bool TInvertResult = false>
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
 * \tparam TInvertResult Invert the result of the rule
 * \param min The minimum range
 * \param max The maximum range
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, bool TInvertResult = false>
constexpr ChainedArguments<TValue>
RSizeRange(fge::net::SizeType min, fge::net::SizeType max, ChainedArguments<TValue>&& args);

/**
 * \brief Size must equal rule, check if the size is equal to the provided one
 *
 * \see RSizeRange
 *
 * \tparam TValue The type of the value
 * \tparam TInvertResult Invert the result of the rule
 * \param a The value that will be compared
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, bool TInvertResult = false>
constexpr ChainedArguments<TValue> RSizeMustEqual(fge::net::SizeType a, ChainedArguments<TValue>&& args);

/**
 * \brief Check if the extracted string is a valid UTF8 string
 *
 * This function will extract the string, make sure that you apply size rules
 * before in order to avoid bad extraction.
 *
 * \tparam TValue The type of the value
 * \tparam TInvertResult Invert the result of the rule
 * \param args The chained argument
 * \return The chained argument
 */
template<class TValue, bool TInvertResult = false>
constexpr ChainedArguments<TValue> RMustValidUtf8(ChainedArguments<TValue>&& args);

/**
 * @}
 */

} // namespace rules
} // namespace fge::net

#include "network_manager.inl"

#endif // _FGE_NETWORK_MANAGER_HPP_INCLUDED

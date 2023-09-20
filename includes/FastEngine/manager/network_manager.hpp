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

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_client.hpp"
#include "FastEngine/C_packet.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/object/C_object.hpp"
#include <optional>

#define FGE_NET_BAD_HEADER 0

/**
 * \ingroup networkRules
 * @{
 */

/**
 * \brief Macro that indicate the beginning of a network rules chain
 *
 * This will create a scope and a ChainedArguments variable.
 */
#define FGE_NET_RULES_START                                                                                            \
    {                                                                                                                  \
        auto chainedArgs_ =
/**
 * \brief Macro that will check extract the ChainedArguments
 *
 * \see fge::net::rules::ChainedArguments::checkExtract
 */
#define FGE_NET_RULES_CHECK_EXTRACT chainedArgs_.checkExtract()
/**
 * \brief Macro that will check is everything is valid and return if not
 */
#define FGE_NET_RULES_TRY                                                                                              \
    if (!FGE_NET_RULES_CHECK_EXTRACT)                                                                                  \
    {                                                                                                                  \
        return;                                                                                                        \
    }
/**
 * \brief Macro that will check is everything is valid and do something else instead
 *
 * \param else_ The code that will be executed when rules is invalid
 */
#define FGE_NET_RULES_TRY_ELSE(else_)                                                                                  \
    if (!FGE_NET_RULES_CHECK_EXTRACT)                                                                                  \
    {                                                                                                                  \
        else_                                                                                                          \
    }
/**
 * \brief Macro that will get the value stored in the ChainedArguments and move it
 */
#define FGE_NET_RULES_RESULT std::move(chainedArgs_._value.value())
/**
 * \brief Macro that will get the value stored in the ChainedArguments
 */
#define FGE_NET_RULES_RESULT_N chainedArgs_._value.value()
/**
 * \brief Macro that will made everything in order to affect the ChainedArguments value into the provided variable
 *
 * This macro will first check if the packet is valid after Packet extraction and will
 * copy or move the value into the provided variable.
 *
 * \param var_ The variable that will receive the valid value
 */
#define FGE_NET_RULES_AFFECT(var_)                                                                                     \
    FGE_NET_RULES_TRY if constexpr (std::is_move_constructible<decltype(var_)>::value)                                 \
    {                                                                                                                  \
        (var_) = FGE_NET_RULES_RESULT;                                                                                 \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
        (var_) = FGE_NET_RULES_RESULT_N;                                                                               \
    }
/**
 * \brief Macro that will made everything in order to affect the ChainedArguments value into the provided function/method
 *
 * \see FGE_NET_RULES_AFFECT
 *
 * \param setter_ The function to be called in order to affect the valid value
 */
#define FGE_NET_RULES_SETTER(setter_) FGE_NET_RULES_TRY setter_(FGE_NET_RULES_RESULT);
/**
 * \brief Same as FGE_NET_RULES_AFFECT with custom \b else condition
 *
 * \see FGE_NET_RULES_AFFECT
 *
 * \param var_ The variable that will receive the valid value
 * \param else_ The code that will be executed when rules is invalid
 */
#define FGE_NET_RULES_AFFECT_ELSE(var_, else_)                                                                         \
    FGE_NET_RULES_TRY_ELSE(else_) if constexpr (std::is_move_constructible<decltype(var_)>::value)                     \
    {                                                                                                                  \
        (var_) = FGE_NET_RULES_RESULT;                                                                                 \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
        (var_) = FGE_NET_RULES_RESULT_N;                                                                               \
    }
/**
 * \brief Same as FGE_NET_RULES_SETTER with custom \b else condition
 *
 * \see FGE_NET_RULES_AFFECT
 *
 * \param setter_ The function to be called in order to affect the valid value
 * \param else_ The code that will be executed when rules is invalid
 */
#define FGE_NET_RULES_SETTER_ELSE(setter_, else_) FGE_NET_RULES_TRY_ELSE(else_) setter_(FGE_NET_RULES_RESULT);
/**
 * \brief End the rules scope
 */
#define FGE_NET_RULES_END }
#define FGE_NET_RULES_AFFECT_END(var_)                                                                                 \
    FGE_NET_RULES_AFFECT(var_)                                                                                         \
    FGE_NET_RULES_END
#define FGE_NET_RULES_SETTER_END(setter_)                                                                              \
    FGE_NET_RULES_SETTER(setter_)                                                                                      \
    FGE_NET_RULES_END
#define FGE_NET_RULES_AFFECT_END_ELSE(var_, else_)                                                                     \
    FGE_NET_RULES_AFFECT_ELSE(var_, else_)                                                                             \
    FGE_NET_RULES_END
#define FGE_NET_RULES_SETTER_END_ELSE(setter_, else_)                                                                  \
    FGE_NET_RULES_SETTER_ELSE(setter_, else_)                                                                          \
    FGE_NET_RULES_END
/**
 * @}
 */

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
struct ChainedArguments
{
    ChainedArguments() = default;
    ChainedArguments(fge::net::Packet const& pck) :
            _pck(&pck)
    {}
    ChainedArguments(fge::net::Packet const* pck) :
            _pck(pck)
    {}

    fge::net::Packet const* _pck; ///< This should be never null
    std::optional<TValue> _value{std::nullopt};

    /**
     * \brief Make sure that the value is extracted and return \b true if everything is valid
     *
     * \return The validity of the Packet
     */
    bool checkExtract();
    /**
     * \brief Make sure that the value is extracted and return the extracted value
     *
     * \return The extracted value
     */
    TValue& extract();
    /**
     * \brief Peek without changing the read position a certain value
     *
     * \tparam TPeek The type of the value that will be peeked
     * \return A copy of the peeked value
     */
    template<class TPeek>
    TPeek peek();
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
fge::net::rules::ChainedArguments<TValue>
RRange(TValue const& min, TValue const& max, fge::net::rules::ChainedArguments<TValue> args);

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
fge::net::rules::ChainedArguments<TValue> RMustEqual(TValue const& a, fge::net::rules::ChainedArguments<TValue> args);

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
fge::net::rules::ChainedArguments<TValue> RStrictLess(TValue less, fge::net::rules::ChainedArguments<TValue> args);

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
fge::net::rules::ChainedArguments<TValue> RLess(TValue less, fge::net::rules::ChainedArguments<TValue> args);

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
fge::net::rules::ChainedArguments<TValue>
RSizeRange(fge::net::SizeType min, fge::net::SizeType max, fge::net::rules::ChainedArguments<TValue> args);

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
fge::net::rules::ChainedArguments<TValue> RSizeMustEqual(fge::net::SizeType a,
                                                         fge::net::rules::ChainedArguments<TValue> args);

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
fge::net::rules::ChainedArguments<TValue> RMustValidUtf8(fge::net::rules::ChainedArguments<TValue> args);

/**
 * @}
 */

} // namespace rules
} // namespace fge::net

#include "network_manager.inl"

#endif // _FGE_NETWORK_MANAGER_HPP_INCLUDED

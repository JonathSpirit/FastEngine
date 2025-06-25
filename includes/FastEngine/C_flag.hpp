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

#ifndef _FGE_C_FLAG_HPP_INCLUDED
#define _FGE_C_FLAG_HPP_INCLUDED

namespace fge
{

/**
 * \class BooleanFlag
 * \ingroup utility
 * \brief A class to handle a boolean flag
 *
 * A flag becomes \b true only once when the input is held \b true and then waits for the input to return to \b false
 * before being set again.
 */
class BooleanFlag
{
public:
    constexpr BooleanFlag(bool defaultValue = false) :
            g_flag(defaultValue)
    {}

    /**
     * \brief Check the input and return the flag value
     *
     * This function will only return \b true once, and wait for the input to return \b false before being set again.
     *
     * \param input The boolean input to check
     * \return The flag value
     */
    constexpr bool check(bool input)
    {
        if (!this->g_flag)
        {
            this->g_flag = input;
            return input;
        }
        if (!input)
        {
            this->g_flag = false;
        }
        return false;
    }

    /**
     * \brief Manually set the flag value
     *
     * \param value The boolean value to set
     */
    constexpr void set(bool value) { this->g_flag = value; }
    /**
     * \brief Get the flag value
     *
     * \return The flag value
     */
    [[nodiscard]] constexpr bool get() const { return this->g_flag; }

    /**
     * \brief Manually set the flag value operator
     *
     * \param value The boolean value to set
     * \return The flag value
     */
    constexpr bool operator=(bool value) { return this->g_flag = value; }

    /**
     * \brief Get the flag value operator
     *
     * \return The flag value
     */
    constexpr operator bool() const { return this->g_flag; }

private:
    bool g_flag;
};

/**
 * \class EnumFlags
 * \ingroup utility
 * \brief A class to handle "flags" for an enum type
 *
 * This class allows you to manage a set of flags for an enum type, where each flag is represented by a value of the enum.
 * It provides methods to set, unset, toggle flags, check if a flag is set, and retrieve the current flags.
 *
 * \tparam EnumType The enum type for which flags are managed.
 */
template<typename EnumType>
class EnumFlags
{
    static_assert(std::is_enum_v<EnumType>, "EnumFlags can only be used with enum types");

public:
    using Type = std::underlying_type_t<EnumType>;

    constexpr EnumFlags(Type value = Type{0}) :
            g_flags(static_cast<Type>(value))
    {}

    EnumFlags& set(Type flag)
    {
        this->g_flags |= static_cast<Type>(flag);
        return *this;
    }
    EnumFlags& unset(Type flag)
    {
        this->g_flags &= ~static_cast<Type>(flag);
        return *this;
    }
    EnumFlags& toggle(Type flag)
    {
        this->g_flags ^= static_cast<Type>(flag);
        return *this;
    }

    [[nodiscard]] constexpr bool has(Type flag) const { return (this->g_flags & static_cast<Type>(flag)) == flag; }

    [[nodiscard]] constexpr Type get() const { return this->g_flags; }
    constexpr EnumFlags& operator=(Type value)
    {
        this->g_flags = value;
        return *this;
    }

private:
    Type g_flags;
};

template<typename EnumType>
using EnumFlags_t = fge::EnumFlags<EnumType>::Type;

} // namespace fge

#endif // _FGE_C_FLAG_HPP_INCLUDED

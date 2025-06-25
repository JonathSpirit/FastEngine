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

} // namespace fge

#endif // _FGE_C_FLAG_HPP_INCLUDED

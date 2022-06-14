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

#ifndef _FGE_C_FLAG_HPP_INCLUDED
#define _FGE_C_FLAG_HPP_INCLUDED

namespace fge
{

/**
 * \class Flag
 * \ingroup utility
 * \brief A class to handle flags
 *
 * A flag will be set to \b true only once, and wait for the input to be \b false before being set again.
 */
class Flag
{
public:
    inline Flag(bool defaultValue=false) :
        g_flag(defaultValue)
    {}

    /**
     * \brief Check the input and return the flag value
     *
     * This function will only return \b true once, and wait for the input to be \b false before being set again.
     *
     * \param input The boolean input to check
     * \return The flag value
     */
    inline bool check(bool input)
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
    inline void set(bool value)
    {
        this->g_flag = value;
    }
    /**
     * \brief Get the flag value
     *
     * \return The flag value
     */
    [[nodiscard]] inline bool get() const
    {
        return this->g_flag;
    }

    /**
     * \brief Manually set the flag value operator
     *
     * \param value The boolean value to set
     * \return The flag value
     */
    inline bool operator=(bool value)
    {
        return this->g_flag = value;
    }

    /**
     * \brief Get the flag value operator
     *
     * \return The flag value
     */
    inline operator bool() const
    {
        return this->g_flag;
    }

private:
    bool g_flag;
};

}//end fge

#endif // _FGE_C_FLAG_HPP_INCLUDED

/*
 * Copyright 2026 Guillaume Guillet
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

#ifndef _FGE_EXCEPT_HPP_INCLUDED_
#define _FGE_EXCEPT_HPP_INCLUDED_

#include <exception>
#include <string>
#include <string_view>

namespace fge
{

class Exception : public std::exception
{
public:
    explicit Exception(std::string_view sv) :
            g_what(sv)
    {}
    ~Exception() override = default;

    [[nodiscard]] char const* what() const noexcept override { return this->g_what.c_str(); }

private:
    std::string g_what;
};

} // namespace fge

#endif // _FGE_EXCEPT_HPP_INCLUDED_

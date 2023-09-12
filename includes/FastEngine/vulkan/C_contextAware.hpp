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

#ifndef _FGE_VULKAN_C_CONTEXTAWARE_HPP_INCLUDED
#define _FGE_VULKAN_C_CONTEXTAWARE_HPP_INCLUDED

#include "FastEngine/fge_except.hpp"

namespace fge::vulkan
{

class Context;

class ContextAware
{
public:
    explicit constexpr ContextAware(Context const& context) :
            g_context{&context}
    {}
    constexpr ContextAware(ContextAware const& r) = default;
    constexpr ContextAware(ContextAware&& r) noexcept = default;
    constexpr virtual ~ContextAware() = default;

    ContextAware& operator=(ContextAware const& r) = delete;
    ContextAware& operator=(ContextAware&& r) noexcept = delete;

    [[nodiscard]] constexpr Context const& getContext() const { return *this->g_context; }
    inline void swapContext(Context const& context)
    {
        this->destroy();
        this->g_context = &context;
    }

    virtual void destroy() = 0;

protected:
    inline void verifyContext(ContextAware const& r)
    {
        if (this->g_context != r.g_context)
        {
            throw fge::Exception("ContextAware objects assignment with different Context !");
        }
    }

private:
    Context const* g_context{nullptr};
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_CONTEXTAWARE_HPP_INCLUDED

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

#ifndef _FGE_VULKAN_C_BLENDMODE_HPP_INCLUDED
#define _FGE_VULKAN_C_BLENDMODE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include <cstdint>
#include <functional>

namespace fge::vulkan
{

struct BlendMode
{
    constexpr BlendMode() noexcept = default;
    constexpr BlendMode(VkBlendFactor srcBlendFactor,
                        VkBlendFactor dstBlendFactor,
                        VkBlendOp blendOp = VK_BLEND_OP_ADD) noexcept :
            _srcColorBlendFactor(srcBlendFactor),
            _dstColorBlendFactor(dstBlendFactor),
            _colorBlendOp(blendOp),
            _srcAlphaBlendFactor(srcBlendFactor),
            _dstAlphaBlendFactor(dstBlendFactor),
            _alphaBlendOp(blendOp)
    {}
    constexpr BlendMode(VkBlendFactor srcColorBlendFactor,
                        VkBlendFactor dstColorBlendFactor,
                        VkBlendOp colorBlendOp,
                        VkBlendFactor srcAlphaBlendFactor,
                        VkBlendFactor dstAlphaBlendFactor,
                        VkBlendOp alphaBlendOp) noexcept :
            _srcColorBlendFactor(srcColorBlendFactor),
            _dstColorBlendFactor(dstColorBlendFactor),
            _colorBlendOp(colorBlendOp),
            _srcAlphaBlendFactor(srcAlphaBlendFactor),
            _dstAlphaBlendFactor(dstAlphaBlendFactor),
            _alphaBlendOp(alphaBlendOp)
    {}

    [[nodiscard]] constexpr bool operator==(BlendMode const& right) const
    {
        return this->_srcColorBlendFactor == right._srcColorBlendFactor &&
               this->_dstColorBlendFactor == right._dstColorBlendFactor && this->_colorBlendOp == right._colorBlendOp &&
               this->_srcAlphaBlendFactor == right._srcAlphaBlendFactor &&
               this->_dstAlphaBlendFactor == right._dstAlphaBlendFactor && this->_alphaBlendOp == right._alphaBlendOp;
    }
    [[nodiscard]] constexpr bool operator!=(BlendMode const& right) const { return !this->operator==(right); }

    VkBlendFactor _srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor _dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp _colorBlendOp = VK_BLEND_OP_ADD;
    VkBlendFactor _srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor _dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp _alphaBlendOp = VK_BLEND_OP_ADD;
};

constexpr BlendMode BlendAlpha(VK_BLEND_FACTOR_SRC_ALPHA,
                               VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                               VK_BLEND_OP_ADD,
                               VK_BLEND_FACTOR_ONE,
                               VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                               VK_BLEND_OP_ADD); //!< Blend source and dest according to dest alpha
constexpr BlendMode BlendAdd(VK_BLEND_FACTOR_SRC_ALPHA,
                             VK_BLEND_FACTOR_ONE,
                             VK_BLEND_OP_ADD,
                             VK_BLEND_FACTOR_ONE,
                             VK_BLEND_FACTOR_ONE,
                             VK_BLEND_OP_ADD); //!< Add source to dest
constexpr BlendMode
        BlendMultiply(VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD); //!< Multiply source and dest
constexpr BlendMode
        BlendMin(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_MIN); //!< Take minimum between source and dest
constexpr BlendMode
        BlendMax(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_MAX); //!< Take maximum between source and dest
constexpr BlendMode
        BlendNone(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD); //!< Overwrite dest with source

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_BLENDMODE_HPP_INCLUDED

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

#ifndef _FGE_VULKAN_C_BLENDMODE_HPP_INCLUDED
#define _FGE_VULKAN_C_BLENDMODE_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "volk.h"
#include <cstdint>
#include <functional>

namespace fge::vulkan
{

struct BlendMode
{
    BlendMode() noexcept = default;
    BlendMode(VkBlendFactor srcBlendFactor, VkBlendFactor dstBlendFactor, VkBlendOp blendOp = VK_BLEND_OP_ADD) noexcept
            :
            _srcColorBlendFactor(srcBlendFactor),
            _dstColorBlendFactor(dstBlendFactor),
            _colorBlendOp(blendOp),
            _srcAlphaBlendFactor(srcBlendFactor),
            _dstAlphaBlendFactor(dstBlendFactor),
            _alphaBlendOp(blendOp)
    {}
    BlendMode(VkBlendFactor srcColorBlendFactor,
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

    [[nodiscard]] bool operator==(const BlendMode& right) const
    {
        return this->_srcColorBlendFactor == right._srcColorBlendFactor &&
               this->_dstColorBlendFactor == right._dstColorBlendFactor && this->_colorBlendOp == right._colorBlendOp &&
               this->_srcAlphaBlendFactor == right._srcAlphaBlendFactor &&
               this->_dstAlphaBlendFactor == right._dstAlphaBlendFactor && this->_alphaBlendOp == right._alphaBlendOp;
    }
    [[nodiscard]] bool operator!=(const BlendMode& right) const { return !this->operator==(right); }

    VkBlendFactor _srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor _dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp _colorBlendOp = VK_BLEND_OP_ADD;
    VkBlendFactor _srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor _dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp _alphaBlendOp = VK_BLEND_OP_ADD;
};

FGE_API extern const BlendMode BlendAlpha;    //!< Blend source and dest according to dest alpha
FGE_API extern const BlendMode BlendAdd;      //!< Add source to dest
FGE_API extern const BlendMode BlendMultiply; //!< Multiply source and dest
FGE_API extern const BlendMode BlendMin;      //!< Take minimum between source and dest
FGE_API extern const BlendMode BlendMax;      //!< Take maximum between source and dest
FGE_API extern const BlendMode BlendNone;     //!< Overwrite dest with source

struct BlendModeHash
{
    std::size_t operator()(const BlendMode& k) const
    {
        return std::hash<uint32_t>()(
                (static_cast<uint32_t>(k._srcColorBlendFactor) << 25) |
                (static_cast<uint32_t>(k._dstColorBlendFactor) << 20) | (static_cast<uint32_t>(k._colorBlendOp) << 15) |
                (static_cast<uint32_t>(k._srcAlphaBlendFactor) << 10) |
                (static_cast<uint32_t>(k._dstAlphaBlendFactor) << 5) | (static_cast<uint32_t>(k._alphaBlendOp)));
    }
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_BLENDMODE_HPP_INCLUDED

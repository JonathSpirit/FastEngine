#ifndef _FGE_VULKAN_C_BLENDMODE_HPP_INCLUDED
#define _FGE_VULKAN_C_BLENDMODE_HPP_INCLUDED

#include "volk.h"

namespace fge::vulkan
{

struct BlendMode
{
    BlendMode() = default;
    BlendMode(VkBlendFactor srcBlendFactor, VkBlendFactor dstBlendFactor, VkBlendOp blendOp = VK_BLEND_OP_ADD) :
            _srcColorBlendFactor(srcBlendFactor),
            _dstColorBlendFactor(dstBlendFactor),
            _colorBlendOp(blendOp),
            _srcAlphaBlendFactor(srcBlendFactor),
            _dstAlphaBlendFactor(dstBlendFactor),
            _alphaBlendOp(blendOp)
    {}
    BlendMode(VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorBlendOp,
              VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp) :
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
               this->_dstColorBlendFactor == right._dstColorBlendFactor &&
               this->_colorBlendOp == right._colorBlendOp &&
               this->_srcAlphaBlendFactor == right._srcAlphaBlendFactor &&
               this->_dstAlphaBlendFactor == right._dstAlphaBlendFactor &&
               this->_alphaBlendOp == right._alphaBlendOp;
    }
    [[nodiscard]] bool operator!=(const BlendMode& right) const
    {
        return !this->operator==(right);
    }

    VkBlendFactor _srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor _dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp _colorBlendOp = VK_BLEND_OP_ADD;
    VkBlendFactor _srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor _dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp _alphaBlendOp = VK_BLEND_OP_ADD;
};

extern const BlendMode BlendAlpha;    //!< Blend source and dest according to dest alpha
extern const BlendMode BlendAdd;      //!< Add source to dest
extern const BlendMode BlendMultiply; //!< Multiply source and dest
extern const BlendMode BlendMin;      //!< Take minimum between source and dest
extern const BlendMode BlendMax;      //!< Take maximum between source and dest
extern const BlendMode BlendNone;     //!< Overwrite dest with source

}//end fge::vulkan

#endif //_FGE_VULKAN_C_BLENDMODE_HPP_INCLUDED

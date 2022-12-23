#ifndef _FGE_VULKAN_C_DRAWABLE_HPP_INCLUDED
#define _FGE_VULKAN_C_DRAWABLE_HPP_INCLUDED

#include <FastEngine/vulkan/C_graphicPipeline.hpp>
#include <FastEngine/graphic/C_transform.hpp>

namespace fge
{

class RenderTarget;
class RenderStates;

class Drawable
{
public:
    Drawable() = default;
    virtual ~Drawable() = default;

    virtual void draw(RenderTarget& target, const RenderStates& states) const = 0;

protected:
    fge::vulkan::GraphicPipeline _g_graphicPipeline;
};

}//end fge


#endif // _FGE_VULKAN_C_DRAWABLE_HPP_INCLUDED

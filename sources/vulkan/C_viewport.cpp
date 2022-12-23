#include "FastEngine/vulkan/C_viewport.hpp"

namespace fge::vulkan
{

Viewport::Viewport() :
        g_viewport{.x=0.0f, .y=0.0f, .width=0.0f, .height=0.0f, .minDepth=0.0f, .maxDepth=1.0f}
{}
Viewport::Viewport(float x, float y, float width, float height) :
        g_viewport{.x=x, .y=y, .width=width, .height=height, .minDepth=0.0f, .maxDepth=1.0f}
{}

void Viewport::setPosition(float x, float y)
{
    this->g_viewport.x = x;
    this->g_viewport.y = y;
}
void Viewport::setSize(float width, float height)
{
    this->g_viewport.width = width;
    this->g_viewport.height = height;
}

float Viewport::getPositionX() const
{
    return this->g_viewport.x;
}
float Viewport::getPositionY() const
{
    return this->g_viewport.y;
}

float Viewport::getWidth() const
{
    return this->g_viewport.width;
}
float Viewport::getHeight() const
{
    return this->g_viewport.height;
}

const VkViewport& Viewport::getViewport() const
{
    return this->g_viewport;
}

}//end fge::vulkan
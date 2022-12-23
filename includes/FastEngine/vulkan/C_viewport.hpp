#ifndef _FGE_VULKAN_C_VIEWPORT_HPP_INCLUDED
#define _FGE_VULKAN_C_VIEWPORT_HPP_INCLUDED

#include "volk.h"

namespace fge::vulkan
{

class Viewport
{
public:
    Viewport();
    Viewport(float x, float y, float width, float height);
    ~Viewport() = default;

    [[nodiscard]] bool operator==(const Viewport& r) const
    {
        return this->g_viewport.x == r.g_viewport.x &&
                this->g_viewport.y == r.g_viewport.y &&
                this->g_viewport.width == r.g_viewport.width &&
                this->g_viewport.height == r.g_viewport.height;
    }
    [[nodiscard]] bool operator!=(const Viewport& r) const
    {
        return !this->operator==(r);
    }

    void setPosition(float x, float y);
    void setSize(float width, float height);

    [[nodiscard]] float getPositionX() const;
    [[nodiscard]] float getPositionY() const;

    [[nodiscard]] float getWidth() const;
    [[nodiscard]] float getHeight() const;

    [[nodiscard]] const VkViewport& getViewport() const;

private:
    VkViewport g_viewport;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_VIEWPORT_HPP_INCLUDED

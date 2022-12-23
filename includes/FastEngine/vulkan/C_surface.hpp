#ifndef _FGE_VULKAN_C_SURFACE_HPP_INCLUDED
#define _FGE_VULKAN_C_SURFACE_HPP_INCLUDED

#include "SDL_vulkan.h"
#include "volk.h"

namespace fge::vulkan
{

class Instance;

class Surface
{
public:
    Surface();
    Surface(const Surface& r) = delete;
    Surface(Surface&& r) noexcept;
    ~Surface();

    Surface& operator=(const Surface& r) = delete;
    Surface& operator=(Surface&& r) noexcept = delete;

    void create(Instance& instance);
    void destroy();

    [[nodiscard]] VkSurfaceKHR getSurface() const;

    [[nodiscard]] Instance& getInstance();
    [[nodiscard]] const Instance& getInstance() const;

private:
    VkSurfaceKHR g_surface;
    Instance* g_instance;
};

}//end vulkan

#endif //_FGE_VULKAN_C_SURFACE_HPP_INCLUDED

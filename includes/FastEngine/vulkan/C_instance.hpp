#ifndef _FGE_VULKAN_C_INSTANCE_HPP_INCLUDED
#define _FGE_VULKAN_C_INSTANCE_HPP_INCLUDED

#include <string>
#include <vector>
#include "SDL_vulkan.h"
#include "volk.h"
#include <glm/glm.hpp>
#include "C_physicalDevice.hpp"

namespace fge::vulkan
{

class Instance
{
public:
    Instance();
    Instance(const Instance& r) = delete;
    Instance(Instance&& r) noexcept;
    ~Instance();

    Instance& operator=(const Instance& r) = delete;
    Instance& operator=(Instance&& r) noexcept = delete;

    void create(SDL_Window* window, std::string applicationName, uint16_t versionMajor=1, uint16_t versionMinor=0, uint16_t versionPatch=0);
    void destroy();

    [[nodiscard]] const std::string& getApplicationName() const;

    [[nodiscard]] VkInstance getInstance() const;
    [[nodiscard]] SDL_Window* getWindow() const;

    [[nodiscard]] glm::vec<2, int> getWindowSize() const;

    [[nodiscard]] const std::vector<PhysicalDevice>& getPhysicalDevices() const;
    [[nodiscard]] PhysicalDevice pickPhysicalDevice(VkSurfaceKHR surface);

    static bool checkValidationLayerSupport();

private:
    void enumeratePhysicalDevices();

    VkInstance g_instance;
    std::string g_applicationName;

    SDL_Window* g_window;

    std::vector<PhysicalDevice> g_physicalDevices;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_INSTANCE_HPP_INCLUDED

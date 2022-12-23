#ifndef _FGE_VULKAN_C_TRANSFORM_HPP_INCLUDED
#define _FGE_VULKAN_C_TRANSFORM_HPP_INCLUDED

#include <glm/glm.hpp>

namespace fge
{

struct Transform
{
    alignas(16) glm::mat4 _modelTransform{1.0f};
    alignas(16) glm::mat4 _viewTransform{1.0f};
};

}//end fge

#endif // _FGE_VULKAN_C_TRANSFORM_HPP_INCLUDED

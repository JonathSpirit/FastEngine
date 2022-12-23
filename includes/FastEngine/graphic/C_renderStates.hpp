#ifndef _FGE_VULKAN_C_RENDERSTATES_HPP_INCLUDED
#define _FGE_VULKAN_C_RENDERSTATES_HPP_INCLUDED

#include <glm/glm.hpp>

namespace fge
{

namespace vulkan
{

class TextureImage;

}//end vulkan

class Transformable;

class RenderStates
{
public:
    RenderStates() = default;
    explicit RenderStates(const fge::Transformable* transformable) :
            _transformable(transformable)
    {}
    explicit RenderStates(const fge::vulkan::TextureImage* textureImage) :
            _textureImage(textureImage)
    {}
    RenderStates(const fge::Transformable* transformable, const fge::vulkan::TextureImage* textureImage) :
            _transformable(transformable),
            _textureImage(textureImage)
    {}

    //glm::mat4 _modelTransform{1.0f};
    const fge::Transformable* _transformable{nullptr};
    const fge::vulkan::TextureImage* _textureImage{nullptr};
};

}//end fge

#endif // _FGE_VULKAN_C_RENDERSTATES_HPP_INCLUDED

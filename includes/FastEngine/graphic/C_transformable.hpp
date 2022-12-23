#ifndef _FGE_VULKAN_C_TRANSFORMABLE_HPP_INCLUDED
#define _FGE_VULKAN_C_TRANSFORMABLE_HPP_INCLUDED

#include "glm/glm.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/graphic/C_vector.hpp"
#include "FastEngine/graphic/C_transform.hpp"

namespace fge
{

class Transformable
{
public:
    Transformable();
    virtual ~Transformable();

    void setPosition(const Vector2f& position);
    const Vector2f& getPosition() const;
    void move(const Vector2f& offset);

    void setRotation(float angle);
    float getRotation() const;
    void rotate(float angle);

    void setScale(const Vector2f& factors);
    const Vector2f& getScale() const;
    void scale(const Vector2f& factor);

    void setOrigin(const Vector2f& origin);
    const Vector2f& getOrigin() const;

    const glm::mat4& getTransform() const;
    const glm::mat4& getInverseTransform() const;

    void destroy();

    void setViewMatrix(const glm::mat4& viewMatrix) const;
    void updateUniformBuffer(const fge::vulkan::Context& context) const;
    [[nodiscard]] const fge::vulkan::DescriptorSet& getDescriptorSet() const;

private:
    Vector2f          g_origin;                     //!< Origin of translation/rotation/scaling of the object
    Vector2f          g_position;                   //!< Position of the object in the 2D world
    float             g_rotation;                   //!< Orientation of the object, in degrees
    Vector2f          g_scale;                      //!< Scale of the object
    mutable fge::Transform g_transform;             //!< Combined transformation of the object
    mutable bool      g_transformNeedUpdate;        //!< Does the transform need to be recomputed?
    mutable glm::mat4 g_inverseTransform;           //!< Combined transformation of the object
    mutable bool      g_inverseTransformNeedUpdate; //!< Does the transform need to be recomputed?

    mutable fge::vulkan::DescriptorSet g_descriptorSet;
    mutable fge::vulkan::UniformBuffer g_uniformBuffer;
    mutable bool g_uniformBufferNeedUpdate;
};

}//end fge


#endif // _FGE_VULKAN_C_TRANSFORMABLE_HPP_INCLUDED
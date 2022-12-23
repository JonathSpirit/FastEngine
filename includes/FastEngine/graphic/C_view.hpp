#ifndef _FGE_VULKAN_C_VIEW_HPP_INCLUDED
#define _FGE_VULKAN_C_VIEW_HPP_INCLUDED

#include <glm/glm.hpp>
#include <FastEngine/graphic/C_vector.hpp>
#include <FastEngine/vulkan/C_viewport.hpp>
#include <FastEngine/graphic/C_rect.hpp>

namespace fge
{

class View
{
public:
    View();
    explicit View(const fge::vulkan::Viewport& viewport);
    View(const Vector2f& center, const Vector2f& size);

    void setCenter(const Vector2f& center);
    const Vector2f& getCenter() const;

    void setSize(const Vector2f& size);
    const Vector2f& getSize() const;

    void setRotation(float angleDeg);
    float getRotation() const;

    void setFactorViewport(const fge::RectFloat& factorViewport);
    const fge::RectFloat& getFactorViewport() const;

    void reset(const fge::vulkan::Viewport& viewport);
    void move(const Vector2f& offset);
    void rotate(float angle);
    void zoom(float factor);

    const glm::mat4& getTransform() const;
    const glm::mat4& getInverseTransform() const;

private:
    Vector2f g_center;              //!< Center of the view, in scene coordinates
    Vector2f g_size;                //!< Size of the view, in scene coordinates
    float g_rotation;            //!< Angle of rotation of the view rectangle, in degrees
    fge::RectFloat g_factorViewport;            //!< Viewport rectangle, expressed as a factor of the render-target's size
    mutable glm::mat4 g_transform;           //!< Precomputed projection transform corresponding to the view
    mutable glm::mat4 g_inverseTransform;    //!< Precomputed inverse projection transform corresponding to the view
    mutable bool g_transformUpdated;    //!< Internal state telling if the transform needs to be updated
    mutable bool g_invTransformUpdated; //!< Internal state telling if the inverse transform needs to be updated
};

}//end fge


#endif //_FGE_VULKAN_C_VIEW_HPP_INCLUDED
/*
 * Copyright 2024 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FGE_GRAPHIC_C_VIEW_HPP_INCLUDED
#define _FGE_GRAPHIC_C_VIEW_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_vector.hpp"
#include "FastEngine/vulkan/C_viewport.hpp"

namespace fge
{

/**
 * \class View
 * \ingroup graphics
 * \brief Define a camera in a 2D scene
 *
 * A view is a 2D camera that defines what part of the 2D scene is visible.
 */
class FGE_API View
{
public:
    View();
    explicit View(fge::vulkan::Viewport const& viewport);
    View(Vector2f const& center, Vector2f const& size);

    void setCenter(Vector2f const& center);
    Vector2f const& getCenter() const;

    /**
     * \brief Set the size of the view
     *
     * A size smaller than the target will display a zoomed area,
     * while a size greater than the target will show a bigger area.
     *
     * \param size New size of the view
     */
    void setSize(Vector2f const& size);
    Vector2f const& getSize() const;

    void setRotation(float angleDeg);
    float getRotation() const;

    /**
     * \brief Set the viewport rectangle of the view
     *
     * The viewport is the rectangle into which the contents of the view are displayed,
     * expressed as a factor (between 0 and 1) of the size of the RenderTarget to which the view is applied.
     *
     * For example, a view which takes the left side of the target would be defined by:
     * \code view.setViewport({{0.0f, 0.0f}, {0.5f, 1.0f}}); \endcode
     *
     * \param factorViewport New viewport rectangle, expressed as a factor of the render-target's size
     */
    void setFactorViewport(fge::RectFloat const& factorViewport);
    fge::RectFloat const& getFactorViewport() const;

    /**
     * \brief Reset the view to the given viewport
     *
     * The size and center of the view are adjusted so that the entire viewport is visible.
     * The rotation is reset to 0.
     *
     * \param viewport New viewport to apply to the view
     */
    void reset(fge::vulkan::Viewport const& viewport);
    /**
     * \brief Helper function to move the view
     *
     * \param offset Offset to apply to the view
     */
    void move(Vector2f const& offset);
    /**
     * \brief Helper function to rotate the view
     *
     * \param angle Angle to apply to the view, in degrees
     */
    void rotate(float angle);
    /**
     * \brief Helper function to zoom the view
     *
     * \param factor Zoom factor to apply to the view
     */
    void zoom(float factor);
    /**
     * \brief Resize the view and move it relatively to old center in order to expand it
     *
     * \param newSize New size of the view
     */
    void resizeFixCenter(fge::Vector2f const& newSize);

    /**
     * \brief Get the combined transform of the view
     *
     * This also apply a orthogonal projection matrix.
     *
     * \return A glm::mat4 of the view
     */
    [[nodiscard]] glm::mat4 const& getTransform() const;
    [[nodiscard]] glm::mat4 const& getInverseTransform() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix() const;
    [[nodiscard]] glm::mat4 getInverseProjectionMatrix() const;

private:
    fge::Vector2f g_center;
    fge::Vector2f g_size;
    float g_rotation; //!< Rotation angle of the view, in degrees
    fge::RectFloat g_factorViewport;
    mutable glm::mat4 g_transform;
    mutable glm::mat4 g_inverseTransform;
    mutable bool g_transformUpdated;
    mutable bool g_invTransformUpdated;
};

} // namespace fge


#endif //_FGE_GRAPHIC_C_VIEW_HPP_INCLUDED
/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_GRAPHIC_C_TRANSFORMABLE_HPP_INCLUDED
#define _FGE_GRAPHIC_C_TRANSFORMABLE_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_vector.hpp"

namespace fge
{

class FGE_API Transformable
{
public:
    Transformable();
    virtual ~Transformable() = default;

    void setPosition(Vector2f const& position);
    Vector2f const& getPosition() const;
    void move(Vector2f const& offset);

    void setRotation(float angle);
    float getRotation() const;
    void rotate(float angle);

    void setScale(Vector2f const& factors);
    void setScale(float factor);
    Vector2f const& getScale() const;
    void scale(Vector2f const& factor);
    void scale(float factor);

    void setOrigin(Vector2f const& origin);
    Vector2f const& getOrigin() const;

    glm::mat4 const& getTransform() const;
    glm::mat4 const& getInverseTransform() const;

private:
    Vector2f g_origin;                         //!< Origin of translation/rotation/scaling of the object
    Vector2f g_position;                       //!< Position of the object in the 2D world
    float g_rotation;                          //!< Orientation of the object, in degrees
    Vector2f g_scale;                          //!< Scale of the object
    mutable glm::mat4 g_transform;             //!< Combined transformation of the object
    mutable bool g_transformNeedUpdate;        //!< Does the transform need to be recomputed?
    mutable glm::mat4 g_inverseTransform;      //!< Combined transformation of the object
    mutable bool g_inverseTransformNeedUpdate; //!< Does the transform need to be recomputed?
};

} // namespace fge


#endif // _FGE_GRAPHIC_C_TRANSFORMABLE_HPP_INCLUDED
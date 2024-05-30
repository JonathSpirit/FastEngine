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

#ifndef _FGE_GRAPHIC_C_TRANSFORM_HPP_INCLUDED
#define _FGE_GRAPHIC_C_TRANSFORM_HPP_INCLUDED

#include "FastEngine/C_vector.hpp"

namespace fge
{

struct TransformUboData
{
    alignas(16) mutable glm::mat4 _modelTransform{1.0f};
    alignas(16) mutable glm::mat4 _viewTransform{1.0f};

    constexpr static unsigned int uboSize = sizeof(_modelTransform) + sizeof(_viewTransform);
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_TRANSFORM_HPP_INCLUDED

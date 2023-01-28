/*
 * Copyright 2022 Guillaume Guillet
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

#include "FastEngine/vulkan/C_blendMode.hpp"

namespace fge::vulkan
{

const BlendMode BlendAlpha(VK_BLEND_FACTOR_SRC_ALPHA,
                           VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                           VK_BLEND_OP_ADD,
                           VK_BLEND_FACTOR_ONE,
                           VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                           VK_BLEND_OP_ADD);
const BlendMode BlendAdd(VK_BLEND_FACTOR_SRC_ALPHA,
                         VK_BLEND_FACTOR_ONE,
                         VK_BLEND_OP_ADD,
                         VK_BLEND_FACTOR_ONE,
                         VK_BLEND_FACTOR_ONE,
                         VK_BLEND_OP_ADD);
const BlendMode BlendMultiply(VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
const BlendMode BlendMin(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_MIN);
const BlendMode BlendMax(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_MAX);
const BlendMode BlendNone(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

} // namespace fge::vulkan
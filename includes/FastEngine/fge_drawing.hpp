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

#ifndef _FGE_DRAWING_HPP_INCLUDED_
#define _FGE_DRAWING_HPP_INCLUDED_
#ifndef FGE_DEF_SERVER

    #include <FastEngine/fastengine_extern.hpp>

namespace fge
{
namespace debug
{

/** TODO
FGE_API void DrawCenteredCircle(sf::RenderTarget& target,
                                const sf::Vector2f& position,
                                float radius,
                                const sf::Color& fillColor,
                                const sf::Color& outColor,
                                float outThickness);

FGE_API void DrawCenteredRect(sf::RenderTarget& target,
                              const sf::Vector2f& position,
                              const sf::Vector2f& size,
                              const sf::Color& fillColor,
                              const sf::Color& outColor,
                              float outThickness);
**/

} // namespace debug
} // namespace fge

#endif //FGE_DEF_SERVER
#endif // _FGE_DRAWING_HPP_INCLUDED_

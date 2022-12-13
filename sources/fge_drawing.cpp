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

#include "FastEngine/fge_drawing.hpp"

namespace fge
{
namespace debug
{

void DrawCenteredCircle(sf::RenderTarget& target,
                        const sf::Vector2f& position,
                        float radius,
                        const sf::Color& fillColor,
                        const sf::Color& outColor,
                        float outThickness)
{
    sf::CircleShape circle(radius);
    circle.setPosition(position);
    circle.setOrigin(radius, radius);
    circle.setFillColor(fillColor);
    circle.setOutlineColor(outColor);
    circle.setOutlineThickness(outThickness);
    target.draw(circle);
}

void DrawCenteredRect(sf::RenderTarget& target,
                      const sf::Vector2f& position,
                      const sf::Vector2f& size,
                      const sf::Color& fillColor,
                      const sf::Color& outColor,
                      float outThickness)
{
    sf::RectangleShape rect(size);
    rect.setPosition(position);
    rect.setOrigin(size.x / 2.0f, size.y / 2.0f);
    rect.setFillColor(fillColor);
    rect.setOutlineColor(outColor);
    rect.setOutlineThickness(outThickness);
    target.draw(rect);
}

} // namespace debug
} // namespace fge

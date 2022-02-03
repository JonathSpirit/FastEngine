#include "FastEngine/fge_drawing.hpp"

namespace fge
{
namespace debug
{

void FGE_API DrawCenteredCircle(sf::RenderTarget& target, const sf::Vector2f& position, float radius,
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

void FGE_API DrawCenteredRect(sf::RenderTarget& target, const sf::Vector2f& position, const sf::Vector2f& size,
                              const sf::Color& fillColor,
                              const sf::Color& outColor,
                              float outThickness)
{
    sf::RectangleShape rect(size);
    rect.setPosition(position);
    rect.setOrigin(size.x/2.0f, size.y/2.0f);
    rect.setFillColor(fillColor);
    rect.setOutlineColor(outColor);
    rect.setOutlineThickness(outThickness);
    target.draw(rect);
}

}//end debug
}//end fge

#ifndef _FGE_DRAWING_HPP_INCLUDED_
#define _FGE_DRAWING_HPP_INCLUDED_

#include <FastEngine/fastengine_extern.hpp>
#include <SFML/Graphics.hpp>

namespace fge
{
namespace debug
{

FGE_API void DrawCenteredCircle(sf::RenderTarget& target, const sf::Vector2f& position, float radius,
                                const sf::Color& fillColor,
                                const sf::Color& outColor,
                                float outThickness);

FGE_API void DrawCenteredRect(sf::RenderTarget& target, const sf::Vector2f& position, const sf::Vector2f& size,
                              const sf::Color& fillColor,
                              const sf::Color& outColor,
                              float outThickness);

}//end debug
}//end fge

#endif // _FGE_DRAWING_HPP_INCLUDED_

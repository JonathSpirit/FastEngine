#include <C_customObject.hpp>
#include <definition.hpp>
#include <FastEngine/C_random.hpp>
#include <FastEngine/extra/extra_function.hpp>

namespace ls
{

bool CustomObject::worldTick()
{
    return false;
}

sf::Vector2f ClampToMapLimit(const sf::Vector2f& position)
{
    return {std::clamp<float>(position.x, LIFESIM_MAP_SIZE_MINX, LIFESIM_MAP_SIZE_MAXX),
            std::clamp<float>(position.y, LIFESIM_MAP_SIZE_MINY, LIFESIM_MAP_SIZE_MAXY)};
}
sf::Vector2f GetRandomPositionFromCenter(const sf::Vector2f& center, float maxDistance)
{
    sf::Vector2f forwardVector = fge::GetForwardVector( fge::_random.range<float>(0.0f, 360.0f) );
    forwardVector *= fge::_random.range<float>(0.0f, maxDistance);

    return ls::ClampToMapLimit(forwardVector + center);
}
sf::Vector2f GetRandomPosition()
{
    sf::Vector2f position( fge::_random.rangeVec2(LIFESIM_MAP_SIZE_MINX, LIFESIM_MAP_SIZE_MAXX,
                                                  LIFESIM_MAP_SIZE_MINY, LIFESIM_MAP_SIZE_MAXY) );

    return position;
}

}//end ls

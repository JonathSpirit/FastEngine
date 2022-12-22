#include <C_moveable.hpp>
#include <FastEngine/extra/extra_function.hpp>

namespace ls
{

void Moveable::setTargetPos(const sf::Vector2f& pos)
{
    this->_g_targetPos = pos;
    this->_g_finish = false;
}
bool Moveable::updateMoveable(sf::Transformable& transformable, const std::chrono::milliseconds& deltaTime)
{
    if (this->_g_finish)
    {
        return true;
    }
    float deltaTimeFloat = fge::DurationToSecondFloat(deltaTime);
    transformable.setPosition(fge::ReachVector(transformable.getPosition(), this->_g_targetPos, 60.0f, deltaTimeFloat));
    if (transformable.getPosition() == this->_g_targetPos)
    {
        this->_g_finish = true;
        return true;
    }
    return false;
}

} // namespace ls

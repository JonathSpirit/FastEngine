#ifndef _FGE_C_LIGHTOBSTACLE_HPP_INCLUDED
#define _FGE_C_LIGHTOBSTACLE_HPP_INCLUDED

#include <SFML/System/Vector2.hpp>
#include <FastEngine/C_lightSystem.hpp>
#include <vector>

namespace fge
{

class ObjLight;

using ListOfPoints = std::vector<sf::Vector2f>;

/**
 * \class LightObstacle
 * \ingroup graphics
 * \brief A base class to define an obstacle for the light system
 *
 * An obstacle is a group of points that define the shape of the object.
 */
class LightObstacle : public fge::ObstacleComponent
{
    LightObstacle() :
            fge::ObstacleComponent(this)
    {}
    LightObstacle(const fge::LightObstacle& r) :
            fge::ObstacleComponent(r),
            _g_myPoints(r._g_myPoints)
    {
        this->_g_lightSystemGate.setData(this);
    }

    fge::LightObstacle& operator=(const fge::LightObstacle& r)
    {
        this->_g_lightSystemGate = r._g_lightSystemGate;
        this->_g_myPoints = r._g_myPoints;
        this->_g_lightSystemGate.setData(this);
        return *this;
    }

protected:
    fge::ListOfPoints _g_myPoints;

    friend class fge::ObjLight;
};

}//end fge

#endif // _FGE_C_LIGHTOBSTACLE_HPP_INCLUDED

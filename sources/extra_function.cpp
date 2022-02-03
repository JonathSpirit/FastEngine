#include "FastEngine/extra_function.hpp"

#include <cmath>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <SFML/System/Vector2.hpp>
#include <re2/re2.h>

namespace fge
{

namespace
{
    /**CONVEX HULL**/
    // Implementation of Andrew's monotone chain 2D convex hull algorithm.
    // Asymptotic complexity: O(n log n).
    // Practical performance: 0.5-1.0 seconds for n=1000000 on a 1GHz machine.

    /**
    Compare function for the vector
    **/
    bool CompareVector(const sf::Vector2f &a, const sf::Vector2f &b)
    {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    }

    /**
    2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
    Returns a positive value, if OAB makes a counter-clockwise turn,
    negative for clockwise turn, and zero if the points are col-linear.
    **/
    double GetCrossProductVector(const sf::Vector2f &O, const sf::Vector2f &A, const sf::Vector2f &B)
    {
        return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
    }
}

///Utility

bool FGE_API IsEngineBuiltInDebugMode()
{
    #ifdef ___DEBUG_MODE_
        return true;
    #else
        return false;
    #endif // ___DEBUG_MODE_
}

std::size_t FGE_API GetFilesInFolder(std::list<std::string>& buffer, const std::string& path, const std::string& regexFilter, bool ignoreDirectory, bool onlyFilename, bool recursive)
{
    RE2 re(regexFilter);
    if ( !re.ok() )
    {
        return 0;
    }

    std::size_t actualSize = buffer.size();
    if (recursive)
    {
        for (const auto & entry : std::filesystem::recursive_directory_iterator(path))
        {
            if (ignoreDirectory && entry.is_directory())
            {
                continue;
            }

            if ( RE2::FullMatch(entry.path().filename().string(), re) )
            {
                if (onlyFilename)
                {
                    buffer.push_back( entry.path().filename().string() );
                }
                else
                {
                    buffer.push_back( entry.path().string() );
                }
            }
        }
    }
    else
    {
        for (const auto & entry : std::filesystem::directory_iterator(path))
        {
            if (ignoreDirectory && entry.is_directory())
            {
                continue;
            }

            if ( RE2::FullMatch(entry.path().filename().string(), re) )
            {
                if (onlyFilename)
                {
                    buffer.push_back( entry.path().filename().string() );
                }
                else
                {
                    buffer.push_back( entry.path().string() );
                }
            }
        }
    }

    return buffer.size() - actualSize;
}

///Detection
bool FGE_API IsMouseOn(const sf::RenderWindow& window, const sf::FloatRect& zone)
{
    return zone.contains( window.mapPixelToCoords( sf::Mouse::getPosition(window) ) );
}
bool FGE_API IsMouseOn(const sf::Vector2f& mousePos, const sf::FloatRect& zone)
{
    return zone.contains( mousePos );
}

bool FGE_API IsPressed(const sf::RenderWindow& window, const sf::FloatRect& zone, sf::Mouse::Button button)
{
    if ( zone.contains( window.mapPixelToCoords( sf::Mouse::getPosition(window) ) ) )
    {
        return sf::Mouse::isButtonPressed(button);
    }
    return false;
}
bool FGE_API IsPressed (const fge::Event& evt, const sf::Vector2f& mouse_pos, const sf::FloatRect& zone, sf::Mouse::Button button)
{
    if ( zone.contains( mouse_pos ) )
    {
        return evt.isMouseButtonPressed(button);
    }
    return false;
}

///Reach
sf::Vector2f FGE_API ReachVector(const sf::Vector2f& position, const sf::Vector2f& target, float speed, float deltaTime)
{
    float travelDistance = speed * deltaTime;
    sf::Vector2 direction = fge::NormalizeVector2(target-position);
    float actualDistance = fge::GetDistanceBetween(position, target);

    if (travelDistance >= actualDistance)
    {//We already reached the target
        return target;
    }
    return position + direction*travelDistance;
}
float FGE_API ReachRotation(float rotation, float target, float speed, float deltaTime, fge::TurnMode turnMode)
{
    rotation = static_cast<float>( std::fmod(rotation, 360) );
    if (rotation < 0)
    {
        rotation += 360.0f;
    }
    target = static_cast<float>( std::fmod(target, 360) );
    if (target < 0)
    {
        target += 360.0f;
    }

    float travelDistance = speed * deltaTime;
    float actualDistance;

    switch (turnMode)
    {
    case fge::TurnMode::TURN_ANTICLOCKWISE:
        actualDistance = rotation - target;
        if (actualDistance < 0)
        {
            actualDistance += 360.0f;
        }
        break;
    case fge::TurnMode::TURN_CLOCKWISE:
        actualDistance = target - rotation;
        if (actualDistance < 0)
        {
            actualDistance += 360.0f;
        }
        break;
    default:
        {
            float antiClockwiseDistance = rotation - target;
            if (antiClockwiseDistance < 0)
            {
                antiClockwiseDistance += 360.0f;
            }

            float clockwiseDistance = target - rotation;
            if (clockwiseDistance < 0)
            {
                clockwiseDistance += 360.0f;
            }

            if ( clockwiseDistance < antiClockwiseDistance )
            {
                actualDistance = clockwiseDistance;
                turnMode = fge::TurnMode::TURN_CLOCKWISE;
            }
            else
            {
                actualDistance = antiClockwiseDistance;
                turnMode = fge::TurnMode::TURN_ANTICLOCKWISE;
            }
        }
        break;
    }

    if (travelDistance >= actualDistance)
    {//We already reached the target
        return target;
    }

    if (turnMode == fge::TurnMode::TURN_ANTICLOCKWISE)
    {//anti-clockwise
        rotation -= travelDistance;
    }
    else
    {//clockwise
        rotation += travelDistance;
    }

    rotation = static_cast<float>( std::fmod(rotation, 360) );
    if (rotation < 0)
    {
        return rotation + 360.0f;
    }
    else
    {
        return rotation;
    }
}

///2D Math
float FGE_API ConvertRadToDeg(float rad)
{
    return static_cast<float>(std::fmod( (rad * 180.0f / M_PI) + 360.0f, 360.0f ));
}
float FGE_API ConvertDegToRad(float deg)
{
    return deg * M_PI / 180.0f;
}

float FGE_API GetDeterminant(const sf::Vector2f& vecCol1, const sf::Vector2f& vecCol2)
{
    return vecCol1.x*vecCol2.y - vecCol1.y*vecCol2.x;
}
float FGE_API GetDotProduct(const sf::Vector2f& vec1, const sf::Vector2f& vec2)
{
    return vec1.x*vec2.x + vec1.y*vec2.y;
}
float FGE_API GetMagnitude(const sf::Vector2f& vec)
{
    return std::sqrt(vec.x*vec.x + vec.y*vec.y);
}
float FGE_API GetRotation(const sf::Vector2f& vec)
{
    return fge::ConvertRadToDeg( std::atan2(vec.y, vec.x) );
}
float FGE_API GetRotationBetween(const sf::Vector2f& vec1, const sf::Vector2f& vec2)
{
    return fge::ConvertRadToDeg( std::atan2(fge::GetDeterminant(vec1, vec2), fge::GetDotProduct(vec1, vec2)) );
}
float FGE_API GetDistanceBetween(const sf::Vector2f& pos1, const sf::Vector2f& pos2)
{
    return fge::GetMagnitude(pos2 - pos1);
}

sf::Vector2f FGE_API GetForwardVector(float rotation)
{
    rotation *= M_PI / 180.0f;
    return sf::Vector2f( std::cos(rotation), std::sin(rotation) );
}
sf::Vector2f FGE_API GetBackwardVector(float rotation)
{
    rotation *= M_PI / 180.0f;
    return -sf::Vector2f( std::cos(rotation), std::sin(rotation) );
}
sf::Vector2f FGE_API GetLeftVector(float rotation)
{
    rotation = (rotation-90.0f) * M_PI / 180.0f;
    return sf::Vector2f( std::cos(rotation), std::sin(rotation) );
}
sf::Vector2f FGE_API GetRightVector(float rotation)
{
    rotation = (rotation+90.0f) * M_PI / 180.0f;
    return sf::Vector2f( std::cos(rotation), std::sin(rotation) );
}

void FGE_API GetConvexHull(const std::vector<sf::Vector2f>& input, std::vector<sf::Vector2f>& output)
{
	size_t n = input.size(), k = 0;
	if (n <= 3)
    {
        output = input;
        return;
    }

    std::vector<sf::Vector2f> listOfPoint(input);

    output.resize(2*n);

	// Sort points lexicographically
    std::sort(listOfPoint.begin(), listOfPoint.end(), CompareVector);

	// Build lower hull
	for (size_t i = 0; i < n; ++i)
    {
		while (k >= 2 && GetCrossProductVector(output[k-2], output[k-1], listOfPoint[i]) <= 0)
        {
            --k;
        }
		output[k++] = listOfPoint[i];
	}

	// Build upper hull
	for (size_t i = n-1, t = k+1; i > 0; --i)
    {
		while (k >= t && GetCrossProductVector(output[k-2], output[k-1], listOfPoint[i-1]) <= 0)
		{
		    --k;
		}
		output[k++] = listOfPoint[i-1];
	}

	output.resize(k-1);
}

///View
sf::Vector2f FGE_API GetViewSizePercentage(const sf::View& view, const sf::View& defaultView)
{
    return sf::Vector2f( (view.getSize().x*100.0f) / defaultView.getSize().x, (view.getSize().y*100.0f) / defaultView.getSize().y );
}
sf::Vector2f FGE_API SetViewSizePercentage(float percentage, const sf::View& defaultView)
{
    return sf::Vector2f( (percentage*defaultView.getSize().x)/100.0f, (percentage*defaultView.getSize().y)/100.0f );
}
sf::Vector2f FGE_API SetViewSizePercentage(const sf::Vector2f& percentage, const sf::View& defaultView)
{
    return sf::Vector2f( (percentage.x*defaultView.getSize().x)/100.0f, (percentage.y*defaultView.getSize().y)/100.0f );
}

///Render
sf::IntRect FGE_API CoordToPixelRect(const sf::FloatRect& rect, const sf::RenderTarget& target)
{
    sf::Vector2i positions[4] =
    {
        target.mapCoordsToPixel( sf::Vector2f(rect.left, rect.top) ),
        target.mapCoordsToPixel( sf::Vector2f(rect.left+rect.width, rect.top) ),
        target.mapCoordsToPixel( sf::Vector2f(rect.left, rect.top+rect.height) ),
        target.mapCoordsToPixel( sf::Vector2f(rect.left+rect.width, rect.top+rect.height) )
    };

    return fge::ToRect(positions, 4);
}
sf::IntRect FGE_API CoordToPixelRect(const sf::FloatRect& rect, const sf::RenderTarget& target, const sf::View& view)
{
    sf::Vector2i positions[4] =
    {
        target.mapCoordsToPixel( sf::Vector2f(rect.left, rect.top), view ),
        target.mapCoordsToPixel( sf::Vector2f(rect.left+rect.width, rect.top), view ),
        target.mapCoordsToPixel( sf::Vector2f(rect.left, rect.top+rect.height), view ),
        target.mapCoordsToPixel( sf::Vector2f(rect.left+rect.width, rect.top+rect.height), view )
    };

    return fge::ToRect(positions, 4);
}
sf::FloatRect FGE_API PixelToCoordRect(const sf::IntRect& rect, const sf::RenderTarget& target)
{
    sf::Vector2f positions[4] =
    {
        target.mapPixelToCoords( sf::Vector2i(rect.left, rect.top) ),
        target.mapPixelToCoords( sf::Vector2i(rect.left+rect.width, rect.top) ),
        target.mapPixelToCoords( sf::Vector2i(rect.left, rect.top+rect.height) ),
        target.mapPixelToCoords( sf::Vector2i(rect.left+rect.width, rect.top+rect.height) )
    };

    return fge::ToRect(positions, 4);
}
sf::FloatRect FGE_API PixelToCoordRect(const sf::IntRect& rect, const sf::RenderTarget& target, const sf::View& view)
{
    sf::Vector2f positions[4] =
    {
        target.mapPixelToCoords( sf::Vector2i(rect.left, rect.top), view ),
        target.mapPixelToCoords( sf::Vector2i(rect.left+rect.width, rect.top), view ),
        target.mapPixelToCoords( sf::Vector2i(rect.left, rect.top+rect.height), view ),
        target.mapPixelToCoords( sf::Vector2i(rect.left+rect.width, rect.top+rect.height), view )
    };

    return fge::ToRect(positions, 4);
}

sf::FloatRect FGE_API GetScreenRect(const sf::RenderTarget& target)
{
    sf::Vector2f positions[4] =
    {
        target.mapPixelToCoords( sf::Vector2i(0,0) ),
        target.mapPixelToCoords( sf::Vector2i(target.getSize().x,0) ),
        target.mapPixelToCoords( sf::Vector2i(0,target.getSize().y) ),
        target.mapPixelToCoords( sf::Vector2i(target.getSize().x,target.getSize().y) )
    };

    return fge::ToRect(positions, 4);
}
sf::FloatRect FGE_API GetScreenRect(const sf::RenderTarget& target, const sf::View& view)
{
    sf::Vector2f positions[4] =
    {
        target.mapPixelToCoords( sf::Vector2i(0,0), view ),
        target.mapPixelToCoords( sf::Vector2i(target.getSize().x,0), view ),
        target.mapPixelToCoords( sf::Vector2i(0,target.getSize().y), view ),
        target.mapPixelToCoords( sf::Vector2i(target.getSize().x,target.getSize().y), view )
    };

    return fge::ToRect(positions, 4);
}

///Json
bool FGE_API LoadJsonFromFile(const std::string& path, nlohmann::json& j)
{
    std::ifstream file(path);
    if ( !file )
    {
        file.close();
        return false;
    }

    try
    {
        file >> j;
        file.close();
        return true;
    }
    catch (std::exception& e)
    {
        file.close();
        return false;
    }
}
bool FGE_API SaveJsonToFile(const std::string& path, const nlohmann::json& j, int fieldWidth)
{
    std::ofstream file(path);
    if ( file )
    {
        file << std::setw(fieldWidth) << j << std::endl;
        file.close();
        return true;
    }
    file.close();
    return false;
}

}//end fge

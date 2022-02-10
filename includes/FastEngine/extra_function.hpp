#ifndef _FGE_EXTRA_FUNCTION_HPP_INCLUDED
#define _FGE_EXTRA_FUNCTION_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_object.hpp>
#include <SFML/Graphics.hpp>
#include <json.hpp>

namespace fge
{

enum TurnMode
{
    TURN_CLOCKWISE,
    TURN_ANTICLOCKWISE,

    TURN_AUTO
};

///Utility
inline char UnicodeToChar(uint32_t unicode);

bool FGE_API IsEngineBuiltInDebugMode();

std::size_t FGE_API GetFilesInFolder(std::list<std::string>& buffer, const std::string& path, const std::string& regexFilter=".+", bool ignoreDirectory=true, bool onlyFilename=true, bool recursive=false);

///Detection
bool FGE_API IsMouseOn(const sf::RenderWindow& window, const sf::FloatRect& zone);
bool FGE_API IsMouseOn(const sf::Vector2f& mousePos, const sf::FloatRect& zone);

bool FGE_API IsPressed(const sf::RenderWindow& window, const sf::FloatRect& zone, sf::Mouse::Button button=sf::Mouse::Button::Left);
bool FGE_API IsPressed(const fge::Event& evt, const sf::Vector2f& mouse_pos, const sf::FloatRect& zone, sf::Mouse::Button button=sf::Mouse::Button::Left);

///Position/Rectangle
template<typename T>
sf::Rect<T> ToRect(const sf::Vector2<T>& pos1, const sf::Vector2<T>& pos2);
template<typename T>
sf::Rect<T> ToRect(const std::vector<sf::Vector2<T> >& pos);
template<typename T>
sf::Rect<T> ToRect(const sf::Vector2<T>* pos, std::size_t size);

///Color
inline sf::Color SetAlpha(const sf::Color& color, uint8_t alpha);
inline sf::Color SetRed(const sf::Color& color, uint8_t red);
inline sf::Color SetGreen(const sf::Color& color, uint8_t green);
inline sf::Color SetBlue(const sf::Color& color, uint8_t blue);

inline sf::Color&& SetAlpha(sf::Color&& color, uint8_t alpha);
inline sf::Color&& SetRed(sf::Color&& color, uint8_t red);
inline sf::Color&& SetGreen(sf::Color&& color, uint8_t green);
inline sf::Color&& SetBlue(sf::Color&& color, uint8_t blue);

///Reach
sf::Vector2f FGE_API ReachVector(const sf::Vector2f& position, const sf::Vector2f& target, float speed, float deltaTime);
float FGE_API ReachRotation(float rotation, float target, float speed, float deltaTime, fge::TurnMode turnMode);

template<typename T>
T ReachValue( T value, T target, T speed, float deltaTime );

///2D Math
float FGE_API ConvertRadToDeg(float rad);
float FGE_API ConvertDegToRad(float deg);

float FGE_API GetDeterminant(const sf::Vector2f& vecCol1, const sf::Vector2f& vecCol2);
float FGE_API GetDotProduct(const sf::Vector2f& vec1, const sf::Vector2f& vec2);
float FGE_API GetMagnitude(const sf::Vector2f& vec);
float FGE_API GetRotation(const sf::Vector2f& vec);
float FGE_API GetRotationBetween(const sf::Vector2f& vec1, const sf::Vector2f& vec2);
float FGE_API GetDistanceBetween(const sf::Vector2f& pos1, const sf::Vector2f& pos2);

template<typename T>
sf::Vector2f NormalizeVector2(const sf::Vector2<T>& vec);

template<typename TIterator>
TIterator GetNearestVector(const sf::Vector2f& vec, const TIterator& pointsBegin, const TIterator& pointsEnd);

sf::Vector2f FGE_API GetForwardVector(float rotation);
sf::Vector2f FGE_API GetBackwardVector(float rotation);
sf::Vector2f FGE_API GetLeftVector(float rotation);
sf::Vector2f FGE_API GetRightVector(float rotation);

/*
Implementation of Andrew's monotone chain 2D convex hull algorithm.
Asymptotic complexity: O(n log n).
Practical performance: 0.5-1.0 seconds for n=1000000 on a 1GHz machine.
*/
void FGE_API GetConvexHull(const std::vector<sf::Vector2f>& input, std::vector<sf::Vector2f>& output);

///View
sf::Vector2f FGE_API GetViewSizePercentage(const sf::View& view, const sf::View& defaultView);
sf::Vector2f FGE_API SetViewSizePercentage(float percentage, const sf::View& defaultView);
sf::Vector2f FGE_API SetViewSizePercentage(const sf::Vector2f& percentage, const sf::View& defaultView);

///Render
sf::IntRect FGE_API CoordToPixelRect(const sf::FloatRect& rect, const sf::RenderTarget& target);
sf::IntRect FGE_API CoordToPixelRect(const sf::FloatRect& rect, const sf::RenderTarget& target, const sf::View& view);
sf::FloatRect FGE_API PixelToCoordRect(const sf::IntRect& rect, const sf::RenderTarget& target);
sf::FloatRect FGE_API PixelToCoordRect(const sf::IntRect& rect, const sf::RenderTarget& target, const sf::View& view);

sf::FloatRect FGE_API GetScreenRect(const sf::RenderTarget& target);
sf::FloatRect FGE_API GetScreenRect(const sf::RenderTarget& target, const sf::View& view);

///Time
inline float DurationToSecondFloat(auto duration);

///Json
bool FGE_API LoadJsonFromFile(const std::string& path, nlohmann::json& j);
bool FGE_API SaveJsonToFile(const std::string& path, const nlohmann::json& j, int fieldWidth=2);

}//end fge

#include <FastEngine/extra_function.inl>

#endif // _FGE_EXTRA_FUNCTION_HPP_INCLUDED

#ifndef _FGE_EXTRA_FUNCTION_HPP_INCLUDED
#define _FGE_EXTRA_FUNCTION_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_object.hpp>
#include <SFML/Graphics.hpp>
#include <json.hpp>

#define FGE_MATH_PI 3.14159265358979323846

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

FGE_API bool IsEngineBuiltInDebugMode();

FGE_API std::size_t GetFilesInFolder(std::list<std::string>& buffer, const std::string& path, const std::string& regexFilter=".+", bool ignoreDirectory=true, bool onlyFilename=true, bool recursive=false);

///Detection
FGE_API bool IsMouseOn(const sf::RenderWindow& window, const sf::FloatRect& zone);
FGE_API bool IsMouseOn(const sf::Vector2f& mousePos, const sf::FloatRect& zone);

FGE_API bool IsPressed(const sf::RenderWindow& window, const sf::FloatRect& zone, sf::Mouse::Button button=sf::Mouse::Button::Left);
FGE_API bool IsPressed(const fge::Event& evt, const sf::Vector2f& mouse_pos, const sf::FloatRect& zone, sf::Mouse::Button button=sf::Mouse::Button::Left);

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
FGE_API sf::Vector2f ReachVector(const sf::Vector2f& position, const sf::Vector2f& target, float speed, float deltaTime);
FGE_API float ReachRotation(float rotation, float target, float speed, float deltaTime, fge::TurnMode turnMode);

template<typename T>
T ReachValue( T value, T target, T speed, float deltaTime );

///2D Math
FGE_API float ConvertRadToDeg(float rad);
FGE_API float ConvertDegToRad(float deg);

FGE_API float GetDeterminant(const sf::Vector2f& vecCol1, const sf::Vector2f& vecCol2);
FGE_API float GetDotProduct(const sf::Vector2f& vec1, const sf::Vector2f& vec2);
FGE_API float GetMagnitude(const sf::Vector2f& vec);
FGE_API float GetRotation(const sf::Vector2f& vec);
FGE_API float GetRotationBetween(const sf::Vector2f& vec1, const sf::Vector2f& vec2);
FGE_API float GetDistanceBetween(const sf::Vector2f& pos1, const sf::Vector2f& pos2);

template<typename T>
sf::Vector2f NormalizeVector2(const sf::Vector2<T>& vec);

template<typename TIterator>
TIterator GetNearestVector(const sf::Vector2f& vec, const TIterator& pointsBegin, const TIterator& pointsEnd);

FGE_API sf::Vector2f GetForwardVector(float rotation);
FGE_API sf::Vector2f GetBackwardVector(float rotation);
FGE_API sf::Vector2f GetLeftVector(float rotation);
FGE_API sf::Vector2f GetRightVector(float rotation);

/*
Implementation of Andrew's monotone chain 2D convex hull algorithm.
Asymptotic complexity: O(n log n).
Practical performance: 0.5-1.0 seconds for n=1000000 on a 1GHz machine.
*/
FGE_API void GetConvexHull(const std::vector<sf::Vector2f>& input, std::vector<sf::Vector2f>& output);

///View
FGE_API sf::Vector2f GetViewSizePercentage(const sf::View& view, const sf::View& defaultView);
FGE_API sf::Vector2f SetViewSizePercentage(float percentage, const sf::View& defaultView);
FGE_API sf::Vector2f SetViewSizePercentage(const sf::Vector2f& percentage, const sf::View& defaultView);

///Render
FGE_API sf::IntRect CoordToPixelRect(const sf::FloatRect& rect, const sf::RenderTarget& target);
FGE_API sf::IntRect CoordToPixelRect(const sf::FloatRect& rect, const sf::RenderTarget& target, const sf::View& view);
FGE_API sf::FloatRect PixelToCoordRect(const sf::IntRect& rect, const sf::RenderTarget& target);
FGE_API sf::FloatRect PixelToCoordRect(const sf::IntRect& rect, const sf::RenderTarget& target, const sf::View& view);

FGE_API sf::FloatRect GetScreenRect(const sf::RenderTarget& target);
FGE_API sf::FloatRect GetScreenRect(const sf::RenderTarget& target, const sf::View& view);

///Time
template<class T>
inline float DurationToSecondFloat(T duration);

///Json
FGE_API bool LoadJsonFromFile(const std::string& path, nlohmann::json& j);
FGE_API bool SaveJsonToFile(const std::string& path, const nlohmann::json& j, int fieldWidth=2);

}//end fge

#include <FastEngine/extra_function.inl>

#endif // _FGE_EXTRA_FUNCTION_HPP_INCLUDED

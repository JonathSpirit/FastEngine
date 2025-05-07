/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_EXTRA_FUNCTION_HPP_INCLUDED
#define _FGE_EXTRA_FUNCTION_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_event.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/graphic/C_color.hpp"
#include "FastEngine/graphic/C_renderTarget.hpp"
#include "FastEngine/graphic/C_view.hpp"
#include "SDL_mouse.h"
#include "json.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"
#include <array>
#include <filesystem>
#include <list>

#define FGE_MATH_SQRT2 1.41421356237309504880
#define FGE_MATH_PI 3.14159265358979323846

namespace fge
{

class ObjectData;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;

struct Line
{
    inline Line() = default;
    inline Line(fge::Vector2f const& start, fge::Vector2f const& end) :
            _start(start),
            _end(end)
    {}
    inline Line(fge::Vector2f const& origin, fge::Vector2f const& direction, float length) :
            _start(origin),
            _end(origin + direction * length)
    {}

    [[nodiscard]] inline fge::Vector2f getDirection() const { return glm::normalize(this->_end - this->_start); }
    [[nodiscard]] inline float getLength() const { return glm::length(this->_end - this->_start); }

    fge::Vector2f _start;
    fge::Vector2f _end;
};

struct Intersection
{
    fge::Vector2f _point;
    float _normA;
    float _normB;
};

enum TurnMode
{
    TURN_CLOCKWISE,
    TURN_ANTICLOCKWISE,

    TURN_AUTO
};

///Utility
[[nodiscard]] inline char UnicodeToChar(uint32_t unicode);

[[nodiscard]] FGE_API bool IsEngineBuiltInDebugMode();

FGE_API bool SetSystemCursor(SDL_SystemCursor id);

FGE_API std::size_t GetFilesInFolder(std::list<std::string>& buffer,
                                     std::filesystem::path const& path,
                                     std::string const& regexFilter = ".+",
                                     bool ignoreDirectory = true,
                                     bool onlyFilename = true,
                                     bool recursive = false);

FGE_API bool SetVirtualTerminalSequenceSupport();
FGE_API void SetConsoleCmdTitle(char const* title);

[[nodiscard]] FGE_API void* AlignedAlloc(std::size_t size, std::size_t alignment);
FGE_API void AlignedFree(void* data);

FGE_API void Sleep(std::chrono::microseconds time);

struct AlignedDeleter
{
    void operator()(void* p) const { AlignedFree(p); };
};

/*
Implementation of Austin Appleby MurmurHash2 algorithm.
https://sites.google.com/site/murmurhash/
public domain
*/
[[nodiscard]] FGE_API std::size_t Hash(void const* key, std::size_t len, std::size_t seed = 0xc70f6907UL);

template<typename TFloat>
[[nodiscard]] inline TFloat LimitRangeAngle(TFloat angleDegree);

///Detection
#ifndef FGE_DEF_SERVER
[[nodiscard]] FGE_API bool IsMouseOn(fge::RenderTarget const& target, fge::RectFloat const& zone);
[[nodiscard]] FGE_API bool IsMouseOn(fge::Vector2f const& mousePos, fge::RectFloat const& zone);

[[nodiscard]] FGE_API bool IsPressed(fge::Event const& evt,
                                     fge::Vector2f const& mouse_pos,
                                     fge::RectFloat const& zone,
                                     uint8_t button = SDL_BUTTON_LEFT);
#endif //FGE_DEF_SERVER

enum class IntersectionOptions
{
    I_NORM_LIMITS,
    I_STRICT_NORM_LIMITS,
    I_NO_NORM_LIMITS,

    I_DEFAULT = I_NORM_LIMITS
};

[[nodiscard]] FGE_API std::optional<fge::Intersection>
CheckIntersection(fge::Line const& lineA,
                  fge::Line const& lineB,
                  IntersectionOptions option = IntersectionOptions::I_DEFAULT);
[[nodiscard]] FGE_API std::optional<fge::Intersection>
CheckIntersection(fge::Vector2f const& position,
                  fge::Vector2f const& direction,
                  fge::Line const& line,
                  IntersectionOptions option = IntersectionOptions::I_DEFAULT);

[[nodiscard]] inline bool IsVertexInCone(fge::Line const& line1,
                                         fge::Line const& line2,
                                         fge::Vector2f const& origin,
                                         fge::Vector2f const& vertex);
[[nodiscard]] inline bool IsVertexInCone(float coneAngle,
                                         fge::Vector2f const& direction,
                                         fge::Vector2f const& origin,
                                         fge::Vector2f const& vertex);

///Position/Rectangle
template<typename T>
[[nodiscard]] fge::Rect<T> ToRect(fge::Vector2<T> const& pos1, fge::Vector2<T> const& pos2);
template<typename T>
[[nodiscard]] fge::Rect<T> ToRect(std::vector<fge::Vector2<T>> const& pos);
template<typename T>
[[nodiscard]] fge::Rect<T> ToRect(fge::Vector2<T> const* pos, std::size_t size);

///Color
[[nodiscard]] inline fge::Color SetAlpha(fge::Color color, uint8_t alpha);
[[nodiscard]] inline fge::Color SetRed(fge::Color color, uint8_t red);
[[nodiscard]] inline fge::Color SetGreen(fge::Color color, uint8_t green);
[[nodiscard]] inline fge::Color SetBlue(fge::Color color, uint8_t blue);

///Reach
[[nodiscard]] FGE_API fge::Vector2f
ReachVector(fge::Vector2f const& position, fge::Vector2f const& target, float speed, float deltaTime);
[[nodiscard]] FGE_API float
ReachRotation(float rotation, float target, float speed, float deltaTime, fge::TurnMode turnMode);

template<typename T>
[[nodiscard]] T ReachValue(T value, T target, T speed, float deltaTime);

///2D Math
[[nodiscard]] inline constexpr float Cross2d(fge::Vector2f const& vec1, fge::Vector2f const& vec2);
[[nodiscard]] inline fge::Vector2f GetSegmentNormal(fge::Vector2f const& vec1, fge::Vector2f const& vec2);
[[nodiscard]] inline constexpr float GetAngle(fge::Vector2f const& vec);
[[nodiscard]] inline constexpr float GetAngleBetween(fge::Vector2f const& vec1, fge::Vector2f const& vec2);
[[nodiscard]] inline float GetDistanceBetween(fge::Vector2f const& vec1, fge::Vector2f const& vec2);
[[nodiscard]] inline float
GetShortestDistanceBetween(fge::Vector2f const& point, fge::Vector2f const& lineStart, fge::Vector2f const& lineEnd);

template<typename TIterator>
[[nodiscard]] TIterator
GetNearestPoint(fge::Vector2f const& point, TIterator const& pointsBegin, TIterator const& pointsEnd);

[[nodiscard]] inline constexpr fge::Vector2f GetForwardVector(float angle);
[[nodiscard]] inline constexpr fge::Vector2f GetBackwardVector(float angle);
[[nodiscard]] inline constexpr fge::Vector2f GetLeftVector(float angle);
[[nodiscard]] inline constexpr fge::Vector2f GetRightVector(float angle);

[[nodiscard]] inline constexpr float DotSquare(fge::Vector2f const& vec);

[[nodiscard]] inline constexpr float
GetHandedness(fge::Vector2f const& vec1, fge::Vector2f const& vec2, fge::Vector2f const& vec3);

//Convert a 'x' value ranging from xMin-Xmax to a 'y' value ranging from yMin-Ymax
[[nodiscard]] inline constexpr float ConvertRange(float x, float xMin, float xMax, float yMin, float yMax);
[[nodiscard]] inline constexpr fge::Vector2f ConvertRange(fge::Vector2f const& x,
                                                          fge::Vector2f const& xMin,
                                                          fge::Vector2f const& xMax,
                                                          fge::Vector2f const& yMin,
                                                          fge::Vector2f const& yMax);

[[nodiscard]] inline constexpr fge::Vector2f MapCircleToSquareCoords(fge::Vector2f const& circleCoords);
[[nodiscard]] inline constexpr fge::Vector2f MapSquareToCircleCoords(fge::Vector2f const& squareCoords);

/*
Implementation of Andrew's monotone chain 2D convex hull algorithm.
Asymptotic complexity: O(n log n).
Practical performance: 0.5-1.0 seconds for n=1000000 on a 1GHz machine.
*/
FGE_API void GetConvexHull(std::vector<fge::Vector2f> const& input, std::vector<fge::Vector2f>& output);

///View
[[nodiscard]] FGE_API fge::Vector2f GetViewSizePercentage(fge::View const& view, fge::View const& defaultView);
[[nodiscard]] FGE_API fge::Vector2f SetViewSizePercentage(float percentage, fge::View const& defaultView);
[[nodiscard]] FGE_API fge::Vector2f SetViewSizePercentage(fge::Vector2f const& percentage,
                                                          fge::View const& defaultView);

[[nodiscard]] FGE_API fge::Vector2f
TransposePointFromAnotherView(fge::View const& pointView, fge::Vector2f const& point, fge::View const& newView);

enum class ClipClampModes
{
    CLIP_CLAMP_NOTHING,
    CLIP_CLAMP_STRETCH,
    CLIP_CLAMP_PUSH,
    CLIP_CLAMP_HIDE
};
[[nodiscard]] FGE_API fge::View ClipView(fge::View const& view,
                                         fge::RenderTarget const& target,
                                         fge::RectFloat const& worldCoordClipRect,
                                         fge::ClipClampModes clampMode);

///Render
[[nodiscard]] FGE_API fge::RectFloat GetScreenRect(fge::RenderTarget const& target);
[[nodiscard]] FGE_API fge::RectFloat GetScreenRect(fge::RenderTarget const& target, fge::View const& view);

///Time
template<class T>
[[nodiscard]] inline float DurationToSecondFloat(T duration);

///Json
FGE_API bool LoadJsonFromFile(std::filesystem::path const& path, nlohmann::json& j);
FGE_API bool LoadOrderedJsonFromFile(std::filesystem::path const& path, nlohmann::ordered_json& j);
FGE_API bool SaveJsonToFile(std::filesystem::path const& path, nlohmann::json const& j, int fieldWidth = 2);

///Path
[[nodiscard]] FGE_API std::filesystem::path MakeRelativePathToBasePathIfExist(std::filesystem::path const& basePath,
                                                                              std::filesystem::path const& path);

} // namespace fge

#include "extra_function.inl"

#endif // _FGE_EXTRA_FUNCTION_HPP_INCLUDED

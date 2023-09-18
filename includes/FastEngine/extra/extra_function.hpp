/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_event.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/graphic/C_color.hpp"
#include "FastEngine/graphic/C_renderTarget.hpp"
#include "FastEngine/graphic/C_view.hpp"
#include "SDL_mouse.h"
#include "json.hpp"
#include <array>
#include <filesystem>
#include <list>

#define FGE_MATH_PI 3.14159265358979323846

namespace fge
{

using Quad = std::array<fge::Vector2f, 4>;

enum TurnMode
{
    TURN_CLOCKWISE,
    TURN_ANTICLOCKWISE,

    TURN_AUTO
};

///Utility
inline char UnicodeToChar(uint32_t unicode);

FGE_API bool IsEngineBuiltInDebugMode();

FGE_API bool SetSystemCursor(SDL_SystemCursor id);

FGE_API std::size_t GetFilesInFolder(std::list<std::string>& buffer,
                                     const std::filesystem::path& path,
                                     const std::string& regexFilter = ".+",
                                     bool ignoreDirectory = true,
                                     bool onlyFilename = true,
                                     bool recursive = false);

FGE_API bool SetVirtualTerminalSequenceSupport();
FGE_API void SetConsoleCmdTitle(const char* title);

FGE_API void* AlignedAlloc(std::size_t size, std::size_t alignment);
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
FGE_API std::size_t Hash(const void* key, std::size_t len, std::size_t seed = 0xc70f6907UL);

///Detection
#ifndef FGE_DEF_SERVER
FGE_API bool IsMouseOn(const fge::RenderTarget& target, const fge::RectFloat& zone);
FGE_API bool IsMouseOn(const fge::Vector2f& mousePos, const fge::RectFloat& zone);

FGE_API bool IsPressed(const fge::Event& evt,
                       const fge::Vector2f& mouse_pos,
                       const fge::RectFloat& zone,
                       uint8_t button = SDL_BUTTON_LEFT);
#endif //FGE_DEF_SERVER

FGE_API bool IsContained(fge::Quad const& quad, fge::Vector2f const& point);

///Position/Rectangle
template<typename T>
fge::Rect<T> ToRect(const fge::Vector2<T>& pos1, const fge::Vector2<T>& pos2);
template<typename T>
fge::Rect<T> ToRect(const std::vector<fge::Vector2<T>>& pos);
template<typename T>
fge::Rect<T> ToRect(const fge::Vector2<T>* pos, std::size_t size);

///Color
inline fge::Color SetAlpha(fge::Color color, uint8_t alpha);
inline fge::Color SetRed(fge::Color color, uint8_t red);
inline fge::Color SetGreen(fge::Color color, uint8_t green);
inline fge::Color SetBlue(fge::Color color, uint8_t blue);

///Reach
FGE_API fge::Vector2f
ReachVector(const fge::Vector2f& position, const fge::Vector2f& target, float speed, float deltaTime);
FGE_API float ReachRotation(float rotation, float target, float speed, float deltaTime, fge::TurnMode turnMode);

template<typename T>
T ReachValue(T value, T target, T speed, float deltaTime);

///2D Math
inline constexpr float Cross2d(fge::Vector2f const& vec1, fge::Vector2f const& vec2);
inline fge::Vector2f GetSegmentNormal(fge::Vector2f const& vec1, fge::Vector2f const& vec2);
inline constexpr float GetAngle(fge::Vector2f const& vec);
inline constexpr float GetAngleBetween(fge::Vector2f const& vec1, fge::Vector2f const& vec2);
inline float GetDistanceBetween(fge::Vector2f const& vec1, fge::Vector2f const& vec2);
inline float
GetShortestDistanceBetween(fge::Vector2f const& point, fge::Vector2f const& lineStart, fge::Vector2f const& lineEnd);

template<typename TIterator>
TIterator GetNearestPoint(fge::Vector2f const& point, TIterator const& pointsBegin, TIterator const& pointsEnd);

inline constexpr fge::Vector2f GetForwardVector(float angle);
inline constexpr fge::Vector2f GetBackwardVector(float angle);
inline constexpr fge::Vector2f GetLeftVector(float angle);
inline constexpr fge::Vector2f GetRightVector(float angle);

/*
Implementation of Andrew's monotone chain 2D convex hull algorithm.
Asymptotic complexity: O(n log n).
Practical performance: 0.5-1.0 seconds for n=1000000 on a 1GHz machine.
*/
FGE_API void GetConvexHull(std::vector<fge::Vector2f> const& input, std::vector<fge::Vector2f>& output);

///View
FGE_API fge::Vector2f GetViewSizePercentage(const fge::View& view, const fge::View& defaultView);
FGE_API fge::Vector2f SetViewSizePercentage(float percentage, const fge::View& defaultView);
FGE_API fge::Vector2f SetViewSizePercentage(const fge::Vector2f& percentage, const fge::View& defaultView);

FGE_API fge::Vector2f
TransposePointFromAnotherView(const fge::View& pointView, const fge::Vector2f& point, const fge::View& newView);

enum class ClipClampModes
{
    CLIP_CLAMP_NOTHING,
    CLIP_CLAMP_STRETCH,
    CLIP_CLAMP_PUSH,
    CLIP_CLAMP_HIDE
};
FGE_API fge::View ClipView(const fge::View& view,
                           const fge::RenderTarget& target,
                           const fge::RectFloat& worldCoordClipRect,
                           fge::ClipClampModes clampMode);

///Render
FGE_API fge::RectInt CoordToPixelRect(const fge::RectFloat& rect, const fge::RenderTarget& target);
FGE_API fge::RectInt
CoordToPixelRect(const fge::RectFloat& rect, const fge::RenderTarget& target, const fge::View& view);
FGE_API fge::RectFloat PixelToCoordRect(const fge::RectInt& rect, const fge::RenderTarget& target);
FGE_API fge::RectFloat
PixelToCoordRect(const fge::RectInt& rect, const fge::RenderTarget& target, const fge::View& view);

FGE_API fge::RectFloat GetScreenRect(const fge::RenderTarget& target);
FGE_API fge::RectFloat GetScreenRect(const fge::RenderTarget& target, const fge::View& view);

///Time
template<class T>
inline float DurationToSecondFloat(T duration);

///Json
FGE_API bool LoadJsonFromFile(const std::filesystem::path& path, nlohmann::json& j);
FGE_API bool SaveJsonToFile(const std::filesystem::path& path, const nlohmann::json& j, int fieldWidth = 2);

} // namespace fge

#include "extra_function.inl"

#endif // _FGE_EXTRA_FUNCTION_HPP_INCLUDED

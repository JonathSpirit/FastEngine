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

#include "FastEngine/extra/extra_function.hpp"

#include "glm/gtx/perpendicular.hpp"
#include "re2.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <timeapi.h>
    #include <windows.h>
#endif //_WIN32

namespace fge
{

namespace
{

inline bool CompareVector(const fge::Vector2f& a, const fge::Vector2f& b)
{
    return a.x < b.x || (a.x == b.x && a.y < b.y);
}

inline float GetCrossProductVector(const fge::Vector2f& O, const fge::Vector2f& A, const fge::Vector2f& B)
{
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

} // namespace

///Utility

bool IsEngineBuiltInDebugMode()
{
#ifdef FGE_DEF_DEBUG
    return true;
#else
    return false;
#endif // FGE_DEF_DEBUG
}

bool SetSystemCursor(SDL_SystemCursor id)
{
    struct CursorDeleter
    {
        void operator()(SDL_Cursor* cursor) const { SDL_FreeCursor(cursor); };
    };

    static std::unordered_map<SDL_SystemCursor, std::unique_ptr<SDL_Cursor, CursorDeleter>> cursors;

    auto it = cursors.find(id);
    if (it != cursors.end())
    {
        SDL_SetCursor(it->second.get());
        return true;
    }

    SDL_Cursor* newCursor = SDL_CreateSystemCursor(id);
    if (newCursor == nullptr)
    {
        SDL_SetCursor(SDL_GetDefaultCursor());
    }
    else
    {
        cursors[id].reset(newCursor);
        SDL_SetCursor(newCursor);
        return true;
    }
    return false;
}

std::size_t GetFilesInFolder(std::list<std::string>& buffer,
                             const std::filesystem::path& path,
                             const std::string& regexFilter,
                             bool ignoreDirectory,
                             bool onlyFilename,
                             bool recursive)
{
    RE2 re(regexFilter);
    if (!re.ok())
    {
        return 0;
    }

    std::size_t actualSize = buffer.size();
    if (recursive)
    {
        for (const auto& entry: std::filesystem::recursive_directory_iterator(path))
        {
            if (ignoreDirectory && entry.is_directory())
            {
                continue;
            }

            if (RE2::FullMatch(entry.path().filename().string(), re))
            {
                if (onlyFilename)
                {
                    buffer.push_back(entry.path().filename().string());
                }
                else
                {
                    buffer.push_back(entry.path().string());
                }
            }
        }
    }
    else
    {
        for (const auto& entry: std::filesystem::directory_iterator(path))
        {
            if (ignoreDirectory && entry.is_directory())
            {
                continue;
            }

            if (RE2::FullMatch(entry.path().filename().string(), re))
            {
                if (onlyFilename)
                {
                    buffer.push_back(entry.path().filename().string());
                }
                else
                {
                    buffer.push_back(entry.path().string());
                }
            }
        }
    }

    return buffer.size() - actualSize;
}

bool SetVirtualTerminalSequenceSupport()
{
#ifdef _WIN32
    HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (stdHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD dwMode;
    if (GetConsoleMode(stdHandle, &dwMode) == 0)
    {
        return false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    return SetConsoleMode(stdHandle, dwMode) != 0;
#else
    return true;
#endif //_WIN32
}
void SetConsoleCmdTitle(const char* title)
{
    if (title == nullptr)
    {
        return;
    }

#ifdef _WIN32
    HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (stdHandle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD dwMode;
    if (GetConsoleMode(stdHandle, &dwMode) == 0)
    {
        return;
    }

    if ((dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) > 0)
    {
        std::cout << "\033]0;" << title << "\007";
    }
    else
    {
        /*
         * From MSDN :
         * https://learn.microsoft.com/en-us/windows/console/setconsoletitle
         *
         * This document describes console platform functionality that is no longer a part of our ecosystem roadmap.
         * We do not recommend using this content in new products, but we will continue to support existing usages
         * for the indefinite future. Our preferred modern solution focuses on virtual terminal sequences for
         * maximum compatibility in cross-platform scenarios. You can find more information about this design decision
         * in our classic console vs. virtual terminal document.
         */

    #ifdef SetConsoleTitle
        SetConsoleTitle(title);
    #endif
    }
#else
    std::cout << "\033]0;" << title << "\007";
#endif //_WIN32
}

void* AlignedAlloc(std::size_t size, std::size_t alignment)
{
    void* data = nullptr;

#if defined(_MSC_VER) || defined(__MINGW32__)
    data = _aligned_malloc(size, alignment);
#else
    int res = posix_memalign(&data, alignment, size);
    if (res != 0)
    {
        data = nullptr;
    }
#endif
    return data;
}
void AlignedFree(void* data)
{
    if (data != nullptr)
    {
#if defined(_MSC_VER) || defined(__MINGW32__)
        _aligned_free(data);
#else
        free(data);
#endif
    }
}

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */
void Sleep(std::chrono::microseconds time)
{
    if (time == std::chrono::microseconds{0})
    {
        return;
    }

#ifdef _WIN32
    // Get the supported timer resolutions on this system
    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(TIMECAPS));

    // Set the timer resolution to the minimum for the Sleep call
    timeBeginPeriod(tc.wPeriodMin);

    // Wait...
    ::Sleep(static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(time).count()));

    // Reset the timer resolution back to the system default
    timeEndPeriod(tc.wPeriodMin);
#else
    int64_t usecs = time.count();

    // Construct the time to wait
    timespec ti;
    ti.tv_nsec = static_cast<long>((usecs % 1000000) * 1000);
    ti.tv_sec = static_cast<time_t>(usecs / 1000000);

    // Wait...
    // If nanosleep returns -1, we check errno. If it is EINTR
    // nanosleep was interrupted and has set ti to the remaining
    // duration. We continue sleeping until the complete duration
    // has passed. We stop sleeping if it was due to an error.
    while ((nanosleep(&ti, &ti) == -1) && (errno == EINTR))
    {}
#endif
}

#if SIZE_MAX == UINT64_MAX
// MurmurHash2, 64-bit versions, by Austin Appleby
std::size_t Hash(const void* key, std::size_t len, std::size_t seed)
{
    constexpr std::size_t m = 0xc6a4a7935bd1e995;
    constexpr int32_t r = 47;

    std::size_t h = seed ^ (len * m);

    const uint8_t* data = static_cast<const uint8_t*>(key);

    while (len >= 8)
    {
        std::size_t k = *reinterpret_cast<const std::size_t*>(data);

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;

        data += 8;
        len -= 8;
    }

    switch (len)
    {
    case 7:
        h ^= static_cast<std::size_t>(data[6]) << 48;
        [[fallthrough]];
    case 6:
        h ^= static_cast<std::size_t>(data[5]) << 40;
        [[fallthrough]];
    case 5:
        h ^= static_cast<std::size_t>(data[4]) << 32;
        [[fallthrough]];
    case 4:
        h ^= static_cast<std::size_t>(data[3]) << 24;
        [[fallthrough]];
    case 3:
        h ^= static_cast<std::size_t>(data[2]) << 16;
        [[fallthrough]];
    case 2:
        h ^= static_cast<std::size_t>(data[1]) << 8;
        [[fallthrough]];
    case 1:
        h ^= static_cast<std::size_t>(data[0]);
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}
#else
// MurmurHash2, 32-bit versions, by Austin Appleby
std::size_t Hash(const void* key, std::size_t len, std::size_t seed)
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.

    constexpr std::size_t m = 0x5bd1e995;
    constexpr int32_t r = 24;

    // Initialize the hash to a 'random' value

    std::size_t h = seed ^ len;

    // Mix 4 bytes at a time into the hash

    const uint8_t* data = static_cast<const uint8_t*>(key);

    while (len >= 4)
    {
        std::size_t k = *reinterpret_cast<const std::size_t*>(data);

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    // Handle the last few bytes of the input array

    switch (len)
    {
    case 3:
        h ^= static_cast<std::size_t>(data[2]) << 16;
        [[fallthrough]];
    case 2:
        h ^= static_cast<std::size_t>(data[1]) << 8;
        [[fallthrough]];
    case 1:
        h ^= static_cast<std::size_t>(data[0]);
        h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}
#endif

///Detection
#ifndef FGE_DEF_SERVER
bool IsMouseOn(const fge::RenderTarget& target, const fge::RectFloat& zone)
{
    int x = 0;
    int y = 0;
    SDL_GetMouseState(&x, &y);
    return zone.contains(target.mapPixelToCoords({x, y}));
}
bool IsMouseOn(const fge::Vector2f& mousePos, const fge::RectFloat& zone)
{
    return zone.contains(mousePos);
}

bool IsPressed(const fge::Event& evt, const fge::Vector2f& mouse_pos, const fge::RectFloat& zone, uint8_t button)
{
    if (zone.contains(mouse_pos))
    {
        return evt.isMouseButtonPressed(button);
    }
    return false;
}
#endif //FGE_DEF_SERVER

bool IsContained(fge::Quad const& quad, fge::Vector2f const& point)
{
    auto base1 = glm::length(quad[1] - quad[0]);
    auto base2 = glm::length(quad[2] - quad[1]);
    auto base3 = glm::length(quad[3] - quad[2]);
    auto base4 = glm::length(quad[0] - quad[3]);

    auto distance1 = fge::GetShortestDistanceBetween(point, quad[0], quad[1]);
    auto distance2 = fge::GetShortestDistanceBetween(point, quad[1], quad[2]);
    auto distance3 = fge::GetShortestDistanceBetween(point, quad[2], quad[3]);
    auto distance4 = fge::GetShortestDistanceBetween(point, quad[3], quad[0]);

    auto computedArea = (base1 * distance1 + base2 * distance2 + base3 * distance3 + base4 * distance4) / 2.0f;

    auto base = glm::length(quad[1] - quad[3]);

    distance1 = fge::GetShortestDistanceBetween(quad[0], quad[3], quad[1]);
    distance2 = fge::GetShortestDistanceBetween(quad[2], quad[3], quad[1]);

    auto area = (base * distance1 + base * distance2) / 2.0f;

    return std::abs(computedArea - area) <= 0.001f;
}
bool CheckIntersection(fge::Quad const& quadA, fge::Quad const& quadB)
{
    //Checking all lines intersection
    for (std::size_t ia = 0; ia < quadA.size(); ++ia)
    {
        fge::Line const lineA{quadA[ia], quadA[(ia + 1) % quadA.size()]};
        for (std::size_t ib = 0; ib < quadB.size(); ++ib)
        {
            fge::Line const lineB{quadB[ib], quadB[(ib + 1) % quadB.size()]};
            if (fge::CheckIntersection(lineA, lineB))
            {
                return true;
            }
        }
    }

    //Great nothing intersect but the quadB can always be completely inside quadA
    return fge::IsContained(quadA, quadB[0]); //We take a point of quadB to check if it is inside quadA
}
std::optional<fge::Intersection> CheckIntersection(fge::Line const& lineA, fge::Line const& lineB)
{
    /*
     * Original from : https://github.com/MiguelMJ/Candle
     * License : MIT
     * Author : Miguel Mejía Jiménez
     * Modified by : Guillaume Guillet
     */

    auto const directionA = glm::normalize(lineA._end - lineA._start);
    auto const directionB = glm::normalize(lineB._end - lineB._start);

    fge::Intersection result;

    //When the lines are parallel, we consider that there is not intersection.
    auto const dot = std::abs(glm::dot(directionA, directionB));
    if (dot >= 0.999f && dot <= 1.001f)
    {
        return std::nullopt;
    }

    //Math resolving, you can find more information here : https://ncase.me/sight-and-light/
    if ((std::abs(directionB.y) >= 0.0f && std::abs(directionB.x) < 0.001f) ||
        (std::abs(directionA.y) < 0.001f && std::abs(directionA.x) >= 0.0f))
    {
        result._normB =
                (directionA.x * (lineA._start.y - lineB._start.y) + directionA.y * (lineB._start.x - lineA._start.x)) /
                (directionB.y * directionA.x - directionB.x * directionA.y);
        result._normA = (lineB._start.x + directionB.x * result._normB - lineA._start.x) / directionA.x;
    }
    else
    {
        result._normA =
                (directionB.x * (lineB._start.y - lineA._start.y) + directionB.y * (lineA._start.x - lineB._start.x)) /
                (directionA.y * directionB.x - directionA.x * directionB.y);
        result._normB = (lineA._start.x + directionA.x * result._normA - lineB._start.x) / directionB.x;
    }

    //Make sure that there is actually an intersection
    if ((result._normB > 0.0f) && (result._normA > 0.0f) && (result._normA < glm::length(lineA._end - lineA._start)))
    {
        result._point = lineA._start + directionA * result._normA;
        return result;
    }
    return std::nullopt;
}

///Reach
fge::Vector2f ReachVector(const fge::Vector2f& position, const fge::Vector2f& target, float speed, float deltaTime)
{
    float travelDistance = speed * deltaTime;
    fge::Vector2f direction = glm::normalize(target - position);
    float actualDistance = fge::GetDistanceBetween(position, target);

    if (travelDistance >= actualDistance)
    { //We already reached the target
        return target;
    }
    return position + direction * travelDistance;
}
float ReachRotation(float rotation, float target, float speed, float deltaTime, fge::TurnMode turnMode)
{
    rotation = static_cast<float>(std::fmod(rotation, 360));
    if (rotation < 0)
    {
        rotation += 360.0f;
    }
    target = static_cast<float>(std::fmod(target, 360));
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

        if (clockwiseDistance < antiClockwiseDistance)
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
    { //We already reached the target
        return target;
    }

    if (turnMode == fge::TurnMode::TURN_ANTICLOCKWISE)
    { //anti-clockwise
        rotation -= travelDistance;
    }
    else
    { //clockwise
        rotation += travelDistance;
    }

    rotation = static_cast<float>(std::fmod(rotation, 360));
    if (rotation < 0)
    {
        return rotation + 360.0f;
    }
    return rotation;
}

///2D Math
void GetConvexHull(std::vector<fge::Vector2f> const& input, std::vector<fge::Vector2f>& output)
{
    std::size_t n = input.size();
    std::size_t k = 0;
    if (n <= 3)
    {
        output = input;
        return;
    }

    std::vector<fge::Vector2f> listOfPoint(input);

    output.resize(2 * n);

    // Sort points lexicographically
    std::sort(listOfPoint.begin(), listOfPoint.end(), CompareVector);

    // Build lower hull
    for (size_t i = 0; i < n; ++i)
    {
        while (k >= 2 && GetCrossProductVector(output[k - 2], output[k - 1], listOfPoint[i]) <= 0)
        {
            --k;
        }
        output[k++] = listOfPoint[i];
    }

    // Build upper hull
    for (size_t i = n - 1, t = k + 1; i > 0; --i)
    {
        while (k >= t && GetCrossProductVector(output[k - 2], output[k - 1], listOfPoint[i - 1]) <= 0)
        {
            --k;
        }
        output[k++] = listOfPoint[i - 1];
    }

    output.resize(k - 1);
}

///View
fge::Vector2f GetViewSizePercentage(const fge::View& view, const fge::View& defaultView)
{
    return {(view.getSize().x * 100.0f) / defaultView.getSize().x,
            (view.getSize().y * 100.0f) / defaultView.getSize().y};
}
fge::Vector2f SetViewSizePercentage(float percentage, const fge::View& defaultView)
{
    return {(percentage * defaultView.getSize().x) / 100.0f, (percentage * defaultView.getSize().y) / 100.0f};
}
fge::Vector2f SetViewSizePercentage(const fge::Vector2f& percentage, const fge::View& defaultView)
{
    return {(percentage.x * defaultView.getSize().x) / 100.0f, (percentage.y * defaultView.getSize().y) / 100.0f};
}

fge::Vector2f
TransposePointFromAnotherView(const fge::View& pointView, const fge::Vector2f& point, const fge::View& newView)
{
    const fge::Vector2f normalized = pointView.getTransform() * point;
    return newView.getInverseTransform() * normalized;
}

fge::View ClipView(const fge::View& view,
                   const fge::RenderTarget& target,
                   const fge::RectFloat& worldCoordClipRect,
                   fge::ClipClampModes clampMode)
{
    fge::View clippedView = view;

    //Compute offset with respect of default view
    auto oldViewPort = view.getFactorViewport();
    auto defaultViewSize = target.getDefaultView().getSize();
    fge::Vector2f whatCenterShouldBe = {
            defaultViewSize.x * oldViewPort._x + (defaultViewSize.x * oldViewPort._width) / 2.0f,
            defaultViewSize.y * oldViewPort._y + (defaultViewSize.y * oldViewPort._height) / 2.0f};
    fge::Vector2f centerOffset = whatCenterShouldBe - view.getCenter();

    //Compute clip view
    fge::Vector2f clipPositionStart = worldCoordClipRect.getPosition() + centerOffset;
    fge::Vector2f clipPositionEnd = worldCoordClipRect.getPosition() + centerOffset + worldCoordClipRect.getSize();

    clipPositionStart.x = clipPositionStart.x / static_cast<float>(target.getDefaultView().getSize().x);
    clipPositionStart.y = clipPositionStart.y / static_cast<float>(target.getDefaultView().getSize().y);

    clipPositionEnd.x = clipPositionEnd.x / static_cast<float>(target.getDefaultView().getSize().x);
    clipPositionEnd.y = clipPositionEnd.y / static_cast<float>(target.getDefaultView().getSize().y);

    fge::RectFloat viewPort{clipPositionStart, clipPositionEnd - clipPositionStart};
    fge::Vector2f viewSize{worldCoordClipRect.getSize()};
    fge::Vector2f viewCenter{worldCoordClipRect.getPosition() +
                             fge::Vector2f{worldCoordClipRect._width / 2.0f, worldCoordClipRect._height / 2.0f}};

    //Clamping
    switch (clampMode)
    {
    default:
    case fge::ClipClampModes::CLIP_CLAMP_NOTHING:
        break;
    case fge::ClipClampModes::CLIP_CLAMP_STRETCH:
    {
        viewPort._x = std::clamp(viewPort._x, oldViewPort._x, 1.0f);
        viewPort._y = std::clamp(viewPort._y, oldViewPort._y, 1.0f);

        if (viewPort._x + viewPort._width > oldViewPort._x + oldViewPort._width)
        {
            viewPort._width =
                    viewPort._width - ((viewPort._x + viewPort._width) - (oldViewPort._x + oldViewPort._width));
            if (viewPort._width < 0.0f)
            {
                viewPort._width = 0.0f;
            }
        }
        if (viewPort._y + viewPort._height > oldViewPort._y + oldViewPort._height)
        {
            viewPort._height =
                    viewPort._height - ((viewPort._y + viewPort._height) - (oldViewPort._y + oldViewPort._height));
            if (viewPort._height < 0.0f)
            {
                viewPort._height = 0.0f;
            }
        }
    }
    break;
    case fge::ClipClampModes::CLIP_CLAMP_PUSH:
    {
        viewPort._x = std::clamp(viewPort._x, oldViewPort._x, 1.0f);
        viewPort._y = std::clamp(viewPort._y, oldViewPort._y, 1.0f);

        if (viewPort._x + viewPort._width > oldViewPort._x + oldViewPort._width)
        {
            float oldWidth = viewPort._width;
            viewPort._width =
                    viewPort._width - ((viewPort._x + viewPort._width) - (oldViewPort._x + oldViewPort._width));
            if (viewPort._width < 0.0f)
            {
                viewPort._width = 0.0f;
            }
            viewSize.x *= viewPort._width / oldWidth;
        }
        if (viewPort._y + viewPort._height > oldViewPort._y + oldViewPort._height)
        {
            float oldHeight = viewPort._height;
            viewPort._height =
                    viewPort._height - ((viewPort._y + viewPort._height) - (oldViewPort._y + oldViewPort._height));
            if (viewPort._height < 0.0f)
            {
                viewPort._height = 0.0f;
            }
            viewSize.y *= viewPort._height / oldHeight;
        }
    }
    break;
    case fge::ClipClampModes::CLIP_CLAMP_HIDE:
    {
        viewPort._x = std::clamp(viewPort._x, oldViewPort._x, 1.0f);
        viewPort._y = std::clamp(viewPort._y, oldViewPort._y, 1.0f);

        if (viewPort._x + viewPort._width > oldViewPort._x + oldViewPort._width)
        {
            float oldWidth = viewPort._width;
            viewPort._width =
                    viewPort._width - ((viewPort._x + viewPort._width) - (oldViewPort._x + oldViewPort._width));
            if (viewPort._width < 0.0f)
            {
                viewPort._width = 0.0f;
            }
            viewSize.x *= viewPort._width / oldWidth;
            viewCenter.x -= (worldCoordClipRect._width - viewSize.x) / 2.0f;
        }
        if (viewPort._y + viewPort._height > oldViewPort._y + oldViewPort._height)
        {
            float oldHeight = viewPort._height;
            viewPort._height =
                    viewPort._height - ((viewPort._y + viewPort._height) - (oldViewPort._y + oldViewPort._height));
            if (viewPort._height < 0.0f)
            {
                viewPort._height = 0.0f;
            }
            viewSize.y *= viewPort._height / oldHeight;
            viewCenter.y -= (worldCoordClipRect._height - viewSize.y) / 2.0f;
        }
    }
    break;
    }

    clippedView.setFactorViewport(viewPort);
    clippedView.setCenter(viewCenter);
    clippedView.setSize(viewSize);

    return clippedView;
}

///Render
fge::RectInt CoordToPixelRect(const fge::RectFloat& rect, const fge::RenderTarget& target)
{
    fge::Vector2i positions[4] = {
            target.mapCoordsToPixel(fge::Vector2f(rect._x, rect._y)),
            target.mapCoordsToPixel(fge::Vector2f(rect._x + rect._width, rect._y)),
            target.mapCoordsToPixel(fge::Vector2f(rect._x, rect._y + rect._height)),
            target.mapCoordsToPixel(fge::Vector2f(rect._x + rect._width, rect._y + rect._height))};

    return fge::ToRect(positions, 4);
}
fge::RectInt CoordToPixelRect(const fge::RectFloat& rect, const fge::RenderTarget& target, const fge::View& view)
{
    fge::Vector2i positions[4] = {
            target.mapCoordsToPixel(fge::Vector2f(rect._x, rect._y), view),
            target.mapCoordsToPixel(fge::Vector2f(rect._x + rect._width, rect._y), view),
            target.mapCoordsToPixel(fge::Vector2f(rect._x, rect._y + rect._height), view),
            target.mapCoordsToPixel(fge::Vector2f(rect._x + rect._width, rect._y + rect._height), view)};

    return fge::ToRect(positions, 4);
}
fge::RectFloat PixelToCoordRect(const fge::RectInt& rect, const fge::RenderTarget& target)
{
    fge::Vector2f positions[4] = {
            target.mapPixelToCoords(fge::Vector2i(rect._x, rect._y)),
            target.mapPixelToCoords(fge::Vector2i(rect._x + rect._width, rect._y)),
            target.mapPixelToCoords(fge::Vector2i(rect._x, rect._y + rect._height)),
            target.mapPixelToCoords(fge::Vector2i(rect._x + rect._width, rect._y + rect._height))};

    return fge::ToRect(positions, 4);
}
fge::RectFloat PixelToCoordRect(const fge::RectInt& rect, const fge::RenderTarget& target, const fge::View& view)
{
    fge::Vector2f positions[4] = {
            target.mapPixelToCoords(fge::Vector2i(rect._x, rect._y), view),
            target.mapPixelToCoords(fge::Vector2i(rect._x + rect._width, rect._y), view),
            target.mapPixelToCoords(fge::Vector2i(rect._x, rect._y + rect._height), view),
            target.mapPixelToCoords(fge::Vector2i(rect._x + rect._width, rect._y + rect._height), view)};

    return fge::ToRect(positions, 4);
}

fge::RectFloat GetScreenRect(const fge::RenderTarget& target)
{
    fge::Vector2f positions[4] = {target.mapPixelToCoords(fge::Vector2i(0, 0)),
                                  target.mapPixelToCoords(fge::Vector2i(target.getSize().x, 0)),
                                  target.mapPixelToCoords(fge::Vector2i(0, target.getSize().y)),
                                  target.mapPixelToCoords(fge::Vector2i(target.getSize().x, target.getSize().y))};

    return fge::ToRect(positions, 4);
}
fge::RectFloat GetScreenRect(const fge::RenderTarget& target, const fge::View& view)
{
    fge::Vector2f positions[4] = {target.mapPixelToCoords(fge::Vector2i(0, 0), view),
                                  target.mapPixelToCoords(fge::Vector2i(target.getSize().x, 0), view),
                                  target.mapPixelToCoords(fge::Vector2i(0, target.getSize().y), view),
                                  target.mapPixelToCoords(fge::Vector2i(target.getSize().x, target.getSize().y), view)};

    return fge::ToRect(positions, 4);
}

///Json
bool LoadJsonFromFile(const std::filesystem::path& path, nlohmann::json& j)
{
    std::ifstream file(path);
    if (!file)
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
bool SaveJsonToFile(const std::filesystem::path& path, const nlohmann::json& j, int fieldWidth)
{
    std::ofstream file(path);
    if (file)
    {
        file << std::setw(fieldWidth) << j << std::endl;
        file.close();
        return true;
    }
    file.close();
    return false;
}

} // namespace fge

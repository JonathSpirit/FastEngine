/*
 * Copyright 2022 Guillaume Guillet
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

#include "SFML/System/Vector2.hpp"
#include "re2.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif //_WIN32

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
bool CompareVector(const fge::Vector2f& a, const fge::Vector2f& b)
{
    return a.x < b.x || (a.x == b.x && a.y < b.y);
}

/**
    2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
    Returns a positive value, if OAB makes a counter-clockwise turn,
    negative for clockwise turn, and zero if the points are col-linear.
    **/
double GetCrossProductVector(const fge::Vector2f& O, const fge::Vector2f& A, const fge::Vector2f& B)
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

///Detection
#ifndef FGE_DEF_SERVER
bool IsMouseOn(const fge::RenderWindow& window, const fge::RectFloat& zone)
{
    int x = 0;
    int y = 0;
    SDL_GetMouseState(&x, &y);
    return zone.contains(window.mapPixelToCoords({x,y}));
}
bool IsMouseOn(const fge::Vector2f& mousePos, const fge::RectFloat& zone)
{
    return zone.contains(mousePos);
}

bool IsPressed(const fge::Event& evt,
               const fge::Vector2f& mouse_pos,
               const fge::RectFloat& zone,
               uint8_t button)
{
    if (zone.contains(mouse_pos))
    {
        return evt.isMouseButtonPressed(button);
    }
    return false;
}
#endif //FGE_DEF_SERVER

///Reach
fge::Vector2f ReachVector(const fge::Vector2f& position, const fge::Vector2f& target, float speed, float deltaTime)
{
    float travelDistance = speed * deltaTime;
    fge::Vector2f direction = fge::NormalizeVector2(target - position);
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
float ConvertRadToDeg(float rad)
{
    return static_cast<float>(std::fmod((rad * 180.0f / static_cast<float>(FGE_MATH_PI)) + 360.0f, 360.0f));
}
float ConvertDegToRad(float deg)
{
    return deg * static_cast<float>(FGE_MATH_PI) / 180.0f;
}

float GetDeterminant(const fge::Vector2f& vecCol1, const fge::Vector2f& vecCol2)
{
    return vecCol1.x * vecCol2.y - vecCol1.y * vecCol2.x;
}
float GetDotProduct(const fge::Vector2f& vec1, const fge::Vector2f& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y;
}
float GetMagnitude(const fge::Vector2f& vec)
{
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}
float GetRotation(const fge::Vector2f& vec)
{
    return fge::ConvertRadToDeg(std::atan2(vec.y, vec.x));
}
float GetRotationBetween(const fge::Vector2f& vec1, const fge::Vector2f& vec2)
{
    return fge::ConvertRadToDeg(std::atan2(fge::GetDeterminant(vec1, vec2), fge::GetDotProduct(vec1, vec2)));
}
float GetDistanceBetween(const fge::Vector2f& pos1, const fge::Vector2f& pos2)
{
    return fge::GetMagnitude(pos2 - pos1);
}

fge::Vector2f GetForwardVector(float rotation)
{
    rotation *= static_cast<float>(FGE_MATH_PI) / 180.0f;
    return {std::cos(rotation), std::sin(rotation)};
}
fge::Vector2f GetBackwardVector(float rotation)
{
    rotation *= static_cast<float>(FGE_MATH_PI) / 180.0f;
    return -fge::Vector2f(std::cos(rotation), std::sin(rotation));
}
fge::Vector2f GetLeftVector(float rotation)
{
    rotation = (rotation - 90.0f) * static_cast<float>(FGE_MATH_PI) / 180.0f;
    return {std::cos(rotation), std::sin(rotation)};
}
fge::Vector2f GetRightVector(float rotation)
{
    rotation = (rotation + 90.0f) * static_cast<float>(FGE_MATH_PI) / 180.0f;
    return {std::cos(rotation), std::sin(rotation)};
}

void GetConvexHull(const std::vector<fge::Vector2f>& input, std::vector<fge::Vector2f>& output)
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
    fge::Vector2i positions[4] = {target.mapCoordsToPixel(fge::Vector2f(rect._x, rect._y)),
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
    fge::Vector2f positions[4] = {target.mapPixelToCoords(fge::Vector2i(rect._x, rect._y)),
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

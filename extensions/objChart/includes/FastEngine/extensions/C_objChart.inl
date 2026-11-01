/*
 * Copyright 2026 Guillaume Guillet
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

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <limits>

namespace fge
{

template<fge::ObjChartValueType T>
ObjChart<T>::ObjChart() :
        g_frameVertices(fge::vulkan::GetActiveContext()),
        g_pointVertices(fge::vulkan::GetActiveContext())
{
    this->g_frameVertices.create(0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    this->g_pointVertices.create(0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    this->refreshAutoRanges();
    this->applyTargets(true);
}
template<fge::ObjChartValueType T>
ObjChart<T>::ObjChart(fge::Font font, fge::Vector2f const& size) :
        ObjChart()
{
    this->g_font = std::move(font);
    this->g_size = size;
}

template<fge::ObjChartValueType T>
void ObjChart<T>::first([[maybe_unused]] fge::Scene& scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::callbackRegister([[maybe_unused]] fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->detachAll();

    this->g_guiElementHandler = guiElementHandlerPtr;
    if (guiElementHandlerPtr == nullptr)
    {
        return;
    }

    guiElementHandlerPtr->_onGuiResized.addObjectFunctor(&fge::ObjChart<T>::onGuiResized, this, this);

    this->refreshSize(guiElementHandlerPtr->_lastSize);
}

template<fge::ObjChartValueType T>
FGE_OBJ_UPDATE_BODY(ObjChart<T>)
{
    if (!this->g_animating)
    {
        return;
    }

    if (!this->g_animationEnabled || this->g_animationSpeed <= 0.0f)
    {
        this->applyTargets(true);
        return;
    }

    //Exponential approach, independent of the frame rate
    T const deltaSeconds = std::chrono::duration<T>(deltaTime).count();
    T const factor = T{1} - std::exp(-static_cast<T>(this->g_animationSpeed) * deltaSeconds);

    auto const approach = [factor](T& current, T targetValue, T epsilon) {
        if (!std::isfinite(current) || !std::isfinite(targetValue))
        {
            current = targetValue;
            return false;
        }

        T const diff = targetValue - current;
        if (std::abs(diff) <= epsilon)
        {
            current = targetValue;
            return false;
        }
        current += diff * factor;
        return true;
    };

    T const epsilonX =
            std::max(this->g_axisX._target.getLength(), this->g_axisX._displayed.getLength()) * static_cast<T>(1e-4);
    T const epsilonY =
            std::max(this->g_axisY._target.getLength(), this->g_axisY._displayed.getLength()) * static_cast<T>(1e-4);

    bool stillAnimating = false;

    stillAnimating |= approach(this->g_axisX._displayed._min, this->g_axisX._target._min, epsilonX);
    stillAnimating |= approach(this->g_axisX._displayed._max, this->g_axisX._target._max, epsilonX);
    stillAnimating |= approach(this->g_axisY._displayed._min, this->g_axisY._target._min, epsilonY);
    stillAnimating |= approach(this->g_axisY._displayed._max, this->g_axisY._target._max, epsilonY);

    for (auto& point: this->g_points)
    {
        stillAnimating |= approach(point._displayed.x, point._target.x, epsilonX);
        stillAnimating |= approach(point._displayed.y, point._target.y, epsilonY);
    }

    this->g_animating = stillAnimating;
    this->g_needGeometryUpdate = true;
}

#ifndef FGE_DEF_SERVER
template<fge::ObjChartValueType T>
FGE_OBJ_DRAW_BODY(ObjChart<T>)
{
    this->ensureGeometryUpdate();

    auto const transformIndex = target.requestGlobalTransform(*this, states._resTransform);

    auto copyStates = states.copy();
    copyStates._resTransform.set(transformIndex);

    if (this->g_frameVertices.getCount() > 0)
    {
        copyStates._vertexBuffer = &this->g_frameVertices;
        target.draw(copyStates);
    }

    if (this->g_pointVertices.getCount() > 0)
    {
        copyStates._vertexBuffer = &this->g_pointVertices;
        target.draw(copyStates);
    }

    auto textStates = states.copy();
    textStates._resTransform.set(transformIndex);

    for (auto const& label: this->g_xLabels)
    {
        label.draw(target, textStates);
    }
    for (auto const& label: this->g_yLabels)
    {
        label.draw(target, textStates);
    }
}
#endif

template<fge::ObjChartValueType T>
void ObjChart<T>::setSize(fge::Vector2f const& size)
{
    this->g_dynamicSize.reset();
    this->g_size = {std::max(size.x, 0.0f), std::max(size.y, 0.0f)};
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setDynamicSize(fge::DynamicSize const& size)
{
    this->g_dynamicSize = size;
    if (this->g_guiElementHandler != nullptr)
    {
        this->refreshSize(this->g_guiElementHandler->_lastSize);
    }
}
template<fge::ObjChartValueType T>
fge::Vector2f const& ObjChart<T>::getSize() const
{
    return this->g_size;
}

template<fge::ObjChartValueType T>
void ObjChart<T>::setPadding(Padding const& padding)
{
    this->g_padding = padding;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
typename ObjChart<T>::Padding const& ObjChart<T>::getPadding() const
{
    return this->g_padding;
}

template<fge::ObjChartValueType T>
fge::RectFloat ObjChart<T>::getPlotArea() const
{
    return {{this->g_padding._left, this->g_padding._top},
            {std::max(this->g_size.x - this->g_padding._left - this->g_padding._right, 0.0f),
             std::max(this->g_size.y - this->g_padding._top - this->g_padding._bottom, 0.0f)}};
}

template<fge::ObjChartValueType T>
std::size_t ObjChart<T>::addPoint(Point const& point, std::optional<fge::Color> color)
{
    this->g_points.push_back({point, point, color});
    this->refreshAutoRanges();
    this->applyTargets(false);
    return this->g_points.size() - 1;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setPoint(std::size_t index, Point const& point)
{
    this->g_points[index]._target = point;
    this->refreshAutoRanges();
    this->applyTargets(false);
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setPoints(std::span<Point const> points)
{
    std::size_t const commonCount = std::min(points.size(), this->g_points.size());

    for (std::size_t i = 0; i < commonCount; ++i)
    {
        this->g_points[i]._target = points[i];
    }
    this->g_points.resize(commonCount);
    for (std::size_t i = commonCount; i < points.size(); ++i)
    {
        this->g_points.push_back({points[i], points[i], std::nullopt});
    }

    this->refreshAutoRanges();
    this->applyTargets(false);
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setPointColor(std::size_t index, std::optional<fge::Color> color)
{
    this->g_points[index]._color = color;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::removePoint(std::size_t index)
{
    this->g_points.erase(this->g_points.begin() + static_cast<std::ptrdiff_t>(index));
    this->refreshAutoRanges();
    this->applyTargets(false);
}
template<fge::ObjChartValueType T>
void ObjChart<T>::clearPoints()
{
    this->g_points.clear();
    this->refreshAutoRanges();
    this->applyTargets(false);
}

template<fge::ObjChartValueType T>
std::size_t ObjChart<T>::getPointCount() const
{
    return this->g_points.size();
}
template<fge::ObjChartValueType T>
typename ObjChart<T>::Point const& ObjChart<T>::getPoint(std::size_t index) const
{
    return this->g_points[index]._target;
}
template<fge::ObjChartValueType T>
typename ObjChart<T>::Point const& ObjChart<T>::getDisplayedPoint(std::size_t index) const
{
    return this->g_points[index]._displayed;
}
template<fge::ObjChartValueType T>
std::optional<fge::Color> const& ObjChart<T>::getPointColor(std::size_t index) const
{
    return this->g_points[index]._color;
}

template<fge::ObjChartValueType T>
void ObjChart<T>::setXRange(T min, T max)
{
    if (min > max)
    {
        std::swap(min, max);
    }
    if (min == max)
    {
        max = min + T{1};
    }

    this->g_axisX._mode = RangeModes::RANGE_FIXED;
    this->g_axisX._target = {min, max};
    this->applyTargets(false);
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setYRange(T min, T max)
{
    if (min > max)
    {
        std::swap(min, max);
    }
    if (min == max)
    {
        max = min + T{1};
    }

    this->g_axisY._mode = RangeModes::RANGE_FIXED;
    this->g_axisY._target = {min, max};
    this->applyTargets(false);
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setXRangeAuto()
{
    this->g_axisX._mode = RangeModes::RANGE_AUTO;
    this->refreshAutoRanges();
    this->applyTargets(false);
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setYRangeAuto()
{
    this->g_axisY._mode = RangeModes::RANGE_AUTO;
    this->refreshAutoRanges();
    this->applyTargets(false);
}

template<fge::ObjChartValueType T>
typename ObjChart<T>::RangeModes ObjChart<T>::getXRangeMode() const
{
    return this->g_axisX._mode;
}
template<fge::ObjChartValueType T>
typename ObjChart<T>::RangeModes ObjChart<T>::getYRangeMode() const
{
    return this->g_axisY._mode;
}
template<fge::ObjChartValueType T>
typename ObjChart<T>::Range const& ObjChart<T>::getXRange() const
{
    return this->g_axisX._displayed;
}
template<fge::ObjChartValueType T>
typename ObjChart<T>::Range const& ObjChart<T>::getYRange() const
{
    return this->g_axisY._displayed;
}

template<fge::ObjChartValueType T>
void ObjChart<T>::setTickCount(std::size_t xCount, std::size_t yCount)
{
    this->g_axisX._tickCount = std::max<std::size_t>(xCount, 1);
    this->g_axisY._tickCount = std::max<std::size_t>(yCount, 1);
    this->refreshAutoRanges();
    this->applyTargets(false);
}
template<fge::ObjChartValueType T>
fge::Vector2<std::size_t> ObjChart<T>::getTickCount() const
{
    return {this->g_axisX._tickCount, this->g_axisY._tickCount};
}

template<fge::ObjChartValueType T>
void ObjChart<T>::enableAnimation(bool enable)
{
    this->g_animationEnabled = enable;
    if (!enable)
    {
        this->applyTargets(true);
    }
}
template<fge::ObjChartValueType T>
bool ObjChart<T>::isAnimationEnabled() const
{
    return this->g_animationEnabled;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setAnimationSpeed(float speed)
{
    this->g_animationSpeed = std::max(speed, 0.0f);
}
template<fge::ObjChartValueType T>
float ObjChart<T>::getAnimationSpeed() const
{
    return this->g_animationSpeed;
}
template<fge::ObjChartValueType T>
bool ObjChart<T>::isAnimating() const
{
    return this->g_animating;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::finishAnimation()
{
    this->applyTargets(true);
}

template<fge::ObjChartValueType T>
void ObjChart<T>::setFont(fge::Font font)
{
    this->g_font = std::move(font);
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
fge::Font const& ObjChart<T>::getFont() const
{
    return this->g_font;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setCharacterSize(fge::CharacterSize size)
{
    this->g_characterSize = size;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
fge::CharacterSize ObjChart<T>::getCharacterSize() const
{
    return this->g_characterSize;
}

template<fge::ObjChartValueType T>
void ObjChart<T>::setPointShape(PointShapes shape)
{
    this->g_pointShape = shape;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
typename ObjChart<T>::PointShapes ObjChart<T>::getPointShape() const
{
    return this->g_pointShape;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setPointSize(float size)
{
    this->g_pointSize = std::max(size, 0.0f);
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
float ObjChart<T>::getPointSize() const
{
    return this->g_pointSize;
}

template<fge::ObjChartValueType T>
void ObjChart<T>::setDefaultPointColor(fge::Color color)
{
    this->g_colorDefaultPoint = color;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
fge::Color ObjChart<T>::getDefaultPointColor() const
{
    return this->g_colorDefaultPoint;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setBackgroundColor(fge::Color color)
{
    this->g_colorBackground = color;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
fge::Color ObjChart<T>::getBackgroundColor() const
{
    return this->g_colorBackground;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setAxisColor(fge::Color color)
{
    this->g_colorAxis = color;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
fge::Color ObjChart<T>::getAxisColor() const
{
    return this->g_colorAxis;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setGridColor(fge::Color color)
{
    this->g_colorGrid = color;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
fge::Color ObjChart<T>::getGridColor() const
{
    return this->g_colorGrid;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setLabelColor(fge::Color color)
{
    this->g_colorLabel = color;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
fge::Color ObjChart<T>::getLabelColor() const
{
    return this->g_colorLabel;
}

template<fge::ObjChartValueType T>
void ObjChart<T>::showGrid(bool show)
{
    this->g_showGrid = show;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
bool ObjChart<T>::isGridShown() const
{
    return this->g_showGrid;
}

template<fge::ObjChartValueType T>
void ObjChart<T>::showLines(bool show)
{
    this->g_showLines = show;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
bool ObjChart<T>::isLinesShown() const
{
    return this->g_showLines;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setLineThickness(float thickness)
{
    this->g_lineThickness = std::max(thickness, 0.0f);
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
float ObjChart<T>::getLineThickness() const
{
    return this->g_lineThickness;
}
template<fge::ObjChartValueType T>
void ObjChart<T>::setLineColor(fge::Color color)
{
    this->g_colorLine = color;
    this->g_needGeometryUpdate = true;
}
template<fge::ObjChartValueType T>
fge::Color ObjChart<T>::getLineColor() const
{
    return this->g_colorLine;
}

template<fge::ObjChartValueType T>
fge::Vector2f ObjChart<T>::mapValueToLocal(Point const& value) const
{
    auto const plotArea = this->getPlotArea();
    auto const& rangeX = this->g_axisX._displayed;
    auto const& rangeY = this->g_axisY._displayed;

    T const ratioX = rangeX.getLength() != T{0} ? (value.x - rangeX._min) / rangeX.getLength() : static_cast<T>(0.5);
    T const ratioY = rangeY.getLength() != T{0} ? (value.y - rangeY._min) / rangeY.getLength() : static_cast<T>(0.5);

    return {plotArea._x + static_cast<float>(ratioX) * plotArea._width,
            plotArea._y + plotArea._height - static_cast<float>(ratioY) * plotArea._height};
}
template<fge::ObjChartValueType T>
typename ObjChart<T>::Point ObjChart<T>::mapLocalToValue(fge::Vector2f const& position) const
{
    auto const plotArea = this->getPlotArea();
    auto const& rangeX = this->g_axisX._displayed;
    auto const& rangeY = this->g_axisY._displayed;

    T const ratioX = plotArea._width != 0.0f ? static_cast<T>((position.x - plotArea._x) / plotArea._width) : T{0};
    T const ratioY = plotArea._height != 0.0f
                             ? static_cast<T>((plotArea._y + plotArea._height - position.y) / plotArea._height)
                             : T{0};

    return {rangeX._min + ratioX * rangeX.getLength(), rangeY._min + ratioY * rangeY.getLength()};
}

template<fge::ObjChartValueType T>
char const* ObjChart<T>::getClassName() const
{
    if constexpr (std::same_as<T, float>)
    {
        return FGE_OBJCHART_CLASSNAME_FLOAT;
    }
    else
    {
        return FGE_OBJCHART_CLASSNAME_DOUBLE;
    }
}
template<fge::ObjChartValueType T>
char const* ObjChart<T>::getReadableClassName() const
{
    return "chart";
}

template<fge::ObjChartValueType T>
fge::RectFloat ObjChart<T>::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
template<fge::ObjChartValueType T>
fge::RectFloat ObjChart<T>::getLocalBounds() const
{
    return {{0.0f, 0.0f}, this->g_size};
}

template<fge::ObjChartValueType T>
void ObjChart<T>::onGuiResized([[maybe_unused]] fge::GuiElementHandler const& handler, fge::Vector2f const& size)
{
    this->refreshSize(size);
}

template<fge::ObjChartValueType T>
void ObjChart<T>::refreshSize(fge::Vector2f const& targetSize)
{
    if (!this->g_dynamicSize)
    {
        return;
    }

    auto const size = this->g_dynamicSize->getSize(this->getPosition(), targetSize);
    fge::Vector2f const newSize{std::max(size.x, 0.0f), std::max(size.y, 0.0f)};
    if (newSize != this->g_size)
    {
        this->g_size = newSize;
        this->g_needGeometryUpdate = true;
    }
}
template<fge::ObjChartValueType T>
void ObjChart<T>::refreshAutoRanges(AxisData& axis, bool isXAxis)
{
    if (axis._mode != RangeModes::RANGE_AUTO)
    {
        return;
    }

    T min = std::numeric_limits<T>::max();
    T max = std::numeric_limits<T>::lowest();
    for (auto const& point: this->g_points)
    {
        T const value = isXAxis ? point._target.x : point._target.y;
        if (std::isfinite(value))
        {
            min = std::min(min, value);
            max = std::max(max, value);
        }
    }

    if (min > max)
    { //No valid points
        axis._target = {T{0}, T{1}};
        return;
    }

    if (max - min <= T{0})
    {
        T const pad = min != T{0} ? std::abs(min) * static_cast<T>(0.1) : T{1};
        min -= pad;
        max += pad;
    }
    else
    { //Keep a small margin so points don't touch the borders
        T const pad = (max - min) * static_cast<T>(0.05);
        min -= pad;
        max += pad;
    }

    T const step = computeStep(max - min, axis._tickCount);
    axis._target = {std::floor(min / step) * step, std::ceil(max / step) * step};
}
template<fge::ObjChartValueType T>
void ObjChart<T>::refreshAutoRanges()
{
    this->refreshAutoRanges(this->g_axisX, true);
    this->refreshAutoRanges(this->g_axisY, false);
}
template<fge::ObjChartValueType T>
void ObjChart<T>::applyTargets(bool forceSnap)
{
    if (forceSnap || !this->g_animationEnabled)
    {
        this->g_axisX._displayed = this->g_axisX._target;
        this->g_axisY._displayed = this->g_axisY._target;
        for (auto& point: this->g_points)
        {
            point._displayed = point._target;
        }
        this->g_animating = false;
    }
    else
    {
        this->g_animating = true;
    }

    this->g_needGeometryUpdate = true;
}

template<fge::ObjChartValueType T>
void ObjChart<T>::ensureGeometryUpdate() const
{
    if (!this->g_needGeometryUpdate)
    {
        return;
    }
    this->g_needGeometryUpdate = false;

    this->g_frameVertices.clear();
    this->g_pointVertices.clear();

    this->g_frameVertices.appendRect({0.0f, 0.0f}, this->g_size, this->g_colorBackground);

    auto const plotArea = this->getPlotArea();
    if (plotArea._width <= 0.0f || plotArea._height <= 0.0f)
    {
        this->g_xLabels.clear();
        this->g_yLabels.clear();
        return;
    }

    //Grid, ticks and labels
    this->buildAxis(this->g_axisX, true, plotArea);
    this->buildAxis(this->g_axisY, false, plotArea);

    //Axis lines
    this->g_frameVertices.appendRect({plotArea._x - FGE_OBJCHART_LINE_THICKNESS, plotArea._y},
                                     {FGE_OBJCHART_LINE_THICKNESS, plotArea._height + FGE_OBJCHART_LINE_THICKNESS},
                                     this->g_colorAxis);

    this->g_frameVertices.appendRect({plotArea._x - FGE_OBJCHART_LINE_THICKNESS, plotArea._y + plotArea._height},
                                     {plotArea._width + FGE_OBJCHART_LINE_THICKNESS, FGE_OBJCHART_LINE_THICKNESS},
                                     this->g_colorAxis);

    //Lines, appended before the points so the points are drawn on top
    if (this->g_showLines && this->g_lineThickness > 0.0f)
    {
        for (std::size_t i = 1; i < this->g_points.size(); ++i)
        {
            this->appendLine(this->mapValueToLocal(this->g_points[i - 1]._displayed),
                             this->mapValueToLocal(this->g_points[i]._displayed), plotArea);
        }
    }

    //Points
    if (this->g_pointSize <= 0.0f)
    {
        return;
    }

    float const halfSize = this->g_pointSize / 2.0f;
    constexpr float tolerance = 0.5f;

    for (auto const& point: this->g_points)
    {
        auto const center = this->mapValueToLocal(point._displayed);
        if (!std::isfinite(center.x) || !std::isfinite(center.y) || center.x < plotArea._x - tolerance ||
            center.y < plotArea._y - tolerance || center.x > plotArea._x + plotArea._width + tolerance ||
            center.y > plotArea._y + plotArea._height + tolerance)
        {
            continue;
        }

        fge::Color const color = point._color.value_or(this->g_colorDefaultPoint);

        switch (this->g_pointShape)
        {
        case PointShapes::SHAPE_SQUARE:
            this->g_pointVertices.appendRect(center - fge::Vector2f{halfSize, halfSize},
                                             {this->g_pointSize, this->g_pointSize}, color);
            break;
        case PointShapes::SHAPE_DIAMOND:
        {
            fge::Vector2f const top{center.x, center.y - halfSize};
            fge::Vector2f const right{center.x + halfSize, center.y};
            fge::Vector2f const bottom{center.x, center.y + halfSize};
            fge::Vector2f const left{center.x - halfSize, center.y};
            this->g_pointVertices.appendTriangle(top, right, bottom, color);
            this->g_pointVertices.appendTriangle(top, bottom, left, color);
            break;
        }
        case PointShapes::SHAPE_CIRCLE:
        {
            this->g_pointVertices.appendCircle(center, halfSize, color, FGE_OBJCHART_CIRCLE_SEGMENTS);
            break;
        }
        }
    }
}
template<fge::ObjChartValueType T>
void ObjChart<T>::buildAxis(AxisData const& axis, bool isXAxis, fge::RectFloat const& plotArea) const
{
    auto& labels = isXAxis ? this->g_xLabels : this->g_yLabels;
    std::size_t labelCount = 0;

    Range const& range = axis._displayed;
    T const step = computeStep(range.getLength(), axis._tickCount);
    T const firstTick = std::ceil(range._min / step) * step;

    //Number of decimals needed to display the step
    int const decimals =
            step >= T{1} ? 0 : std::clamp(static_cast<int>(std::ceil(-std::log10(step) - static_cast<T>(1e-6))), 0, 12);

    bool const haveFont = this->g_font.valid();

    for (std::size_t i = 0; i < FGE_OBJCHART_MAX_TICKS; ++i)
    {
        T value = firstTick + static_cast<T>(i) * step;
        if (value > range._max + step * static_cast<T>(1e-6))
        {
            break;
        }
        if (std::abs(value) < step * static_cast<T>(1e-6))
        { //Avoid "-0"
            value = T{0};
        }

        auto const position = std::round(isXAxis ? this->mapValueToLocal({value, range._min}).x
                                                 : this->mapValueToLocal({this->g_axisX._displayed._min, value}).y);

        if (isXAxis)
        {
            if (this->g_showGrid)
            {
                this->g_frameVertices.appendRect({position, plotArea._y},
                                                 {FGE_OBJCHART_LINE_THICKNESS, plotArea._height}, this->g_colorGrid);
            }
            this->g_frameVertices.appendRect({position, plotArea._y + plotArea._height},
                                             {FGE_OBJCHART_LINE_THICKNESS, FGE_OBJCHART_TICK_LENGTH},
                                             this->g_colorAxis);
        }
        else
        {
            if (this->g_showGrid)
            {
                this->g_frameVertices.appendRect({plotArea._x, position},
                                                 {plotArea._width, FGE_OBJCHART_LINE_THICKNESS}, this->g_colorGrid);
            }
            this->g_frameVertices.appendRect(
                    {plotArea._x - FGE_OBJCHART_TICK_LENGTH - FGE_OBJCHART_LINE_THICKNESS, position},
                    {FGE_OBJCHART_TICK_LENGTH, FGE_OBJCHART_LINE_THICKNESS}, this->g_colorAxis);
        }

        if (!haveFont)
        {
            continue;
        }

        char buffer[64];
        int const length = std::snprintf(buffer, sizeof(buffer), "%.*f", decimals, static_cast<double>(value));
        if (length <= 0)
        {
            continue;
        }

        if (labelCount >= labels.size())
        {
            labels.emplace_back();
        }
        auto& label = labels[labelCount++];
        label.setFont(this->g_font);
        label.setCharacterSize(this->g_characterSize);
        label.setFillColor(this->g_colorLabel);
        //Note: passing the char array directly would use the whole array as a literal (including garbage)
        label.setString(
                tiny_utf8::string{buffer, std::min<std::size_t>(static_cast<std::size_t>(length), sizeof(buffer) - 1)});

        auto const bounds = label.getLocalBounds();
        if (isXAxis)
        {
            label.setOrigin({std::round(bounds._x + bounds._width / 2.0f), bounds._y});
            label.setPosition({position, plotArea._y + plotArea._height + FGE_OBJCHART_TICK_LENGTH + 3.0f});
        }
        else
        {
            label.setOrigin({bounds._x + bounds._width, std::round(bounds._y + bounds._height / 2.0f)});
            label.setPosition({plotArea._x - FGE_OBJCHART_TICK_LENGTH - FGE_OBJCHART_LINE_THICKNESS - 3.0f, position});
        }
    }

    labels.resize(labelCount);
}
template<fge::ObjChartValueType T>
void ObjChart<T>::appendLine(fge::Vector2f a, fge::Vector2f b, fge::RectFloat const& plotArea) const
{
    if (!std::isfinite(a.x) || !std::isfinite(a.y) || !std::isfinite(b.x) || !std::isfinite(b.y))
    {
        return;
    }

    //Clip the segment to the plot area (Liang-Barsky), points outside the ranges must not draw over the labels
    float const dx = b.x - a.x;
    float const dy = b.y - a.y;
    float const p[4] = {-dx, dx, -dy, dy};
    float const q[4] = {a.x - plotArea._x, plotArea._x + plotArea._width - a.x, a.y - plotArea._y,
                        plotArea._y + plotArea._height - a.y};

    float t0 = 0.0f;
    float t1 = 1.0f;
    for (std::size_t i = 0; i < 4; ++i)
    {
        if (p[i] == 0.0f)
        { //Parallel to this border
            if (q[i] < 0.0f)
            {
                return;
            }
            continue;
        }

        float const r = q[i] / p[i];
        if (p[i] < 0.0f)
        {
            if (r > t1)
            {
                return;
            }
            t0 = std::max(t0, r);
        }
        else
        {
            if (r < t0)
            {
                return;
            }
            t1 = std::min(t1, r);
        }
    }

    fge::Vector2f const start{a.x + t0 * dx, a.y + t0 * dy};
    fge::Vector2f const end{a.x + t1 * dx, a.y + t1 * dy};

    //Build the segment as a quad of the line thickness
    float const length = std::hypot(end.x - start.x, end.y - start.y);
    if (length <= 1e-4f)
    {
        return;
    }

    float const halfThickness = this->g_lineThickness / 2.0f;
    fge::Vector2f const normal{-(end.y - start.y) / length * halfThickness, (end.x - start.x) / length * halfThickness};

    fge::Vector2f const startA{start.x + normal.x, start.y + normal.y};
    fge::Vector2f const startB{start.x - normal.x, start.y - normal.y};
    fge::Vector2f const endA{end.x + normal.x, end.y + normal.y};
    fge::Vector2f const endB{end.x - normal.x, end.y - normal.y};

    this->g_pointVertices.appendTriangle(startA, endA, endB, this->g_colorLine);
    this->g_pointVertices.appendTriangle(startA, endB, startB, this->g_colorLine);
}

template<fge::ObjChartValueType T>
T ObjChart<T>::computeStep(T length, std::size_t tickCount)
{
    if (!(length > T{0}) || !std::isfinite(length))
    {
        return T{1};
    }

    T const rawStep = length / static_cast<T>(std::max<std::size_t>(tickCount, 1));
    T const magnitude = std::pow(T{10}, std::floor(std::log10(rawStep)));
    T const normalized = rawStep / magnitude;

    T step;
    if (normalized < T{1.5})
    {
        step = T{1};
    }
    else if (normalized < T{3})
    {
        step = T{2};
    }
    else if (normalized < T{7})
    {
        step = T{5};
    }
    else
    {
        step = T{10};
    }

    return step * magnitude;
}

} // namespace fge

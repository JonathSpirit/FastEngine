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

#ifndef _FGE_EXT_C_OBJCHART_HPP_INCLUDED
#define _FGE_EXT_C_OBJCHART_HPP_INCLUDED

#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_vector.hpp"
#include "FastEngine/graphic/C_color.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_object.hpp"
#include "FastEngine/vulkan/C_vertexBuffer.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <concepts>
#include <optional>
#include <span>
#include <string>
#include <vector>

#define FGE_OBJCHART_CLASSNAME_FLOAT "FGE:EXT:OBJ:CHART:F"
#define FGE_OBJCHART_CLASSNAME_DOUBLE "FGE:EXT:OBJ:CHART:D"

#define FGE_OBJCHART_DEFAULT_SIZE_X 320.0f
#define FGE_OBJCHART_DEFAULT_SIZE_Y 200.0f
#define FGE_OBJCHART_DEFAULT_TICK_COUNT 5
#define FGE_OBJCHART_DEFAULT_POINT_SIZE 6.0f
#define FGE_OBJCHART_DEFAULT_LINE_THICKNESS 2.0f
#define FGE_OBJCHART_DEFAULT_CHARACTER_SIZE 12
#define FGE_OBJCHART_DEFAULT_ANIMATION_SPEED 6.0f
#define FGE_OBJCHART_TICK_LENGTH 4.0f
#define FGE_OBJCHART_LINE_THICKNESS 1.0f
#define FGE_OBJCHART_MAX_TICKS 64
#define FGE_OBJCHART_CIRCLE_SEGMENTS 12

namespace fge
{

/**
 * Brief The data types supported by ObjChart
 */
template<class T>
concept ObjChartValueType = std::same_as<T, float> || std::same_as<T, double>;

/**
 * \class ObjChart
 * \ingroup extensions
 * \brief A point chart (scatter plot) with linear axes
 *
 * Consecutive points (in index order) can be linked by straight lines (enabled by default, see showLines()).
 *
 * The chart can be configured to store its data as float or double (see ObjChartF and ObjChartD).
 * Axes ranges can be computed automatically from the points, or fixed by the user.
 *
 * When animation is enabled (the default), modifying a point does not teleport it,
 * the displayed point gently moves toward its new position. Auto ranges are animated the same way.
 *
 * The chart can be sized with a fixed size or with a fge::DynamicSize relative to its
 * fge::GuiElementHandler (useful inside a fge::ObjWindow).
 *
 * \tparam T The data type, float or double
 */
template<fge::ObjChartValueType T>
class ObjChart : public fge::Object, public fge::Subscriber
{
public:
    using ValueType = T;
    using Point = fge::Vector2<T>;

    struct Range
    {
        T _min{0};
        T _max{1};

        [[nodiscard]] constexpr T getLength() const { return this->_max - this->_min; }
        [[nodiscard]] constexpr bool operator==(Range const& r) const = default;
    };

    struct Padding
    {
        float _left{48.0f};
        float _top{10.0f};
        float _right{14.0f};
        float _bottom{26.0f};
    };

    enum class RangeModes
    {
        RANGE_AUTO,
        RANGE_FIXED
    };

    enum class PointShapes
    {
        SHAPE_SQUARE,
        SHAPE_CIRCLE,
        SHAPE_DIAMOND
    };

    ObjChart();
    explicit ObjChart(fge::Font font,
                      fge::Vector2f const& size = {FGE_OBJCHART_DEFAULT_SIZE_X, FGE_OBJCHART_DEFAULT_SIZE_Y});
    ~ObjChart() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjChart<T>)

    void first(fge::Scene& scene) override;
    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;
    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE

    //Size

    /**
     * \brief Set a fixed size for the chart
     *
     * This disables any dynamic size previously set.
     */
    void setSize(fge::Vector2f const& size);
    /**
     * \brief Set a dynamic size, relative to the GuiElementHandler the chart is registered to
     */
    void setDynamicSize(fge::DynamicSize const& size);
    [[nodiscard]] fge::Vector2f const& getSize() const;

    void setPadding(Padding const& padding);
    [[nodiscard]] Padding const& getPadding() const;

    /**
     * \brief Get the area (in local coordinates) where the points are drawn
     */
    [[nodiscard]] fge::RectFloat getPlotArea() const;

    //Points

    std::size_t addPoint(Point const& point, std::optional<fge::Color> color = std::nullopt);
    /**
     * \brief Set the target position of a point
     *
     * If animation is enabled, the displayed point will gently move toward this new position.
     */
    void setPoint(std::size_t index, Point const& point);
    /**
     * \brief Replace all the points
     *
     * Existing points (same index) are moved toward their new position,
     * extra points are added and the remaining ones are removed.
     */
    void setPoints(std::span<Point const> points);
    void setPointColor(std::size_t index, std::optional<fge::Color> color);
    void removePoint(std::size_t index);
    void clearPoints();

    [[nodiscard]] std::size_t getPointCount() const;
    [[nodiscard]] Point const& getPoint(std::size_t index) const;
    [[nodiscard]] Point const& getDisplayedPoint(std::size_t index) const;
    [[nodiscard]] std::optional<fge::Color> const& getPointColor(std::size_t index) const;

    //Axes

    void setXRange(T min, T max);
    void setYRange(T min, T max);
    void setXRangeAuto();
    void setYRangeAuto();

    [[nodiscard]] RangeModes getXRangeMode() const;
    [[nodiscard]] RangeModes getYRangeMode() const;
    /**
     * \brief Get the currently displayed X range (can be in transition if animated)
     */
    [[nodiscard]] Range const& getXRange() const;
    /**
     * \brief Get the currently displayed Y range (can be in transition if animated)
     */
    [[nodiscard]] Range const& getYRange() const;

    /**
     * \brief Set the desired number of ticks for each axis
     *
     * The real count can slightly differ as ticks are placed on "nice" values (1, 2, 5 * 10^n).
     */
    void setTickCount(std::size_t xCount, std::size_t yCount);
    [[nodiscard]] fge::Vector2<std::size_t> getTickCount() const;

    //Animation

    void enableAnimation(bool enable);
    [[nodiscard]] bool isAnimationEnabled() const;
    /**
     * \brief Set the animation speed
     *
     * The displayed values exponentially approach their target, the speed is the rate per second.
     * A higher value means a faster animation.
     */
    void setAnimationSpeed(float speed);
    [[nodiscard]] float getAnimationSpeed() const;
    [[nodiscard]] bool isAnimating() const;
    /**
     * \brief Immediately set every displayed value to its target
     */
    void finishAnimation();

    //Style

    void setFont(fge::Font font);
    [[nodiscard]] fge::Font const& getFont() const;
    void setCharacterSize(fge::CharacterSize size);
    [[nodiscard]] fge::CharacterSize getCharacterSize() const;

    void setPointShape(PointShapes shape);
    [[nodiscard]] PointShapes getPointShape() const;
    void setPointSize(float size);
    [[nodiscard]] float getPointSize() const;

    void setDefaultPointColor(fge::Color color);
    [[nodiscard]] fge::Color getDefaultPointColor() const;
    void setBackgroundColor(fge::Color color);
    [[nodiscard]] fge::Color getBackgroundColor() const;
    void setAxisColor(fge::Color color);
    [[nodiscard]] fge::Color getAxisColor() const;
    void setGridColor(fge::Color color);
    [[nodiscard]] fge::Color getGridColor() const;
    void setLabelColor(fge::Color color);
    [[nodiscard]] fge::Color getLabelColor() const;

    void showGrid(bool show);
    [[nodiscard]] bool isGridShown() const;

    /**
     * \brief Show or hide the straight lines linking consecutive points
     *
     * Points are linked in index order (not sorted by X), a line is drawn between point i and point i+1.
     */
    void showLines(bool show);
    [[nodiscard]] bool isLinesShown() const;
    void setLineThickness(float thickness);
    [[nodiscard]] float getLineThickness() const;
    void setLineColor(fge::Color color);
    [[nodiscard]] fge::Color getLineColor() const;

    //Mapping

    /**
     * \brief Convert a data value into a local position, using the displayed ranges
     */
    [[nodiscard]] fge::Vector2f mapValueToLocal(Point const& value) const;
    /**
     * \brief Convert a local position into a data value, using the displayed ranges
     */
    [[nodiscard]] Point mapLocalToValue(fge::Vector2f const& position) const;

    [[nodiscard]] char const* getClassName() const override;
    [[nodiscard]] char const* getReadableClassName() const override;

    [[nodiscard]] fge::RectFloat getGlobalBounds() const override;
    [[nodiscard]] fge::RectFloat getLocalBounds() const override;

private:
    struct PointData
    {
        Point _target;
        Point _displayed;
        std::optional<fge::Color> _color;
    };

    struct AxisData
    {
        RangeModes _mode{RangeModes::RANGE_AUTO};
        Range _target{};
        Range _displayed{};
        std::size_t _tickCount{FGE_OBJCHART_DEFAULT_TICK_COUNT};
    };

    void onGuiResized(fge::GuiElementHandler const& handler, fge::Vector2f const& size);

    void refreshSize(fge::Vector2f const& targetSize);
    void refreshAutoRanges(AxisData& axis, bool isXAxis);
    void refreshAutoRanges();
    void applyTargets(bool forceSnap);

    void ensureGeometryUpdate() const;
    void buildAxis(AxisData const& axis, bool isXAxis, fge::RectFloat const& plotArea) const;
    void appendLine(fge::Vector2f a, fge::Vector2f b, fge::RectFloat const& plotArea) const;

    [[nodiscard]] static T computeStep(T length, std::size_t tickCount);

    fge::GuiElementHandler* g_guiElementHandler{nullptr};
    std::optional<fge::DynamicSize> g_dynamicSize;
    fge::Vector2f g_size{FGE_OBJCHART_DEFAULT_SIZE_X, FGE_OBJCHART_DEFAULT_SIZE_Y};
    Padding g_padding;

    std::vector<PointData> g_points;
    AxisData g_axisX;
    AxisData g_axisY;

    bool g_animationEnabled{true};
    float g_animationSpeed{FGE_OBJCHART_DEFAULT_ANIMATION_SPEED};
    bool g_animating{false};

    fge::Font g_font;
    fge::CharacterSize g_characterSize{FGE_OBJCHART_DEFAULT_CHARACTER_SIZE};

    PointShapes g_pointShape{PointShapes::SHAPE_CIRCLE};
    float g_pointSize{FGE_OBJCHART_DEFAULT_POINT_SIZE};

    fge::Color g_colorDefaultPoint{30, 110, 220};
    fge::Color g_colorBackground{250, 250, 250};
    fge::Color g_colorAxis{60, 60, 60};
    fge::Color g_colorGrid{220, 220, 220};
    fge::Color g_colorLabel{40, 40, 40};
    bool g_showGrid{true};
    bool g_showLines{true};
    float g_lineThickness{FGE_OBJCHART_DEFAULT_LINE_THICKNESS};
    fge::Color g_colorLine{30, 110, 220};

    mutable bool g_needGeometryUpdate{true};
    mutable fge::vulkan::VertexBuffer g_frameVertices;
    mutable fge::vulkan::VertexBuffer g_pointVertices;
    mutable std::vector<fge::ObjText> g_xLabels;
    mutable std::vector<fge::ObjText> g_yLabels;
};

using ObjChartF = ObjChart<float>;
using ObjChartD = ObjChart<double>;

} // namespace fge

#include "C_objChart.inl"

#endif // _FGE_EXT_C_OBJCHART_HPP_INCLUDED

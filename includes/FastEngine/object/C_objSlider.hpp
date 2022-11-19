#ifndef _FGE_C_OBJSLIDER_HPP_INCLUDED
#define _FGE_C_OBJSLIDER_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/object/C_object.hpp"
#include "FastEngine/object/C_objSprite.hpp"
#include "FastEngine/C_guiElement.hpp"

#define FGE_OBJSLIDER_CLASSNAME "FGE:OBJ:SLIDER"

namespace fge
{

class FGE_API ObjSlider : public fge::Object, public fge::Subscriber, public fge::GuiElement
{
public:
    enum class PositionMode
    {
        MODE_MANUAL,
        MODE_AUTO
    };

    ObjSlider() = default;
    ~ObjSlider() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSlider)

    fge::GuiElement* getGuiElement() override
    {
        return this;
    }

    void first(fge::Scene* scene) override;
    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;
    FGE_OBJ_DRAW_DECLARE

    void setBottomOffset(float offset);
    float getBottomOffset() const;

    void setCursorRatio(float ratio);
    float getCursorRatio() const;
    bool isScrollPressed() const;

    void setPositionMode(ObjSlider::PositionMode modeX, ObjSlider::PositionMode modeY);

    void refreshPosition(const sf::Vector2f& targetSize);

    const char* getClassName() const override;

    const char* getReadableClassName() const override;

    fge::CallbackHandler<float> _onSlide;

private:
    void onGuiMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, fge::GuiElementContext& context);
    void onMouseButtonReleased(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg);
    void onMouseMoved(const fge::Event& evt, const sf::Event::MouseMoveEvent& arg);

    void onGuiResized(const fge::GuiElementHandler& handler, const sf::Vector2f& size);

    void onGuiVerify(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context) override;

    mutable sf::RectangleShape g_scrollRect;
    mutable sf::RectangleShape g_scrollBaseRect;

    fge::GuiElementHandler* g_guiElementHandler{nullptr};

    float g_bottomOffset{70.0f};
    ObjSlider::PositionMode g_positionModeX{ObjSlider::PositionMode::MODE_AUTO};
    ObjSlider::PositionMode g_positionModeY{ObjSlider::PositionMode::MODE_AUTO};

    bool g_scrollPressed{false};
    float g_scrollRelativePosY{0.0f};
    float g_scrollPositionY{0.0f};
};

}//end fge

#endif //_FGE_C_OBJSLIDER_HPP_INCLUDED
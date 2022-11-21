#ifndef _FGE_C_OBJSLIDER_HPP_INCLUDED
#define _FGE_C_OBJSLIDER_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/object/C_object.hpp"
#include "FastEngine/C_guiElement.hpp"

#define FGE_OBJSLIDER_CLASSNAME "FGE:OBJ:SLIDER"

namespace fge
{

class FGE_API ObjSlider : public fge::Object, public fge::Subscriber, public fge::GuiElement
{
public:
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

    void setSize(const fge::DynamicSize& size);
    sf::Vector2f getSize() const;

    void setCursorRatio(float ratio);
    float getCursorRatio() const;
    bool isScrollPressed() const;

    void refreshSize();

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    sf::FloatRect getGlobalBounds() const override;
    sf::FloatRect getLocalBounds() const override;

    fge::CallbackHandler<float> _onSlide;

private:
    void onGuiMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, fge::GuiElementContext& context);
    void onMouseButtonReleased(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg);
    void onMouseMoved(const fge::Event& evt, const sf::Event::MouseMoveEvent& arg);

    void onGuiResized(const fge::GuiElementHandler& handler, const sf::Vector2f& size);

    void onGuiVerify(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context) override;

    void refreshSize(const sf::Vector2f& targetSize);

    mutable sf::RectangleShape g_scrollRect;
    mutable sf::RectangleShape g_scrollBaseRect;

    fge::GuiElementHandler* g_guiElementHandler{nullptr};

    fge::DynamicSize g_size;

    bool g_scrollPressed{false};
    float g_scrollPositionY{0.0f};
    float g_scrollLastPositionY{0.0f};
    float g_lastMousePositionY{0.0f};
};

}//end fge

#endif //_FGE_C_OBJSLIDER_HPP_INCLUDED
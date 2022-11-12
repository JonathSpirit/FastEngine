#ifndef _FGE_C_OBJTEXTLIST_HPP_INCLUDED
#define _FGE_C_OBJTEXTLIST_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/object/C_object.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/C_guiElement.hpp"
#include <deque>

#define FGE_OBJTEXTLIST_CLASSNAME "FGE:OBJ:TEXTLIST"

namespace fge
{

class ObjWindow;

class FGE_API ObjTextList : public fge::Object, public fge::Subscriber
{
public:
    ObjTextList() = default;
    ~ObjTextList() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjTextList)

    void first(fge::Scene* scene) override;
    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;
    FGE_OBJ_DRAW_DECLARE

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    void onGuiResized(const fge::GuiElementHandler& handler, const sf::Vector2f& size);

    void addString(tiny_utf8::string string);
    std::size_t getStringsSize() const;
    tiny_utf8::string& getString(std::size_t index);
    const tiny_utf8::string& getString(std::size_t index) const;
    void removeAllStrings();

    void setFont(const fge::Font& font);
    const fge::Font& getFont() const;

    void setBottomOffset(float offset);
    float getBottomOffset() const;

    void setCursorRatio(float ratio);
    float getCursorRatio() const;
    bool isScrollPressed() const;

    void setMaxStrings(std::size_t max);
    std::size_t getMaxStrings() const;

private:
    void refreshPosition(const sf::Vector2f& size);
    void onGuiMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, fge::GuiElementContext& context);
    void onMouseButtonReleased(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg);
    void onMouseMoved(const fge::Event& evt, const sf::Event::MouseMoveEvent& arg);

    fge::ObjWindow* g_parentPtr{nullptr};
    mutable fge::ObjText g_text;
    mutable sf::RectangleShape g_scrollRect;
    mutable sf::RectangleShape g_scrollBaseRect;
    fge::TunnelGate<fge::GuiElement> g_defaultElementGate;
    //fge::GuiElementDefault g_defaultElement{FGE_WINDOW_DEFAULT_PRIORITY};

    fge::GuiElementHandler* g_guiElementHandler{nullptr};

    std::deque<tiny_utf8::string> g_stringList;
    std::size_t g_maxStrings{100};
    float g_bottomOffset{70.0f};

    bool g_scrollPressed{false};
    float g_scrollRelativePosY{0.0f};
    float g_scrollPositionY{0.0f};
};

}//end fge

#endif //_FGE_C_OBJTEXTLIST_HPP_INCLUDED
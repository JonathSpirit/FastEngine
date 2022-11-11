#ifndef _FGE_C_OBJWINDOW_HPP_INCLUDED
#define _FGE_C_OBJWINDOW_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/object/C_object.hpp"
#include "FastEngine/C_scene.hpp"

#include "FastEngine/C_tileset.hpp"
#include "FastEngine/object/C_objSprite.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/C_guiElement.hpp"
#include <deque>

#define FGE_WINDOW_DEFAULT_PRIORITY 1
#define FGE_WINDOW_DEFAULT_SIZE_X 120.0f
#define FGE_WINDOW_DEFAULT_SIZE_Y 200.0f
#define FGE_WINDOW_PIXEL_SIZE 6
#define FGE_WINDOW_RANGEMAX_PRIORITY 300

#define FGE_OBJWINDOW_CLASSNAME "FGE:OBJ:WINDOW"
#define FGE_OBJTEXTLIST_CLASSNAME "FGE:OBJ:TEXTLIST"

namespace fge
{

class ObjWindow : public fge::Object, public fge::Subscriber, public fge::GuiElementRecursive
{
public:
    enum class SizeMode
    {
        MODE_FREE,
        MODE_FIXED
    };

    ObjWindow();
    ~ObjWindow() override = default;

    fge::Object* copy() override
    {
        return new fge::ObjWindow();
    }

    fge::GuiElement* getGuiElement() override
    {
        return this;
    }

    void first(fge::Scene* scene) override;
    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;
    void removed(fge::Scene* scene) override;

    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    sf::FloatRect getGlobalBounds() const override;
    sf::FloatRect getLocalBounds() const override;

    void setHeight(float height);
    void setSize(const sf::Vector2f& size);
    const sf::Vector2f& getSize() const;
    sf::Vector2f getDrawAreaSize() const;

    void showExitButton(bool enable);
    void makeMovable(bool enable);
    void makeResizable(bool enable);

    void setResizeMode(ObjWindow::SizeMode modeX, ObjWindow::SizeMode modeY);

    void setViewCenterOffset(const sf::Vector2f& offset);
    const sf::Vector2f& getViewCenterOffset() const;

    static fge::ObjWindow* getWindowFromScene(fge::Scene* scene)
    {
        if (scene != nullptr)
        {
            fge::ObjectDataShared obj{nullptr};
            if ( scene->_properties.getProperty("parent").get(obj) )
            {
                return reinterpret_cast<fge::ObjWindow*>( obj->getObject() );
            }
        }
        return nullptr;
    }

    void setThisAsParent(const fge::ObjectDataShared& object)
    {
        auto myObj = this->_myObjectData.lock();
        object->setParent(myObj);
    }

    fge::Scene _windowScene;
    fge::GuiElementHandler _windowHandler;
    mutable sf::View _windowView;

private:
    void onGuiVerify(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context) override;

    void onGuiMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, fge::GuiElementContext& context);
    void onMouseButtonReleased(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg);
    void onMouseMoved(const fge::Event& evt, const sf::Event::MouseMoveEvent& arg);

    void onPlanUpdate(fge::Scene* scene, fge::ObjectPlan plan);

    void onRefreshGlobalScale(const sf::Vector2f& scale);

    void refreshRectBounds();

    bool g_movingWindowFlag{false};
    bool g_resizeWindowFlag{false};
    sf::Vector2f g_mouseClickLastPosition;
    sf::Vector2f g_mouseClickLastSize;
    sf::Vector2f g_size{FGE_WINDOW_DEFAULT_SIZE_X, FGE_WINDOW_DEFAULT_SIZE_Y};

    bool g_showCloseButton{true};
    bool g_makeMovable{true};
    bool g_makeResizable{true};

    fge::ObjWindow::SizeMode g_resizeModeX{fge::ObjWindow::SizeMode::MODE_FREE};
    fge::ObjWindow::SizeMode g_resizeModeY{fge::ObjWindow::SizeMode::MODE_FREE};

    fge::GuiElementHandler* g_guiElementHandler{nullptr};

    sf::Vector2f g_viewCenterOffset;

    ///Textures
    sf::FloatRect g_windowMoveRect;
    sf::FloatRect g_windowMinimizeRect;
    sf::FloatRect g_windowCloseRect;
    sf::FloatRect g_windowResizeRect;
    fge::Texture g_windowMinimizeTexture{"gui_window_minimize"};
    fge::Texture g_windowCloseTexture{"gui_window_close"};
    fge::Texture g_windowResizeTexture{"gui_window_resize"};
    fge::TileSet g_windowTextures{"gui_window", {FGE_WINDOW_PIXEL_SIZE, FGE_WINDOW_PIXEL_SIZE}};
    mutable fge::ObjSprite g_sprite;
};

class ObjTextList : public fge::Object, public fge::Subscriber
{
public:
    ObjTextList() = default;
    ~ObjTextList() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjTextList)

    void first(fge::Scene* scene) override;
    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;
    FGE_OBJ_DRAW_DECLARE

    void onGuiResized(const fge::GuiElementHandler& handler, const sf::Vector2f& size);

    void addString(std::string string);
    std::size_t getStringsSize() const;
    std::string& getString(std::size_t index);
    const std::string& getString(std::size_t index) const;
    void removeAllStrings();

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
    fge::GuiElementDefault g_defaultElement{FGE_WINDOW_DEFAULT_PRIORITY};

    std::deque<std::string> g_stringList;
    std::size_t g_maxStrings{100};
    float g_bottomOffset{70.0f};

    bool g_scrollPressed{false};
    float g_scrollRelativePosY{0.0f};
    float g_scrollPositionY{0.0f};
};

}//end fge

#endif //_FGE_C_OBJWINDOW_HPP_INCLUDED
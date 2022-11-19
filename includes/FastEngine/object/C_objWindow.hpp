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

#ifndef _FGE_C_OBJWINDOW_HPP_INCLUDED
#define _FGE_C_OBJWINDOW_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/object/C_object.hpp"
#include "FastEngine/C_scene.hpp"

#include "FastEngine/C_tileset.hpp"
#include "FastEngine/object/C_objSprite.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/C_guiElement.hpp"

#define FGE_WINDOW_DEFAULT_PRIORITY 1
#define FGE_WINDOW_DEFAULT_SIZE_X 120.0f
#define FGE_WINDOW_DEFAULT_SIZE_Y 200.0f
#define FGE_WINDOW_PIXEL_SIZE 6
#define FGE_WINDOW_RANGEMAX_PRIORITY 300

#define FGE_OBJWINDOW_CLASSNAME "FGE:OBJ:WINDOW"
#define FGE_OBJWINDOW_SCENE_PARENT_PROPERTY "_OBJWINDOW_PARENT_"

namespace fge
{

class FGE_API ObjWindow : public fge::Object, public fge::Subscriber, public fge::GuiElementRecursive
{
public:
    enum class ResizeModes
    {
        MODE_FREE,
        MODE_FIXED
    };

    ObjWindow();
    ~ObjWindow() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjWindow)

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

    void setResizeMode(ObjWindow::ResizeModes modeX, ObjWindow::ResizeModes modeY);

    void setViewCenterOffset(const sf::Vector2f& offset);
    const sf::Vector2f& getViewCenterOffset() const;

    static fge::ObjWindow* getWindowObjectFromScene(fge::Scene* scene);

    void setTextureMinimize(fge::Texture texture);
    void setTextureClose(fge::Texture texture);
    void setTextureResize(fge::Texture texture);

    const fge::Texture& getTextureMinimize() const;
    const fge::Texture& getTextureClose() const;
    const fge::Texture& getTextureResize() const;

    fge::TileSet& getTileSet();
    const fge::TileSet& getTileSet() const;

    void refreshRectBounds();

    fge::Scene _windowScene;
    fge::GuiElementHandler _windowHandler;
    mutable std::shared_ptr<sf::View> _windowView;

private:
    void onGuiVerify(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context) override;

    void onGuiMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, fge::GuiElementContext& context);
    void onMouseButtonReleased(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg);
    void onMouseMoved(const fge::Event& evt, const sf::Event::MouseMoveEvent& arg);

    void onPlanUpdate(fge::Scene* scene, fge::ObjectPlan plan);
    void onNewObject(fge::Scene* scene, fge::ObjectDataShared object);

    void onRefreshGlobalScale(const sf::Vector2f& scale);

    bool g_movingWindowFlag{false};
    bool g_resizeWindowFlag{false};
    sf::Vector2f g_mouseClickLastPosition;
    sf::Vector2f g_mouseClickLastSize;
    sf::Vector2f g_size{FGE_WINDOW_DEFAULT_SIZE_X, FGE_WINDOW_DEFAULT_SIZE_Y};

    bool g_showCloseButton{true};
    bool g_makeMovable{true};
    bool g_makeResizable{true};

    fge::ObjWindow::ResizeModes g_resizeModeX{fge::ObjWindow::ResizeModes::MODE_FREE};
    fge::ObjWindow::ResizeModes g_resizeModeY{fge::ObjWindow::ResizeModes::MODE_FREE};

    fge::GuiElementHandler* g_guiElementHandler{nullptr};

    sf::Vector2f g_viewCenterOffset;

    //Textures
    fge::Texture g_textureWindowMinimize{};
    fge::Texture g_textureWindowClose{};
    fge::Texture g_textureWindowResize{};
    fge::TileSet g_tileSetWindow{{}, {FGE_WINDOW_PIXEL_SIZE, FGE_WINDOW_PIXEL_SIZE}};

    sf::FloatRect g_windowMoveRect;
    sf::FloatRect g_windowMinimizeRect;
    sf::FloatRect g_windowCloseRect;
    sf::FloatRect g_windowResizeRect;
    
    mutable fge::ObjSprite g_sprite;
};

}//end fge

#endif //_FGE_C_OBJWINDOW_HPP_INCLUDED
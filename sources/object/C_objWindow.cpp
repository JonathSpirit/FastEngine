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

#include "FastEngine/object/C_objWindow.hpp"
#include "FastEngine/extra/extra_function.hpp"

#define FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT 20.0f
#define FGE_WINDOW_DRAW_RESIZE_TEXTURE_OFFSET 1
#define FGE_WINDOW_DRAW_CLOSE_TEXTURE_OFFSET 5
#define FGE_WINDOW_DRAW_MINIMIZE_TEXTURE_OFFSET 5

#define FGE_WINDOW_DRAW_TILESET_UP_LEFT_CORNER_FRAME 0
#define FGE_WINDOW_DRAW_TILESET_UP_FRAME 1
#define FGE_WINDOW_DRAW_TILESET_FILL 2
#define FGE_WINDOW_DRAW_TILESET_LEFT_FRAME 3
#define FGE_WINDOW_DRAW_TILESET_UP_LIMIT 4
#define FGE_WINDOW_DRAW_TILESET_UP_RIGHT_CORNER_FRAME 5
#define FGE_WINDOW_DRAW_TILESET_RIGHT_FRAME 6
#define FGE_WINDOW_DRAW_TILESET_DOWN_LIMIT 7
#define FGE_WINDOW_DRAW_TILESET_DOWN_LEFT_CORNER_FRAME 8
#define FGE_WINDOW_DRAW_TILESET_DOWN_RIGHT_CORNER_FRAME 9
#define FGE_WINDOW_DRAW_TILESET_DOWN_FRAME 10

namespace fge
{

ObjWindow::ObjWindow() :
    fge::GuiElement(FGE_WINDOW_DEFAULT_PRIORITY)
{
}

void ObjWindow::first(fge::Scene* scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

    this->refreshRectBounds();

    this->setPriority(FGE_WINDOW_DEFAULT_PRIORITY);
    this->setScale(fge::GuiElement::getGlobalGuiScale());

    this->_windowScene._properties.setProperty(FGE_OBJWINDOW_SCENE_PARENT_PROPERTY, this);
    this->_windowScene.setLinkedRenderTarget( scene->getLinkedRenderTarget() );
    this->_windowView.reset(new sf::View{});
    this->_windowScene.setCustomView(this->_windowView);
}
void ObjWindow::callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->detachAll();

    this->_windowHandler._lastSize = this->getDrawAreaSize();
    this->_windowHandler.setEvent(event);
    if (guiElementHandlerPtr != nullptr)
    {
        this->_windowHandler.setRenderTarget(guiElementHandlerPtr->getRenderTarget());
    }
    this->_windowScene.setCallbackContext({&event, &this->_windowHandler});

    this->_windowScene._onNewObject.add( new fge::CallbackFunctorObject(&fge::ObjWindow::onNewObject, this), this );

    fge::GuiElement::_onGlobalGuiScaleChange.add(new fge::CallbackFunctorObject(&fge::ObjWindow::onRefreshGlobalScale, this), this);
    this->_myObjectData.lock()->getLinkedScene()->_onPlanUpdate.add( new fge::CallbackFunctorObject(&fge::ObjWindow::onPlanUpdate, this), this );

    this->g_guiElementHandler = guiElementHandlerPtr;

    guiElementHandlerPtr->_onGuiVerify.add( new fge::CallbackFunctorObject(&fge::ObjWindow::onGuiVerify, this), this );
    this->_onGuiMouseButtonPressed.add(new fge::CallbackFunctorObject(&fge::ObjWindow::onGuiMouseButtonPressed, this), this);

    event._onMouseMoved.add(new fge::CallbackFunctorObject(&fge::ObjWindow::onMouseMoved, this), this);
    event._onMouseButtonReleased.add(new fge::CallbackFunctorObject(&fge::ObjWindow::onMouseButtonReleased, this), this);
}
void ObjWindow::removed([[maybe_unused]] fge::Scene* scene)
{
    this->detachAll();
}

#ifdef FGE_DEF_SERVER
FGE_OBJ_UPDATE_BODY(ObjWindow)
{
    this->_windowScene.update(event, deltaTime);
}
#else
FGE_OBJ_UPDATE_BODY(ObjWindow)
{
    this->_windowScene.update(screen, event, deltaTime);
}
#endif

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjWindow)
{
    *this->_windowView = target.getDefaultView();
    auto backupView = target.getView();
    target.setView(*this->_windowView);

    states.transform *= this->getTransform();

    this->g_sprite.setTexture(this->g_tileSetWindow.getTexture());

    //Fill
    this->g_sprite.setScale({(this->g_size.x - FGE_WINDOW_PIXEL_SIZE / 2.0f) / FGE_WINDOW_PIXEL_SIZE, (this->g_size.y - FGE_WINDOW_PIXEL_SIZE / 2.0f) / FGE_WINDOW_PIXEL_SIZE});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_FILL).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(FGE_WINDOW_PIXEL_SIZE / 2.0f, FGE_WINDOW_PIXEL_SIZE / 2.0f);
    target.draw(this->g_sprite, states);

    //Limit
    this->g_sprite.setScale({this->g_size.x / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_LIMIT).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(0.0f, FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT - FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({this->g_size.x / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_LIMIT).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(0.0f, FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT);
    target.draw(this->g_sprite, states);

    //Frame
    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_LEFT_CORNER_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(0, 0);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({(this->g_size.x - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(FGE_WINDOW_PIXEL_SIZE, 0);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_RIGHT_CORNER_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(this->g_size.x - FGE_WINDOW_PIXEL_SIZE, 0);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({1.0f, (this->g_size.y - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_LEFT_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(0, FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_LEFT_CORNER_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(0, this->g_size.y - FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({(this->g_size.x - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(FGE_WINDOW_PIXEL_SIZE, this->g_size.y - FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_RIGHT_CORNER_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(this->g_size.x - FGE_WINDOW_PIXEL_SIZE, this->g_size.y - FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({1.0f, (this->g_size.y - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE});
    this->g_sprite.setTextureRect( this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_RIGHT_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(this->g_size.x - FGE_WINDOW_PIXEL_SIZE, FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    //Others
    if (this->g_makeResizable)
    {
        this->g_sprite.setScale({1.0f, 1.0f});
        this->g_sprite.setTexture(this->g_textureWindowResize, true);
        this->g_sprite.setPosition(this->g_windowResizeRect.getPosition());
        target.draw(this->g_sprite, states);
    }

    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTexture(this->g_textureWindowClose, true);
    this->g_sprite.setPosition(this->g_windowCloseRect.getPosition());
    this->g_sprite.setColor( this->g_showCloseButton ? sf::Color::White : sf::Color(100,100,100,200) );
    target.draw(this->g_sprite, states);
    this->g_sprite.setColor(sf::Color::White);

    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTexture(this->g_textureWindowMinimize, true);
    this->g_sprite.setPosition(this->g_windowMinimizeRect.getPosition());
    target.draw(this->g_sprite, states);

    //Drawing elements
    auto worldCoord = states.transform.transformRect({sf::Vector2f{0.0f,FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT}, this->getDrawAreaSize()});
    *this->_windowView = fge::ClipView(*this->_windowView, target,
                                       worldCoord, fge::ClipClampModes::CLIP_CLAMP_NOTHING);
    this->_windowView->setCenter(this->_windowView->getCenter() - (worldCoord.getPosition()-states.transform.transformPoint({})) );

    this->_windowScene.draw(target, false, sf::Color::White, states);

    target.setView(backupView);
}
#endif

const char* ObjWindow::getClassName() const
{
    return FGE_OBJWINDOW_CLASSNAME;
}
const char* ObjWindow::getReadableClassName() const
{
    return "window";
}

sf::FloatRect ObjWindow::getGlobalBounds() const
{
    return this->getTransform().transformRect(this->getLocalBounds());
}
sf::FloatRect ObjWindow::getLocalBounds() const
{
    return {{0,0}, this->g_size};
}

void ObjWindow::setHeight(float height)
{
    this->setSize({this->g_size.x, height});
}
void ObjWindow::setSize(const sf::Vector2f& size)
{
    const sf::RenderTarget& renderTarget = this->g_guiElementHandler->getRenderTarget();

    this->g_size.x = std::clamp(size.x, static_cast<float>(this->g_textureWindowResize.getTextureSize().x * 3), renderTarget.getDefaultView().getSize().x);
    this->g_size.y = std::clamp(size.y, static_cast<float>(this->g_textureWindowResize.getTextureSize().y) + FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT, renderTarget.getDefaultView().getSize().y);

    this->refreshRectBounds();
    this->_windowHandler._onGuiResized.call(this->_windowHandler, this->getDrawAreaSize());
    this->_windowHandler._lastSize = this->getDrawAreaSize();
}
const sf::Vector2f& ObjWindow::getSize() const
{
    return this->g_size;
}
sf::Vector2f ObjWindow::getDrawAreaSize() const
{
    return this->g_size - sf::Vector2f{0.0f, FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT};
}

void ObjWindow::showExitButton(bool enable)
{
    this->g_showCloseButton = enable;
}
void ObjWindow::makeMovable(bool enable)
{
    this->g_makeMovable = enable;
}
void ObjWindow::makeResizable(bool enable)
{
    this->g_makeResizable = enable;
}

void ObjWindow::setResizeMode(ObjWindow::ResizeModes modeX, ObjWindow::ResizeModes modeY)
{
    this->g_resizeModeX = modeX;
    this->g_resizeModeY = modeY;
}

void ObjWindow::setViewCenterOffset(const sf::Vector2f& offset)
{
    this->g_viewCenterOffset = offset;
}
const sf::Vector2f& ObjWindow::getViewCenterOffset() const
{
    return this->g_viewCenterOffset;
}

fge::ObjWindow* ObjWindow::getWindowObjectFromScene(fge::Scene* scene)
{
    if (scene != nullptr)
    {
        return scene->_properties.getProperty(FGE_OBJWINDOW_SCENE_PARENT_PROPERTY).get<fge::ObjWindow*>().value_or(nullptr);
    }
    return nullptr;
}

void ObjWindow::onGuiVerify(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context)
{
    if (context._recursive)
    {
        fge::GuiElementContext context2{};
        context2._mouseGuiPosition = context._mouseGuiPosition;
        context2._mousePosition = context._mousePosition;
        context2._handler = &this->_windowHandler;
        context2._keepAliveObject = context._keepAliveObject;

        context._keepAliveObject->push_back(this->_myObjectData.lock());

        this->_windowHandler._onGuiVerify.call(evt, evtType, context2);

        if (context2._prioritizedElement != nullptr)
        {
            if (context2._prioritizedElement->isRecursive())
            {
                context2._recursive = true;
                context2._prioritizedElement->onGuiVerify(evt, evtType, context2);
            }
        }

        context._prioritizedElement = context2._prioritizedElement;
        context._index = context2._index;
    }
    else
    {
        if ( this->verifyPriority(context._prioritizedElement) )
        {
            sf::FloatRect rect{{0.0f,0.0f}, {this->g_size.x, this->g_size.y}};

            sf::Transform transform = this->getParentsTransform()*this->getTransform();

            if ( transform.transformRect(rect).contains(context._mouseGuiPosition) )
            {
                context._prioritizedElement = this;
            }
        }
    }
}

void ObjWindow::onGuiMouseButtonPressed([[maybe_unused]] const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, fge::GuiElementContext& context)
{
    if (arg.button == sf::Mouse::Left)
    {
        auto myObjectData = this->_myObjectData.lock();
        myObjectData->getLinkedScene()->setObjectPlanBot(myObjectData->getSid());

        auto transform = this->getParentsTransform() * this->getTransform();

        if (this->g_showCloseButton)
        {
            sf::FloatRect closeRectangle{transform.transformRect(this->g_windowCloseRect)};
            if (closeRectangle.contains(context._mouseGuiPosition))
            {
                myObjectData->getLinkedScene()->delObject(myObjectData->getSid());
                return;
            }
        }

        if (this->g_makeMovable)
        {
            sf::FloatRect moveRectangle{transform.transformRect(this->g_windowMoveRect)};
            if (moveRectangle.contains(context._mouseGuiPosition))
            {
                this->g_movingWindowFlag = true;
                this->g_mouseClickLastPosition = this->getPosition() - context._mouseGuiPosition;
                myObjectData->getLinkedScene()->callCmd("setCursor", this, sf::Cursor::Type::SizeAll, nullptr);
                return;
            }
        }

        if (this->g_makeResizable)
        {
            sf::FloatRect resizeRectangle{transform.transformRect(this->g_windowResizeRect)};
            if (resizeRectangle.contains(context._mouseGuiPosition))
            {
                this->g_resizeWindowFlag = true;
                this->g_mouseClickLastPosition = context._mouseGuiPosition;
                this->g_mouseClickLastSize = this->g_size;
                myObjectData->getLinkedScene()->callCmd("setCursor", this, sf::Cursor::Type::SizeBottomRight, nullptr);
                return;
            }
        }
    }
}
void ObjWindow::onMouseButtonReleased([[maybe_unused]] const fge::Event& evt, const sf::Event::MouseButtonEvent& arg)
{
    if (arg.button == sf::Mouse::Left)
    {
        if (this->g_movingWindowFlag || this->g_resizeWindowFlag)
        {
            this->g_movingWindowFlag = false;
            this->g_resizeWindowFlag = false;
            this->_myObjectData.lock()->getLinkedScene()->callCmd("setCursor", this, sf::Cursor::Type::Arrow, nullptr);
        }
    }
}
void ObjWindow::onMouseMoved([[maybe_unused]] const fge::Event& evt, const sf::Event::MouseMoveEvent& arg)
{
    if (this->g_movingWindowFlag)
    {
        const sf::RenderTarget& renderTarget = this->g_guiElementHandler->getRenderTarget();

        sf::Vector2f mousePos = renderTarget.mapPixelToCoords({arg.x, arg.y}, renderTarget.getDefaultView());

        sf::Vector2f newPosition = mousePos + this->g_mouseClickLastPosition;
        newPosition.x = std::clamp(newPosition.x, 0.0f, renderTarget.getDefaultView().getSize().x-static_cast<float>(this->g_textureWindowResize.getTextureSize().x));
        newPosition.y = std::clamp(newPosition.y, 0.0f, renderTarget.getDefaultView().getSize().y - FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT);

        this->setPosition(newPosition);
    }
    else if (this->g_resizeWindowFlag)
    {
        const sf::RenderTarget& renderTarget = this->g_guiElementHandler->getRenderTarget();

        sf::Vector2f mousePos = renderTarget.mapPixelToCoords({arg.x, arg.y}, renderTarget.getDefaultView());

        sf::Vector2f mouseDiff{this->g_resizeModeX == ObjWindow::ResizeModes::MODE_FREE ? (mousePos.x - this->g_mouseClickLastPosition.x) / this->getScale().x : 0.0f,
                               this->g_resizeModeY == ObjWindow::ResizeModes::MODE_FREE ? (mousePos.y - this->g_mouseClickLastPosition.y) / this->getScale().y : 0.0f};

        this->setSize(this->g_mouseClickLastSize + mouseDiff);
    }
}

void ObjWindow::onPlanUpdate([[maybe_unused]] fge::Scene* scene, fge::ObjectPlan plan)
{
    auto myObjectData = this->_myObjectData.lock();
    if (myObjectData->getPlan() == plan)
    {
        if (myObjectData->getPlanDepth() == FGE_SCENE_BAD_PLANDEPTH)
        {
            this->setPriority(FGE_WINDOW_DEFAULT_PRIORITY);
        }
        else
        {
            this->setPriority(FGE_WINDOW_RANGEMAX_PRIORITY - myObjectData->getPlanDepth());
        }
    }
}
void ObjWindow::onNewObject([[maybe_unused]] fge::Scene* scene, fge::ObjectDataShared object)
{
    object->setParent(this->_myObjectData.lock());
}

void ObjWindow::onRefreshGlobalScale(const sf::Vector2f& scale)
{
    this->setScale(scale);
    this->_windowHandler._onGuiResized.call(this->_windowHandler, this->getDrawAreaSize());
}

void ObjWindow::setTextureMinimize(fge::Texture texture)
{
    this->g_textureWindowMinimize = std::move(texture);
    this->refreshRectBounds();
}
void ObjWindow::setTextureClose(fge::Texture texture)
{
    this->g_textureWindowClose = std::move(texture);
    this->refreshRectBounds();
}
void ObjWindow::setTextureResize(fge::Texture texture)
{
    this->g_textureWindowResize = std::move(texture);
    this->refreshRectBounds();
}

const fge::Texture& ObjWindow::getTextureMinimize() const
{
    return this->g_textureWindowMinimize;
}
const fge::Texture& ObjWindow::getTextureClose() const
{
    return this->g_textureWindowClose;
}
const fge::Texture& ObjWindow::getTextureResize() const
{
    return this->g_textureWindowResize;
}

fge::TileSet& ObjWindow::getTileSet()
{
    return this->g_tileSetWindow;
}
const fge::TileSet& ObjWindow::getTileSet() const
{
    return this->g_tileSetWindow;
}

void ObjWindow::refreshRectBounds()
{
    this->g_windowMoveRect.width = this->g_size.x;
    this->g_windowMoveRect.height = FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT;

    this->g_windowResizeRect.left = this->g_size.x-static_cast<float>(this->g_textureWindowResize.getTextureSize().x + FGE_WINDOW_DRAW_RESIZE_TEXTURE_OFFSET);
    this->g_windowResizeRect.top = this->g_size.y-static_cast<float>(this->g_textureWindowResize.getTextureSize().y + FGE_WINDOW_DRAW_RESIZE_TEXTURE_OFFSET);
    this->g_windowResizeRect.width = static_cast<float>(this->g_textureWindowResize.getTextureSize().x);
    this->g_windowResizeRect.height = static_cast<float>(this->g_textureWindowResize.getTextureSize().y);

    this->g_windowCloseRect.left = this->g_size.x-static_cast<float>(this->g_textureWindowClose.getTextureSize().x + FGE_WINDOW_DRAW_CLOSE_TEXTURE_OFFSET);
    this->g_windowCloseRect.top = FGE_WINDOW_DRAW_CLOSE_TEXTURE_OFFSET;
    this->g_windowCloseRect.width = static_cast<float>(this->g_textureWindowClose.getTextureSize().x);
    this->g_windowCloseRect.height = static_cast<float>(this->g_textureWindowClose.getTextureSize().y);

    this->g_windowMinimizeRect.left = this->g_size.x-static_cast<float>(this->g_textureWindowMinimize.getTextureSize().x + this->g_textureWindowClose.getTextureSize().x + FGE_WINDOW_DRAW_MINIMIZE_TEXTURE_OFFSET);
    this->g_windowMinimizeRect.top = FGE_WINDOW_DRAW_MINIMIZE_TEXTURE_OFFSET;
    this->g_windowMinimizeRect.width = static_cast<float>(this->g_textureWindowMinimize.getTextureSize().x);
    this->g_windowMinimizeRect.height = static_cast<float>(this->g_textureWindowMinimize.getTextureSize().y);
}

}//end fge
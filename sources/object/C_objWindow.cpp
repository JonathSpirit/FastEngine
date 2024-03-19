/*
 * Copyright 2024 Guillaume Guillet
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
{}
ObjWindow::ObjWindow(ObjWindow const& r) :
        fge::GuiElement(r),
        fge::Object(r),
        fge::Subscriber(r),

        g_movingWindowFlag(false),
        g_resizeWindowFlag(false),
        g_size(r.g_size),

        g_showCloseButton(r.g_showCloseButton),
        g_makeMovable(r.g_makeMovable),
        g_makeResizable(r.g_makeResizable),

        g_resizeModeX(r.g_resizeModeX),
        g_resizeModeY(r.g_resizeModeY),

        g_guiElementHandler(nullptr),

        g_viewCenterOffset(r.g_viewCenterOffset),

        g_textureWindowMinimize(r.g_textureWindowMinimize),
        g_textureWindowClose(r.g_textureWindowClose),
        g_textureWindowResize(r.g_textureWindowResize),
        g_tileSetWindow(r.g_tileSetWindow)
{
    this->_windowScene._properties = r._windowScene._properties;

    //                 old sid         new sid
    std::unordered_map<fge::ObjectSid, fge::ObjectSid> oldSidMap;

    this->_windowScene.delAllObject(false);
    for (auto const& objectData: r._windowScene)
    {
        auto newObject =
                this->_windowScene.newObject(FGE_NEWOBJECT_PTR(objectData->getObject()->copy()), objectData->getPlan(),
                                             FGE_SCENE_BAD_SID, objectData->getType(), true);
        if (newObject)
        {
            oldSidMap[objectData->getSid()] = newObject->getSid();
        }
    }

    //Moving all anchors successor and anchor target as they don't point anymore to the wanted ones
    for (auto const& objectData: r._windowScene)
    { //TODO: Use that code inside of scene copy too ?
        if (auto oldSuccessor = objectData->getObject()->getAnchorSuccessor().lock())
        {
            auto itNewObjectSid = oldSidMap.find(objectData->getSid());
            auto itNewSuccessorSid = oldSidMap.find(oldSuccessor->getSid());

            if (itNewObjectSid != oldSidMap.end() && itNewSuccessorSid != oldSidMap.end())
            {
                auto newObject = this->_windowScene.getObject(itNewObjectSid->second);
                auto newSuccessor = this->_windowScene.getObject(itNewSuccessorSid->second);

                newObject->getObject()->setAnchorSuccessor(newSuccessor);
            }
        }

        auto oldTargetSid = objectData->getObject()->getAnchorTarget();
        if (oldTargetSid != FGE_SCENE_BAD_SID)
        {
            auto itNewObjectSid = oldSidMap.find(objectData->getSid());
            auto itNewTargetSid = oldSidMap.find(oldTargetSid);

            if (itNewObjectSid != oldSidMap.end() && itNewTargetSid != oldSidMap.end())
            {
                auto newObject = this->_windowScene.getObject(itNewObjectSid->second);
                newObject->getObject()->setAnchorTarget(itNewTargetSid->second);
            }
        }
    }

    this->g_spriteBatches.setTexture(this->g_tileSetWindow.getTexture());
}

void ObjWindow::first(fge::Scene* scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

    this->refreshRectBounds();

    this->setScale(fge::GuiElement::getGlobalGuiScale());

    this->_windowScene._properties.setProperty(FGE_OBJWINDOW_SCENE_PARENT_PROPERTY, this);
    this->_windowScene.setLinkedRenderTarget(scene->getLinkedRenderTarget());
    this->_windowView.reset(new fge::View{});
    this->_windowScene.setCustomView(this->_windowView);

    //Call "first" on pre-existent objects (copied from another scene)
    auto myObjectData = this->_myObjectData.lock();
    for (auto const& objectData: this->_windowScene)
    {
        //TODO: Make sure that the scene correctly transfer parent reference when copying
        objectData->setParent(myObjectData); //Be sure that the parent is set
        objectData->getObject()->first(scene);
    }

    this->refreshTextures();
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

    this->_windowScene._onObjectAdded.addObjectFunctor(&fge::ObjWindow::onObjectAdded, this, this);

    fge::GuiElement::_onGlobalGuiScaleChange.addObjectFunctor(&fge::ObjWindow::onRefreshGlobalScale, this, this);
    this->_myObjectData.lock()->getLinkedScene()->_onPlanUpdate.addObjectFunctor(&fge::ObjWindow::onPlanUpdate, this,
                                                                                 this);

    this->g_guiElementHandler = guiElementHandlerPtr;

    guiElementHandlerPtr->_onGuiVerify.addObjectFunctor(&fge::ObjWindow::onGuiVerify, this, this);
    this->_onGuiMouseButtonPressed.addObjectFunctor(&fge::ObjWindow::onGuiMouseButtonPressed, this, this);

    event._onMouseMotion.addObjectFunctor(&fge::ObjWindow::onMouseMoved, this, this);
    event._onMouseButtonUp.addObjectFunctor(&fge::ObjWindow::onMouseButtonReleased, this, this);

    //Call "callbackRegister" on pre-existent objects (copied from another scene)
    for (auto const& objectData: this->_windowScene)
    {
        objectData->getObject()->callbackRegister(event, &this->_windowHandler);
        objectData->getObject()->needAnchorUpdate(false);
    }
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

    auto copyStates = states.copy(this->_transform.start(*this, states._resTransform.get()));

    target.draw(this->g_spriteBatches, copyStates);

    //Others
    if (this->g_makeResizable)
    {
        target.draw(this->g_spriteResize, copyStates);
    }

    target.draw(this->g_spriteClose, copyStates);
    target.draw(this->g_spriteMinimize, copyStates);

    //Drawing elements
    auto worldCoord =
            copyStates._resTransform.get()->getData()._modelTransform *
            fge::RectFloat{fge::Vector2f{0.0f, FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT}, this->getDrawAreaSize()};
    *this->_windowView = fge::ClipView(*this->_windowView, target, worldCoord, fge::ClipClampModes::CLIP_CLAMP_NOTHING);
    this->_windowView->setCenter(
            this->_windowView->getCenter() -
            (worldCoord.getPosition() - copyStates._resTransform.get()->getData()._modelTransform * fge::Vector2f{}));

    this->_windowScene.draw(target, copyStates);

    target.setView(backupView);
}
#endif

char const* ObjWindow::getClassName() const
{
    return FGE_OBJWINDOW_CLASSNAME;
}
char const* ObjWindow::getReadableClassName() const
{
    return "window";
}

fge::RectFloat ObjWindow::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjWindow::getLocalBounds() const
{
    return {{0, 0}, this->g_size};
}

void ObjWindow::setHeight(float height)
{
    this->setSize({this->g_size.x, height});
}
void ObjWindow::setSize(fge::Vector2f const& size)
{
    fge::RenderTarget const& renderTarget = this->g_guiElementHandler->getRenderTarget();

    this->g_size.x = std::clamp(size.x, static_cast<float>(this->g_textureWindowResize.getTextureSize().x * 3),
                                renderTarget.getDefaultView().getSize().x);
    this->g_size.y = std::clamp(size.y,
                                static_cast<float>(this->g_textureWindowResize.getTextureSize().y) +
                                        FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT,
                                renderTarget.getDefaultView().getSize().y);

    this->refreshRectBounds();
    this->_windowHandler._onGuiResized.call(this->_windowHandler, this->getDrawAreaSize());
    this->_windowHandler._lastSize = this->getDrawAreaSize();
}
fge::Vector2f const& ObjWindow::getSize() const
{
    return this->g_size;
}
fge::Vector2f ObjWindow::getDrawAreaSize() const
{
    return this->g_size - fge::Vector2f{0.0f, FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT};
}

void ObjWindow::showExitButton(bool enable)
{
    this->g_showCloseButton = enable;
    this->g_spriteClose.setColor(enable ? fge::Color::White : fge::Color(100, 100, 100, 200));
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

void ObjWindow::setViewCenterOffset(fge::Vector2f const& offset)
{
    this->g_viewCenterOffset = offset;
}
fge::Vector2f const& ObjWindow::getViewCenterOffset() const
{
    return this->g_viewCenterOffset;
}

fge::ObjWindow* ObjWindow::getWindowObjectFromScene(fge::Scene* scene)
{
    if (scene != nullptr)
    {
        return scene->_properties.getProperty(FGE_OBJWINDOW_SCENE_PARENT_PROPERTY)
                .get<fge::ObjWindow*>()
                .value_or(nullptr);
    }
    return nullptr;
}

void ObjWindow::onGuiVerify(fge::Event const& evt, SDL_EventType evtType, fge::GuiElementContext& context)
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
        if (this->verifyPriority(context._prioritizedElement))
        {
            fge::RectFloat const rect{{0.0f, 0.0f}, {this->g_size.x, this->g_size.y}};

            auto transform = this->getParentsTransform() * this->getTransform();

            if ((transform * rect).contains(context._mouseGuiPosition))
            {
                context._prioritizedElement = this;
            }
        }
    }
}

void ObjWindow::onGuiMouseButtonPressed([[maybe_unused]] fge::Event const& evt,
                                        SDL_MouseButtonEvent const& arg,
                                        fge::GuiElementContext& context)
{
    if (arg.button == SDL_BUTTON_LEFT)
    {
        auto myObjectData = this->_myObjectData.lock();
        myObjectData->getLinkedScene()->setObjectPlanBot(myObjectData->getSid());
        myObjectData->getLinkedScene()->updateAllPlanDepth(myObjectData->getPlan());

        auto transform = this->getParentsTransform() * this->getTransform();

        if (this->g_showCloseButton)
        {
            fge::RectFloat const closeRectangle{transform * this->g_windowCloseRect};
            if (closeRectangle.contains(context._mouseGuiPosition))
            {
                this->_onWindowClose.call(*this);
                myObjectData->getLinkedScene()->delObject(myObjectData->getSid());
                return;
            }
        }

        if (this->g_makeMovable)
        {
            fge::RectFloat const moveRectangle{transform * this->g_windowMoveRect};
            if (moveRectangle.contains(context._mouseGuiPosition))
            {
                this->g_movingWindowFlag = true;
                this->g_mouseClickLastPosition = this->getPosition() - context._mouseGuiPosition;
                fge::SetSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
                return;
            }
        }

        if (this->g_makeResizable)
        {
            fge::RectFloat const resizeRectangle{transform * this->g_windowResizeRect};
            if (resizeRectangle.contains(context._mouseGuiPosition))
            {
                this->g_resizeWindowFlag = true;
                this->g_mouseClickLastPosition = context._mouseGuiPosition;
                this->g_mouseClickLastSize = this->g_size;
                fge::SetSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
                return;
            }
        }
    }
}
void ObjWindow::onMouseButtonReleased([[maybe_unused]] fge::Event const& evt, SDL_MouseButtonEvent const& arg)
{
    if (arg.button == SDL_BUTTON_LEFT)
    {
        if (this->g_movingWindowFlag || this->g_resizeWindowFlag)
        {
            this->g_movingWindowFlag = false;
            this->g_resizeWindowFlag = false;
            fge::SetSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        }
    }
}
void ObjWindow::onMouseMoved([[maybe_unused]] fge::Event const& evt, SDL_MouseMotionEvent const& arg)
{
    if (this->g_movingWindowFlag)
    {
        fge::RenderTarget const& renderTarget = this->g_guiElementHandler->getRenderTarget();

        fge::Vector2f mousePos = renderTarget.mapPixelToCoords({arg.x, arg.y}, renderTarget.getDefaultView());

        fge::Vector2f newPosition = mousePos + this->g_mouseClickLastPosition;
        newPosition.x = std::clamp(newPosition.x, 0.0f,
                                   renderTarget.getDefaultView().getSize().x -
                                           static_cast<float>(this->g_textureWindowResize.getTextureSize().x));
        newPosition.y = std::clamp(newPosition.y, 0.0f,
                                   renderTarget.getDefaultView().getSize().y - FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT);

        this->setPosition(newPosition);
    }
    else if (this->g_resizeWindowFlag)
    {
        fge::RenderTarget const& renderTarget = this->g_guiElementHandler->getRenderTarget();

        fge::Vector2f mousePos = renderTarget.mapPixelToCoords({arg.x, arg.y}, renderTarget.getDefaultView());

        fge::Vector2f mouseDiff{this->g_resizeModeX == ObjWindow::ResizeModes::MODE_FREE
                                        ? (mousePos.x - this->g_mouseClickLastPosition.x) / this->getScale().x
                                        : 0.0f,
                                this->g_resizeModeY == ObjWindow::ResizeModes::MODE_FREE
                                        ? (mousePos.y - this->g_mouseClickLastPosition.y) / this->getScale().y
                                        : 0.0f};

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
            myObjectData->getLinkedScene()->updatePlanDepth(myObjectData->getSid());
        }
        this->setPriority(myObjectData->getPlan() + myObjectData->getPlanDepth());
    }
}
void ObjWindow::onObjectAdded([[maybe_unused]] fge::Scene* scene, fge::ObjectDataShared const& object)
{
    object->setParent(this->_myObjectData.lock());
}

void ObjWindow::onRefreshGlobalScale(fge::Vector2f const& scale)
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

fge::Texture const& ObjWindow::getTextureMinimize() const
{
    return this->g_textureWindowMinimize;
}
fge::Texture const& ObjWindow::getTextureClose() const
{
    return this->g_textureWindowClose;
}
fge::Texture const& ObjWindow::getTextureResize() const
{
    return this->g_textureWindowResize;
}

void ObjWindow::setTexture(fge::Texture texture)
{
    this->g_tileSetWindow.setTexture(std::move(texture));
    this->g_spriteBatches.setTexture(this->g_tileSetWindow.getTexture());
    this->refreshTextures();
}
void ObjWindow::setTileSet(fge::TileSet const& tileSet)
{
    this->g_tileSetWindow = tileSet;
    this->g_spriteBatches.setTexture(this->g_tileSetWindow.getTexture());
    this->refreshTextures();
}
void ObjWindow::setTileSet(fge::TileSet&& tileSet)
{
    this->g_tileSetWindow = std::move(tileSet);
    this->g_tileSetWindow.setTexture(this->g_tileSetWindow.getTexture());
    this->refreshTextures();
}
fge::TileSet const& ObjWindow::getTileSet() const
{
    return this->g_tileSetWindow;
}

void ObjWindow::refreshRectBounds()
{
    this->g_windowMoveRect._width = this->g_size.x;
    this->g_windowMoveRect._height = FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT;

    this->g_windowResizeRect._x = this->g_size.x - static_cast<float>(this->g_textureWindowResize.getTextureSize().x +
                                                                      FGE_WINDOW_DRAW_RESIZE_TEXTURE_OFFSET);
    this->g_windowResizeRect._y = this->g_size.y - static_cast<float>(this->g_textureWindowResize.getTextureSize().y +
                                                                      FGE_WINDOW_DRAW_RESIZE_TEXTURE_OFFSET);
    this->g_windowResizeRect._width = static_cast<float>(this->g_textureWindowResize.getTextureSize().x);
    this->g_windowResizeRect._height = static_cast<float>(this->g_textureWindowResize.getTextureSize().y);

    this->g_windowCloseRect._x = this->g_size.x - static_cast<float>(this->g_textureWindowClose.getTextureSize().x +
                                                                     FGE_WINDOW_DRAW_CLOSE_TEXTURE_OFFSET);
    this->g_windowCloseRect._y = FGE_WINDOW_DRAW_CLOSE_TEXTURE_OFFSET;
    this->g_windowCloseRect._width = static_cast<float>(this->g_textureWindowClose.getTextureSize().x);
    this->g_windowCloseRect._height = static_cast<float>(this->g_textureWindowClose.getTextureSize().y);

    this->g_windowMinimizeRect._x =
            this->g_size.x -
            static_cast<float>(this->g_textureWindowMinimize.getTextureSize().x +
                               this->g_textureWindowClose.getTextureSize().x + FGE_WINDOW_DRAW_MINIMIZE_TEXTURE_OFFSET);
    this->g_windowMinimizeRect._y = FGE_WINDOW_DRAW_MINIMIZE_TEXTURE_OFFSET;
    this->g_windowMinimizeRect._width = static_cast<float>(this->g_textureWindowMinimize.getTextureSize().x);
    this->g_windowMinimizeRect._height = static_cast<float>(this->g_textureWindowMinimize.getTextureSize().y);

    this->refreshTextures();
}
void ObjWindow::refreshTextures()
{
    //Prepare sprite batches
    this->g_spriteBatches.resize(11);
    std::size_t index = 0;

    fge::Transformable* transformable = nullptr;

    //Fill
    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_FILL).value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({(this->g_size.x - FGE_WINDOW_PIXEL_SIZE / 2.0f) / FGE_WINDOW_PIXEL_SIZE,
                             (this->g_size.y - FGE_WINDOW_PIXEL_SIZE / 2.0f) / FGE_WINDOW_PIXEL_SIZE});
    transformable->setPosition({FGE_WINDOW_PIXEL_SIZE / 2.0f, FGE_WINDOW_PIXEL_SIZE / 2.0f});

    //Limit
    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_LIMIT).value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({this->g_size.x / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    transformable->setPosition({0.0f, FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT - FGE_WINDOW_PIXEL_SIZE});

    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_LIMIT).value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({this->g_size.x / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    transformable->setPosition({0.0f, FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT});

    //Frame
    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_LEFT_CORNER_FRAME)
                           .value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({1.0f, 1.0f});
    transformable->setPosition({0.0f, 0.0f});

    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_FRAME).value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({(this->g_size.x - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    transformable->setPosition({FGE_WINDOW_PIXEL_SIZE, 0.0f});

    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_RIGHT_CORNER_FRAME)
                           .value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({1.0f, 1.0f});
    transformable->setPosition({this->g_size.x - FGE_WINDOW_PIXEL_SIZE, 0.0f});

    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_LEFT_FRAME).value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({1.0f, (this->g_size.y - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE});
    transformable->setPosition({0.0f, FGE_WINDOW_PIXEL_SIZE});

    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_LEFT_CORNER_FRAME)
                           .value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({1.0f, 1.0f});
    transformable->setPosition({0, this->g_size.y - FGE_WINDOW_PIXEL_SIZE});

    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_FRAME).value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({(this->g_size.x - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    transformable->setPosition({FGE_WINDOW_PIXEL_SIZE, this->g_size.y - FGE_WINDOW_PIXEL_SIZE});

    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_RIGHT_CORNER_FRAME)
                           .value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({1.0f, 1.0f});
    transformable->setPosition({this->g_size.x - FGE_WINDOW_PIXEL_SIZE, this->g_size.y - FGE_WINDOW_PIXEL_SIZE});

    this->g_spriteBatches.setTextureRect(
            index, this->g_tileSetWindow.getTextureRect(FGE_WINDOW_DRAW_TILESET_RIGHT_FRAME).value_or(fge::RectInt{}));
    transformable = this->g_spriteBatches.getTransformable(index++);
    transformable->setScale({1.0f, (this->g_size.y - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE});
    transformable->setPosition({this->g_size.x - FGE_WINDOW_PIXEL_SIZE, FGE_WINDOW_PIXEL_SIZE});

    //Others
    if (this->g_makeResizable)
    {
        this->g_spriteResize.setScale({1.0f, 1.0f});
        this->g_spriteResize.setTexture(this->g_textureWindowResize, true);
        this->g_spriteResize.setPosition(this->g_windowResizeRect.getPosition());
    }

    this->g_spriteClose.setScale({1.0f, 1.0f});
    this->g_spriteClose.setTexture(this->g_textureWindowClose, true);
    this->g_spriteClose.setPosition(this->g_windowCloseRect.getPosition());
    this->g_spriteClose.setColor(this->g_showCloseButton ? fge::Color::White : fge::Color(100, 100, 100, 200));

    this->g_spriteMinimize.setScale({1.0f, 1.0f});
    this->g_spriteMinimize.setTexture(this->g_textureWindowMinimize, true);
    this->g_spriteMinimize.setPosition(this->g_windowMinimizeRect.getPosition());
}

} // namespace fge
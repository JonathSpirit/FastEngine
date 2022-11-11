#include "FastEngine/object/C_objWindow.hpp"
#include "FastEngine/extra/extra_function.hpp"

#define FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT 30.0f
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

//ObjWindow

ObjWindow::ObjWindow() :
    fge::GuiElement(FGE_WINDOW_DEFAULT_PRIORITY)
{
}

void ObjWindow::first(fge::Scene* scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

    this->refreshRectBounds();

    this->setPriority(FGE_WINDOW_DEFAULT_PRIORITY);
    this->setScale( fge::GuiElement::getGlobalGuiScale()/*fge::ObjWindow::g_globalScale*/ );

    this->_windowScene._properties.setProperty("parent", this->_myObjectData.lock());
    this->_windowScene.setLinkedRenderTarget( scene->getLinkedRenderTarget() );
}
void ObjWindow::callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->detachAll();

    this->_windowHandler.setEvent(event);
    if (guiElementHandlerPtr != nullptr)
    {
        this->_windowHandler.setRenderTarget(guiElementHandlerPtr->getRenderTarget());
    }
    this->_windowScene.setCallbackContext({&event, &this->_windowHandler});

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
    auto parentStates = states;
    sf::View backupView = target.getView();
    this->_windowView = target.getDefaultView();
    target.setView(this->_windowView);

    sf::RectangleShape rectTest;
    rectTest.setPosition(parentStates.transform.transformPoint({0.0f, FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT}) + this->getPosition() );
    rectTest.setSize(this->getDrawAreaSize());
    rectTest.setFillColor(sf::Color::Transparent);
    rectTest.setOutlineThickness(1.0f);
    rectTest.setOutlineColor(sf::Color::Red);
    target.draw(rectTest);

    states.transform *= this->getTransform();

    this->g_sprite.setTexture(this->g_windowTextures.getTexture());

    //Fill
    this->g_sprite.setScale({(this->g_size.x - FGE_WINDOW_PIXEL_SIZE / 2.0f) / FGE_WINDOW_PIXEL_SIZE, (this->g_size.y - FGE_WINDOW_PIXEL_SIZE / 2.0f) / FGE_WINDOW_PIXEL_SIZE});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_FILL).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(FGE_WINDOW_PIXEL_SIZE / 2.0f, FGE_WINDOW_PIXEL_SIZE / 2.0f);
    target.draw(this->g_sprite, states);

    //Limit
    this->g_sprite.setScale({this->g_size.x / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_LIMIT).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(0.0f, FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT - FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({this->g_size.x / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_LIMIT).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(0.0f, FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT);
    target.draw(this->g_sprite, states);

    //Frame
    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_LEFT_CORNER_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(0, 0);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({(this->g_size.x - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(FGE_WINDOW_PIXEL_SIZE, 0);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_UP_RIGHT_CORNER_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(this->g_size.x - FGE_WINDOW_PIXEL_SIZE, 0);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({1.0f, (this->g_size.y - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_LEFT_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(0, FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_LEFT_CORNER_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(0, this->g_size.y - FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({(this->g_size.x - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE, 1.0f});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(FGE_WINDOW_PIXEL_SIZE, this->g_size.y - FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_DOWN_RIGHT_CORNER_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(this->g_size.x - FGE_WINDOW_PIXEL_SIZE, this->g_size.y - FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    this->g_sprite.setScale({1.0f, (this->g_size.y - FGE_WINDOW_PIXEL_SIZE * 2) / FGE_WINDOW_PIXEL_SIZE});
    this->g_sprite.setTextureRect( this->g_windowTextures.getTextureRect(FGE_WINDOW_DRAW_TILESET_RIGHT_FRAME).value_or(sf::IntRect{}) );
    this->g_sprite.setPosition(this->g_size.x - FGE_WINDOW_PIXEL_SIZE, FGE_WINDOW_PIXEL_SIZE);
    target.draw(this->g_sprite, states);

    //Others
    if (this->g_makeResizable)
    {
        this->g_sprite.setScale({1.0f, 1.0f});
        this->g_sprite.setTexture(this->g_windowResizeTexture, true);
        this->g_sprite.setPosition(this->g_windowResizeRect.getPosition());
        target.draw(this->g_sprite, states);
    }

    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTexture(this->g_windowCloseTexture, true);
    this->g_sprite.setPosition(this->g_windowCloseRect.getPosition());
    this->g_sprite.setColor( this->g_showCloseButton ? sf::Color::White : sf::Color(100,100,100,200) );
    target.draw(this->g_sprite, states);
    this->g_sprite.setColor(sf::Color::White);

    this->g_sprite.setScale({1.0f, 1.0f});
    this->g_sprite.setTexture(this->g_windowMinimizeTexture, true);
    this->g_sprite.setPosition(this->g_windowMinimizeRect.getPosition());
    target.draw(this->g_sprite, states);

    ///Drawing elements
    auto globalRect = states.transform.transformRect({{0.0f,0.0f}, this->g_size});

    sf::Vector2f screenPositionStart = this->_windowView.getTransform().transformPoint( this->getPosition() + parentStates.transform.transformPoint({0.0f, this->g_windowMoveRect.height}) );
    sf::Vector2f screenPositionEnd = this->_windowView.getTransform().transformPoint( globalRect.getPosition() + globalRect.getSize() );

    screenPositionStart = sf::Vector2f{(screenPositionStart.x+1.0f)/2.0f, 1.0f-(screenPositionStart.y+1.0f)/2.0f};
    screenPositionEnd = {(screenPositionEnd.x+1.0f)/2.0f, 1.0f-(screenPositionEnd.y+1.0f)/2.0f};

    this->_windowView.setCenter(sf::Vector2f{globalRect.width / 2.0f, (globalRect.height - this->g_windowMoveRect.height) / 2.0f} + this->g_viewCenterOffset);
    this->_windowView.setViewport(sf::FloatRect{screenPositionStart, screenPositionEnd - screenPositionStart});
    this->_windowView.setSize(globalRect.getSize() - sf::Vector2f{0.0f, this->g_windowMoveRect.height});
    target.setView(this->_windowView);

    this->_windowScene.draw(target, false);

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

    this->g_size.x = std::clamp(size.x, static_cast<float>(this->g_windowResizeTexture.getTextureSize().x*3), renderTarget.getDefaultView().getSize().x);
    this->g_size.y = std::clamp(size.y, static_cast<float>(this->g_windowResizeTexture.getTextureSize().y) + FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT, renderTarget.getDefaultView().getSize().y);

    this->refreshRectBounds();
    this->_windowHandler._onGuiResized.call(this->_windowHandler, this->g_size);
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

void ObjWindow::setResizeMode(ObjWindow::SizeMode modeX, ObjWindow::SizeMode modeY)
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

void ObjWindow::onGuiVerify(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context)
{
    if (context._recursive)
    {
        fge::GuiElementContext context2{};
        context2._mouseGuiPosition = context._mouseGuiPosition;
        context2._mousePosition = context._mousePosition;
        context2._handler = &this->_windowHandler;

        this->_windowHandler._onGuiVerify.call(evt, evtType, context2);

        if (context2._prioritizedElement != nullptr)
        {
            if (context2._prioritizedElement->isRecursive())
            {
                context2._recursive = true;
                context2._prioritizedElement = nullptr;
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
        newPosition.x = std::clamp(newPosition.x, 0.0f, renderTarget.getDefaultView().getSize().x-static_cast<float>(this->g_windowResizeTexture.getTextureSize().x));
        newPosition.y = std::clamp(newPosition.y, 0.0f, renderTarget.getDefaultView().getSize().y - FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT);

        this->setPosition(newPosition);
    }
    else if (this->g_resizeWindowFlag)
    {
        const sf::RenderTarget& renderTarget = this->g_guiElementHandler->getRenderTarget();

        sf::Vector2f mousePos = renderTarget.mapPixelToCoords({arg.x, arg.y}, renderTarget.getDefaultView());

        sf::Vector2f mouseDiff{this->g_resizeModeX == ObjWindow::SizeMode::MODE_FREE ? (mousePos.x - this->g_mouseClickLastPosition.x)/this->getScale().x : 0.0f,
                               this->g_resizeModeY == ObjWindow::SizeMode::MODE_FREE ? (mousePos.y - this->g_mouseClickLastPosition.y)/this->getScale().y : 0.0f};

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

void ObjWindow::onRefreshGlobalScale(const sf::Vector2f& scale)
{
    this->setScale(scale);
    this->_windowHandler._onGuiResized.call(this->_windowHandler, this->g_size);
}

void ObjWindow::refreshRectBounds()
{
    this->g_windowMoveRect.width = this->g_size.x;
    this->g_windowMoveRect.height = FGE_WINDOW_DRAW_MOVE_RECTANGLE_HEIGHT;

    this->g_windowResizeRect.left = this->g_size.x-static_cast<float>(this->g_windowResizeTexture.getTextureSize().x + FGE_WINDOW_DRAW_RESIZE_TEXTURE_OFFSET);
    this->g_windowResizeRect.top = this->g_size.y-static_cast<float>(this->g_windowResizeTexture.getTextureSize().y + FGE_WINDOW_DRAW_RESIZE_TEXTURE_OFFSET);
    this->g_windowResizeRect.width = static_cast<float>(this->g_windowResizeTexture.getTextureSize().x);
    this->g_windowResizeRect.height = static_cast<float>(this->g_windowResizeTexture.getTextureSize().y);

    this->g_windowCloseRect.left = this->g_size.x-static_cast<float>(this->g_windowCloseTexture.getTextureSize().x + FGE_WINDOW_DRAW_CLOSE_TEXTURE_OFFSET);
    this->g_windowCloseRect.top = FGE_WINDOW_DRAW_CLOSE_TEXTURE_OFFSET;
    this->g_windowCloseRect.width = static_cast<float>(this->g_windowCloseTexture.getTextureSize().x);
    this->g_windowCloseRect.height = static_cast<float>(this->g_windowCloseTexture.getTextureSize().y);

    this->g_windowMinimizeRect.left = this->g_size.x-static_cast<float>(this->g_windowMinimizeTexture.getTextureSize().x + this->g_windowCloseTexture.getTextureSize().x + FGE_WINDOW_DRAW_MINIMIZE_TEXTURE_OFFSET);
    this->g_windowMinimizeRect.top = FGE_WINDOW_DRAW_MINIMIZE_TEXTURE_OFFSET;
    this->g_windowMinimizeRect.width = static_cast<float>(this->g_windowMinimizeTexture.getTextureSize().x);
    this->g_windowMinimizeRect.height = static_cast<float>(this->g_windowMinimizeTexture.getTextureSize().y);
}

//ObjTextList

void ObjTextList::first(fge::Scene* scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;
    this->g_parentPtr = fge::ObjWindow::getWindowFromScene(scene);

    this->g_text.setFont("default");
    this->g_text.setCharacterSize(14);
    this->g_text.setFillColor(sf::Color::White);
    this->g_text.setOutlineColor(sf::Color::Black);
    this->g_text.setOutlineThickness(1.0f);

    this->g_scrollBaseRect.setFillColor(sf::Color{100,100,100,80});
    this->g_scrollRect.setFillColor(sf::Color{160,160,160,80});
    this->g_scrollRect.setOutlineColor(sf::Color{255,255,255,80});

    this->g_defaultElementGate.setData(&this->g_defaultElement);
    ///this->g_parentPtr->_elements.addGate(this->g_defaultElementGate);
    this->g_defaultElement._onGuiMouseButtonPressed.add( new fge::CallbackFunctorObject(&fge::ObjTextList::onGuiMouseButtonPressed, this), this );
}
void ObjTextList::callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    event._onMouseMoved.add(new fge::CallbackFunctorObject(&fge::ObjTextList::onMouseMoved, this), this);
    event._onMouseButtonReleased.add(new fge::CallbackFunctorObject(&fge::ObjTextList::onMouseButtonReleased, this), this);
    guiElementHandlerPtr->_onGuiResized.add( new fge::CallbackFunctorObject(&fge::ObjTextList::onGuiResized, this), this );
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjTextList)
{
    states.transform *= this->getTransform();

    float textOffset = -static_cast<float>(this->g_text.getCharacterSize());

    this->g_text.setPosition(4.0f, textOffset);
    for (std::size_t i=static_cast<std::size_t>(static_cast<float>(this->g_maxStrings-1)*this->getCursorRatio()); i<this->g_stringList.size(); ++i)
    {
        this->g_text.setString(this->g_stringList[i]);
        target.draw(this->g_text, states);

        this->g_text.move(0, textOffset);
    }

    target.draw(this->g_scrollBaseRect, states);
    target.draw(this->g_scrollRect, states);
}
#endif

void ObjTextList::onGuiResized([[maybe_unused]] const fge::GuiElementHandler& handler, const sf::Vector2f& size)
{
    this->refreshPosition(size);
}

void ObjTextList::addString(std::string string)
{
    this->g_stringList.insert(this->g_stringList.begin(), std::move(string));
    if (this->g_stringList.size() > this->g_maxStrings)
    {
        this->g_stringList.erase(this->g_stringList.end()-1);
    }
}
std::size_t ObjTextList::getStringsSize() const
{
    return this->g_stringList.size();
}
std::string& ObjTextList::getString(std::size_t index)
{
    return this->g_stringList[index];
}
const std::string& ObjTextList::getString(std::size_t index) const
{
    return this->g_stringList[index];
}
void ObjTextList::removeAllStrings()
{
    this->g_stringList.clear();
}

void ObjTextList::setBottomOffset(float offset)
{
    this->g_bottomOffset = offset;
}
float ObjTextList::getBottomOffset() const
{
    return this->g_bottomOffset;
}

void ObjTextList::setCursorRatio(float ratio)
{
    this->g_scrollPositionY = -ratio * (this->g_scrollBaseRect.getSize().y-this->g_scrollRect.getSize().y);
    ///this->refreshPosition(this->g_parentPtr);
}
float ObjTextList::getCursorRatio() const
{
    return std::abs( this->g_scrollPositionY / (this->g_scrollBaseRect.getSize().y-this->g_scrollRect.getSize().y) );
}
bool ObjTextList::isScrollPressed() const
{
    return this->g_scrollPressed;
}

void ObjTextList::setMaxStrings(std::size_t max)
{
    this->g_maxStrings = max;
}
std::size_t ObjTextList::getMaxStrings() const
{
    return this->g_maxStrings;
}

void ObjTextList::refreshPosition(const sf::Vector2f& size)
{
    this->g_scrollPositionY = std::clamp(this->g_scrollPositionY, -(this->g_scrollBaseRect.getSize().y - this->g_scrollRect.getSize().y), 0.0f);

    auto windowSize = size;

    this->setPosition( sf::Vector2f{0.0f, windowSize.y - this->g_bottomOffset} );

    this->g_scrollRect.setSize({10.0f, 30.0f});
    this->g_scrollBaseRect.setSize({10.0f, this->getPosition().y});
    this->g_scrollRect.setPosition({windowSize.x - 14.0f, this->g_scrollPositionY});
    this->g_scrollBaseRect.setPosition({windowSize.x - 14.0f, 0.0f});

    this->g_scrollBaseRect.setOrigin(0.0f, this->g_scrollBaseRect.getSize().y);
    this->g_scrollRect.setOrigin(0.0f, this->g_scrollRect.getSize().y);
}

void ObjTextList::onGuiMouseButtonPressed([[maybe_unused]] const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, [[maybe_unused]] fge::GuiElementContext& context)
{
    auto scrollRect = this->getTransform().transformRect(this->g_scrollRect.getGlobalBounds());

    auto mousePosition = this->_myObjectData.lock()->getLinkedScene()->getLinkedRenderTarget()->mapPixelToCoords({arg.x, arg.y}, this->g_parentPtr->_windowView);
    if ( scrollRect.contains(mousePosition) )
    {
        this->g_scrollPressed = true;
        this->g_scrollRelativePosY = (mousePosition.y - this->getPosition().y) - this->g_scrollPositionY;
        this->g_scrollRect.setOutlineThickness(1.0f);
    }
}
void ObjTextList::onMouseButtonReleased([[maybe_unused]] const fge::Event& evt, [[maybe_unused]] const sf::Event::MouseButtonEvent& arg)
{
    if ( this->g_scrollPressed )
    {
        this->g_scrollPressed = false;
        this->g_scrollRect.setOutlineThickness(0.0f);
    }
}
void ObjTextList::onMouseMoved([[maybe_unused]] const fge::Event& evt, const sf::Event::MouseMoveEvent& arg)
{
    if ( this->g_scrollPressed )
    {
        sf::RenderTarget* renderTarget = this->_myObjectData.lock()->getLinkedScene()->getLinkedRenderTarget();

        sf::Vector2f mousePos = renderTarget->mapPixelToCoords({arg.x, arg.y}, this->g_parentPtr->_windowView);

        this->g_scrollPositionY = (mousePos.y-this->g_scrollRelativePosY) - this->getPosition().y;

        ///this->refreshPosition(this->g_parentPtr);
    }
}

}//end fge
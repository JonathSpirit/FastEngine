#include "FastEngine/object/C_objSlider.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/C_scene.hpp"

namespace fge
{

void ObjSlider::first(fge::Scene* scene_ptr)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;
    ///this->setPriority(SC_GUI_PRIORITY_WINDOW);

    this->g_scrollBaseRect.setFillColor(sf::Color{100,100,100,80});
    this->g_scrollRect.setFillColor(sf::Color{180,180,180,80});
    this->g_scrollRect.setOutlineColor(sf::Color{255,255,255,80});

    ///this->refreshPosition(this->g_parentPtr);
}
void ObjSlider::callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->detachAll();

    this->g_guiElementHandler = guiElementHandlerPtr;

    guiElementHandlerPtr->_onGuiVerify.add( new fge::CallbackFunctorObject(&fge::ObjSlider::onGuiVerify, this), this );

    guiElementHandlerPtr->_onGuiResized.add( new fge::CallbackFunctorObject(&fge::ObjSlider::onGuiResized, this), this );
    this->_onGuiMouseButtonPressed.add( new fge::CallbackFunctorObject(&fge::ObjSlider::onGuiMouseButtonPressed, this), this );

    event._onMouseMoved.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onMouseMoved, this), this);
    event._onMouseButtonReleased.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onMouseButtonReleased, this), this);

    this->refreshPosition(guiElementHandlerPtr->_lastSize);
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjSlider)
{
    states.transform *= this->getTransform();

    target.draw(this->g_scrollBaseRect, states);
    target.draw(this->g_scrollRect, states);
}
#endif

void ObjSlider::setBottomOffset(float offset)
{
    this->g_bottomOffset = offset;
    this->refreshPosition(this->g_guiElementHandler->_lastSize);
}
float ObjSlider::getBottomOffset() const
{
    return this->g_bottomOffset;
}

void ObjSlider::setCursorRatio(float ratio)
{
    ratio = std::clamp(ratio, 0.0f, 1.0f);

    this->g_scrollPositionY = ratio * (this->g_scrollBaseRect.getSize().y-this->g_scrollRect.getSize().y);
    this->refreshPosition(this->g_guiElementHandler->_lastSize);
}
float ObjSlider::getCursorRatio() const
{
    return std::clamp( std::abs( this->g_scrollPositionY / (this->g_scrollBaseRect.getSize().y-this->g_scrollRect.getSize().y) ), 0.0f, 1.0f);
}
bool ObjSlider::isScrollPressed() const
{
    return this->g_scrollPressed;
}

void ObjSlider::setPositionMode(ObjSlider::PositionMode modeX, ObjSlider::PositionMode modeY)
{
    this->g_positionModeX = modeX;
    this->g_positionModeY = modeY;
    this->refreshPosition(this->g_guiElementHandler->_lastSize);
}

void ObjSlider::refreshPosition(const sf::Vector2f& targetSize)
{
    this->g_scrollPositionY = std::clamp(this->g_scrollPositionY, 0.0f, this->g_scrollBaseRect.getSize().y - this->g_scrollRect.getSize().y);

    auto windowDrawSize = targetSize;
    //this->setScale( window->getScale() );

    this->setPosition( sf::Vector2f{this->g_positionModeX == ObjSlider::PositionMode::MODE_AUTO ? (windowDrawSize.x-14.0f)*this->getScale().x : this->getPosition().x,
                                    this->g_positionModeY == ObjSlider::PositionMode::MODE_AUTO ? 0.0f : this->getPosition().y} );

    this->g_scrollRect.setSize({10.0f, 30.0f});
    this->g_scrollBaseRect.setSize({10.0f, windowDrawSize.y - this->g_bottomOffset});
    this->g_scrollRect.setPosition({0.0f, this->g_scrollPositionY});
    this->g_scrollBaseRect.setPosition({0.0f, 0.0f});

    this->g_scrollBaseRect.setOrigin(0.0f, 0.0f);
    this->g_scrollRect.setOrigin(0.0f, 0.0f);

    this->_onSlide.call( this->getCursorRatio() );
}

void ObjSlider::onGuiMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, fge::GuiElementContext& context)
{
    auto mousePosition = context._handler->getRenderTarget().mapPixelToCoords({context._mousePosition.x, context._mousePosition.y}/*, this->g_parentPtr->_windowView*/);

    this->g_scrollPressed = true;
    this->g_scrollRelativePosY = (mousePosition.y - this->getPosition().y)/this->getScale().y - this->g_scrollPositionY;
    this->g_scrollRect.setOutlineThickness(2.0f);
}
void ObjSlider::onMouseButtonReleased(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg)
{
    if ( this->g_scrollPressed )
    {
        this->g_scrollPressed = false;
        this->g_scrollRect.setOutlineThickness(0.0f);
    }
}
void ObjSlider::onMouseMoved(const fge::Event& evt, const sf::Event::MouseMoveEvent& arg)
{
    if ( this->g_scrollPressed )
    {
        const sf::RenderTarget& renderTarget = this->g_guiElementHandler->getRenderTarget();//this->_myObjectData.lock()->getLinkedScene()->getLinkedRenderTarget();

        sf::Vector2f mousePos = renderTarget.mapPixelToCoords({arg.x, arg.y}/*, this->g_parentPtr->_windowView*/);

        this->g_scrollPositionY = (mousePos.y-this->g_scrollRelativePosY)/this->getScale().y + this->getPosition().y;

        this->refreshPosition(this->g_guiElementHandler->_lastSize);
    }
}

void ObjSlider::onGuiResized([[maybe_unused]] const fge::GuiElementHandler& handler, const sf::Vector2f& size)
{
    this->refreshPosition(size);
}

void ObjSlider::onGuiVerify(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context)
{
    if (evtType != sf::Event::MouseButtonPressed)
    {
        return;
    }

    if ( this->verifyPriority(context._prioritizedElement) )
    {
        auto transform = this->getParentsTransform() * this->getTransform();

        auto scrollRect = transform.transformRect(this->g_scrollRect.getGlobalBounds());

        //auto mousePosition = context._mouseGuiPosition;//this->getParentsTransform().transformPoint(context._mouseGuiPosition);
        auto mousePosition = context._handler->getRenderTarget().mapPixelToCoords({context._mousePosition.x, context._mousePosition.y},
                                                                                  *this->_myObjectData.lock()->getLinkedScene()->getCustomView());
        if ( scrollRect.contains(mousePosition) )
        {
            context._prioritizedElement = this;
        }
    }
}

const char* ObjSlider::getClassName() const
{
    return FGE_OBJSLIDER_CLASSNAME;
}

const char* ObjSlider::getReadableClassName() const
{
    return "slider";
}

}//end fge
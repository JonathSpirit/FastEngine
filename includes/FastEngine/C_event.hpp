#ifndef _FGE_C_EVENT_HPP_INCLUDED
#define _FGE_C_EVENT_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_callback.hpp>
#include <FastEngine/C_packet.hpp>
#include <SFML/window.hpp>

namespace fge
{

class FGE_API Event
{
public:
    Event() = default;
    Event(const sf::Vector2u& windowSize){this->g_windowSize=windowSize;};
    ~Event() = default;

    void clear();

    void start();
    void process( const sf::Event& sfevt );
    void process( sf::Window& sfscreen );

    bool isKeyPressed( sf::Keyboard::Key sfkey ) const;
    uint32_t getKeyUnicode() const;

    const sf::Vector2u& getWindowSize() const;

    bool isEventType( sf::Event::EventType evtType ) const;

    const sf::Vector2i& getMousePixelPos() const;
    bool isMouseButtonPressed( sf::Mouse::Button sfmouse ) const;

    float getMouseWheelHorizontalDelta() const;
    float getMouseWheelVerticalDelta() const;

    fge::net::Packet& pack( fge::net::Packet& pck );
    fge::net::Packet& unpack( fge::net::Packet& pck );

    std::string getBinaryKeysString() const;
    std::string getBinaryTypesString() const;
    std::string getBinaryMouseButtonsString() const;

    ///Callback
    fge::CallbackHandler<const fge::Event&> _onClosed;
    fge::CallbackHandler<const fge::Event&, const sf::Event::SizeEvent&> _onResized;
    fge::CallbackHandler<const fge::Event&> _onLostFocus;
    fge::CallbackHandler<const fge::Event&> _onGainedFocus;
    fge::CallbackHandler<const fge::Event&, const sf::Event::TextEvent&> _onTextEntered;
    fge::CallbackHandler<const fge::Event&, const sf::Event::KeyEvent&> _onKeyPressed;
    fge::CallbackHandler<const fge::Event&, const sf::Event::KeyEvent&> _onKeyReleased;
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseWheelScrollEvent&> _onMouseWheelScrolled;
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseButtonEvent&> _onMouseButtonPressed;
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseButtonEvent&> _onMouseButtonReleased;
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseMoveEvent&> _onMouseMoved;
    fge::CallbackHandler<const fge::Event&> _onMouseEntered;
    fge::CallbackHandler<const fge::Event&> _onMouseLeft;
    fge::CallbackHandler<const fge::Event&, const sf::Event::JoystickButtonEvent&> _onJoystickButtonPressed;
    fge::CallbackHandler<const fge::Event&, const sf::Event::JoystickButtonEvent&> _onJoystickButtonReleased;
    fge::CallbackHandler<const fge::Event&, const sf::Event::JoystickMoveEvent&> _onJoystickMoved;
    fge::CallbackHandler<const fge::Event&, const sf::Event::JoystickConnectEvent&> _onJoystickConnected;
    fge::CallbackHandler<const fge::Event&, const sf::Event::JoystickConnectEvent&> _onJoystickDisconnected;
    fge::CallbackHandler<const fge::Event&, const sf::Event::TouchEvent&> _onTouchBegan;
    fge::CallbackHandler<const fge::Event&, const sf::Event::TouchEvent&> _onTouchMoved;
    fge::CallbackHandler<const fge::Event&, const sf::Event::TouchEvent&> _onTouchEnded;
    fge::CallbackHandler<const fge::Event&, const sf::Event::SensorEvent&> _onSensorChanged;

private:
    ///Event type
    uint32_t g_types = 0;

    ///Keyboard
    uint32_t g_keys[4] = {0, 0, 0, 0};
    uint32_t g_keyUnicode = 0;

    ///Mouse
    sf::Vector2i g_mousePixelPos;
    uint8_t g_mouseButtons = 0;

    float g_mouseWheelHorizontalDelta = 0;
    float g_mouseWheelVerticalDelta = 0;

    ///Window size
    sf::Vector2u g_windowSize;
};

}//end fge

#endif // _FGE_C_EVENT_HPP_INCLUDED

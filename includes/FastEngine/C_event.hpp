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

#ifndef _FGE_C_EVENT_HPP_INCLUDED
#define _FGE_C_EVENT_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_callback.hpp>
#include <SFML/Window.hpp>

#define FGE_EVENT_DEFAULT_MAXEVENTCOUNT 20

namespace fge
{

namespace net
{

class Packet;

}//end net

/**
 * \class Event
 * \ingroup objectControl
 * \brief This class is a wrapper for SFML events
 *
 * This class regroups all the SFML events in a single class.
 * It also provides a way to send events to the server.
 * All events can be monitored with callbacks.
 *
 * \see CallbackHandler
 */
class FGE_API Event
{
public:
    Event() = default;
    /**
     * \brief Constructor to apply to size of the window
     *
     * \param windowSize The size of the window
     */
    explicit Event(const sf::Vector2u& windowSize) :
            g_windowSize(windowSize)
    {}
    /**
     * \brief Constructor to apply window data
     *
     * \param window The window
     */
#ifndef FGE_DEF_SERVER
    explicit Event(const sf::Window& window) :
            g_windowSize(window.getSize())
    {}
#endif //FGE_DEF_SERVER
    ~Event() = default;

    /**
     * \brief Clear all the events
     */
    void clear();

    /**
     * \brief Start the event loop
     *
     * This function should be called every time you want to process events.
     * It clear some dynamic events like :
     * - Mouse wheel delta
     * - Unicode key text
     * - Event types
     *
     * \see process
     */
    void start();
#ifndef FGE_DEF_SERVER
    /**
     * \brief Process an SFML event
     *
     * Before any attempt to call this function, you should call start.
     *
     * \see start
     *
     * \param sfevt The SFML event
     */
    void process( const sf::Event& sfevt );
    /**
     * \brief Process automatically all the SFML events
     *
     * This function loop through all available events and call process. The
     * start function is called by this function.
     *
     * \param sfscreen The SFML window
     * \param maxEventCount The maximum number of processed events par call
     */
    void process( sf::Window& sfscreen, unsigned int maxEventCount=FGE_EVENT_DEFAULT_MAXEVENTCOUNT );
#endif //FGE_DEF_SERVER

    /**
     * \brief Check if a key is pressed
     *
     * \param sfkey The SFML key
     * \return \b true if the key is pressed, \b false otherwise
     */
    bool isKeyPressed( sf::Keyboard::Key sfkey ) const;
    /**
     * \brief Get the unicode of the last key pressed
     *
     * \return The unicode character
     */
    uint32_t getKeyUnicode() const;

    /**
     * \brief Get the window size
     *
     * \return The window size
     */
    const sf::Vector2u& getWindowSize() const;

    /**
     * \brief Check if the specified SFML event is active
     *
     * \param evtType The SFML event type
     * \return \b true if the event is active, \b false otherwise
     */
    bool isEventType( sf::Event::EventType evtType ) const;

    /**
     * \brief Get the mouse pixel position
     *
     * \return The mouse pixel position
     */
    const sf::Vector2i& getMousePixelPos() const;
    /**
     * \brief Check if the specified mouse button is pressed
     *
     * \param sfmouse The SFML mouse button
     * \return \b true if the button is pressed, \b false otherwise
     */
    bool isMouseButtonPressed( sf::Mouse::Button sfmouse ) const;

    /**
     * \brief Get the horizontal mouse wheel delta
     *
     * \return The horizontal mouse wheel delta
     */
    float getMouseWheelHorizontalDelta() const;
    /**
     * \brief Get the vertical mouse wheel delta
     *
     * \return The vertical mouse wheel delta
     */
    float getMouseWheelVerticalDelta() const;

    /**
     * \brief Pack events data into a network packet
     *
     * \param pck The packet
     * \return The packet
     */
    fge::net::Packet& pack( fge::net::Packet& pck );
    /**
     * \brief Unpack events data from a network packet
     *
     * \param pck The packet
     * \return The packet
     */
    fge::net::Packet& unpack( fge::net::Packet& pck );

    /**
     * \brief Get a binary representation of all keys into a string
     *
     * \return The string
     */
    std::string getBinaryKeysString() const;
    /**
     * \brief Get a binary representation of all SFML event types into a string
     *
     * \return The string
     */
    std::string getBinaryTypesString() const;
    /**
     * \brief Get a binary representation of all mouse buttons into a string
     *
     * \return The string
     */
    std::string getBinaryMouseButtonsString() const;

    //Callbacks
    fge::CallbackHandler<const fge::Event&> _onClosed; ///< Callback called when the window is closed
    fge::CallbackHandler<const fge::Event&, const sf::Event::SizeEvent&> _onResized; ///< Callback called when the window is resized
    fge::CallbackHandler<const fge::Event&> _onLostFocus; ///< Callback called when the window lost focus
    fge::CallbackHandler<const fge::Event&> _onGainedFocus; ///< Callback called when the window gained focus
    fge::CallbackHandler<const fge::Event&, const sf::Event::TextEvent&> _onTextEntered; ///< Callback called when a text is entered
    fge::CallbackHandler<const fge::Event&, const sf::Event::KeyEvent&> _onKeyPressed; ///< Callback called when a key is pressed
    fge::CallbackHandler<const fge::Event&, const sf::Event::KeyEvent&> _onKeyReleased; ///< Callback called when a key is released
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseWheelScrollEvent&> _onMouseWheelScrolled; ///< Callback called when the mouse wheel is scrolled
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseButtonEvent&> _onMouseButtonPressed; ///< Callback called when a mouse button is pressed
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseButtonEvent&> _onMouseButtonReleased; ///< Callback called when a mouse button is released
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseMoveEvent&> _onMouseMoved; ///< Callback called when the mouse is moved
    fge::CallbackHandler<const fge::Event&> _onMouseEntered; ///< Callback called when the mouse enters the window
    fge::CallbackHandler<const fge::Event&> _onMouseLeft; ///< Callback called when the mouse leaves the window
    fge::CallbackHandler<const fge::Event&, const sf::Event::JoystickButtonEvent&> _onJoystickButtonPressed; ///< Callback called when a joystick button is pressed
    fge::CallbackHandler<const fge::Event&, const sf::Event::JoystickButtonEvent&> _onJoystickButtonReleased; ///< Callback called when a joystick button is released
    fge::CallbackHandler<const fge::Event&, const sf::Event::JoystickMoveEvent&> _onJoystickMoved; ///< Callback called when a joystick is moved
    fge::CallbackHandler<const fge::Event&, const sf::Event::JoystickConnectEvent&> _onJoystickConnected; ///< Callback called when a joystick is connected
    fge::CallbackHandler<const fge::Event&, const sf::Event::JoystickConnectEvent&> _onJoystickDisconnected; ///< Callback called when a joystick is disconnected
    fge::CallbackHandler<const fge::Event&, const sf::Event::TouchEvent&> _onTouchBegan; ///< Callback called when mouse touch begins
    fge::CallbackHandler<const fge::Event&, const sf::Event::TouchEvent&> _onTouchMoved; ///< Callback called when the mouse touch is moved
    fge::CallbackHandler<const fge::Event&, const sf::Event::TouchEvent&> _onTouchEnded; ///< Callback called when the mouse touch is released
    fge::CallbackHandler<const fge::Event&, const sf::Event::SensorEvent&> _onSensorChanged; ///< Callback called when a sensor is changed

private:
    //Event type
    uint32_t g_types = 0;

    //Keyboard
    uint32_t g_keys[4] = {0, 0, 0, 0};
    uint32_t g_keyUnicode = 0;

    //Mouse
    sf::Vector2i g_mousePixelPos;
    uint8_t g_mouseButtons = 0;

    float g_mouseWheelHorizontalDelta = 0;
    float g_mouseWheelVerticalDelta = 0;

    //Window size
    sf::Vector2u g_windowSize;
};

}//end fge

#endif // _FGE_C_EVENT_HPP_INCLUDED

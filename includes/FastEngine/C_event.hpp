/*
 * Copyright 2025 Guillaume Guillet
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

#include "FastEngine/fge_extern.hpp"
#include "C_vector.hpp"
#include "FastEngine/C_callback.hpp"
#include "SDL_events.h"

#define FGE_EVENT_KEYCODES_SIZE 12
#define FGE_EVENT_DEFAULT_MAXEVENTCOUNT 20

namespace fge
{

class RenderWindow;

namespace vulkan
{

class SurfaceWindow;

} // namespace vulkan

namespace net
{

class Packet;

} // namespace net

/**
 * \class Event
 * \ingroup objectControl
 * \brief This class is a wrapper for SDL events
 *
 * This class regroups all the SDL events in a single class.
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
     * \brief Constructor to apply to size of the window and the position
     *
     * \param windowSize The size of the window
     * \param windowPosition The position of the window
     */
    explicit Event(fge::Vector2i const& windowSize, fge::Vector2i const& windowPosition) :
            g_windowSize(windowSize),
            g_windowPosition(windowPosition)
    {}
    /**
     * \brief Constructor to apply window data
     *
     * \param surfaceWindow The surface window
     */
#ifndef FGE_DEF_SERVER
    explicit Event(fge::vulkan::SurfaceWindow const& surfaceWindow);
    explicit Event(fge::RenderWindow const& renderWindow);
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
     * \brief Process an SDL event
     *
     * Before any attempt to call this function, you should call start.
     *
     * \see start
     *
     * \param evt The SDL event
     */
    void process(SDL_Event const& evt);
    /**
     * \brief Process automatically SDL events
     *
     * This function loop through all available events and call process. The
     * start function is called by this function.
     *
     * \param maxEventCount The maximum number of processed events par call
     */
    void process(unsigned int maxEventCount = FGE_EVENT_DEFAULT_MAXEVENTCOUNT);
#endif //FGE_DEF_SERVER

    void pushType(SDL_EventType type); ///TODO: add comments
    void popType(SDL_EventType type);

    /**
     * \brief Check if a key is pressed
     *
     * \param keycode The SDL key code
     * \return \b true if the key is pressed, \b false otherwise
     */
    bool isKeyPressed(uint32_t keycode) const;
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
    fge::Vector2i const& getWindowSize() const;
    /**
     * \brief Get the window position
     *
     * \return The window position
     */
    fge::Vector2i const& getWindowPos() const;

    /**
     * \brief Check if the specified SDL event is active
     *
     * \param type The SDL event type
     * \return \b true if the event is active, \b false otherwise
     */
    bool isEventType(uint32_t type) const;

    /**
     * \brief Get the mouse pixel position
     *
     * \return The mouse pixel position
     */
    fge::Vector2i const& getMousePixelPos() const;
    /**
     * \brief Check if the specified mouse button is pressed
     *
     * \param mouseButton The SDL mouse button
     * \return \b true if the button is pressed, \b false otherwise
     */
    bool isMouseButtonPressed(uint8_t mouseButton) const;

    /**
     * \brief Get the horizontal mouse wheel delta
     *
     * \return The horizontal mouse wheel delta
     */
    int getMouseWheelHorizontalDelta() const;
    /**
     * \brief Get the vertical mouse wheel delta
     *
     * \return The vertical mouse wheel delta
     */
    int getMouseWheelVerticalDelta() const;

    /**
     * \brief Pack events data into a network packet
     *
     * \param pck The packet
     * \return The packet
     */
    fge::net::Packet& pack(fge::net::Packet& pck);
    /**
     * \brief Unpack events data from a network packet
     *
     * \param pck The packet
     * \return The packet
     */
    fge::net::Packet& unpack(fge::net::Packet& pck);

    /**
     * \brief Get a binary representation of all keys into a string
     *
     * \return The string
     */
    std::string getBinaryKeysString() const;
    /**
     * \brief Get a binary representation of all SDL event types into a string
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
    mutable fge::CallbackHandler<fge::Event const&, SDL_QuitEvent const&> _onQuit;

    mutable fge::CallbackHandler<fge::Event const&, SDL_CommonEvent const&> _onAppTerminating;
    mutable fge::CallbackHandler<fge::Event const&, SDL_CommonEvent const&> _onAppLowMemory;
    mutable fge::CallbackHandler<fge::Event const&, SDL_CommonEvent const&> _onAppWillEnterBackground;
    mutable fge::CallbackHandler<fge::Event const&, SDL_CommonEvent const&> _onAppDidEnterBackground;
    mutable fge::CallbackHandler<fge::Event const&, SDL_CommonEvent const&> _onAppWillEnterForeground;
    mutable fge::CallbackHandler<fge::Event const&, SDL_CommonEvent const&> _onAppDidEnterForeground;

    mutable fge::CallbackHandler<fge::Event const&, SDL_WindowEvent const&> _onWindowEvent;
    mutable fge::CallbackHandler<fge::Event const&, SDL_SysWMEvent const&> _onSyswmEvent;

    mutable fge::CallbackHandler<fge::Event const&, SDL_KeyboardEvent const&> _onKeyDown;
    mutable fge::CallbackHandler<fge::Event const&, SDL_KeyboardEvent const&> _onKeyUp;
    mutable fge::CallbackHandler<fge::Event const&, SDL_TextEditingEvent const&> _onTextEditing;
    mutable fge::CallbackHandler<fge::Event const&, SDL_TextInputEvent const&> _onTextInput;
    mutable fge::CallbackHandler<fge::Event const&, SDL_CommonEvent const&> _onKeymapChanged;

    mutable fge::CallbackHandler<fge::Event const&, SDL_MouseMotionEvent const&> _onMouseMotion;
    mutable fge::CallbackHandler<fge::Event const&, SDL_MouseButtonEvent const&> _onMouseButtonDown;
    mutable fge::CallbackHandler<fge::Event const&, SDL_MouseButtonEvent const&> _onMouseButtonUp;
    mutable fge::CallbackHandler<fge::Event const&, SDL_MouseWheelEvent const&> _onMouseWheel;

    mutable fge::CallbackHandler<fge::Event const&, SDL_JoyAxisEvent const&> _onJoyAxisMotion;
    mutable fge::CallbackHandler<fge::Event const&, SDL_JoyBallEvent const&> _onJoyBallMotion;
    mutable fge::CallbackHandler<fge::Event const&, SDL_JoyHatEvent const&> _onJoyHatMotion;
    mutable fge::CallbackHandler<fge::Event const&, SDL_JoyButtonEvent const&> _onJoyButtonDown;
    mutable fge::CallbackHandler<fge::Event const&, SDL_JoyButtonEvent const&> _onJoyButtonUp;
    mutable fge::CallbackHandler<fge::Event const&, SDL_JoyDeviceEvent const&> _onJoyDeviceAdded;
    mutable fge::CallbackHandler<fge::Event const&, SDL_JoyDeviceEvent const&> _onJoyDeviceRemoved;

    mutable fge::CallbackHandler<fge::Event const&, SDL_ControllerAxisEvent const&> _onControllerAxisMotion;
    mutable fge::CallbackHandler<fge::Event const&, SDL_ControllerButtonEvent const&> _onControllerButtonDown;
    mutable fge::CallbackHandler<fge::Event const&, SDL_ControllerButtonEvent const&> _onControllerButtonUp;
    mutable fge::CallbackHandler<fge::Event const&, SDL_ControllerDeviceEvent const&> _onControllerDeviceAdded;
    mutable fge::CallbackHandler<fge::Event const&, SDL_ControllerDeviceEvent const&> _onControllerDeviceRemoved;
    mutable fge::CallbackHandler<fge::Event const&, SDL_ControllerDeviceEvent const&> _onControllerDeviceRemapped;

    mutable fge::CallbackHandler<fge::Event const&, SDL_TouchFingerEvent const&> _onFingerDown;
    mutable fge::CallbackHandler<fge::Event const&, SDL_TouchFingerEvent const&> _onFingerUp;
    mutable fge::CallbackHandler<fge::Event const&, SDL_TouchFingerEvent const&> _onFingerMotion;

    mutable fge::CallbackHandler<fge::Event const&, SDL_DollarGestureEvent const&> _onDollarGesture;
    mutable fge::CallbackHandler<fge::Event const&, SDL_DollarGestureEvent const&> _onDollarRecord;
    mutable fge::CallbackHandler<fge::Event const&, SDL_MultiGestureEvent const&> _onMultiGesture;

    mutable fge::CallbackHandler<fge::Event const&, SDL_CommonEvent const&> _onClipboardUpdate;

    mutable fge::CallbackHandler<fge::Event const&, SDL_DropEvent const&> _onDropFile;
    mutable fge::CallbackHandler<fge::Event const&, SDL_DropEvent const&> _onDropText;
    mutable fge::CallbackHandler<fge::Event const&, SDL_DropEvent const&> _onDropBegin;
    mutable fge::CallbackHandler<fge::Event const&, SDL_DropEvent const&> _onDropComplete;

    mutable fge::CallbackHandler<fge::Event const&, SDL_AudioDeviceEvent const&> _onAudioDeviceAdded;
    mutable fge::CallbackHandler<fge::Event const&, SDL_AudioDeviceEvent const&> _onAudioDeviceRemoved;

    mutable fge::CallbackHandler<fge::Event const&, SDL_CommonEvent const&> _onRenderTargetReset;
    mutable fge::CallbackHandler<fge::Event const&, SDL_CommonEvent const&> _onRenderDeviceReset;

private:
    static uint64_t EventTypeToBitMask(uint32_t type);
    static std::size_t KeycodeToBitIndex(uint32_t keyCode);
    static uint32_t UTF8ToUTF32(char const* utf8);

    //Event type
    uint64_t g_types = 0;

    //Keyboard
    uint32_t g_keyCodes[FGE_EVENT_KEYCODES_SIZE] = {0};
    uint32_t g_keyUnicode = 0;

    //Mouse
    fge::Vector2i g_mouseRelativeMotion = {0, 0};
    fge::Vector2i g_mousePixelPosition = {0, 0};
    uint8_t g_mouseButtons = 0;

    int g_mouseWheelHorizontalDelta = 0;
    int g_mouseWheelVerticalDelta = 0;

    //Window size
    fge::Vector2i g_windowSize;
    fge::Vector2i g_windowPosition;
};

} // namespace fge

#endif // _FGE_C_EVENT_HPP_INCLUDED

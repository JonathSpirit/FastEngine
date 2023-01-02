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
#include <FastEngine/graphic/C_vector.hpp>
#include <SDL_video.h>
#include <SDL_events.h>

#define FGE_EVENT_KEYCODES_SIZE 12
#define FGE_EVENT_DEFAULT_MAXEVENTCOUNT 20

namespace fge
{

namespace net
{

class Packet;

} // namespace net

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
     * \brief Constructor to apply to size of the window and the position
     *
     * \param windowSize The size of the window
     * \param windowPosition The position of the window
     */
    explicit Event(const fge::Vector2i& windowSize, const fge::Vector2i& windowPosition) :
            g_windowSize(windowSize),
            g_windowPosition(windowPosition)
    {}
    /**
     * \brief Constructor to apply window data
     *
     * \param window The window
     */
#ifndef FGE_DEF_SERVER
    explicit Event(SDL_Window* window);
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
    void process(const SDL_Event& evt);
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
    const fge::Vector2i& getWindowSize() const;
    /**
     * \brief Get the window position
     *
     * \return The window position
     */
    const fge::Vector2i& getWindowPos() const;

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
    const fge::Vector2i& getMousePixelPos() const;
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
    fge::CallbackHandler<const fge::Event&, const SDL_QuitEvent&> _onQuit;

    fge::CallbackHandler<const fge::Event&, const SDL_CommonEvent&> _onAppTerminating;
    fge::CallbackHandler<const fge::Event&, const SDL_CommonEvent&> _onAppLowMemory;
    fge::CallbackHandler<const fge::Event&, const SDL_CommonEvent&> _onAppWillEnterBackground;
    fge::CallbackHandler<const fge::Event&, const SDL_CommonEvent&> _onAppDidEnterBackground;
    fge::CallbackHandler<const fge::Event&, const SDL_CommonEvent&> _onAppWillEnterForeground;
    fge::CallbackHandler<const fge::Event&, const SDL_CommonEvent&> _onAppDidEnterForeground;

    fge::CallbackHandler<const fge::Event&, const SDL_WindowEvent&> _onWindowEvent;
    fge::CallbackHandler<const fge::Event&, const SDL_SysWMEvent&> _onSyswmEvent;

    fge::CallbackHandler<const fge::Event&, const SDL_KeyboardEvent&> _onKeyDown;
    fge::CallbackHandler<const fge::Event&, const SDL_KeyboardEvent&> _onKeyUp;
    fge::CallbackHandler<const fge::Event&, const SDL_TextEditingEvent&> _onTextEditing;
    fge::CallbackHandler<const fge::Event&, const SDL_TextInputEvent&> _onTextInput;
    fge::CallbackHandler<const fge::Event&, const SDL_CommonEvent&> _onKeymapChanged;

    fge::CallbackHandler<const fge::Event&, const SDL_MouseMotionEvent&> _onMouseMotion;
    fge::CallbackHandler<const fge::Event&, const SDL_MouseButtonEvent&> _onMouseButtonDown;
    fge::CallbackHandler<const fge::Event&, const SDL_MouseButtonEvent&> _onMouseButtonUp;
    fge::CallbackHandler<const fge::Event&, const SDL_MouseWheelEvent&> _onMouseWheel;

    fge::CallbackHandler<const fge::Event&, const SDL_JoyAxisEvent&> _onJoyAxisMotion;
    fge::CallbackHandler<const fge::Event&, const SDL_JoyBallEvent&> _onJoyBallMotion;
    fge::CallbackHandler<const fge::Event&, const SDL_JoyHatEvent&> _onJoyHatMotion;
    fge::CallbackHandler<const fge::Event&, const SDL_JoyButtonEvent&> _onJoyButtonDown;
    fge::CallbackHandler<const fge::Event&, const SDL_JoyButtonEvent&> _onJoyButtonUp;
    fge::CallbackHandler<const fge::Event&, const SDL_JoyDeviceEvent&> _onJoyDeviceAdded;
    fge::CallbackHandler<const fge::Event&, const SDL_JoyDeviceEvent&> _onJoyDeviceRemoved;

    fge::CallbackHandler<const fge::Event&, const SDL_ControllerAxisEvent&> _onControllerAxisMotion;
    fge::CallbackHandler<const fge::Event&, const SDL_ControllerButtonEvent&> _onControllerButtonDown;
    fge::CallbackHandler<const fge::Event&, const SDL_ControllerButtonEvent&> _onControllerButtonUp;
    fge::CallbackHandler<const fge::Event&, const SDL_ControllerDeviceEvent&> _onControllerDeviceAdded;
    fge::CallbackHandler<const fge::Event&, const SDL_ControllerDeviceEvent&> _onControllerDeviceRemoved;
    fge::CallbackHandler<const fge::Event&, const SDL_ControllerDeviceEvent&> _onControllerDeviceRemapped;

    fge::CallbackHandler<const fge::Event&, const SDL_TouchFingerEvent&> _onFingerDown;
    fge::CallbackHandler<const fge::Event&, const SDL_TouchFingerEvent&> _onFingerUp;
    fge::CallbackHandler<const fge::Event&, const SDL_TouchFingerEvent&> _onFingerMotion;

    fge::CallbackHandler<const fge::Event&, const SDL_DollarGestureEvent&> _onDollarGesture;
    fge::CallbackHandler<const fge::Event&, const SDL_DollarGestureEvent&> _onDollarRecord;
    fge::CallbackHandler<const fge::Event&, const SDL_MultiGestureEvent&> _onMultiGesture;

    fge::CallbackHandler<const fge::Event&, const SDL_CommonEvent&> _onClipboardUpdate;

    fge::CallbackHandler<const fge::Event&, const SDL_DropEvent&> _onDropFile;
    fge::CallbackHandler<const fge::Event&, const SDL_DropEvent&> _onDropText;
    fge::CallbackHandler<const fge::Event&, const SDL_DropEvent&> _onDropBegin;
    fge::CallbackHandler<const fge::Event&, const SDL_DropEvent&> _onDropComplete;

    fge::CallbackHandler<const fge::Event&, const SDL_AudioDeviceEvent&> _onAudioDeviceAdded;
    fge::CallbackHandler<const fge::Event&, const SDL_AudioDeviceEvent&> _onAudioDeviceRemoved;

    fge::CallbackHandler<const fge::Event&, const SDL_CommonEvent&> _onRenderTargetReset;
    fge::CallbackHandler<const fge::Event&, const SDL_CommonEvent&> _onRenderDeviceReset;

private:
    static uint64_t EventTypeToBitMask(uint32_t type);
    static std::size_t KeycodeToBitIndex(uint32_t keyCode);
    static uint32_t UTF8ToUTF32(const char* utf8);

    //Event type
    uint64_t g_types = 0;

    //Keyboard
    uint32_t g_keyCodes[FGE_EVENT_KEYCODES_SIZE] = {0};
    uint32_t g_keyUnicode = 0;

    //Mouse
    fge::Vector2i g_mouseRelativeMotion = {0,0};
    fge::Vector2i g_mousePixelPosition = {0,0};
    uint8_t g_mouseButtons = 0;

    int g_mouseWheelHorizontalDelta = 0;
    int g_mouseWheelVerticalDelta = 0;

    //Window size
    fge::Vector2i g_windowSize;
    fge::Vector2i g_windowPosition;
};

} // namespace fge

#endif // _FGE_C_EVENT_HPP_INCLUDED

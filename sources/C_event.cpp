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

#include "FastEngine/C_event.hpp"
#include "FastEngine/graphic/C_renderWindow.hpp"
#include "FastEngine/network/C_packet.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/C_surface.hpp"
#include "tinyutf8.h"

namespace fge
{

#ifndef FGE_DEF_SERVER
Event::Event(SDL_Window* window) //TODO: Use SurfaceWindow instead of SDL_Window
{
    SDL_GetWindowSize(window, &this->g_windowSize.x, &this->g_windowSize.y);
    SDL_GetWindowPosition(window, &this->g_windowPosition.x, &this->g_windowPosition.y);
}
Event::Event(fge::RenderWindow const& renderWindow) :
        Event(reinterpret_cast<fge::vulkan::SurfaceSDLWindow*>(&renderWindow.getSurface())->getWindow())
{}
#endif //FGE_DEF_SERVER

void Event::clear()
{
    //Event type
    this->g_types = 0;

    //Keyboard
    for (auto& keyCode: this->g_keyCodes)
    {
        keyCode = 0;
    }
    this->g_keyUnicode = 0;

    //Mouse
    this->g_mouseRelativeMotion.x = 0;
    this->g_mouseRelativeMotion.y = 0;
    this->g_mousePixelPosition.x = 0;
    this->g_mousePixelPosition.y = 0;
    this->g_mouseButtons = 0;
    this->g_mouseWheelHorizontalDelta = 0;
    this->g_mouseWheelVerticalDelta = 0;

    //Window size
    this->g_windowSize.x = 0;
    this->g_windowSize.y = 0;
    this->g_windowPosition.x = 0;
    this->g_windowPosition.y = 0;
}

void Event::start()
{
    this->g_types = 0;

    this->g_keyUnicode = 0;

    this->g_mouseRelativeMotion.x = 0;
    this->g_mouseRelativeMotion.y = 0;

    this->g_mouseWheelHorizontalDelta = 0;
    this->g_mouseWheelVerticalDelta = 0;
}
#ifndef FGE_DEF_SERVER
void Event::process(SDL_Event const& evt)
{
    this->g_types |= fge::Event::EventTypeToBitMask(evt.type);

    switch (evt.type)
    {
    case SDL_QUIT:
        this->_onQuit.call(*this, evt.quit);
        break;

    case SDL_APP_TERMINATING:
        this->_onAppTerminating.call(*this, evt.common);
        break;
    case SDL_APP_LOWMEMORY:
        this->_onAppLowMemory.call(*this, evt.common);
        break;
    case SDL_APP_WILLENTERBACKGROUND:
        this->_onAppWillEnterBackground.call(*this, evt.common);
        break;
    case SDL_APP_DIDENTERBACKGROUND:
        this->_onAppDidEnterBackground.call(*this, evt.common);
        break;
    case SDL_APP_WILLENTERFOREGROUND:
        this->_onAppWillEnterForeground.call(*this, evt.common);
        break;
    case SDL_APP_DIDENTERFOREGROUND:
        this->_onAppDidEnterForeground.call(*this, evt.common);
        break;

    case SDL_WINDOWEVENT:
        switch (evt.window.event)
        {
        case SDL_WINDOWEVENT_MOVED:
            this->g_windowPosition.x = evt.window.data1;
            this->g_windowPosition.y = evt.window.data2;
            break;
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            this->g_windowSize.x = evt.window.data1;
            this->g_windowSize.y = evt.window.data2;
            break;
        }
        this->_onWindowEvent.call(*this, evt.window);
        break;
    case SDL_SYSWMEVENT:
        this->_onSyswmEvent.call(*this, evt.syswm);
        break;

    case SDL_KEYDOWN:
    {
        std::size_t const index = fge::Event::KeycodeToBitIndex(evt.key.keysym.sym);
        this->g_keyCodes[index / 32] |= (0x80000000 >> (index % 32));
    }
        this->_onKeyDown.call(*this, evt.key);
        break;
    case SDL_KEYUP:
    {
        std::size_t const index = fge::Event::KeycodeToBitIndex(evt.key.keysym.sym);
        this->g_keyCodes[index / 32] &= ~(0x80000000 >> (index % 32));
    }
        this->_onKeyUp.call(*this, evt.key);
        break;
    case SDL_TEXTEDITING:
        this->_onTextEditing.call(*this, evt.edit);
        break;
    case SDL_TEXTINPUT:
    {
        this->g_keyUnicode = fge::Event::UTF8ToUTF32(evt.text.text);
        this->_onTextInput.call(*this, evt.text);
    }
    break;
    case SDL_KEYMAPCHANGED:
        this->_onKeymapChanged.call(*this, evt.common);
        break;

    case SDL_MOUSEMOTION:
        this->g_mouseRelativeMotion.x = evt.motion.xrel;
        this->g_mouseRelativeMotion.y = evt.motion.yrel;
        this->g_mousePixelPosition.x = evt.motion.x;
        this->g_mousePixelPosition.y = evt.motion.y;
        this->_onMouseMotion.call(*this, evt.motion);
        break;
    case SDL_MOUSEBUTTONDOWN:
        this->g_mouseButtons |= SDL_BUTTON(evt.button.button);
        this->g_mousePixelPosition.x = evt.button.x;
        this->g_mousePixelPosition.y = evt.button.y;
        this->_onMouseButtonDown.call(*this, evt.button);
        break;
    case SDL_MOUSEBUTTONUP:
        this->g_mouseButtons &= ~SDL_BUTTON(evt.button.button);
        this->g_mousePixelPosition.x = evt.button.x;
        this->g_mousePixelPosition.y = evt.button.y;
        this->_onMouseButtonUp.call(*this, evt.button);
        break;
    case SDL_MOUSEWHEEL:
        if (evt.wheel.direction == SDL_MOUSEWHEEL_NORMAL)
        {
            this->g_mouseWheelHorizontalDelta = evt.wheel.x;
            this->g_mouseWheelVerticalDelta = evt.wheel.y;
        }
        else
        {
            this->g_mouseWheelHorizontalDelta = evt.wheel.x * -1;
            this->g_mouseWheelVerticalDelta = evt.wheel.y * -1;
        }
        this->_onMouseWheel.call(*this, evt.wheel);
        break;

    case SDL_JOYAXISMOTION:
        this->_onJoyAxisMotion.call(*this, evt.jaxis);
        break;
    case SDL_JOYBALLMOTION:
        this->_onJoyBallMotion.call(*this, evt.jball);
        break;
    case SDL_JOYHATMOTION:
        this->_onJoyHatMotion.call(*this, evt.jhat);
        break;
    case SDL_JOYBUTTONDOWN:
        this->_onJoyButtonDown.call(*this, evt.jbutton);
        break;
    case SDL_JOYBUTTONUP:
        this->_onJoyButtonUp.call(*this, evt.jbutton);
        break;
    case SDL_JOYDEVICEADDED:
        this->_onJoyDeviceAdded.call(*this, evt.jdevice);
        break;
    case SDL_JOYDEVICEREMOVED:
        this->_onJoyDeviceRemoved.call(*this, evt.jdevice);
        break;

    case SDL_CONTROLLERAXISMOTION:
        this->_onControllerAxisMotion.call(*this, evt.caxis);
        break;
    case SDL_CONTROLLERBUTTONDOWN:
        this->_onControllerButtonDown.call(*this, evt.cbutton);
        break;
    case SDL_CONTROLLERBUTTONUP:
        this->_onControllerButtonUp.call(*this, evt.cbutton);
        break;
    case SDL_CONTROLLERDEVICEADDED:
        this->_onControllerDeviceAdded.call(*this, evt.cdevice);
        break;
    case SDL_CONTROLLERDEVICEREMOVED:
        this->_onControllerDeviceRemoved.call(*this, evt.cdevice);
        break;
    case SDL_CONTROLLERDEVICEREMAPPED:
        this->_onControllerDeviceRemapped.call(*this, evt.cdevice);
        break;

    case SDL_FINGERDOWN:
        this->_onFingerDown.call(*this, evt.tfinger);
        break;
    case SDL_FINGERUP:
        this->_onFingerUp.call(*this, evt.tfinger);
        break;
    case SDL_FINGERMOTION:
        this->_onFingerMotion.call(*this, evt.tfinger);
        break;

    case SDL_DOLLARGESTURE:
        this->_onDollarGesture.call(*this, evt.dgesture);
        break;
    case SDL_DOLLARRECORD:
        this->_onDollarRecord.call(*this, evt.dgesture);
        break;
    case SDL_MULTIGESTURE:
        this->_onMultiGesture.call(*this, evt.mgesture);
        break;

    case SDL_CLIPBOARDUPDATE:
        this->_onClipboardUpdate.call(*this, evt.common);
        break;

    case SDL_DROPFILE:
        this->_onDropFile.call(*this, evt.drop);
        break;
    case SDL_DROPTEXT:
        this->_onDropText.call(*this, evt.drop);
        break;
    case SDL_DROPBEGIN:
        this->_onDropBegin.call(*this, evt.drop);
        break;
    case SDL_DROPCOMPLETE:
        this->_onDropComplete.call(*this, evt.drop);
        break;

    case SDL_AUDIODEVICEADDED:
        this->_onAudioDeviceAdded.call(*this, evt.adevice);
        break;
    case SDL_AUDIODEVICEREMOVED:
        this->_onAudioDeviceRemoved.call(*this, evt.adevice);
        break;

    case SDL_RENDER_TARGETS_RESET:
        this->_onRenderTargetReset.call(*this, evt.common);
        break;
    case SDL_RENDER_DEVICE_RESET:
        this->_onRenderDeviceReset.call(*this, evt.common);
        break;
    }
}
void Event::process(unsigned int maxEventCount)
{
    SDL_Event evt;
    this->start();

    while (SDL_PollEvent(&evt) == 1 && maxEventCount > 0)
    {
        this->process(evt);
        --maxEventCount;
    }
}
#endif //FGE_DEF_SERVER

void Event::pushType(SDL_EventType type)
{
    this->g_types |= fge::Event::EventTypeToBitMask(type);
}
void Event::popType(SDL_EventType type)
{
    this->g_types &= ~fge::Event::EventTypeToBitMask(type);
}

bool Event::isKeyPressed(uint32_t keycode) const
{
    std::size_t const index = fge::Event::KeycodeToBitIndex(keycode);
    return static_cast<bool>(this->g_keyCodes[index / 32] & (0x80000000 >> (index % 32)));
}
uint32_t Event::getKeyUnicode() const
{
    return this->g_keyUnicode;
}

fge::Vector2i const& Event::getWindowSize() const
{
    return this->g_windowSize;
}
fge::Vector2i const& Event::getWindowPos() const
{
    return this->g_windowPosition;
}

bool Event::isEventType(uint32_t type) const
{
    return static_cast<bool>(this->g_types & fge::Event::EventTypeToBitMask(type));
}

fge::Vector2i const& Event::getMousePixelPos() const
{
    return this->g_mousePixelPosition;
}
bool Event::isMouseButtonPressed(uint8_t mouseButton) const
{
    return static_cast<bool>(this->g_mouseButtons & SDL_BUTTON(mouseButton));
}

int Event::getMouseWheelHorizontalDelta() const
{
    return this->g_mouseWheelHorizontalDelta;
}
int Event::getMouseWheelVerticalDelta() const
{
    return this->g_mouseWheelVerticalDelta;
}

fge::net::Packet& Event::pack(fge::net::Packet& pck)
{
    pck << this->g_types;
    for (auto keyCode: this->g_keyCodes)
    {
        pck << keyCode;
    }
    pck << this->g_keyUnicode;

    pck << this->g_mouseRelativeMotion;
    pck << this->g_mousePixelPosition;
    pck << this->g_mouseButtons;
    pck << this->g_mouseWheelHorizontalDelta;
    pck << this->g_mouseWheelVerticalDelta;

    pck << this->g_windowSize;
    pck << this->g_windowPosition;

    return pck;
}
fge::net::Packet& Event::unpack(fge::net::Packet& pck)
{
    pck >> this->g_types;
    for (auto& keyCode: this->g_keyCodes)
    {
        pck >> keyCode;
    }
    pck >> this->g_keyUnicode;

    pck >> this->g_mouseRelativeMotion;
    pck >> this->g_mousePixelPosition;
    pck >> this->g_mouseButtons;
    pck >> this->g_mouseWheelHorizontalDelta;
    pck >> this->g_mouseWheelVerticalDelta;

    pck >> this->g_windowSize;
    pck >> this->g_windowPosition;

    return pck;
}

std::string Event::getBinaryKeysString() const
{
    std::string result;

    for (std::size_t i = 0; i < FGE_EVENT_KEYCODES_SIZE; ++i)
    {
        for (std::size_t a = 0; a < 32; ++a)
        {
            result += static_cast<bool>(this->g_keyCodes[i] & (0x80000000 >> a)) ? '1' : '0';
        }
        result += ' ';
    }
    return result;
}
std::string Event::getBinaryTypesString() const
{
    std::string result;

    for (unsigned int i = 0; i < 64; ++i)
    {
        result += static_cast<bool>(this->g_types & (0x8000000000000000 >> i)) ? '1' : '0';
    }
    return result;
}
std::string Event::getBinaryMouseButtonsString() const
{
    std::string result;

    for (unsigned int i = 0; i < 8; ++i)
    {
        result += static_cast<bool>(this->g_mouseButtons & (0x80 >> i)) ? '1' : '0';
    }
    return result;
}

uint64_t Event::EventTypeToBitMask(uint32_t type)
{
    if ((type >= SDL_POLLSENTINEL) || (type < SDL_QUIT))
    {
        return 0;
    }

    /** EXAMPLE FOR GETTING A KEY WITH SDL
     *
     * This is some SDL types that we can encounter :
     * SDL_QUIT = 0x100
     * SDL_DISPLAYEVENT = 0x150
     * SDL_WINDOWEVENT = 0x200
     * SDL_RENDER_TARGETS_RESET = 0x2000
     * SDL_CONTROLLERAXISMOTION = 0x650
     * etc...
     *
     * We assume that a type is 16bits wide.
     *
     * First, we obtain the most significant byte (the less significant byte will be the index) :
     * KEY_SDL_QUIT = 0x1
     * KEY_SDL_DISPLAYEVENT = 0x1
     * KEY_SDL_WINDOWEVENT = 0x2
     * KEY_SDL_RENDER_TARGETS_RESET = 0x20
     * KEY_SDL_CONTROLLERAXISMOTION = 0x6
     *
     * By doing so we are getting a nice possibility of key with 2-3 collision.
     * To avoid collision, we subtract 1 if the type contain 0x0050 :
     * KEY_SDL_QUIT = 0x1
     * KEY_SDL_DISPLAYEVENT = 0x0
     * KEY_SDL_WINDOWEVENT = 0x2
     * KEY_SDL_RENDER_TARGETS_RESET = 0x20
     * KEY_SDL_CONTROLLERAXISMOTION = 0x5
     *
     * That's pretty good, we have a key that goes 0 to 9 exception for
     * types that are 0x1000, 0x1100, 0x1200 that we must convert :
     *
     * (key&0x0F) + 10 + (key&0x10 ? 1 : 0)
     *
     * Now keys will be in this order :
     *
     * Display events => size 1, key 0
     * Application events => size 8, key 1
     * Window events => size 2, key 2
     * Keyboard events => size 5, key 3
     * Mouse events => size 4, key 4
     * Game controller events => size 10, key 5
     * Joystick events => size 7, key 6
     * Touch events => size 3, key 7
     * Gesture events => size 3, key 8
     * Clipboard events => size 1, key 9
     *
     * Render events => size 2, key 10
     *
     * Drag and drop events => size 4, key 11
     * Audio hotplug events => size 2, key 12
     * Sensor events => size 1, key 13
     *
     * Then we can map a bit entry point on a 64bits value with the key :
    **/

    static uint8_t const sdlEventCategoryBitEntryPoint[14] = {
            0,  //Display events => size 1, key 0
            1,  //Application events => size 8, key 1
            9,  //Window events => size 2, key 2
            11, //Keyboard events => size 5, key 3
            16, //Mouse events => size 4, key 4
            20, //Game controller events => size 10, key 5
            30, //Joystick events => size 7, key 6
            37, //Touch events => size 3, key 7
            40, //Gesture events => size 3, key 8
            43, //Clipboard events => size 1, key 9
            44, //Render events => size 2, key 10
            46, //Drag and drop events => size 4, key 11
            50, //Audio hotplug events => size 2, key 12
            52  //Sensor events => size 1, key 13
    };

    //Transform the type into a key
    uint8_t key = static_cast<uint8_t>(type >> 8) - (static_cast<bool>(type & 0x000000F0) ? 1 : 0);
    if (key > 9)
    {
        key = (key & 0x0F) + 10 + (static_cast<bool>(key & 0x10) ? 1 : 0);
    }
    auto const index = static_cast<uint8_t>(type);

    return static_cast<uint64_t>(1) << (sdlEventCategoryBitEntryPoint[key] + index);
}
std::size_t Event::KeycodeToBitIndex(uint32_t keyCode)
{
    if (static_cast<bool>(keyCode & SDLK_SCANCODE_MASK))
    { //Scancode
        return (keyCode & ~SDLK_SCANCODE_MASK) - 57 + 128;
    }
    return keyCode;
}
uint32_t Event::UTF8ToUTF32(char const* utf8)
{
    tiny_utf8::string const str = utf8;
    return str.front();
}

} // namespace fge

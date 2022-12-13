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

#include "FastEngine/C_event.hpp"
#include <FastEngine/C_packet.hpp>

namespace fge
{

void Event::clear()
{
    ///Event type
    this->g_types = 0;

    ///Keyboard
    this->g_keys[0] = 0;
    this->g_keys[1] = 0;
    this->g_keys[2] = 0;
    this->g_keys[3] = 0;
    this->g_keyUnicode = 0;

    ///Mouse
    this->g_mousePixelPos = sf::Vector2i(0, 0);
    this->g_mouseButtons = 0;
    this->g_mouseWheelHorizontalDelta = 0;
    this->g_mouseWheelVerticalDelta = 0;

    ///Window size
    this->g_windowSize = sf::Vector2u(0, 0);
}

void Event::start()
{
    this->g_types = 0;
    this->g_keyUnicode = 0;
    this->g_mouseWheelHorizontalDelta = 0;
    this->g_mouseWheelVerticalDelta = 0;
}
#ifndef FGE_DEF_SERVER
void Event::process(const sf::Event& sfevt)
{
    this->g_types |= (0x80000000 >> sfevt.type);

    switch (sfevt.type)
    {
    case sf::Event::EventType::Closed:
        this->_onClosed.call(*this);
        break;
    case sf::Event::EventType::Resized:
        this->g_windowSize.x = sfevt.size.width;
        this->g_windowSize.y = sfevt.size.height;
        this->_onResized.call(*this, sfevt.size);
        break;
    case sf::Event::EventType::LostFocus:
        this->_onLostFocus.call(*this);
        break;
    case sf::Event::EventType::GainedFocus:
        this->_onGainedFocus.call(*this);
        break;
    case sf::Event::EventType::TextEntered:
        this->g_keyUnicode = sfevt.text.unicode;
        this->_onTextEntered.call(*this, sfevt.text);
        break;
    case sf::Event::EventType::KeyPressed:
        if ((sfevt.key.code >= 0) && (sfevt.key.code < sf::Keyboard::Key::KeyCount))
        {
            this->g_keys[sfevt.key.code / 32] |= (0x80000000 >> (sfevt.key.code % 32));
        }
        this->_onKeyPressed.call(*this, sfevt.key);
        break;
    case sf::Event::EventType::KeyReleased:
        if ((sfevt.key.code >= 0) && (sfevt.key.code < sf::Keyboard::Key::KeyCount))
        {
            this->g_keys[sfevt.key.code / 32] &= ~(0x80000000 >> (sfevt.key.code % 32));
        }
        this->_onKeyReleased.call(*this, sfevt.key);
        break;
    case sf::Event::EventType::MouseWheelScrolled:
        this->g_mousePixelPos = sf::Vector2i(sfevt.mouseWheelScroll.x, sfevt.mouseWheelScroll.y);
        if (sfevt.mouseWheelScroll.wheel == sf::Mouse::Wheel::HorizontalWheel)
        {
            this->g_mouseWheelHorizontalDelta = sfevt.mouseWheelScroll.delta;
        }
        else
        {
            this->g_mouseWheelVerticalDelta = sfevt.mouseWheelScroll.delta;
        }
        this->_onMouseWheelScrolled.call(*this, sfevt.mouseWheelScroll);
        break;
    case sf::Event::EventType::MouseButtonPressed:
        this->g_mouseButtons |= (0x80 >> (sfevt.mouseButton.button));
        this->g_mousePixelPos = sf::Vector2i(sfevt.mouseButton.x, sfevt.mouseButton.y);
        this->_onMouseButtonPressed.call(*this, sfevt.mouseButton);
        break;
    case sf::Event::EventType::MouseButtonReleased:
        this->g_mouseButtons &= ~(0x80 >> (sfevt.mouseButton.button));
        this->g_mousePixelPos = sf::Vector2i(sfevt.mouseButton.x, sfevt.mouseButton.y);
        this->_onMouseButtonReleased.call(*this, sfevt.mouseButton);
        break;
    case sf::Event::EventType::MouseMoved:
        this->g_mousePixelPos = sf::Vector2i(sfevt.mouseMove.x, sfevt.mouseMove.y);
        this->_onMouseMoved.call(*this, sfevt.mouseMove);
        break;
    case sf::Event::EventType::MouseEntered:
        this->_onMouseEntered.call(*this);
        break;
    case sf::Event::EventType::MouseLeft:
        this->_onMouseLeft.call(*this);
        break;
    case sf::Event::EventType::JoystickButtonPressed:
        this->_onJoystickButtonPressed.call(*this, sfevt.joystickButton);
        break;
    case sf::Event::EventType::JoystickButtonReleased:
        this->_onJoystickButtonReleased.call(*this, sfevt.joystickButton);
        break;
    case sf::Event::EventType::JoystickMoved:
        this->_onJoystickMoved.call(*this, sfevt.joystickMove);
        break;
    case sf::Event::EventType::JoystickConnected:
        this->_onJoystickConnected.call(*this, sfevt.joystickConnect);
        break;
    case sf::Event::EventType::JoystickDisconnected:
        this->_onJoystickDisconnected.call(*this, sfevt.joystickConnect);
        break;
    case sf::Event::EventType::TouchBegan:
        this->_onTouchBegan.call(*this, sfevt.touch);
        break;
    case sf::Event::EventType::TouchMoved:
        this->_onTouchMoved.call(*this, sfevt.touch);
        break;
    case sf::Event::EventType::TouchEnded:
        this->_onTouchEnded.call(*this, sfevt.touch);
        break;
    case sf::Event::EventType::SensorChanged:
        this->_onSensorChanged.call(*this, sfevt.sensor);
        break;
    default:
        break;
    }
}
void Event::process(sf::Window& sfscreen, unsigned int maxEventCount)
{
    sf::Event evt{};
    this->start();
    while (sfscreen.pollEvent(evt) && maxEventCount > 0)
    {
        this->process(evt);
        --maxEventCount;
    }
}
#endif //FGE_DEF_SERVER

bool Event::isKeyPressed(sf::Keyboard::Key sfkey) const
{
    return static_cast<bool>(this->g_keys[sfkey / 32] & (0x80000000 >> (sfkey % 32)));
}
uint32_t Event::getKeyUnicode() const
{
    return this->g_keyUnicode;
}

const sf::Vector2u& Event::getWindowSize() const
{
    return this->g_windowSize;
}

bool Event::isEventType(sf::Event::EventType evtType) const
{
    return static_cast<bool>(this->g_types & (0x80000000 >> evtType));
}

const sf::Vector2i& Event::getMousePixelPos() const
{
    return this->g_mousePixelPos;
}
bool Event::isMouseButtonPressed(sf::Mouse::Button sfmouse) const
{
    return static_cast<bool>(this->g_mouseButtons & (0x80 >> (sfmouse)));
}

float Event::getMouseWheelHorizontalDelta() const
{
    return this->g_mouseWheelHorizontalDelta;
}
float Event::getMouseWheelVerticalDelta() const
{
    return this->g_mouseWheelVerticalDelta;
}

fge::net::Packet& Event::pack(fge::net::Packet& pck)
{
    pck << this->g_types;
    pck << this->g_keys[0] << this->g_keys[1] << this->g_keys[2] << this->g_keys[3];
    pck << this->g_keyUnicode;

    pck << this->g_mousePixelPos;
    pck << this->g_mouseButtons;
    pck << this->g_mouseWheelHorizontalDelta << this->g_mouseWheelVerticalDelta;
    return pck;
}
fge::net::Packet& Event::unpack(fge::net::Packet& pck)
{
    pck >> this->g_types;
    pck >> this->g_keys[0] >> this->g_keys[1] >> this->g_keys[2] >> this->g_keys[3];
    pck >> this->g_keyUnicode;

    pck >> this->g_mousePixelPos;
    pck >> this->g_mouseButtons;
    pck >> this->g_mouseWheelHorizontalDelta >> this->g_mouseWheelVerticalDelta;
    return pck;
}

std::string Event::getBinaryKeysString() const
{
    std::string result;

    for (unsigned int i = 0; i < 4; ++i)
    {
        for (unsigned int a = 0; a < 32; ++a)
        {
            result += static_cast<bool>(this->g_keys[i] & (0x80000000 >> a)) ? '1' : '0';
        }
        result += ' ';
    }
    return result;
}
std::string Event::getBinaryTypesString() const
{
    std::string result;

    for (unsigned int i = 0; i < 32; ++i)
    {
        result += static_cast<bool>(this->g_types & (0x80000000 >> i)) ? '1' : '0';
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

} // namespace fge

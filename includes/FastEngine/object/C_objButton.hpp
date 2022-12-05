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

#ifndef _FGE_C_OBJBUTTON_HPP_INCLUDED
#define _FGE_C_OBJBUTTON_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "C_object.hpp"
#include "C_objSprite.hpp"
#include "FastEngine/C_guiElement.hpp"

#define FGE_OBJBUTTON_CLASSNAME "FGE:OBJ:BUTTON"

namespace fge
{

class FGE_API ObjButton : public fge::Object, public fge::Subscriber, public fge::GuiElement
{
public:
    ObjButton();
    ObjButton(fge::Texture textureOn, fge::Texture textureOff, const sf::Vector2f& pos=sf::Vector2f());
    explicit ObjButton(const fge::Texture& texture, const sf::Vector2f& pos=sf::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjButton)

    fge::GuiElement* getGuiElement() override
    {
        return this;
    }

    const fge::Texture& getTextureOn() const;
    const fge::Texture& getTextureOff() const;
    void setTextureOn(const fge::Texture& textureOn);
    void setTextureOff(const fge::Texture& textureOff);

    void setColor(const sf::Color& color);

    void setActiveStat(bool active);
    bool getActiveStat() const;

    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;

    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    sf::FloatRect getGlobalBounds() const override;
    sf::FloatRect getLocalBounds() const override;

    fge::CallbackHandler<fge::ObjButton*> _onButtonPressed;

private:
    void onGuiMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, fge::GuiElementContext& context);
    void onMouseButtonReleased(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg);
    void onGuiMouseMoved(const fge::Event& evt, const sf::Event::MouseMoveEvent& arg, fge::GuiElementContext& context);

    void onGuiVerify(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context) override;

    mutable fge::ObjSprite g_sprite;

    fge::Texture g_textureOn;
    fge::Texture g_textureOff;

    sf::Color g_color;

    bool g_statMouseOn = false;
    bool g_statActive = false;
};

}//end fge

#endif // _FGE_C_OBJBUTTON_HPP_INCLUDED

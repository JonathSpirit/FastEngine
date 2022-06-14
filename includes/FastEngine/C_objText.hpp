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

#ifndef _FGE_C_OBJTEXT_HPP_INCLUDED
#define _FGE_C_OBJTEXT_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_font.hpp>

#define FGE_OBJTEXT_CLASSNAME "FGE:OBJ:TEXT"

namespace fge
{

class FGE_API ObjText : public fge::Object
{
public:
    ObjText();
    explicit ObjText(const fge::Font& font, const sf::Vector2f& position=sf::Vector2f());
    ObjText(const fge::Font& font, const std::string& txt, const sf::Vector2f& position=sf::Vector2f());
    ObjText(const fge::Font& font, const char* txt, const sf::Vector2f& position=sf::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjText)

    void setFont(const fge::Font& font);
    const fge::Font& getFont() const;

    inline void setString(const sf::String& string)
    {
        this->g_text.setString(string);
    }

    inline void setCharacterSize(unsigned int size)
    {
        this->g_text.setCharacterSize(size);
    }

    inline void setLineSpacing(float spacingFactor)
    {
        this->g_text.setLineSpacing(spacingFactor);
    }
    inline void setLetterSpacing(float spacingFactor)
    {
        this->g_text.setLetterSpacing(spacingFactor);
    }

    inline void setStyle(uint32_t style)
    {
        this->g_text.setStyle(style);
    }

    inline void setFillColor(const sf::Color& color)
    {
        this->g_text.setFillColor(color);
    }
    inline void setOutlineColor(const sf::Color& color)
    {
        this->g_text.setOutlineColor(color);
    }

    inline void setOutlineThickness(float thickness)
    {
        this->g_text.setOutlineThickness(thickness);
    }

    inline const sf::String& getString() const
    {
        return this->g_text.getString();
    }

    inline unsigned int getCharacterSize() const
    {
        return this->g_text.getCharacterSize();
    }

    inline float getLetterSpacing() const
    {
        return this->g_text.getLetterSpacing();
    }
    inline float getLineSpacing() const
    {
        return this->g_text.getLineSpacing();
    }

    inline uint32_t getStyle() const
    {
        return this->g_text.getStyle();
    }

    inline const sf::Color& getFillColor() const
    {
        return this->g_text.getFillColor();
    }
    inline const sf::Color& getOutlineColor() const
    {
        return this->g_text.getOutlineColor();
    }

    inline float getOutlineThickness() const
    {
        return this->g_text.getOutlineThickness();
    }

    inline sf::Vector2f findCharacterPos(std::size_t index) const
    {
        return this->g_text.findCharacterPos(index);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void save(nlohmann::json& jsonObject, fge::Scene* scene_ptr) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene_ptr) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    std::string getClassName() const override;
    std::string getReadableClassName() const override;

    sf::FloatRect getGlobalBounds() const override;
    sf::FloatRect getLocalBounds() const override;

private:
    mutable sf::Text g_text;
    fge::Font g_font;
};

}//end fge

#endif // _FGE_C_OBJTEXT_HPP_INCLUDED

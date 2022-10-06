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

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_font.hpp>
#include <vector>

#define FGE_OBJTEXT_CLASSNAME "FGE:OBJ:TEXT"

namespace fge
{

class ObjText;

class FGE_API Character : public sf::Transformable, public sf::Drawable
{
public:
    Character() = default;
    Character(const sf::Color& fillColor, const sf::Color& outlineColor);

    void clear();
    void addLine(bool outlineVertices, float lineLength, float lineTop, float offset, float thickness, float outlineThickness = 0.0f);
    void addGlyphQuad(bool outlineVertices, const sf::Vector2f& size, const sf::Glyph& glyph, float italicShear);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void setFillColor(const sf::Color& color);
    void setOutlineColor(const sf::Color& color);

    [[nodiscard]] const sf::Color& getFillColor() const;
    [[nodiscard]] const sf::Color& getOutlineColor() const;

    void setVisibility(bool visibility);
    [[nodiscard]] bool isVisible() const;

    [[nodiscard]] uint32_t getUnicode() const;

private:
    friend ObjText;

    sf::VertexArray g_vertices{sf::PrimitiveType::Triangles}; /// Vertex array containing the fill geometry
    sf::VertexArray g_outlineVertices{sf::PrimitiveType::Triangles}; /// Vertex array containing the outline geometry

    sf::Color g_fillColor{255,255,255};
    sf::Color g_outlineColor{0,0,0};

    uint32_t g_unicodeChar;

    bool g_visibility{true};
};

class FGE_API ObjText : public fge::Object
{
public:
    using CharacterSize = uint16_t;

    enum Style : uint8_t
    {
        Regular       = 0,      /// Regular characters, no style
        Bold          = 1 << 0, /// Bold characters
        Italic        = 1 << 1, /// Italic characters
        Underlined    = 1 << 2, /// Underlined characters
        StrikeThrough = 1 << 3  /// Strike through characters
    };

    ObjText() = default;
    ObjText(const std::string& string, const fge::Font& font, const sf::Vector2f& position = {}, fge::ObjText::CharacterSize characterSize = 30);
    explicit ObjText(const fge::Font& font, const sf::Vector2f& position = {}, fge::ObjText::CharacterSize characterSize = 30);

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjText)

    void setFont(const fge::Font& font);
    const fge::Font& getFont() const;

    void setUtf8String(const std::string& string);

    void setCharacterSize(fge::ObjText::CharacterSize size);

    void setLineSpacing(float spacingFactor);
    void setLetterSpacing(float spacingFactor);

    void setStyle(std::underlying_type<Style>::type style);

    void setFillColor(const sf::Color& color);
    void setOutlineColor(const sf::Color& color);

    void setOutlineThickness(float thickness);

    const std::string& getUtf8String() const;

    fge::ObjText::CharacterSize getCharacterSize() const;

    float getLetterSpacing() const;
    float getLineSpacing() const;

    std::underlying_type<Style>::type getStyle() const;

    const sf::Color& getFillColor() const;
    const sf::Color& getOutlineColor() const;

    float getOutlineThickness() const;

    sf::Vector2f findCharacterPos(std::size_t index) const;

    std::vector<fge::Character>& getCharacters();
    const std::vector<fge::Character>& getCharacters() const;

    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    sf::FloatRect getGlobalBounds() const override;
    sf::FloatRect getLocalBounds() const override;

private:
    void ensureGeometryUpdate() const;

    std::string g_utf8String; /// String to display
    fge::Font g_font; /// Font used to display the string
    fge::ObjText::CharacterSize g_characterSize{30}; /// Base size of characters, in pixels
    float g_letterSpacingFactor{1.0f}; /// Spacing factor between letters
    float g_lineSpacingFactor{1.0f}; /// Spacing factor between lines
    std::underlying_type<Style>::type g_style{Regular}; /// Text style (see Style enum)
    sf::Color g_fillColor{255,255,255}; /// Text fill color
    sf::Color g_outlineColor{0,0,0}; /// Text outline color
    float g_outlineThickness{0.0f}; /// Thickness of the text's outline

    mutable std::vector<Character> g_characters;
    mutable sf::FloatRect g_bounds; /// Bounding rectangle of the text (in local coordinates)
    mutable bool g_geometryNeedUpdate{false}; /// Does the geometry need to be recomputed?
    mutable uint64_t g_fontTextureId{0}; /// The font texture id
};

}//end fge

#endif // _FGE_C_OBJTEXT_HPP_INCLUDED

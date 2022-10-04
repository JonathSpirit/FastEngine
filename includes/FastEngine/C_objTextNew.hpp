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

#ifndef _FGE_C_OBJTEXTNEW_HPP_INCLUDED
#define _FGE_C_OBJTEXTNEW_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_font.hpp>
#include <vector>

#define FGE_OBJTEXTNEW_CLASSNAME "FGE:OBJ:TEXTNEW"

namespace fge
{

class ObjTextNew;

class FGE_API Character : public sf::Transformable
{
public:
    Character() = default;
    Character(const sf::Color& fillColor, const sf::Color& outlineColor);

    void clear();
    void addLine(bool outlineVertices, float lineLength, float lineTop, float offset, float thickness, float outlineThickness = 0.0f);
    void addGlyphQuad(bool outlineVertices, const sf::Vector2f& size, const sf::Glyph& glyph, float italicShear);

    void setFillColor(const sf::Color& color);
    void setOutlineColor(const sf::Color& color);

    [[nodiscard]] const sf::Color& getFillColor() const;
    [[nodiscard]] const sf::Color& getOutlineColor() const;

private:
    friend ObjTextNew;

    sf::VertexArray g_vertices{sf::PrimitiveType::Triangles}; /// Vertex array containing the fill geometry
    sf::VertexArray g_outlineVertices{sf::PrimitiveType::Triangles}; /// Vertex array containing the outline geometry

    sf::Color g_fillColor{255,255,255};
    sf::Color g_outlineColor{0,0,0};
};

class FGE_API ObjTextNew : public fge::Object
{
public:
    enum Style : uint8_t
    {
        Regular       = 0,      /// Regular characters, no style
        Bold          = 1 << 0, /// Bold characters
        Italic        = 1 << 1, /// Italic characters
        Underlined    = 1 << 2, /// Underlined characters
        StrikeThrough = 1 << 3  /// Strike through characters
    };

    ObjTextNew();
    ObjTextNew(const sf::String& string, const fge::Font& font, const sf::Vector2f& position = {}, unsigned int characterSize = 30);

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjTextNew)

    void setFont(const fge::Font& font);
    const fge::Font& getFont() const;

    void setString(const sf::String& string);

    void setCharacterSize(unsigned int size);

    void setLineSpacing(float spacingFactor);
    void setLetterSpacing(float spacingFactor);

    void setStyle(uint32_t style);

    void setFillColor(const sf::Color& color);
    void setOutlineColor(const sf::Color& color);

    void setOutlineThickness(float thickness);

    const sf::String& getString() const;

    unsigned int getCharacterSize() const;

    float getLetterSpacing() const;
    float getLineSpacing() const;

    uint32_t getStyle() const;

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

    sf::String g_string; /// String to display
    fge::Font g_font; /// Font used to display the string
    uint16_t g_characterSize{30}; /// Base size of characters, in pixels
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

#endif // _FGE_C_OBJTEXTNEW_HPP_INCLUDED
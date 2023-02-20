/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/fastengine_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/C_font.hpp"
#include "FastEngine/graphic/C_glyph.hpp"
#include "tinyutf8.h"
#include <vector>

#define FGE_OBJTEXT_CLASSNAME "FGE:OBJ:TEXT"

namespace fge
{

class ObjText;

class FGE_API Character : public fge::Transformable, public fge::Drawable
{
public:
    Character();
    Character(const fge::Color& fillColor, const fge::Color& outlineColor);

    void clear();
    void addLine(bool outlineVertices,
                 float lineLength,
                 float lineTop,
                 float offset,
                 float thickness,
                 float outlineThickness = 0.0f);
    void addGlyphQuad(bool outlineVertices,
                      const fge::Vector2f& size,
                      const fge::Glyph& glyph,
                      const fge::Vector2i& textureSize,
                      float italicShear);

    void draw(fge::RenderTarget& target, const fge::RenderStates& states) const override;
    void drawVertices(bool outlineVertices, fge::RenderTarget& target, const fge::RenderStates& states) const;

    void setFillColor(const fge::Color& color);
    void setOutlineColor(const fge::Color& color);

    [[nodiscard]] const fge::Color& getFillColor() const;
    [[nodiscard]] const fge::Color& getOutlineColor() const;

    void setVisibility(bool visibility);
    [[nodiscard]] bool isVisible() const;

    [[nodiscard]] uint32_t getUnicode() const;

private:
    friend ObjText;

    fge::vulkan::VertexBuffer g_vertices;        /// Vertex array containing the fill geometry
    fge::vulkan::VertexBuffer g_outlineVertices; /// Vertex array containing the outline geometry

    fge::Color g_fillColor{255, 255, 255};
    fge::Color g_outlineColor{0, 0, 0};

    uint32_t g_unicodeChar{0};

    bool g_visibility{true};
};

class FGE_API ObjText : public fge::Object
{
public:
    enum Style : uint8_t
    {
        Regular = 0,           /// Regular characters, no style
        Bold = 1 << 0,         /// Bold characters
        Italic = 1 << 1,       /// Italic characters
        Underlined = 1 << 2,   /// Underlined characters
        StrikeThrough = 1 << 3 /// Strike through characters
    };

    ObjText() = default;
    ObjText(tiny_utf8::string string,
            fge::Font font,
            const fge::Vector2f& position = {},
            fge::CharacterSize characterSize = 30);
    explicit ObjText(fge::Font font, const fge::Vector2f& position = {}, fge::CharacterSize characterSize = 30);

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjText)

    void setFont(fge::Font font);
    const fge::Font& getFont() const;

    void setString(tiny_utf8::string string);

    void setCharacterSize(fge::CharacterSize size);

    void setLineSpacingFactor(float spacingFactor);
    void setLetterSpacingFactor(float spacingFactor);

    void setStyle(std::underlying_type<Style>::type style);

    void setFillColor(const fge::Color& color);
    void setOutlineColor(const fge::Color& color);

    void setOutlineThickness(float thickness);

    const tiny_utf8::string& getString() const;

    fge::CharacterSize getCharacterSize() const;

    float getLetterSpacingFactor() const;
    float getLineSpacingFactor() const;
    float getLineSpacing() const;

    std::underlying_type<Style>::type getStyle() const;

    const fge::Color& getFillColor() const;
    const fge::Color& getOutlineColor() const;

    float getOutlineThickness() const;

    fge::Vector2f findCharacterPos(std::size_t index) const;

    std::vector<fge::Character>& getCharacters();
    const std::vector<fge::Character>& getCharacters() const;

    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

private:
    void ensureGeometryUpdate() const;

    tiny_utf8::string g_string;                         /// String to display
    fge::Font g_font;                                   /// Font used to display the string
    fge::CharacterSize g_characterSize{30};             /// Base size of characters, in pixels
    float g_letterSpacingFactor{1.0f};                  /// Spacing factor between letters
    float g_lineSpacingFactor{1.0f};                    /// Spacing factor between lines
    std::underlying_type<Style>::type g_style{Regular}; /// Text style (see Style enum)
    fge::Color g_fillColor{255, 255, 255};              /// Text fill color
    fge::Color g_outlineColor{0, 0, 0};                 /// Text outline color
    float g_outlineThickness{0.0f};                     /// Thickness of the text's outline

    mutable std::vector<Character> g_characters;
    mutable fge::RectFloat g_bounds;                    /// Bounding rectangle of the text (in local coordinates)
    mutable bool g_geometryNeedUpdate{false};           /// Does the geometry need to be recomputed?
    mutable uint32_t g_fontTextureModificationCount{0}; /// The font texture id
};

} // namespace fge

#endif // _FGE_C_OBJTEXT_HPP_INCLUDED

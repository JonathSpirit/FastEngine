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

#define private public //TODO: A bit hacky, must be changed in future !
#include "SFML/Graphics/Texture.hpp"
#undef private

#include "FastEngine/arbitraryJsonTypes.hpp"
#include "FastEngine/manager/font_manager.hpp"
#include "FastEngine/object/C_objText.hpp"

namespace fge
{

Character::Character(const fge::Color& fillColor, const fge::Color& outlineColor) :
        g_fillColor(fillColor),
        g_outlineColor(outlineColor)
{}

void Character::clear()
{
    this->g_vertices.clear();
    this->g_outlineVertices.clear();
}
// Add an underline or strikethrough line to the vertex array
void Character::addLine(bool outlineVertices,
                        float lineLength,
                        float lineTop,
                        float offset,
                        float thickness,
                        float outlineThickness)
{
    fge::vulkan::VertexBuffer* vertices = outlineVertices ? &this->g_outlineVertices : &this->g_vertices;
    fge::Color color = outlineVertices ? this->g_outlineColor : this->g_fillColor;

    const float top = std::floor(lineTop + offset - (thickness / 2.0f) + 0.5f);
    const float bottom = top + std::floor(thickness + 0.5f);

    vertices->append(fge::vulkan::Vertex{{-outlineThickness, top - outlineThickness}, color, {1, 1}});
    vertices->append(fge::vulkan::Vertex{{lineLength + outlineThickness, top - outlineThickness}, color, {1, 1}});
    vertices->append(fge::vulkan::Vertex{{-outlineThickness, bottom + outlineThickness}, color, {1, 1}});
    vertices->append(fge::vulkan::Vertex{{-outlineThickness, bottom + outlineThickness}, color, {1, 1}});
    vertices->append(fge::vulkan::Vertex{{lineLength + outlineThickness, top - outlineThickness}, color, {1, 1}});
    vertices->append(fge::vulkan::Vertex{{lineLength + outlineThickness, bottom + outlineThickness}, color, {1, 1}});
}

// Add a glyph quad to the vertex array
void Character::addGlyphQuad(bool outlineVertices, const fge::Vector2f& size, const sf::Glyph& glyph, float italicShear)
{
    fge::vulkan::VertexBuffer* vertices = outlineVertices ? &this->g_outlineVertices : &this->g_vertices;
    fge::Color color = outlineVertices ? this->g_outlineColor : this->g_fillColor;

    const float padding = 1.0f;

    const float left = glyph.bounds.left - padding;
    const float top = glyph.bounds.top - padding;
    const float right = glyph.bounds.left + glyph.bounds.width + padding;
    const float bottom = glyph.bounds.top + glyph.bounds.height + padding;

    float u1 = static_cast<float>(glyph.textureRect.left) - padding;
    float v1 = static_cast<float>(glyph.textureRect.top) - padding;
    float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + padding;
    float v2 = static_cast<float>(glyph.textureRect.top + glyph.textureRect.height) + padding;

    vertices->append(fge::vulkan::Vertex{{size.x + left - italicShear * top, size.y + top}, color, {u1, v1}});
    vertices->append(fge::vulkan::Vertex{{size.x + right - italicShear * top, size.y + top}, color, {u2, v1}});
    vertices->append(fge::vulkan::Vertex{{size.x + left - italicShear * bottom, size.y + bottom}, color, {u1, v2}});
    vertices->append(fge::vulkan::Vertex{{size.x + left - italicShear * bottom, size.y + bottom}, color, {u1, v2}});
    vertices->append(fge::vulkan::Vertex{{size.x + right - italicShear * top, size.y + top}, color, {u2, v1}});
    vertices->append(fge::vulkan::Vertex{{size.x + right - italicShear * bottom, size.y + bottom}, color, {u2, v2}});
}

void Character::draw(fge::RenderTarget& target, const fge::RenderStates& states) const
{
    if (this->g_visibility)
    {
        auto copyStates = states.copy(this);
        copyStates._modelTransform *= this->getTransform();

        this->_g_graphicPipeline.setVertexBuffer(&this->g_outlineVertices);
        target.draw(this->_g_graphicPipeline, states);
        this->_g_graphicPipeline.setVertexBuffer(&this->g_vertices);
        target.draw(this->_g_graphicPipeline, states);
    }
}
void Character::drawVertices(bool outlineVertices, fge::RenderTarget& target, const fge::RenderStates& states) const
{
    if (this->g_visibility)
    {
        auto copyStates = states.copy(this);
        copyStates._modelTransform *= this->getTransform();

        if (outlineVertices)
        {
            this->_g_graphicPipeline.setVertexBuffer(&this->g_outlineVertices);
            target.draw(this->_g_graphicPipeline, states);
        }
        else
        {
            this->_g_graphicPipeline.setVertexBuffer(&this->g_vertices);
            target.draw(this->_g_graphicPipeline, states);
        }
    }
}

void Character::setFillColor(const fge::Color& color)
{
    this->g_fillColor = color;
    for (std::size_t i = 0; i < this->g_vertices.getVertexCount(); ++i)
    {
        this->g_vertices.getVertices()[i]._color = color;
    }
}
void Character::setOutlineColor(const fge::Color& color)
{
    this->g_outlineColor = color;
    for (std::size_t i = 0; i < this->g_outlineVertices.getVertexCount(); ++i)
    {
        this->g_outlineVertices.getVertices()[i]._color = color;
    }
}

const fge::Color& Character::getFillColor() const
{
    return this->g_fillColor;
}
const fge::Color& Character::getOutlineColor() const
{
    return this->g_outlineColor;
}

void Character::setVisibility(bool visibility)
{
    this->g_visibility = visibility;
}
bool Character::isVisible() const
{
    return this->g_visibility;
}

uint32_t Character::getUnicode() const
{
    return this->g_unicodeChar;
}

//ObjText

ObjText::ObjText(tiny_utf8::string string,
                 fge::Font font,
                 const fge::Vector2f& position,
                 fge::ObjText::CharacterSize characterSize) :
        g_font(std::move(font)),
        g_characterSize(characterSize),
        g_geometryNeedUpdate(true)
{
    this->setString(std::move(string));
    this->setPosition(position);
}
ObjText::ObjText(fge::Font font, const fge::Vector2f& position, fge::ObjText::CharacterSize characterSize) :
        g_font(std::move(font)),
        g_characterSize(characterSize)
{
    this->setPosition(position);
}

void ObjText::setFont(fge::Font font)
{
    this->g_font = std::move(font);
}
const fge::Font& ObjText::getFont() const
{
    return this->g_font;
}

void ObjText::setString(tiny_utf8::string string)
{
    if (this->g_string != string)
    {
        this->g_string = std::move(string);
        this->g_geometryNeedUpdate = true;
    }
}

void ObjText::setCharacterSize(fge::ObjText::CharacterSize size)
{
    if (this->g_characterSize != size)
    {
        this->g_characterSize = size;
        this->g_geometryNeedUpdate = true;
    }
}

void ObjText::setLineSpacingFactor(float spacingFactor)
{
    if (this->g_lineSpacingFactor != spacingFactor)
    {
        this->g_lineSpacingFactor = spacingFactor;
        this->g_geometryNeedUpdate = true;
    }
}
void ObjText::setLetterSpacingFactor(float spacingFactor)
{
    if (this->g_letterSpacingFactor != spacingFactor)
    {
        this->g_letterSpacingFactor = spacingFactor;
        this->g_geometryNeedUpdate = true;
    }
}

void ObjText::setStyle(std::underlying_type<Style>::type style)
{
    if (this->g_style != style)
    {
        this->g_style = style;
        this->g_geometryNeedUpdate = true;
    }
}

void ObjText::setFillColor(const fge::Color& color)
{
    if (color != this->g_fillColor)
    {
        this->g_fillColor = color;

        // Change vertex colors directly, no need to update whole geometry
        // (if geometry is updated anyway, we can skip this step)
        if (!this->g_geometryNeedUpdate)
        {
            for (auto& character: this->g_characters)
            {
                character.setFillColor(color);
            }
        }
    }
}
void ObjText::setOutlineColor(const fge::Color& color)
{
    if (color != this->g_outlineColor)
    {
        this->g_outlineColor = color;

        // Change vertex colors directly, no need to update whole geometry
        // (if geometry is updated anyway, we can skip this step)
        if (!this->g_geometryNeedUpdate)
        {
            for (auto& character: this->g_characters)
            {
                character.setOutlineColor(color);
            }
        }
    }
}

void ObjText::setOutlineThickness(float thickness)
{
    if (thickness != this->g_outlineThickness)
    {
        this->g_outlineThickness = thickness;
        this->g_geometryNeedUpdate = true;
    }
}

const tiny_utf8::string& ObjText::getString() const
{
    return this->g_string;
}

fge::ObjText::CharacterSize ObjText::getCharacterSize() const
{
    return this->g_characterSize;
}

float ObjText::getLetterSpacingFactor() const
{
    return this->g_letterSpacingFactor;
}
float ObjText::getLineSpacingFactor() const
{
    return this->g_lineSpacingFactor;
}
float ObjText::getLineSpacing() const
{
    return this->g_font.getData()->_font->getLineSpacing(this->g_characterSize) * this->g_lineSpacingFactor;
}

std::underlying_type<ObjText::Style>::type ObjText::getStyle() const
{
    return this->g_style;
}

const fge::Color& ObjText::getFillColor() const
{
    return this->g_fillColor;
}
const fge::Color& ObjText::getOutlineColor() const
{
    return this->g_outlineColor;
}

float ObjText::getOutlineThickness() const
{
    return this->g_outlineThickness;
}

fge::Vector2f ObjText::findCharacterPos(std::size_t index) const
{
    if (this->g_characters.empty())
    {
        return {};
    }

    // Adjust the index if it's out of range
    if (index > this->g_characters.size())
    {
        index = this->g_characters.size();
    }

    return this->getTransform() * this->g_characters[index].getPosition();
}

std::vector<fge::Character>& ObjText::getCharacters()
{
    return this->g_characters;
}
const std::vector<fge::Character>& ObjText::getCharacters() const
{
    return this->g_characters;
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjText)
{
    if (this->g_font.valid())
    {
        this->ensureGeometryUpdate();

        auto copyStates = states.copy(this);

        copyStates._modelTransform *= getTransform();
        ///copyStates._textureImage = &this->g_font.getData()->_font->getTexture(this->g_characterSize); TODO

        if (this->g_outlineThickness != 0.0f)
        {
            for (const auto& character: this->g_characters)
            {
                character.drawVertices(true, target, copyStates);
            }
        }

        for (const auto& character: this->g_characters)
        {
            character.drawVertices(false, target, copyStates);
        }
    }
}
#endif

void ObjText::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);

    jsonObject["string"] = this->g_string;
    jsonObject["font"] = this->g_font;
    jsonObject["characterSize"] = this->g_characterSize;
    jsonObject["letterSpacing"] = this->g_letterSpacingFactor;
    jsonObject["lineSpacing"] = this->g_lineSpacingFactor;
    jsonObject["style"] = static_cast<std::underlying_type<Style>::type>(this->g_style);
    jsonObject["fillColor"] = static_cast<uint32_t>(this->g_fillColor.toInteger());
    jsonObject["outlineColor"] = static_cast<uint32_t>(this->g_outlineColor.toInteger());
    jsonObject["outlineThickness"] = this->g_outlineThickness;
}
void ObjText::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);

    this->g_string = jsonObject.value<tiny_utf8::string>("string", {});
    this->g_font = jsonObject.value<fge::Font>("font", FGE_FONT_BAD);
    this->g_characterSize = jsonObject.value<CharacterSize>("characterSize", 30);
    this->g_letterSpacingFactor = jsonObject.value<float>("letterSpacing", 1.0f);
    this->g_lineSpacingFactor = jsonObject.value<float>("lineSpacing", 1.0f);
    this->g_style = jsonObject.value<std::underlying_type<Style>::type>("style", Regular);
    this->g_fillColor = fge::Color{jsonObject.value<uint32_t>("fillColor", fge::Color::White.toInteger())};
    this->g_outlineColor = fge::Color{jsonObject.value<uint32_t>("outlineColor", fge::Color::Black.toInteger())};
    this->g_outlineThickness = jsonObject.value<float>("outlineThickness", 0.0f);

    this->g_geometryNeedUpdate = true;
}

void ObjText::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_string;
    pck << this->g_font;
    pck << this->g_characterSize;
    pck << this->g_letterSpacingFactor << this->g_lineSpacingFactor;
    pck << this->g_style;
    pck << this->g_fillColor << this->g_outlineColor;
    pck << this->g_outlineThickness;
}
void ObjText::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_string;
    pck >> this->g_font;
    pck >> this->g_characterSize;
    pck >> this->g_letterSpacingFactor >> this->g_lineSpacingFactor;
    pck >> this->g_style;
    pck >> this->g_fillColor >> this->g_outlineColor;
    pck >> this->g_outlineThickness;

    this->g_geometryNeedUpdate = true;
}

const char* ObjText::getClassName() const
{
    return FGE_OBJTEXT_CLASSNAME;
}
const char* ObjText::getReadableClassName() const
{
    return "text";
}

fge::RectFloat ObjText::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjText::getLocalBounds() const
{
    this->ensureGeometryUpdate();
    return this->g_bounds;
}

void ObjText::ensureGeometryUpdate() const
{
    if (!this->g_font.valid())
    {
        return;
    }

    const auto* font = static_cast<const sf::Font*>(this->g_font);

    // Do nothing, if geometry has not changed and the font texture has not changed
    if (!this->g_geometryNeedUpdate &&
        this->g_font.getData()->_font->getTexture(this->g_characterSize).m_cacheId == this->g_fontTextureId)
    {
        return;
    }

    // Save the current fonts texture id
    this->g_fontTextureId = font->getTexture(this->g_characterSize).m_cacheId;

    // Mark geometry as updated
    this->g_geometryNeedUpdate = false;

    // Clear the previous geometry
    this->g_characters.clear();
    this->g_bounds = fge::RectFloat();

    // No text: nothing to draw
    if (this->g_string.empty())
    {
        return;
    }

    // Compute values related to the text style
    bool isBold = this->g_style & Bold;
    bool isUnderlined = this->g_style & Underlined;
    bool isStrikeThrough = this->g_style & StrikeThrough;
    float italicShear = (this->g_style & Italic) ? 0.209f : 0.f; // 12 degrees in radians
    float underlineOffset = font->getUnderlinePosition(this->g_characterSize);
    float underlineThickness = font->getUnderlineThickness(this->g_characterSize);

    // Compute the location of the strike through dynamically
    // We use the center point of the lowercase 'x' glyph as the reference
    // We reuse the underline thickness as the thickness of the strike through as well
    sf::FloatRect xBounds = font->getGlyph(U'x', this->g_characterSize, isBold).bounds;
    float strikeThroughOffset = xBounds.top + xBounds.height / 2.f;

    // Precompute the variables needed by the algorithm
    float whitespaceWidth = font->getGlyph(U' ', this->g_characterSize, isBold).advance;
    float letterSpacing = (whitespaceWidth / 3.f) * (this->g_letterSpacingFactor - 1.f);
    whitespaceWidth += letterSpacing;
    float lineSpacing = font->getLineSpacing(this->g_characterSize) * this->g_lineSpacingFactor;

    sf::Vector2f position{0.0f, 0.0f};

    // Create one quad for each character
    float minX = static_cast<float>(this->g_characterSize);
    float minY = static_cast<float>(this->g_characterSize);
    float maxX = 0.f;
    float maxY = 0.f;

    uint32_t prevChar = 0;
    for (auto it = this->g_string.begin(); it != this->g_string.end(); ++it)
    {
        uint32_t curChar = static_cast<uint32_t>(*it);

        // Skip the \r char to avoid weird graphical issues
        if (curChar == U'\r')
        {
            continue;
        }

        sf::Vector2f size{0.0f, static_cast<float>(this->g_characterSize)};

        Character& character = this->g_characters.emplace_back(this->g_fillColor, this->g_outlineColor);
        character.g_unicodeChar = curChar;

        // Apply the kerning offset
        position.x += font->getKerning(prevChar, curChar, this->g_characterSize, isBold);

        // If we're using the underlined style and there's a new line, draw a line
        if (isUnderlined && (curChar == U'\n' && prevChar != U'\n'))
        {
            character.addLine(false, size.x, size.y, underlineOffset, underlineThickness);

            if (this->g_outlineThickness != 0.0f)
            {
                character.addLine(true, size.x, size.y, underlineOffset, underlineThickness, this->g_outlineThickness);
            }
        }

        // If we're using the strike through style and there's a new line, draw a line across all characters
        if (isStrikeThrough && (curChar == U'\n' && prevChar != U'\n'))
        {
            character.addLine(false, size.x, size.y, strikeThroughOffset, underlineThickness);

            if (this->g_outlineThickness != 0)
            {
                character.addLine(true, size.x, size.y, strikeThroughOffset, underlineThickness,
                                  this->g_outlineThickness);
            }
        }

        prevChar = curChar;

        // Handle special characters
        if ((curChar == U' ') || (curChar == U'\n') || (curChar == U'\t'))
        {
            // Update the current bounds (min coordinates)
            minX = std::min(minX, size.x);
            minY = std::min(minY, size.y);

            switch (curChar)
            {
            case U' ':
                position.x += whitespaceWidth;
                size.x -= whitespaceWidth;
                break;
            case U'\t':
                position.x += whitespaceWidth * 4.0f;
                size.x -= whitespaceWidth * 4.0f;
                break;
            case U'\n':
                position.y += lineSpacing;
                position.x = 0.0f;
                break;
            }

            if ((curChar == U' ') || (curChar == U'\t'))
            {
                if (isUnderlined)
                {
                    character.addLine(false, size.x, size.y, underlineOffset, underlineThickness);

                    if (this->g_outlineThickness != 0.0f)
                    {
                        character.addLine(true, size.x, size.y, underlineOffset, underlineThickness,
                                          this->g_outlineThickness);
                    }
                }

                // If we're using the strike through style, add the last line across all characters
                if (isStrikeThrough)
                {
                    character.addLine(false, size.x, size.y, strikeThroughOffset, underlineThickness);

                    if (this->g_outlineThickness != 0.0f)
                    {
                        character.addLine(true, size.x, size.y, strikeThroughOffset, underlineThickness,
                                          this->g_outlineThickness);
                    }
                }
            }

            character.setPosition(position);

            // Update the current bounds (max coordinates)
            maxX = std::max(maxX, size.x + position.x);
            maxY = std::max(maxY, size.y + position.y);

            // Next glyph, no need to create a quad for whitespace
            continue;
        }

        // Apply the outline
        if (this->g_outlineThickness != 0.0f)
        {
            const sf::Glyph& glyph = font->getGlyph(curChar, this->g_characterSize, isBold, this->g_outlineThickness);

            // Add the outline glyph to the vertices
            character.addGlyphQuad(true, size, glyph, italicShear);
        }

        // Extract the current glyph's description
        const sf::Glyph& glyph = font->getGlyph(curChar, this->g_characterSize, isBold);

        // Add the glyph to the vertices
        character.addGlyphQuad(false, size, glyph, italicShear);
        character.setPosition(position);

        float characterLength = glyph.advance + letterSpacing;

        if (isUnderlined && (characterLength > 0.0f))
        {
            character.addLine(false, characterLength, size.y, underlineOffset, underlineThickness);

            if (this->g_outlineThickness != 0.0f)
            {
                character.addLine(true, characterLength, size.y, underlineOffset, underlineThickness,
                                  this->g_outlineThickness);
            }
        }

        // If we're using the strike through style, add the last line across all characters
        if (isStrikeThrough && (characterLength > 0.0f))
        {
            character.addLine(false, characterLength, size.y, strikeThroughOffset, underlineThickness);

            if (this->g_outlineThickness != 0.0f)
            {
                character.addLine(true, characterLength, size.y, strikeThroughOffset, underlineThickness,
                                  this->g_outlineThickness);
            }
        }

        // Update the current bounds
        float left = glyph.bounds.left;
        float top = glyph.bounds.top;
        float right = glyph.bounds.left + glyph.bounds.width;
        float bottom = glyph.bounds.top + glyph.bounds.height;

        minX = std::min(minX, size.x + position.x + left - italicShear * bottom);
        maxX = std::max(maxX, size.x + position.x + right - italicShear * top);
        minY = std::min(minY, size.y + position.y + top);
        maxY = std::max(maxY, size.y + position.y + bottom);

        // Advance to the next character
        position.x += characterLength;
    }

    // If we're using outline, update the current bounds
    if (this->g_outlineThickness != 0.0f)
    {
        float outline = std::abs(std::ceil(this->g_outlineThickness));
        minX -= outline;
        maxX += outline;
        minY -= outline;
        maxY += outline;
    }

    // Update the bounding rectangle
    this->g_bounds._x = minX;
    this->g_bounds._y = minY;
    this->g_bounds._width = maxX - minX;
    this->g_bounds._height = maxY - minY;
}

} // namespace fge

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

#include "FastEngine/C_objTextNew.hpp"
#include "FastEngine/font_manager.hpp"

namespace fge
{

void Character::clear()
{
    this->_character = 0;
    this->_vertices.clear();
    this->_outlineVertices.clear();
}
// Add an underline or strikethrough line to the vertex array
void Character::addLine(bool outlineVertices, float lineLength, float lineTop, const sf::Color& color, float offset, float thickness, float outlineThickness)
{
    sf::VertexArray* vertices = outlineVertices ? &this->_outlineVertices : &this->_vertices;

    float top = std::floor(lineTop + offset - (thickness / 2.0f) + 0.5f);
    float bottom = top + std::floor(thickness + 0.5f);

    vertices->append(sf::Vertex(sf::Vector2f(-outlineThickness,             top    - outlineThickness), color, sf::Vector2f(1, 1)));
    vertices->append(sf::Vertex(sf::Vector2f(lineLength + outlineThickness, top    - outlineThickness), color, sf::Vector2f(1, 1)));
    vertices->append(sf::Vertex(sf::Vector2f(-outlineThickness,             bottom + outlineThickness), color, sf::Vector2f(1, 1)));
    vertices->append(sf::Vertex(sf::Vector2f(-outlineThickness,             bottom + outlineThickness), color, sf::Vector2f(1, 1)));
    vertices->append(sf::Vertex(sf::Vector2f(lineLength + outlineThickness, top    - outlineThickness), color, sf::Vector2f(1, 1)));
    vertices->append(sf::Vertex(sf::Vector2f(lineLength + outlineThickness, bottom + outlineThickness), color, sf::Vector2f(1, 1)));
}

// Add a glyph quad to the vertex array
void Character::addGlyphQuad(bool outlineVertices, const sf::Vector2f& position, const sf::Color& color, const sf::Glyph& glyph, float italicShear)
{
    sf::VertexArray* vertices = outlineVertices ? &this->_outlineVertices : &this->_vertices;

    float padding = 1.0f;

    float left   = glyph.bounds.left - padding;
    float top    = glyph.bounds.top - padding;
    float right  = glyph.bounds.left + glyph.bounds.width + padding;
    float bottom = glyph.bounds.top  + glyph.bounds.height + padding;

    float u1 = static_cast<float>(glyph.textureRect.left) - padding;
    float v1 = static_cast<float>(glyph.textureRect.top) - padding;
    float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + padding;
    float v2 = static_cast<float>(glyph.textureRect.top  + glyph.textureRect.height) + padding;

    vertices->append(sf::Vertex(sf::Vector2f(position.x + left  - italicShear * top   , position.y + top),    color, sf::Vector2f(u1, v1)));
    vertices->append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * top   , position.y + top),    color, sf::Vector2f(u2, v1)));
    vertices->append(sf::Vertex(sf::Vector2f(position.x + left  - italicShear * bottom, position.y + bottom), color, sf::Vector2f(u1, v2)));
    vertices->append(sf::Vertex(sf::Vector2f(position.x + left  - italicShear * bottom, position.y + bottom), color, sf::Vector2f(u1, v2)));
    vertices->append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * top   , position.y + top),    color, sf::Vector2f(u2, v1)));
    vertices->append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * bottom, position.y + bottom), color, sf::Vector2f(u2, v2)));
}

ObjTextNew::ObjTextNew() = default;
ObjTextNew::ObjTextNew(const sf::String& string, const fge::Font& font, const sf::Vector2f& position, unsigned int characterSize) :
    g_font(font),
    g_string(string),
    g_characterSize(characterSize),
    g_geometryNeedUpdate(true)
{
    this->setPosition(position);
}

void ObjTextNew::setFont(const fge::Font& font)
{
    this->g_font = font;
    //this->g_text.setFont(this->g_font);
}
const fge::Font& ObjTextNew::getFont() const
{
    return this->g_font;
}

void ObjTextNew::setString(const sf::String& string)
{
    if (this->g_string != string)
    {
        this->g_string = string;
        this->g_geometryNeedUpdate = true;
    }
}

void ObjTextNew::setCharacterSize(unsigned int size)
{
    if (this->g_characterSize != size)
    {
        this->g_characterSize = size;
        this->g_geometryNeedUpdate = true;
    }
}

void ObjTextNew::setLineSpacing(float spacingFactor)
{
    if (this->g_lineSpacingFactor != spacingFactor)
    {
        this->g_lineSpacingFactor = spacingFactor;
        this->g_geometryNeedUpdate = true;
    }
}
void ObjTextNew::setLetterSpacing(float spacingFactor)
{
    if (this->g_letterSpacingFactor != spacingFactor)
    {
        this->g_letterSpacingFactor = spacingFactor;
        this->g_geometryNeedUpdate = true;
    }
}

void ObjTextNew::setStyle(uint32_t style)
{
    if (this->g_style != style)
    {
        this->g_style = style;
        this->g_geometryNeedUpdate = true;
    }
}

void ObjTextNew::setFillColor(const sf::Color& color)
{
    if (color != this->g_fillColor)
    {
        this->g_fillColor = color;

        // Change vertex colors directly, no need to update whole geometry
        // (if geometry is updated anyway, we can skip this step)
        if (!this->g_geometryNeedUpdate)
        {
            for (auto& character : this->g_characters)
            {
                for (std::size_t i = 0; i < character._vertices.getVertexCount(); ++i)
                {
                    character._vertices[i].color = this->g_fillColor;
                }
            }
        }
    }
}
void ObjTextNew::setOutlineColor(const sf::Color& color)
{
    if (color != this->g_outlineColor)
    {
        this->g_outlineColor = color;

        // Change vertex colors directly, no need to update whole geometry
        // (if geometry is updated anyway, we can skip this step)
        if (!this->g_geometryNeedUpdate)
        {
            for (auto& character : this->g_characters)
            {
                for (std::size_t i = 0; i < character._outlineVertices.getVertexCount(); ++i)
                {
                    character._outlineVertices[i].color = this->g_outlineColor;
                }
            }
        }
    }
}

void ObjTextNew::setOutlineThickness(float thickness)
{
    if (thickness != this->g_outlineThickness)
    {
        this->g_outlineThickness = thickness;
        this->g_geometryNeedUpdate = true;
    }
}

const sf::String& ObjTextNew::getString() const
{
    return this->g_string;
}

unsigned int ObjTextNew::getCharacterSize() const
{
    return this->g_characterSize;
}

float ObjTextNew::getLetterSpacing() const
{
    return this->g_letterSpacingFactor;
}
float ObjTextNew::getLineSpacing() const
{
    return this->g_lineSpacingFactor;
}

uint32_t ObjTextNew::getStyle() const
{
    return this->g_style;
}

const sf::Color& ObjTextNew::getFillColor() const
{
    return this->g_fillColor;
}
const sf::Color& ObjTextNew::getOutlineColor() const
{
    return this->g_outlineColor;
}

float ObjTextNew::getOutlineThickness() const
{
    return this->g_outlineThickness;
}

sf::Vector2f ObjTextNew::findCharacterPos(std::size_t index) const
{
    // Make sure that we have a valid font
    if (!this->g_font.valid())
    {
        return {};
    }

    // Adjust the index if it's out of range
    if (index > this->g_string.getSize())
    {
        index = this->g_string.getSize();
    }

    // Precompute the variables needed by the algorithm
    const auto* font = static_cast<const sf::Font*>(this->g_font);
    bool  isBold          = this->g_style & Bold;
    float whitespaceWidth = font->getGlyph(L' ', this->g_characterSize, isBold).advance;
    float letterSpacing   = ( whitespaceWidth / 3.f ) * ( this->g_letterSpacingFactor - 1.f );
    whitespaceWidth      += letterSpacing;
    float lineSpacing     = font->getLineSpacing(this->g_characterSize) * this->g_lineSpacingFactor;

    // Compute the position
    sf::Vector2f position;
    uint32_t prevChar = 0;
    for (std::size_t i = 0; i < index; ++i)
    {
        uint32_t curChar = this->g_string[i];

        // Apply the kerning offset
        position.x += font->getKerning(prevChar, curChar, this->g_characterSize, isBold);
        prevChar = curChar;

        // Handle special characters
        switch (curChar)
        {
        case ' ':  position.x += whitespaceWidth;             continue;
        case '\t': position.x += whitespaceWidth * 4;         continue;
        case '\n': position.y += lineSpacing; position.x = 0; continue;
        }

        // For regular characters, add the advance offset of the glyph
        position.x += font->getGlyph(curChar, this->g_characterSize, isBold).advance + letterSpacing;
    }

    // Transform the position to global coordinates
    position = getTransform().transformPoint(position);

    return position;
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjTextNew)
{
    if (this->g_font.valid())
    {
        this->ensureGeometryUpdate();

        states.transform *= getTransform();
        states.texture = &this->g_font.getData()->_font->getTexture(this->g_characterSize);

        for (const auto& character : this->g_characters)
        {
            sf::RenderStates statesCharacter{states};
            statesCharacter.transform *= character.getTransform();

            // Only draw the outline if there is something to draw
            if (this->g_outlineThickness != 0)
            {
                target.draw(character._outlineVertices, statesCharacter);
            }
            target.draw(character._vertices, statesCharacter);
        }
    }
}
#endif

void ObjTextNew::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    /*fge::Object::save(jsonObject, scene);

    jsonObject["font"] = this->g_font;

    std::basic_string<uint32_t> tmpString = this->g_text.getString().toUtf32();
    jsonObject["string"] = tmpString;

    jsonObject["characterSize"] = static_cast<uint16_t>(this->g_text.getCharacterSize());
    jsonObject["letterSpacing"] = this->g_text.getLetterSpacing();
    jsonObject["lineSpacing"] = this->g_text.getLineSpacing();
    jsonObject["style"] = static_cast<uint32_t>(this->g_text.getStyle());
    jsonObject["fillColor"] = static_cast<uint32_t>(this->g_text.getFillColor().toInteger());
    jsonObject["outlineColor"] = static_cast<uint32_t>(this->g_text.getOutlineColor().toInteger());
    jsonObject["outlineThickness"] = this->g_text.getOutlineThickness();*/
}
void ObjTextNew::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    /*fge::Object::load(jsonObject, scene);

    this->g_font = jsonObject.value<std::string>("font", FGE_FONT_BAD);

    std::basic_string<uint32_t> tmpString = jsonObject.value<std::basic_string<uint32_t> >("string", std::basic_string<uint32_t>());
    this->g_text.setString( sf::String(tmpString) );

    this->g_text.setCharacterSize( jsonObject.value<uint16_t>("characterSize", this->g_text.getCharacterSize()) );
    this->g_text.setLetterSpacing( jsonObject.value<float>("letterSpacing", this->g_text.getLetterSpacing()) );
    this->g_text.setLineSpacing( jsonObject.value<float>("lineSpacing", this->g_text.getLineSpacing()) );
    this->g_text.setStyle( jsonObject.value<uint32_t>("style", this->g_text.getStyle()) );
    this->g_text.setFillColor( sf::Color(jsonObject.value<uint32_t>("fillColor", 0)) );
    this->g_text.setOutlineColor( sf::Color(jsonObject.value<uint32_t>("outlineColor", 0)) );
    this->g_text.setOutlineThickness( jsonObject.value<float>("outlineThickness", 0.0f) );*/
}

void ObjTextNew::pack(fge::net::Packet& pck)
{
    /*fge::Object::pack(pck);

    pck << this->g_font;

    pck << this->g_text.getString();
    pck << static_cast<uint16_t>(this->g_text.getCharacterSize());
    pck << this->g_text.getLetterSpacing();
    pck << this->g_text.getLineSpacing();
    pck << static_cast<uint32_t>(this->g_text.getStyle());
    pck << this->g_text.getFillColor();
    pck << this->g_text.getOutlineColor();
    pck << this->g_text.getOutlineThickness();*/
}
void ObjTextNew::unpack(fge::net::Packet& pck)
{
    /*fge::Object::unpack(pck);

    sf::String tmpString;
    uint16_t tmpUint16=0;
    uint32_t tmpUint32=0;
    float tmpFloat=0;
    sf::Color tmpColor;

    pck >> this->g_font;

    pck >> tmpString;
    this->g_text.setString(tmpString);
    pck >> tmpUint16;
    this->g_text.setCharacterSize(tmpUint16);
    pck >> tmpFloat;
    this->g_text.setLetterSpacing(tmpFloat);
    pck >> tmpFloat;
    this->g_text.setLineSpacing(tmpFloat);
    pck >> tmpUint32;
    this->g_text.setStyle(tmpUint32);
    pck >> tmpColor;
    this->g_text.setFillColor(tmpColor);
    pck >> tmpColor;
    this->g_text.setOutlineColor(tmpColor);
    pck >> tmpFloat;
    this->g_text.setOutlineThickness(tmpFloat);*/
}

const char* ObjTextNew::getClassName() const
{
    return FGE_OBJTEXTNEW_CLASSNAME;
}
const char* ObjTextNew::getReadableClassName() const
{
    return "text";
}

sf::FloatRect ObjTextNew::getGlobalBounds() const
{
    return this->getTransform().transformRect(this->getLocalBounds());
}
sf::FloatRect ObjTextNew::getLocalBounds() const
{
    this->ensureGeometryUpdate();
    return this->g_bounds;
}

void ObjTextNew::ensureGeometryUpdate() const
{
    if (!this->g_font.valid())
    {
        return;
    }

    // Do nothing, if geometry has not changed and the font texture has not changed
    if (!this->g_geometryNeedUpdate /**&& this->g_font.getData()->_font->getTexture(this->g_characterSize).m_cacheId == m_fontTextureId**/)
    {
        return;
    }

    // Save the current fonts texture id
    ///m_fontTextureId = m_font->getTexture(this->g_characterSize).m_cacheId;

    // Mark geometry as updated
    this->g_geometryNeedUpdate = false;

    // Clear the previous geometry
    this->g_characters.clear();
    this->g_bounds = sf::FloatRect();

    // No text: nothing to draw
    if (this->g_string.isEmpty())
    {
        return;
    }

    // Compute values related to the text style
    bool  isBold             = this->g_style & Bold;
    bool  isUnderlined       = this->g_style & Underlined;
    bool  isStrikeThrough    = this->g_style & StrikeThrough;
    float italicShear        = (this->g_style & Italic) ? 0.209f : 0.f; // 12 degrees in radians
    const auto* font = static_cast<const sf::Font*>(this->g_font);
    float underlineOffset    = font->getUnderlinePosition(this->g_characterSize);
    float underlineThickness = font->getUnderlineThickness(this->g_characterSize);

    // Compute the location of the strike through dynamically
    // We use the center point of the lowercase 'x' glyph as the reference
    // We reuse the underline thickness as the thickness of the strike through as well
    sf::FloatRect xBounds = font->getGlyph(L'x', this->g_characterSize, isBold).bounds;
    float strikeThroughOffset = xBounds.top + xBounds.height / 2.f;

    // Precompute the variables needed by the algorithm
    float whitespaceWidth = font->getGlyph(L' ', this->g_characterSize, isBold).advance;
    float letterSpacing   = ( whitespaceWidth / 3.f ) * ( this->g_letterSpacingFactor - 1.f );
    whitespaceWidth      += letterSpacing;
    float lineSpacing     = font->getLineSpacing(this->g_characterSize) * this->g_lineSpacingFactor;
    ///float x               = 0.f;
    ///float y               = static_cast<float>(this->g_characterSize);
    sf::Vector2f position{0.0f, 0.0f};

    // Create one quad for each character
    float minX = static_cast<float>(this->g_characterSize);
    float minY = static_cast<float>(this->g_characterSize);
    float maxX = 0.f;
    float maxY = 0.f;
    uint32_t prevChar = 0;
    for (std::size_t i = 0; i < this->g_string.getSize(); ++i)
    {
        uint32_t curChar = this->g_string[i];

        // Skip the \r char to avoid weird graphical issues
        if (curChar == L'\r')
        {
            continue;
        }

        sf::Vector2f size{0.0f, static_cast<float>(this->g_characterSize)};

        Character& character = this->g_characters.emplace_back();

        // Apply the kerning offset
        position.x += font->getKerning(prevChar, curChar, this->g_characterSize, isBold);

        // If we're using the underlined style and there's a new line, draw a line
        if (isUnderlined && (curChar == L'\n' && prevChar != L'\n'))
        {
            character.addLine(false, size.x, size.y, this->g_fillColor, underlineOffset, underlineThickness);

            if (this->g_outlineThickness != 0.0f)
            {
                character.addLine(true, size.x, size.y, this->g_outlineColor, underlineOffset, underlineThickness, this->g_outlineThickness);
            }
        }

        // If we're using the strike through style and there's a new line, draw a line across all characters
        if (isStrikeThrough && (curChar == L'\n' && prevChar != L'\n'))
        {
            character.addLine(false, size.x, size.y, this->g_fillColor, strikeThroughOffset, underlineThickness);

            if (this->g_outlineThickness != 0)
            {
                character.addLine(true, size.x, size.y, this->g_outlineColor, strikeThroughOffset, underlineThickness, this->g_outlineThickness);
            }
        }

        prevChar = curChar;

        // Handle special characters
        if ((curChar == L' ') || (curChar == L'\n') || (curChar == L'\t'))
        {
            // Update the current bounds (min coordinates)
            minX = std::min(minX, size.x);
            minY = std::min(minY, size.y);

            switch (curChar)
            {
            case L' ':  size.x += whitespaceWidth;     break;
            case L'\t': size.x += whitespaceWidth * 4; break;
            case L'\n': position.y += lineSpacing; position.x = 0;  break;
            }

            // Update the current bounds (max coordinates)
            maxX = std::max(maxX, size.x+position.x);
            maxY = std::max(maxY, size.y+position.y);

            // Next glyph, no need to create a quad for whitespace
            continue;
        }

        // Apply the outline
        if (this->g_outlineThickness != 0.0f)
        {
            const sf::Glyph& glyph = font->getGlyph(curChar, this->g_characterSize, isBold, this->g_outlineThickness);

            float left   = glyph.bounds.left;
            float top    = glyph.bounds.top;
            float right  = glyph.bounds.left + glyph.bounds.width;
            float bottom = glyph.bounds.top  + glyph.bounds.height;

            // Add the outline glyph to the vertices
            character.addGlyphQuad(true, size, this->g_outlineColor, glyph, italicShear);

            // Update the current bounds with the outlined glyph bounds
            minX = std::min(minX, size.x + left   - italicShear * bottom);
            maxX = std::max(maxX, size.x + right  - italicShear * top);
            minY = std::min(minY, size.y + top);
            maxY = std::max(maxY, size.y + bottom);
        }

        // Extract the current glyph's description
        const sf::Glyph& glyph = font->getGlyph(curChar, this->g_characterSize, isBold);

        // Add the glyph to the vertices
        character.addGlyphQuad(false, size, this->g_fillColor, glyph, italicShear);
        character.setPosition(position);

        // Update the current bounds with the non outlined glyph bounds
        if (this->g_outlineThickness == 0.0f)
        {
            float left   = glyph.bounds.left;
            float top    = glyph.bounds.top;
            float right  = glyph.bounds.left + glyph.bounds.width;
            float bottom = glyph.bounds.top  + glyph.bounds.height;

            minX = std::min(minX, size.x + left  - italicShear * bottom);
            maxX = std::max(maxX, size.x + right - italicShear * top);
            minY = std::min(minY, size.y + top);
            maxY = std::max(maxY, size.y + bottom);
        }

        // Advance to the next character
        position.x += glyph.advance + letterSpacing;
    }

    // If we're using the underlined style, add the last line
    /**if (isUnderlined && (x > 0))
    {
        character.addLine(m_vertices, x, y, m_fillColor, underlineOffset, underlineThickness);

        if (m_outlineThickness != 0)
            addLine(m_outlineVertices, x, y, m_outlineColor, underlineOffset, underlineThickness, m_outlineThickness);
    }

    // If we're using the strike through style, add the last line across all characters
    if (isStrikeThrough && (x > 0))
    {
        addLine(m_vertices, x, y, m_fillColor, strikeThroughOffset, underlineThickness);

        if (m_outlineThickness != 0)
            addLine(m_outlineVertices, x, y, m_outlineColor, strikeThroughOffset, underlineThickness, m_outlineThickness);
    }**/

    // Update the bounding rectangle
    this->g_bounds.left = minX;
    this->g_bounds.top = minY;
    this->g_bounds.width = maxX - minX;
    this->g_bounds.height = maxY - minY;
}

}//end fge

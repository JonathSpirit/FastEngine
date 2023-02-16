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

#ifndef _FGE_GRAPHIC_C_FONT_HPP_INCLUDED
#define _FGE_GRAPHIC_C_FONT_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include <FastEngine/fastengine_extern.hpp>
#include "FastEngine/C_rect.hpp"
#include <FastEngine/graphic/C_glyph.hpp>
#include <FastEngine/graphic/C_surface.hpp>
#include <FastEngine/vulkan/C_textureImage.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace fge
{

using CharacterSize = uint16_t;

class FGE_API FreeTypeFont
{
public:
    struct Info
    {
        std::string family;
    };

    FreeTypeFont();
    FreeTypeFont(const FreeTypeFont& r) = delete;
    ~FreeTypeFont();

    FreeTypeFont& operator=(const FreeTypeFont& r) = delete;

    bool loadFromFile(const std::filesystem::path& filePath);
    bool loadFromMemory(const void* data, std::size_t size);

    const Info& getInfo() const;

    const Glyph&
    getGlyph(uint32_t codePoint, fge::CharacterSize characterSize, bool bold, float outlineThickness = 0) const;
    bool hasGlyph(uint32_t codePoint) const;

    float getKerning(uint32_t first, uint32_t second, fge::CharacterSize characterSize, bool bold = false) const;
    float getLineSpacing(fge::CharacterSize characterSize) const;
    float getUnderlinePosition(fge::CharacterSize characterSize) const;
    float getUnderlineThickness(fge::CharacterSize characterSize) const;

    const fge::vulkan::TextureImage& getTexture(fge::CharacterSize characterSize) const;

    void setSmooth(bool smooth);
    bool isSmooth() const;

    [[nodiscard]] std::vector<long> getAvailableSize() const;

private:
    struct Row
    {
        Row(unsigned int rowTop, unsigned int rowHeight) :
                _width(0),
                _top(rowTop),
                _height(rowHeight)
        {}

        unsigned int _width;  //!< Current width of the row
        unsigned int _top;    //!< Y position of the row into the texture
        unsigned int _height; //!< Height of the row
    };

    using GlyphTable = std::unordered_map<uint64_t, Glyph>; //!< Table mapping a codepoint to its glyph

    struct Page
    {
        explicit Page(bool smooth);

        GlyphTable _glyphs;                 //!< Table mapping code points to their corresponding glyph
        fge::vulkan::TextureImage _texture; //!< Texture containing the pixels of the glyphs
        unsigned int _nextRow;              //!< Y position of the next new row in the texture
        std::vector<Row> _rows;             //!< List containing the position of all the existing rows
    };

    using PageTable =
            std::unordered_map<fge::CharacterSize, Page>; //!< Table mapping a character size to its page (texture)

    void cleanup();

    Page& loadPage(fge::CharacterSize characterSize) const;
    Glyph loadGlyph(uint32_t codePoint, fge::CharacterSize characterSize, bool bold, float outlineThickness) const;

    fge::RectInt findGlyphRect(Page& page, unsigned int width, unsigned int height) const;

    bool setCurrentSize(fge::CharacterSize characterSize) const;

    void* g_face;      //!< Pointer to the internal font face (it is typeless to avoid exposing implementation details)
    void* g_streamRec; //!< Pointer to the stream rec instance (it is typeless to avoid exposing implementation details)
    void* g_stroker;   //!< Pointer to the stroker (it is typeless to avoid exposing implementation details)
    bool g_isSmooth;   //!< Status of the smooth filter
    Info g_info;       //!< Information about the font
    mutable PageTable g_pages;            //!< Table containing the glyphs pages by character size
    mutable fge::Surface g_surfaceBuffer; //!< Surface holding a glyph's pixels before being written to the texture
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_FONT_HPP_INCLUDED
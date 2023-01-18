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
#include <FastEngine/vulkan/C_textureImage.hpp>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace fge
{

class FGE_API FreeTypeFont
{
public:
    struct Info
    {
        std::string family; //!< The font family
    };

    FreeTypeFont();
    FreeTypeFont(const FreeTypeFont& r) = delete;
    ~FreeTypeFont();

    FreeTypeFont& operator =(const FreeTypeFont& r) = delete;

    bool loadFromFile(const std::filesystem::path& filePath);
    bool loadFromMemory(const void* data, std::size_t size);

    const Info& getInfo() const;

    const Glyph& getGlyph(Uint32 codePoint, unsigned int characterSize, bool bold, float outlineThickness = 0) const;
    bool hasGlyph(Uint32 codePoint) const;

    float getKerning(Uint32 first, Uint32 second, unsigned int characterSize, bool bold = false) const;
    float getLineSpacing(unsigned int characterSize) const;
    float getUnderlinePosition(unsigned int characterSize) const;
    float getUnderlineThickness(unsigned int characterSize) const;

    const fge::vulkan::TextureImage& getTexture(unsigned int characterSize) const;

    void setSmooth(bool smooth);
    bool isSmooth() const;

private:
    struct Row
    {
        Row(unsigned int rowTop, unsigned int rowHeight) : _width(0), _top(rowTop), _height(rowHeight) {}

        unsigned int _width;  //!< Current width of the row
        unsigned int _top;    //!< Y position of the row into the texture
        unsigned int _height; //!< Height of the row
    };

    using GlyphTable = std::map<Uint64, Glyph>; //!< Table mapping a codepoint to its glyph

    struct Page
    {
        explicit Page(bool smooth);

        GlyphTable       _glyphs;  //!< Table mapping code points to their corresponding glyph
        fge::vulkan::TextureImage          _texture; //!< Texture containing the pixels of the glyphs
        unsigned int     _nextRow; //!< Y position of the next new row in the texture
        std::vector<Row> _rows;    //!< List containing the position of all the existing rows
    };

    using PageTable = std::map<unsigned int, Page>; //!< Table mapping a character size to its page (texture)

    void cleanup();

    ////////////////////////////////////////////////////////////
    /// \brief Find or create the glyphs page corresponding to the given character size
    ///
    /// \param characterSize Reference character size
    ///
    /// \return The glyphs page corresponding to \a characterSize
    ///
    ////////////////////////////////////////////////////////////
    Page& loadPage(unsigned int characterSize) const;

    ////////////////////////////////////////////////////////////
    /// \brief Load a new glyph and store it in the cache
    ///
    /// \param codePoint        Unicode code point of the character to load
    /// \param characterSize    Reference character size
    /// \param bold             Retrieve the bold version or the regular one?
    /// \param outlineThickness Thickness of outline (when != 0 the glyph will not be filled)
    ///
    /// \return The glyph corresponding to \a codePoint and \a characterSize
    ///
    ////////////////////////////////////////////////////////////
    Glyph loadGlyph(Uint32 codePoint, unsigned int characterSize, bool bold, float outlineThickness) const;

    ////////////////////////////////////////////////////////////
    /// \brief Find a suitable rectangle within the texture for a glyph
    ///
    /// \param page   Page of glyphs to search in
    /// \param width  Width of the rectangle
    /// \param height Height of the rectangle
    ///
    /// \return Found rectangle within the texture
    ///
    ////////////////////////////////////////////////////////////
    fge::RectInt findGlyphRect(Page& page, unsigned int width, unsigned int height) const;

    ////////////////////////////////////////////////////////////
    /// \brief Make sure that the given size is the current one
    ///
    /// \param characterSize Reference character size
    ///
    /// \return True on success, false if any error happened
    ///
    ////////////////////////////////////////////////////////////
    bool setCurrentSize(unsigned int characterSize) const;


    void*                      g_library;     //!< Pointer to the internal library interface (it is typeless to avoid exposing implementation details)
    void*                      g_face;        //!< Pointer to the internal font face (it is typeless to avoid exposing implementation details)
    void*                      g_streamRec;   //!< Pointer to the stream rec instance (it is typeless to avoid exposing implementation details)
    void*                      g_stroker;     //!< Pointer to the stroker (it is typeless to avoid exposing implementation details)
    bool                       g_isSmooth;    //!< Status of the smooth filter
    Info                       g_info;        //!< Information about the font
    mutable PageTable          g_pages;       //!< Table containing the glyphs pages by character size
    mutable std::vector<uint8_t> g_pixelBuffer; //!< Pixel buffer holding a glyph's pixels before being written to the texture
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_FONT_HPP_INCLUDED
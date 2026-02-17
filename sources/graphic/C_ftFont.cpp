/*
 * Copyright 2026 Guillaume Guillet
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

#include "FastEngine/graphic/C_ftFont.hpp"
#include "FastEngine/manager/font_manager.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_STROKER_H
#include <cmath>

namespace fge
{

namespace
{

uint64_t BuildGlyphKey(float outlineThickness, bool bold, uint32_t index)
{
    static_assert(sizeof(float) == sizeof(uint32_t));

    union
    {
        float _in;
        uint32_t _out;
    } data{._in = outlineThickness};

    return (static_cast<uint64_t>(data._out) << 32) | (static_cast<uint64_t>(bold) << 31) | index;
}

} //end namespace

FreeTypeFont::FreeTypeFont() :
        g_face(nullptr),
        g_streamRec(nullptr),
        g_stroker(nullptr),
        g_isSmooth(true),
        g_info()
{}
FreeTypeFont::~FreeTypeFont()
{
    this->cleanup();
}

bool FreeTypeFont::loadFromFile(std::filesystem::path const& filePath)
{
    // Cleanup the previous resources
    this->cleanup();

    FT_Library library = static_cast<FT_Library>(fge::font::GetFreetypeLibrary());

    // Load the new font face from the specified file
    FT_Face face;
    if (FT_New_Face(library, filePath.string().c_str(), 0, &face) != 0)
    {
        return false;
    }

    // Load the stroker that will be used to outline the font
    FT_Stroker stroker;
    if (FT_Stroker_New(library, &stroker) != 0)
    {
        FT_Done_Face(face);
        return false;
    }

    // Select the unicode character map
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0)
    {
        FT_Stroker_Done(stroker);
        FT_Done_Face(face);
        return false;
    }

    this->g_stroker = stroker;
    this->g_face = face;

    // Store the font information
    this->g_info.family = face->family_name != nullptr ? face->family_name : std::string{};

    return true;
}

bool FreeTypeFont::loadFromMemory(void const* data, std::size_t sizeInBytes)
{
    // Cleanup the previous resources
    this->cleanup();

    FT_Library library = static_cast<FT_Library>(fge::font::GetFreetypeLibrary());

    // Load the new font face from the specified file
    FT_Face face;
    if (FT_New_Memory_Face(library, reinterpret_cast<FT_Byte const*>(data), static_cast<FT_Long>(sizeInBytes), 0,
                           &face) != 0)
    {
        return false;
    }

    // Load the stroker that will be used to outline the font
    FT_Stroker stroker;
    if (FT_Stroker_New(library, &stroker) != 0)
    {
        FT_Done_Face(face);
        return false;
    }

    // Select the Unicode character map
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0)
    {
        FT_Stroker_Done(stroker);
        FT_Done_Face(face);
        return false;
    }

    this->g_stroker = stroker;
    this->g_face = face;

    // Store the font information
    this->g_info.family = face->family_name != nullptr ? face->family_name : std::string{};

    return true;
}

FreeTypeFont::Info const& FreeTypeFont::getInfo() const
{
    return g_info;
}

Glyph const&
FreeTypeFont::getGlyph(uint32_t codePoint, fge::CharacterSize characterSize, bool bold, float outlineThickness) const
{
    // Get the page corresponding to the character size
    GlyphTable& glyphs = this->loadPage(characterSize)._glyphs;

    // Build the key by combining the glyph index (based on code point), bold flag, and outline thickness
    auto key = BuildGlyphKey(outlineThickness, bold, FT_Get_Char_Index(static_cast<FT_Face>(g_face), codePoint));

    // Search the glyph into the cache
    auto it = glyphs.find(key);
    if (it != glyphs.end())
    {
        return it->second;
    }

    // Not found: we have to load it
    Glyph const glyph = this->loadGlyph(codePoint, characterSize, bold, outlineThickness);
    return glyphs.insert(std::make_pair(key, glyph)).first->second;
}
bool FreeTypeFont::hasGlyph(uint32_t codePoint) const
{
    return FT_Get_Char_Index(static_cast<FT_Face>(this->g_face), codePoint) != 0;
}

float FreeTypeFont::getKerning(uint32_t first, uint32_t second, fge::CharacterSize characterSize, bool bold) const
{
    // Special case where first or second is 0 (null character)
    if (first == 0 || second == 0)
    {
        return 0.0f;
    }

    FT_Face face = static_cast<FT_Face>(this->g_face);

    if (face != nullptr && this->setCurrentSize(characterSize))
    {
        // Convert the characters to indices
        FT_UInt const index1 = FT_Get_Char_Index(face, first);
        FT_UInt const index2 = FT_Get_Char_Index(face, second);

        // Retrieve position compensation deltas generated by FT_LOAD_FORCE_AUTOHINT flag
        float firstRsbDelta = static_cast<float>(getGlyph(first, characterSize, bold)._rsbDelta);
        float secondLsbDelta = static_cast<float>(getGlyph(second, characterSize, bold)._lsbDelta);

        // Get the kerning vector if present
        FT_Vector kerning;
        kerning.x = kerning.y = 0;
        if (FT_HAS_KERNING(face))
        {
            FT_Get_Kerning(face, index1, index2, FT_KERNING_UNFITTED, &kerning);
        }

        // X advance is already in pixels for bitmap fonts
        if (!FT_IS_SCALABLE(face))
        {
            return static_cast<float>(kerning.x);
        }

        // Combine kerning with compensation deltas and return the X advance
        // Flooring is required as we use FT_KERNING_UNFITTED flag which is not quantized in 64 based grid
        return std::floor((secondLsbDelta - firstRsbDelta + static_cast<float>(kerning.x) + 32) /
                          static_cast<float>(1 << 6));
    }

    // Invalid font
    return 0.0f;
}

float FreeTypeFont::getLineSpacing(fge::CharacterSize characterSize) const
{
    FT_Face face = static_cast<FT_Face>(this->g_face);

    if (face != nullptr && setCurrentSize(characterSize))
    {
        return static_cast<float>(face->size->metrics.height) / static_cast<float>(1 << 6);
    }
    return 0.0f;
}

float FreeTypeFont::getUnderlinePosition(fge::CharacterSize characterSize) const
{
    FT_Face face = static_cast<FT_Face>(this->g_face);

    if (face != nullptr && setCurrentSize(characterSize))
    {
        // Return a fixed position if font is a bitmap font
        if (!FT_IS_SCALABLE(face))
        {
            return static_cast<float>(characterSize) / 10.0f;
        }

        return -static_cast<float>(FT_MulFix(face->underline_position, face->size->metrics.y_scale)) /
               static_cast<float>(1 << 6);
    }
    return 0.0f;
}

float FreeTypeFont::getUnderlineThickness(fge::CharacterSize characterSize) const
{
    FT_Face face = static_cast<FT_Face>(g_face);

    if (face != nullptr && setCurrentSize(characterSize))
    {
        // Return a fixed thickness if font is a bitmap font
        if (!FT_IS_SCALABLE(face))
        {
            return static_cast<float>(characterSize) / 14.0f;
        }

        return static_cast<float>(FT_MulFix(face->underline_thickness, face->size->metrics.y_scale)) /
               static_cast<float>(1 << 6);
    }
    return 0.0f;
}

fge::vulkan::TextureImage const& FreeTypeFont::getTexture(fge::CharacterSize characterSize) const
{
    return this->loadPage(characterSize)._texture;
}

void FreeTypeFont::setSmooth(bool smooth)
{
    if (smooth != this->g_isSmooth)
    {
        this->g_isSmooth = smooth;

        for (auto& page: this->g_pages)
        {
            page.second._texture.setFilter(this->g_isSmooth ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
        }
    }
}

bool FreeTypeFont::isSmooth() const
{
    return this->g_isSmooth;
}

std::vector<long> FreeTypeFont::getAvailableSize() const
{
    FT_Face face = static_cast<FT_Face>(this->g_face);

    if (face == nullptr)
    {
        return {};
    }

    std::vector<long> result(face->num_fixed_sizes);

    // In the case of bitmap fonts, resizing can
    // fail if the requested size is not available
    if (!FT_IS_SCALABLE(face))
    {
        for (int i = 0; i < face->num_fixed_sizes; ++i)
        {
            result[i] = (face->available_sizes[i].y_ppem + 32) >> 6;
        }
    }
    return result;
}

void FreeTypeFont::cleanup()
{
    // Destroy the stroker
    if (this->g_stroker != nullptr)
    {
        FT_Stroker_Done(static_cast<FT_Stroker>(this->g_stroker));
    }

    // Destroy the font face
    if (this->g_face != nullptr)
    {
        FT_Done_Face(static_cast<FT_Face>(this->g_face));
    }

    // Destroy the stream rec instance, if any (must be done after FT_Done_Face!)
    if (this->g_streamRec != nullptr)
    {
        delete static_cast<FT_StreamRec*>(this->g_streamRec);
    }

    // Reset members
    this->g_face = nullptr;
    this->g_stroker = nullptr;
    this->g_streamRec = nullptr;
    this->g_pages.clear();
    this->g_surfaceBuffer.clear();
}

FreeTypeFont::Page& FreeTypeFont::loadPage(fge::CharacterSize characterSize) const
{
    return this->g_pages.try_emplace(characterSize, this->g_isSmooth).first->second;
}

Glyph FreeTypeFont::loadGlyph(uint32_t codePoint,
                              fge::CharacterSize characterSize,
                              bool bold,
                              float outlineThickness) const
{
    // The glyph to return
    Glyph glyph;

    // First, transform our ugly void* to a FT_Face
    FT_Face face = static_cast<FT_Face>(g_face);
    if (face == nullptr)
    {
        return glyph;
    }

    // Set the character size
    if (!this->setCurrentSize(characterSize))
    {
        return glyph;
    }

    // Load the glyph corresponding to the code point
    FT_Int32 flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT;
    if (outlineThickness != 0.0f)
    {
        flags |= FT_LOAD_NO_BITMAP;
    }
    if (FT_Load_Char(face, codePoint, flags) != 0)
    {
        return glyph;
    }

    // Retrieve the glyph
    FT_Glyph glyphDesc;
    if (FT_Get_Glyph(face->glyph, &glyphDesc) != 0)
    {
        return glyph;
    }

    // Apply bold and outline (there is no fallback for outline) if necessary -- first technique using outline (highest quality)
    FT_Pos const weight = 1 << 6;
    bool const outline = (glyphDesc->format == FT_GLYPH_FORMAT_OUTLINE);
    if (outline)
    {
        if (bold)
        {
            FT_OutlineGlyph outlineGlyph = reinterpret_cast<FT_OutlineGlyph>(glyphDesc);
            FT_Outline_Embolden(&outlineGlyph->outline, weight);
        }

        if (outlineThickness != 0.0f)
        {
            FT_Stroker stroker = static_cast<FT_Stroker>(g_stroker);

            FT_Stroker_Set(stroker, static_cast<FT_Fixed>(outlineThickness * static_cast<float>(1 << 6)),
                           FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
            FT_Glyph_Stroke(&glyphDesc, stroker, 1);
        }
    }

    // Convert the glyph to a bitmap (i.e. rasterize it)
    // Warning! After this line, do not read any data from glyphDesc directly, use
    // bitmapGlyph.root to access the FT_Glyph data.
    FT_Glyph_To_Bitmap(&glyphDesc, FT_RENDER_MODE_NORMAL, nullptr, 1);
    FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyphDesc);
    FT_Bitmap& bitmap = bitmapGlyph->bitmap;

    // Apply bold if necessary -- fallback technique using bitmap (lower quality)
    if (!outline)
    {
        if (bold)
        {
            FT_Bitmap_Embolden(static_cast<FT_Library>(fge::font::GetFreetypeLibrary()), &bitmap, weight, weight);
        }
    }

    // Compute the glyph's advance offset
    glyph._advance = static_cast<float>(bitmapGlyph->root.advance.x >> 16);
    if (bold)
    {
        glyph._advance += static_cast<float>(weight) / static_cast<float>(1 << 6);
    }

    glyph._lsbDelta = static_cast<int>(face->glyph->lsb_delta);
    glyph._rsbDelta = static_cast<int>(face->glyph->rsb_delta);

    unsigned int width = bitmap.width;
    unsigned int height = bitmap.rows;

    if ((width > 0) && (height > 0))
    {
        // Leave a small padding around characters, so that filtering doesn't
        // pollute them with pixels from neighbors
        unsigned int const padding = 2;

        width += 2 * padding;
        height += 2 * padding;

        // Get the glyphs page corresponding to the character size
        Page& page = this->loadPage(characterSize);

        // Find a good position for the new glyph into the texture
        glyph._textureRect = this->findGlyphRect(page, width, height);

        // Make sure the texture data is positioned in the center
        // of the allocated texture rectangle
        glyph._textureRect._x += static_cast<int>(padding);
        glyph._textureRect._y += static_cast<int>(padding);
        glyph._textureRect._width -= static_cast<int>(2 * padding);
        glyph._textureRect._height -= static_cast<int>(2 * padding);

        // Compute the glyph's bounding box
        glyph._bounds._x = static_cast<float>(bitmapGlyph->left);
        glyph._bounds._y = static_cast<float>(-bitmapGlyph->top);
        glyph._bounds._width = static_cast<float>(bitmap.width);
        glyph._bounds._height = static_cast<float>(bitmap.rows);

        // Resize the pixel buffer to the new size and fill it with transparent white pixels
        this->g_surfaceBuffer.create(width, height, fge::Color(255, 255, 255, 0));

        // Extract the glyph's pixels from the bitmap
        uint8_t const* pixels = bitmap.buffer;
        if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
        {
            // Pixels are 1 bit monochrome values
            for (unsigned int y = padding; y < height - padding; ++y)
            {
                for (unsigned int x = padding; x < width - padding; ++x)
                {
                    if ((pixels[(x - padding) / 8] & (1 << (7 - ((x - padding) % 8)))) > 0)
                    {
                        this->g_surfaceBuffer.setPixel(x, y, fge::Color::White);
                    }
                }
                pixels += bitmap.pitch;
            }
        }
        else
        {
            // Pixels are 8 bits gray levels
            for (unsigned int y = padding; y < height - padding; ++y)
            {
                for (unsigned int x = padding; x < width - padding; ++x)
                {
                    this->g_surfaceBuffer.setPixel(x, y, fge::Color(255, 255, 255, pixels[x - padding]));
                }
                pixels += bitmap.pitch;
            }
        }

        // Write the pixels to the texture
        unsigned int const x = static_cast<unsigned int>(glyph._textureRect._x) - padding;
        unsigned int const y = static_cast<unsigned int>(glyph._textureRect._y) - padding;
        page._texture.update(this->g_surfaceBuffer.get(), {x, y});
    }

    // Delete the FT glyph
    FT_Done_Glyph(glyphDesc);

    // Done :)
    return glyph;
}

fge::RectInt FreeTypeFont::findGlyphRect(Page& page, unsigned int width, unsigned int height) const
{
    // Find the line that fits well the glyph
    Row* row = nullptr;
    float bestRatio = 0;
    for (auto it = page._rows.begin(); it != page._rows.end(); ++it)
    {
        float const ratio = static_cast<float>(height) / static_cast<float>(it->_height);

        // Ignore rows that are either too small or too high
        if ((ratio < 0.7f) || (ratio > 1.f))
        {
            continue;
        }

        // Check if there's enough horizontal space left in the row
        if (width > page._texture.getSize().x - it->_width)
        {
            continue;
        }

        // Make sure that this new row is the best found so far
        if (ratio < bestRatio)
        {
            continue;
        }

        // The current row passed all the tests: we can select it
        row = &(*it);
        bestRatio = ratio;
    }

    // If we didn't find a matching row, create a new one (10% taller than the glyph)
    if (row == nullptr)
    {
        unsigned int const rowHeight = height + height / 10;
        while ((page._nextRow + rowHeight >= static_cast<unsigned int>(page._texture.getSize().y)) ||
               (width >= static_cast<unsigned int>(page._texture.getSize().x)))
        {
            // Not enough space: resize the texture if possible
            unsigned int const textureWidth = page._texture.getSize().x;
            unsigned int const textureHeight = page._texture.getSize().y;

            auto maxImageDimension = page._texture.getContext().getPhysicalDevice().getMaxImageDimension2D();

            if ((textureWidth * 2 <= maxImageDimension) && (textureHeight * 2 <= maxImageDimension))
            {
                // Make the texture 2 times bigger
                fge::vulkan::TextureImage newTexture{page._texture.getContext()};
                newTexture.create({textureWidth * 2, textureHeight * 2});
                newTexture.setFilter(g_isSmooth ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
                newTexture.update(page._texture, {0, 0});
                page._texture = std::move(newTexture);
            }
            else
            {
                // Oops, we've reached the maximum texture size...
                return {{0, 0}, {2, 2}};
            }
        }

        // We can now create the new row
        page._rows.emplace_back(page._nextRow, rowHeight);
        page._nextRow += rowHeight;
        row = &page._rows.back();
    }

    // Find the glyph's rectangle on the selected row
    fge::RectInt rect(fge::Rect<unsigned int>({row->_width, row->_top}, {width, height}));

    // Update the row information
    row->_width += width;

    return rect;
}

bool FreeTypeFont::setCurrentSize(fge::CharacterSize characterSize) const
{
    // FT_Set_Pixel_Sizes is an expensive function, so we must call it
    // only when necessary to avoid killing performances

    FT_Face face = static_cast<FT_Face>(this->g_face);
    FT_UShort const currentSize = face->size->metrics.x_ppem;

    if (currentSize != characterSize)
    {
        FT_Error result = FT_Set_Pixel_Sizes(face, 0, characterSize);
        return result == FT_Err_Ok;
    }

    return true;
}

FreeTypeFont::Page::Page(bool smooth) :
        _texture(fge::vulkan::GetActiveContext()),
        _nextRow(3)
{
    // Make sure that the texture is initialized by default
    fge::Surface surface;
    surface.create(128, 128, fge::Color(255, 255, 255, 0));

    // Reserve a 2x2 white square for texturing underlines
    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            surface.setPixel(x, y, fge::Color(255, 255, 255, 255));
        }
    }

    // Create the texture
    this->_texture.create(surface.get());
    this->_texture.setFilter(smooth ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
}

} // namespace fge

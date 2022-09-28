////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2022 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

//modified by Guillaume Guillet for FastEngine server compatibility

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <cassert>

#define HACK_IMG_PTR(x_) reinterpret_cast<sf::Image*>(x_);
#define HACK_IMG_CONST_PTR(x_) reinterpret_cast<const sf::Image*>(x_);

namespace sf
{

////////////////////////////////////////////////////////////
Texture::Texture() :
m_size         (0, 0),
m_actualSize   (0, 0),
m_texture      (0),
m_isSmooth     (false),
m_sRgb         (false),
m_isRepeated   (false),
m_pixelsFlipped(false),
m_fboAttachment(false),
m_hasMipmap    (false),
m_cacheId      (0)
{
}


////////////////////////////////////////////////////////////
Texture::Texture(const Texture& copy) :
m_size         (0, 0),
m_actualSize   (0, 0),
m_texture      (0),
m_isSmooth     (copy.m_isSmooth),
m_sRgb         (copy.m_sRgb),
m_isRepeated   (copy.m_isRepeated),
m_pixelsFlipped(false),
m_fboAttachment(false),
m_hasMipmap    (false),
m_cacheId      (0)
{
}


////////////////////////////////////////////////////////////
Texture::~Texture()
{
    auto* ptr = HACK_IMG_PTR(this->m_cacheId);
    if (ptr != nullptr)
    {
        delete ptr;
    }
}

////////////////////////////////////////////////////////////
bool Texture::create(unsigned int width, unsigned int height)
{
    auto* ptr = HACK_IMG_PTR(this->m_cacheId);
    if (ptr == nullptr)
    {
        ptr = new sf::Image{};
    }

    ptr->create(width, height);
    this->m_size = ptr->getSize();
    return true;
}


////////////////////////////////////////////////////////////
bool Texture::loadFromFile(const std::string& filename, [[maybe_unused]] const IntRect& area)
{
    auto* ptr = HACK_IMG_PTR(this->m_cacheId);
    if (ptr == nullptr)
    {
        ptr = new sf::Image{};
    }
    bool result = ptr->loadFromFile(filename);
    this->m_size = ptr->getSize();
    return result;
}


////////////////////////////////////////////////////////////
bool Texture::loadFromMemory(const void* data, std::size_t size, [[maybe_unused]] const IntRect& area)
{
    auto* ptr = HACK_IMG_PTR(this->m_cacheId);
    if (ptr == nullptr)
    {
        ptr = new sf::Image{};
    }
    bool result = ptr->loadFromMemory(data, size);
    this->m_size = ptr->getSize();
    return result;
}


////////////////////////////////////////////////////////////
bool Texture::loadFromStream(InputStream& stream, [[maybe_unused]] const IntRect& area)
{
    auto* ptr = HACK_IMG_PTR(this->m_cacheId);
    if (ptr == nullptr)
    {
        ptr = new sf::Image{};
    }
    bool result = ptr->loadFromStream(stream);
    this->m_size = ptr->getSize();
    return result;
}


////////////////////////////////////////////////////////////
bool Texture::loadFromImage(const Image& image, [[maybe_unused]] const IntRect& area)
{
    auto* ptr = HACK_IMG_PTR(this->m_cacheId);
    if (ptr == nullptr)
    {
        ptr = new sf::Image{image};
    }
    else
    {
        *ptr = image;
    }
    this->m_size = ptr->getSize();
    return true;
}


////////////////////////////////////////////////////////////
Vector2u Texture::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
Image Texture::copyToImage() const
{
    const auto* ptr = HACK_IMG_CONST_PTR(this->m_cacheId);
    if (ptr == nullptr)
    {
        return {};
    }
    return *ptr;
}


////////////////////////////////////////////////////////////
void Texture::update(const Uint8* pixels)
{
    // Update the whole texture
    update(pixels, m_size.x, m_size.y, 0, 0);
}


////////////////////////////////////////////////////////////
void Texture::update(const Uint8* pixels, unsigned int width, unsigned int height, unsigned int x, unsigned int y)
{
    assert(x + width <= m_size.x);
    assert(y + height <= m_size.y);

    auto* ptr = HACK_IMG_PTR(this->m_cacheId);
    if (ptr == nullptr)
    {
        return;
    }

    const auto* colors = reinterpret_cast<const uint32_t*>(pixels);

    for (unsigned int ix=x; ix<width; ++ix)
    {
        for (unsigned int iy=y; iy<height; ++iy)
        {
            ptr->setPixel(ix, iy, sf::Color{*(colors++)});
        }
    }
}


////////////////////////////////////////////////////////////
void Texture::update(const Texture& texture)
{
    update(texture, 0, 0);
}


////////////////////////////////////////////////////////////
void Texture::update(const Texture& texture, [[maybe_unused]] unsigned int x, [[maybe_unused]] unsigned int y)
{
    const auto* ptr = HACK_IMG_CONST_PTR(texture.m_cacheId);
    if (ptr != nullptr)
    {
        this->loadFromImage(*ptr);
    }
}


////////////////////////////////////////////////////////////
void Texture::update(const Image& image)
{
    update(image, 0, 0);
}


////////////////////////////////////////////////////////////
void Texture::update(const Image& image, [[maybe_unused]] unsigned int x, [[maybe_unused]] unsigned int y)
{
    this->loadFromImage(image);
}


////////////////////////////////////////////////////////////
void Texture::update(const Window& window)
{
}


////////////////////////////////////////////////////////////
void Texture::update(const Window& window, unsigned int x, unsigned int y)
{
}


////////////////////////////////////////////////////////////
void Texture::setSmooth(bool smooth)
{
    m_isSmooth = smooth;
}


////////////////////////////////////////////////////////////
bool Texture::isSmooth() const
{
    return m_isSmooth;
}


////////////////////////////////////////////////////////////
void Texture::setSrgb(bool sRgb)
{
    m_sRgb = sRgb;
}


////////////////////////////////////////////////////////////
bool Texture::isSrgb() const
{
    return m_sRgb;
}


////////////////////////////////////////////////////////////
void Texture::setRepeated(bool repeated)
{
    m_isRepeated = repeated;
}


////////////////////////////////////////////////////////////
bool Texture::isRepeated() const
{
    return m_isRepeated;
}


////////////////////////////////////////////////////////////
bool Texture::generateMipmap()
{
    return false;
}


////////////////////////////////////////////////////////////
void Texture::invalidateMipmap()
{
    m_hasMipmap = false;
}


////////////////////////////////////////////////////////////
void Texture::bind(const Texture* texture, CoordinateType coordinateType)
{
}


////////////////////////////////////////////////////////////
unsigned int Texture::getMaximumSize()
{
    return 16384;
}


////////////////////////////////////////////////////////////
Texture& Texture::operator =(const Texture& right)
{
    const auto* ptr = HACK_IMG_CONST_PTR(right.m_cacheId);
    auto* thisPtr = HACK_IMG_PTR(this->m_cacheId);

    if (ptr == nullptr)
    {
        if (thisPtr != nullptr)
        {
            delete thisPtr;
            thisPtr = nullptr;
        }
    }
    else
    {
        if (thisPtr != nullptr)
        {
            *thisPtr = *ptr;
        }
        else
        {
            thisPtr = new sf::Image{*ptr};
        }
    }

    m_size = right.m_size;
    m_actualSize = right.m_actualSize;
    m_texture = right.m_texture;
    m_isSmooth = right.m_isSmooth;
    m_sRgb = right.m_sRgb;
    m_isRepeated = right.m_isRepeated;
    m_pixelsFlipped = right.m_pixelsFlipped;
    m_fboAttachment = right.m_fboAttachment;
    m_hasMipmap = right.m_hasMipmap;

    return *this;
}


////////////////////////////////////////////////////////////
void Texture::swap(Texture& right)
{
    std::swap(m_size,          right.m_size);
    std::swap(m_actualSize,    right.m_actualSize);
    std::swap(m_texture,       right.m_texture);
    std::swap(m_isSmooth,      right.m_isSmooth);
    std::swap(m_sRgb,          right.m_sRgb);
    std::swap(m_isRepeated,    right.m_isRepeated);
    std::swap(m_pixelsFlipped, right.m_pixelsFlipped);
    std::swap(m_fboAttachment, right.m_fboAttachment);
    std::swap(m_hasMipmap,     right.m_hasMipmap);

    auto cacheCpy = this->m_cacheId;

    this->m_cacheId = right.m_cacheId;
    right.m_cacheId = cacheCpy;
}


////////////////////////////////////////////////////////////
unsigned int Texture::getNativeHandle() const
{
    return m_texture;
}


////////////////////////////////////////////////////////////
unsigned int Texture::getValidSize(unsigned int size)
{
    return size;
}

} // namespace sf

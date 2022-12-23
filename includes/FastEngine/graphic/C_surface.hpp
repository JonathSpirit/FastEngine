#ifndef _FGE_C_SURFACE_HPP_INCLUDED
#define _FGE_C_SURFACE_HPP_INCLUDED

#include <SDL_render.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <cstdint>
#include <optional>
#include <FastEngine/graphic/C_color.hpp>

namespace fge
{

class Surface
{
public:
    Surface();
    Surface(const Surface& r);
    Surface(Surface&& r) noexcept;
    explicit Surface(SDL_Surface* newSurface);
    ~Surface();

    Surface& operator=(const Surface& r);
    Surface& operator=(Surface&& r) noexcept;

    void clear();

    bool create(int width, int height, const fge::Color& color={0, 0, 0, 255});
    bool loadFromFile(const std::filesystem::path& filePath);
    bool loadFromMemory(const void *data, std::size_t size);

    bool saveToFile(const std::filesystem::path& filePath) const;

    [[nodiscard]] glm::vec<2, int> getSize() const;

    void createMaskFromColor(const fge::Color& color, uint8_t alpha=0);

    bool setPixel (int x, int y, const fge::Color& color);
    [[nodiscard]] std::optional<SDL_Color> getPixel (int x, int y) const;

    void flipHorizontally();
    void flipVertically();

    bool blitSurface(const Surface& src, const std::optional<SDL_Rect>& srcRect, std::optional<SDL_Rect>& dstRect);

    bool fillRect(const std::optional<SDL_Rect>& rect, const fge::Color& color);

    bool addBorder(int borderSize, const fge::Color& color);

    void set(SDL_Surface* surface);
    [[nodiscard]] SDL_Surface* get() const;

private:
    SDL_Surface* g_surface;
};

}//end fge

#endif //_FGE_C_SURFACE_HPP_INCLUDED

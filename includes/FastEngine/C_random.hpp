#ifndef _FGE_C_RANDOM_HPP_INCLUDED
#define _FGE_C_RANDOM_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Color.hpp>

//#define PCG_LITTLE_ENDIAN 1
#include <pcg_random.hpp>

#include <string>
#include <random>
#include <chrono>
#include <mutex>
#include <limits>

namespace fge
{

template<typename TEngine>
class Random
{
public:
    Random();
    Random(uint64_t seed);

    void setSeed(uint64_t seed);

    template<typename T>
    T range(T min, T max);

    const TEngine& getEngine() const;
    TEngine& getEngine();

    std::mutex& getMutex();

    template<typename T>
    sf::Vector2<T> rangeVec2(T min_x, T max_x,
                             T min_y, T max_y);

    template<typename T>
    sf::Vector3<T> rangeVec3(T min_x, T max_x,
                             T min_y, T max_y,
                             T min_z, T max_z);

    sf::Color rangeColor(uint32_t min, uint32_t max);
    sf::Color rangeColor(uint8_t min_r, uint8_t max_r,
                         uint8_t min_g, uint8_t max_g,
                         uint8_t min_b, uint8_t max_b,
                         uint8_t min_a, uint8_t max_a );

    template<typename T>
    T rand();

    template<typename T>
    sf::Vector2<T> randVec2();

    template<typename T>
    sf::Vector3<T> randVec3();

    sf::Color randColor();

    std::string randStr(std::size_t length, const std::string& bucket="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy0123456789");

private:
    TEngine g_engine;
    std::mutex g_mutex;
};

FGE_API extern fge::Random<pcg32> __random;

}//end fge

#include <FastEngine/C_random.inl>

#endif // _FGE_C_RANDOM_HPP_INCLUDED

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

#ifndef _FGE_C_RANDOM_HPP_INCLUDED
#define _FGE_C_RANDOM_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <chrono>
#include <limits>
#include <mutex>
#include <random>
#include <string>

namespace fge
{

/**
 * \class Random
 * \ingroup utility
 * \brief A class to generate random numbers
 *
 * This class is a wrapper for the C++ random number generator.
 * It is thread-safe and can be used in a multi-threaded environment.
 */
template<typename TEngine>
class Random
{
public:
    Random();
    /**
     * \brief Initialize the random number generator
     *
     * \param seed a 64-bit seed for the random number generator
     */
    explicit Random(uint64_t seed);

    /**
     * \brief Set the 64-bit seed of the random number generator
     *
     * \param seed a 64-bit seed for the random number generator
     */
    void setSeed(uint64_t seed);

    /**
     * \brief Generate a random number within a range
     *
     * \tparam T Type of the random number
     * \param min Minimum value of the range
     * \param max Maximum value of the range (included)
     * \return The random number
     */
    template<typename T>
    T range(T min, T max);

    /**
     * \brief Get the random engine
     *
     * \return The random engine
     */
    [[nodiscard]] const TEngine& getEngine() const;
    /**
     * \brief Get the random engine
     *
     * \return The random engine
     */
    [[nodiscard]] TEngine& getEngine();

    /**
     * \brief Generate a random vector2 within a range
     *
     * \tparam T Type of the random number in the vector
     * \param min_x Minimum value of the x range
     * \param max_x Maximum value of the x range (included)
     * \param min_y Minimum value of the y range
     * \param max_y Maximum value of the y range (included)
     * \return A random vector2
     */
    template<typename T>
    sf::Vector2<T> rangeVec2(T min_x, T max_x, T min_y, T max_y);

    /**
     * \brief Generate a random vector3 within a range
     *
     * \tparam T Type of the random number in the vector
     * \param min_x Minimum value of the x range
     * \param max_x Maximum value of the x range (included)
     * \param min_y Minimum value of the y range
     * \param max_y Maximum value of the y range (included)
     * \param min_z Minimum value of the z range
     * \param max_z Maximum value of the z range (included)
     * \return A random vector3
     */
    template<typename T>
    sf::Vector3<T> rangeVec3(T min_x, T max_x, T min_y, T max_y, T min_z, T max_z);

    /**
     * \brief Generate a random color
     *
     * \param min Minimum 32-bit value of the range
     * \param max Maximum 32-bit value of the range (included)
     * \return A random color
     */
    sf::Color rangeColor(uint32_t min, uint32_t max);
    /**
     * \brief Generate a random color
     *
     * \param min_r Minimum red value of the range
     * \param max_r Maximum red value of the range (included)
     * \param min_g Minimum green value of the range
     * \param max_g Maximum green value of the range (included)
     * \param min_b Minimum blue value of the range
     * \param max_b Maximum blue value of the range (included)
     * \param min_a Minimum alpha value of the range
     * \param max_a Maximum alpha value of the range (included)
     * \return A random color
     */
    sf::Color rangeColor(uint8_t min_r,
                         uint8_t max_r,
                         uint8_t min_g,
                         uint8_t max_g,
                         uint8_t min_b,
                         uint8_t max_b,
                         uint8_t min_a,
                         uint8_t max_a);

    /**
     * \brief Generate a random value
     *
     * \tparam T Type of the random number
     * \return The random value
     */
    template<typename T>
    T rand();

    /**
     * \brief Generate a random vector2
     *
     * \tparam T Type of the random number in the vector
     * \return A random vector2
     */
    template<typename T>
    sf::Vector2<T> randVec2();

    /**
     * \brief Generate a random vector3
     *
     * \tparam T Type of the random number in the vector
     * \return A random vector3
     */
    template<typename T>
    sf::Vector3<T> randVec3();

    /**
     * \brief Generate a random color
     *
     * \return A random color
     */
    sf::Color randColor();

    /**
     * \brief Generate a random character sequence
     *
     * Generate a random character sequence of the given length and
     * using the given bucket of characters.
     *
     * \param length The length of the character sequence
     * \param bucket The bucket of characters to use
     * \return A random character sequence
     */
    std::string randStr(std::size_t length,
                        const std::string& bucket = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy0123456789");

private:
    TEngine g_engine;
    std::mutex g_mutex;
};

/**
 * \ingroup utility
 * \brief Default random number generator instance
 *
 * This is the default random number generator instance using the
 * mt19937_64 engine.
 */
FGE_API extern fge::Random<std::mt19937_64> _random;

} // namespace fge

#include <FastEngine/C_random.inl>

#endif // _FGE_C_RANDOM_HPP_INCLUDED

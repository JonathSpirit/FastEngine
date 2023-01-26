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

namespace fge
{

template<typename TEngine>
Random<TEngine>::Random() :
        g_engine(std::chrono::system_clock::now().time_since_epoch().count())
{}
template<typename TEngine>
Random<TEngine>::Random(uint64_t seed) :
        g_engine(seed)
{}

template<typename TEngine>
void Random<TEngine>::setSeed(uint64_t seed)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    this->g_engine.seed(seed);
}

template<typename TEngine>
const TEngine& Random<TEngine>::getEngine() const
{
    return this->g_engine;
}
template<typename TEngine>
TEngine& Random<TEngine>::getEngine()
{
    return this->g_engine;
}

template<typename TEngine>
template<typename T>
T Random<TEngine>::range(T min, T max)
{
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic !");
    std::lock_guard<std::mutex> lck(this->g_mutex);

    if constexpr (std::is_floating_point<T>::value)
    {
        std::uniform_real_distribution<T> uniform_dist(min, max);
        return uniform_dist(this->g_engine);
    }
    else
    {
        std::uniform_int_distribution<T> uniform_dist(min, max);
        return uniform_dist(this->g_engine);
    }
}

template<typename TEngine>
template<typename T>
T Random<TEngine>::rand()
{
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic !");
    std::lock_guard<std::mutex> lck(this->g_mutex);

    if constexpr (std::is_floating_point<T>::value)
    {
        std::uniform_real_distribution<T> uniform_dist(-1, 1);
        return uniform_dist(this->g_engine);
    }
    else
    {
        std::uniform_int_distribution<T> uniform_dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        return uniform_dist(this->g_engine);
    }
}

template<typename TEngine>
template<typename T>
fge::Vector2<T> Random<TEngine>::rangeVec2(T min_x, T max_x, T min_y, T max_y)
{
    return fge::Vector2<T>(this->range<T>(min_x, max_x), this->range<T>(min_y, max_y));
}

template<typename TEngine>
template<typename T>
fge::Vector3<T> Random<TEngine>::rangeVec3(T min_x, T max_x, T min_y, T max_y, T min_z, T max_z)
{
    return fge::Vector3<T>(this->range<T>(min_x, max_x), this->range<T>(min_y, max_y), this->range<T>(min_z, max_z));
}

template<typename TEngine>
fge::Color Random<TEngine>::rangeColor(uint32_t min, uint32_t max)
{
    return fge::Color(this->range<uint32_t>(min, max));
}
template<typename TEngine>
fge::Color Random<TEngine>::rangeColor(uint8_t min_r,
                                      uint8_t max_r,
                                      uint8_t min_g,
                                      uint8_t max_g,
                                      uint8_t min_b,
                                      uint8_t max_b,
                                      uint8_t min_a,
                                      uint8_t max_a)
{
    return fge::Color(this->range<uint8_t>(min_r, max_r), this->range<uint8_t>(min_g, max_g),
                      this->range<uint8_t>(min_b, max_b), this->range<uint8_t>(min_a, max_a));
}

template<typename TEngine>
template<typename T>
fge::Vector2<T> Random<TEngine>::randVec2()
{
    return fge::Vector2<T>(this->rand<T>(), this->rand<T>());
}

template<typename TEngine>
template<typename T>
fge::Vector3<T> Random<TEngine>::randVec3()
{
    return fge::Vector3<T>(this->rand<T>(), this->rand<T>(), this->rand<T>());
}

template<typename TEngine>
fge::Color Random<TEngine>::randColor()
{
    return fge::Color(this->rand<uint32_t>());
}

template<typename TEngine>
std::string Random<TEngine>::randStr(std::size_t length, const std::string& bucket)
{
    if ((length == 0) || bucket.empty())
    {
        return {};
    }

    std::string result;
    result.resize(length);

    for (std::size_t i = 0; i < length; ++i)
    {
        result[i] = bucket[this->range<std::size_t>(0, bucket.size() - 1)];
    }

    return result;
}

} // namespace fge

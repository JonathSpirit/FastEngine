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

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>

namespace fge
{

template<fge::SignalValueType T>
Signal<T>::Signal(std::vector<Sample> samples) :
        g_samples(std::move(samples))
{
    for (std::size_t i = 1; i < this->g_samples.size(); ++i)
    {
        if (this->g_samples[i].x <= this->g_samples[i - 1].x)
        {
            throw fge::Exception("Signal : times must be strictly increasing");
        }
    }
}

template<fge::SignalValueType T>
void Signal<T>::addSample(T time, T value)
{
    if (!this->g_samples.empty() && time <= this->g_samples.back().x)
    {
        throw fge::Exception("Signal : times must be strictly increasing");
    }
    this->g_samples.emplace_back(time, value);
}
template<fge::SignalValueType T>
void Signal<T>::addSample(Sample const& sample)
{
    this->addSample(sample.x, sample.y);
}
template<fge::SignalValueType T>
void Signal<T>::setValue(std::size_t index, T value)
{
    this->g_samples[index].y = value;
}
template<fge::SignalValueType T>
T Signal<T>::getValue(T time, bool interpolate) const
{
    if (this->g_samples.empty())
    {
        return T{0};
    }

    //Wrap the time inside [start, end], (end, end + duration] maps to (start, end]
    T const startTime = this->getStartTime();
    T const endTime = this->getEndTime();
    T const duration = endTime - startTime;
    if (duration > T{0} && (time < startTime || time > endTime))
    {
        T elapsed = std::fmod(time - startTime, duration);
        if (elapsed < T{0})
        {
            elapsed += duration;
        }
        time = (elapsed == T{0} && time > endTime) ? endTime : startTime + elapsed;
    }

    //First sample strictly after the time, the previous one is at or before the time
    auto const next = std::upper_bound(this->g_samples.begin(), this->g_samples.end(), time,
                                       [](T t, Sample const& sample) { return t < sample.x; });
    if (next == this->g_samples.begin())
    {
        return next->y;
    }

    auto const previous = next - 1;
    if (!interpolate || next == this->g_samples.end())
    {
        return previous->y;
    }

    T const fraction = (time - previous->x) / (next->x - previous->x);
    return std::lerp(previous->y, next->y, fraction);
}
template<fge::SignalValueType T>
void Signal<T>::reserve(std::size_t count)
{
    this->g_samples.reserve(count);
}
template<fge::SignalValueType T>
void Signal<T>::clear()
{
    this->g_samples.clear();
}

template<fge::SignalValueType T>
std::size_t Signal<T>::getSampleCount() const
{
    return this->g_samples.size();
}
template<fge::SignalValueType T>
bool Signal<T>::isEmpty() const
{
    return this->g_samples.empty();
}
template<fge::SignalValueType T>
typename Signal<T>::Sample const& Signal<T>::getSample(std::size_t index) const
{
    return this->g_samples[index];
}
template<fge::SignalValueType T>
std::vector<typename Signal<T>::Sample> const& Signal<T>::getSamples() const
{
    return this->g_samples;
}

template<fge::SignalValueType T>
T Signal<T>::getStartTime() const
{
    return this->g_samples.empty() ? T{0} : this->g_samples.front().x;
}
template<fge::SignalValueType T>
T Signal<T>::getEndTime() const
{
    return this->g_samples.empty() ? T{0} : this->g_samples.back().x;
}
template<fge::SignalValueType T>
T Signal<T>::getDuration() const
{
    return this->getEndTime() - this->getStartTime();
}
template<fge::SignalValueType T>
T Signal<T>::getAverage() const
{
    if (this->g_samples.empty())
    {
        return T{0};
    }
    if (this->g_samples.size() == 1)
    {
        return this->g_samples.front().y;
    }

    //Kahan summation of the trapezoids, weighted by their relative duration
    T const duration = this->getDuration();
    T average{0};
    T correction{0};
    for (std::size_t i = 1; i < this->g_samples.size(); ++i)
    {
        Sample const& previous = this->g_samples[i - 1];
        Sample const& current = this->g_samples[i];

        T const weight = (current.x - previous.x) / duration;
        T const term = weight * std::midpoint(previous.y, current.y) - correction;
        T const next = average + term;
        correction = (next - average) - term;
        average = next;
    }
    return average;
}

template<fge::SignalValueType T>
typename Signal<T>::const_iterator Signal<T>::begin() const
{
    return this->g_samples.cbegin();
}
template<fge::SignalValueType T>
typename Signal<T>::const_iterator Signal<T>::end() const
{
    return this->g_samples.cend();
}

template<fge::SignalValueType T>
fge::Signal<T> Signal<T>::resample(T sampleRate) const
{
    if (!(sampleRate > T{0}))
    {
        throw fge::Exception("Signal : sample rate must be positive");
    }

    fge::Signal<T> result;
    if (this->g_samples.empty())
    {
        return result;
    }

    T const interval = T{1} / sampleRate;
    T const startTime = this->getStartTime();
    T const endTime = this->getEndTime();
    auto const count = static_cast<std::size_t>(std::floor((endTime - startTime) / interval)) + 1;
    result.reserve(count);

    std::size_t j = 0;
    for (std::size_t i = 0; i < count; ++i)
    {
        T const time = std::fma(static_cast<T>(i), interval, startTime);
        //Floating-point rounding can place the last time just after the end
        if (time > endTime)
        {
            break;
        }

        while (j + 1 < this->g_samples.size() && this->g_samples[j + 1].x < time)
        {
            ++j;
        }

        Sample const& previous = this->g_samples[j];
        if (time == previous.x || j + 1 == this->g_samples.size())
        {
            result.addSample(time, previous.y);
        }
        else
        {
            Sample const& next = this->g_samples[j + 1];
            T const fraction = (time - previous.x) / (next.x - previous.x);
            result.addSample(time, std::lerp(previous.y, next.y, fraction));
        }
    }
    return result;
}
template<fge::SignalValueType T>
fge::Signal<T> Signal<T>::derivative() const
{
    fge::Signal<T> result = *this;
    std::size_t const n = this->g_samples.size();

    switch (n)
    {
    case 0:
        return result;
    case 1:
        result.g_samples[0].y = T{0};
        return result;
    case 2:
    {
        Sample const& a = this->g_samples[0];
        Sample const& b = this->g_samples[1];
        T const slope = (b.y - a.y) / (b.x - a.x);
        result.g_samples[0].y = slope;
        result.g_samples[1].y = slope;
        return result;
    }
    default:
        break;
    }

    //Three-point derivative, the window is shifted at the boundaries
    for (std::size_t i = 0; i < n; ++i)
    {
        std::size_t const j = i == 0 ? 0 : (i == n - 1 ? n - 3 : i - 1);
        Sample const& s0 = this->g_samples[j];
        Sample const& s1 = this->g_samples[j + 1];
        Sample const& s2 = this->g_samples[j + 2];
        T const time = this->g_samples[i].x;

        T const h0 = s1.x - s0.x;
        T const h1 = s2.x - s1.x;
        T const slope0 = (s1.y - s0.y) / h0;
        T const slope1 = (s2.y - s1.y) / h1;
        T const curvature = (slope1 - slope0) / (h0 + h1);
        result.g_samples[i].y = slope0 + curvature * ((time - s0.x) + (time - s1.x));
    }
    return result;
}
template<fge::SignalValueType T>
fge::Signal<T> Signal<T>::integral(T initialValue) const
{
    fge::Signal<T> result = *this;
    if (result.g_samples.empty())
    {
        return result;
    }

    result.g_samples[0].y = initialValue;
    for (std::size_t i = 1; i < this->g_samples.size(); ++i)
    {
        Sample const& previous = this->g_samples[i - 1];
        Sample const& current = this->g_samples[i];

        T const dt = current.x - previous.x;
        result.g_samples[i].y = result.g_samples[i - 1].y + std::midpoint(previous.y, current.y) * dt;
    }
    return result;
}
template<fge::SignalValueType T>
fge::Signal<T> Signal<T>::softLimit(T lower, T upper, T smoothness) const
{
    if (!(lower < upper))
    {
        throw fge::Exception("Signal : soft limit lower bound must be smaller than the upper bound");
    }
    if (!(smoothness > T{0}))
    {
        throw fge::Exception("Signal : soft limit smoothness must be positive");
    }

    fge::Signal<T> result = *this;
    this->applySoftLimit(result, lower, upper, smoothness, T{0});
    return result;
}
template<fge::SignalValueType T>
fge::Signal<T> Signal<T>::softLimitKeepAverage(T lower, T upper, T smoothness) const
{
    return this->softLimitWithAverage(this->getAverage(), lower, upper, smoothness);
}
template<fge::SignalValueType T>
fge::Signal<T> Signal<T>::softLimitWithAverage(T average, T lower, T upper, T smoothness) const
{
    fge::Signal<T> result = this->softLimit(lower, upper, smoothness);
    if (this->g_samples.empty())
    {
        return result;
    }

    //Comparing averages is equivalent to comparing integrals
    T const tolerance = static_cast<T>(FGE_SIGNAL_KEEP_AVERAGE_TOLERANCE) * std::numeric_limits<T>::epsilon() *
                        std::max(std::abs(lower), std::abs(upper));
    if (std::abs(result.getAverage() - average) <= tolerance)
    {
        return result;
    }

    //A positive bias lowers the average, a negative one raises it.
    //Find a [low, high] bias range that contains the requested average.
    T step = std::max(std::abs(lower), std::abs(upper));
    for (auto const& sample: this->g_samples)
    {
        step = std::max(step, std::abs(sample.y));
    }

    T const maxBias = std::numeric_limits<T>::max();
    auto const expandBias = [&](T& bias, bool raiseAverage) {
        for (std::size_t i = 0; i < FGE_SIGNAL_KEEP_AVERAGE_MAX_ITERATIONS; ++i)
        {
            this->applySoftLimit(result, lower, upper, smoothness, bias);
            T const resultAverage = result.getAverage();
            if (raiseAverage ? resultAverage >= average : resultAverage <= average)
            {
                return;
            }
            T const magnitude = std::abs(bias);
            if (magnitude == maxBias)
            {
                return;
            }
            bias = std::copysign(magnitude > maxBias / T{2} ? maxBias : magnitude * T{2}, bias);
        }
    };

    T low = -step;
    T high = step;
    expandBias(low, true);
    expandBias(high, false);

    //Bisection, the closest result is kept in case the tolerance is never reached
    T bestBias = T{0};
    T bestError = std::numeric_limits<T>::infinity();
    for (std::size_t i = 0; i < FGE_SIGNAL_KEEP_AVERAGE_MAX_ITERATIONS; ++i)
    {
        T const bias = std::midpoint(low, high);
        this->applySoftLimit(result, lower, upper, smoothness, bias);
        T const resultAverage = result.getAverage();
        T const error = std::abs(resultAverage - average);
        if (error <= tolerance)
        {
            return result;
        }
        if (error < bestError)
        {
            bestError = error;
            bestBias = bias;
        }

        if (bias == low || bias == high)
        {
            break;
        }
        if (resultAverage > average)
        {
            low = bias;
        }
        else
        {
            high = bias;
        }
    }

    this->applySoftLimit(result, lower, upper, smoothness, bestBias);
    return result;
}
template<fge::SignalValueType T>
fge::Signal<T> Signal<T>::timeStretch(T factor) const
{
    if (!(factor > T{0}))
    {
        throw fge::Exception("Signal : time stretch factor must be positive");
    }

    fge::Signal<T> result;
    result.reserve(this->g_samples.size());

    T const startTime = this->getStartTime();
    for (auto const& sample: this->g_samples)
    {
        result.addSample(startTime + (sample.x - startTime) * factor, sample.y);
    }
    return result;
}

template<fge::SignalValueType T>
bool Signal<T>::saveToFile(std::filesystem::path const& path) const
{
    std::ofstream file(path);
    if (!file)
    {
        return false;
    }

    file << FGE_SIGNAL_CSV_HEADER << '\n';
    file << std::setprecision(std::numeric_limits<T>::max_digits10);
    for (auto const& sample: this->g_samples)
    {
        file << sample.x << ',' << sample.y << '\n';
    }
    return static_cast<bool>(file);
}
template<fge::SignalValueType T>
bool Signal<T>::loadFromFile(std::filesystem::path const& path)
{
    std::ifstream file(path);
    if (!file)
    {
        return false;
    }

    std::vector<Sample> samples;
    std::string line;
    bool firstLine = true;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.empty())
        {
            continue;
        }

        std::istringstream stream(line);
        Sample sample;
        char separator = 0;
        if (!(stream >> sample.x >> separator >> sample.y) || separator != ',')
        {
            if (firstLine)
            {
                //The first line can be a header
                firstLine = false;
                continue;
            }
            return false;
        }
        firstLine = false;

        if (!samples.empty() && sample.x <= samples.back().x)
        {
            return false;
        }
        samples.push_back(sample);
    }

    this->g_samples = std::move(samples);
    return true;
}

template<fge::SignalValueType T>
void Signal<T>::applySoftLimit(fge::Signal<T>& result, T lower, T upper, T smoothness, T bias) const
{
    T const center = std::midpoint(lower, upper);
    T const radius = upper - center;

    for (std::size_t i = 0; i < this->g_samples.size(); ++i)
    {
        T const offset = (this->g_samples[i].y - center) - bias;
        T const magnitude = std::abs(offset);

        //Equivalent forms, keeping the inner power base in [0, 1]
        T limited{0};
        if (radius > T{0})
        {
            limited = magnitude <= radius
                              ? magnitude / std::pow(T{1} + std::pow(magnitude / radius, smoothness), T{1} / smoothness)
                              : radius / std::pow(T{1} + std::pow(radius / magnitude, smoothness), T{1} / smoothness);
        }
        result.g_samples[i].y = std::clamp(center + std::copysign(limited, offset), lower, upper);
    }
}

} // namespace fge

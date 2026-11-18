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
#include <numbers>

namespace fge
{

template<fge::SignalValueType T>
std::size_t SignalGenerator<T>::addWave(Wave const& wave)
{
    this->g_waves.push_back(sanitizeWave(wave));
    return this->g_waves.size() - 1;
}
template<fge::SignalValueType T>
void SignalGenerator<T>::setWave(std::size_t index, Wave const& wave)
{
    this->g_waves[index] = sanitizeWave(wave);
}
template<fge::SignalValueType T>
void SignalGenerator<T>::removeWave(std::size_t index)
{
    this->g_waves.erase(this->g_waves.begin() + static_cast<std::ptrdiff_t>(index));
}
template<fge::SignalValueType T>
void SignalGenerator<T>::clearWaves()
{
    this->g_waves.clear();
}

template<fge::SignalValueType T>
std::size_t SignalGenerator<T>::getWaveCount() const
{
    return this->g_waves.size();
}
template<fge::SignalValueType T>
typename SignalGenerator<T>::Wave const& SignalGenerator<T>::getWave(std::size_t index) const
{
    return this->g_waves[index];
}
template<fge::SignalValueType T>
std::vector<typename SignalGenerator<T>::Wave> const& SignalGenerator<T>::getWaves() const
{
    return this->g_waves;
}

template<fge::SignalValueType T>
T SignalGenerator<T>::getValue(T time) const
{
    T result{0};
    for (auto const& wave: this->g_waves)
    {
        if (wave._type == WaveTypes::WAVE_CONTINUOUS)
        {
            result += wave._offset;
            continue;
        }

        T const cycles = wave._frequency * time + wave._phaseDegrees / T{360};
        T const phase = cycles - std::floor(cycles); //[0, 1)

        T value{0};
        switch (wave._type)
        {
        case WaveTypes::WAVE_SINUSOID:
            value = std::sin(T{2} * std::numbers::pi_v<T> * phase);
            break;
        case WaveTypes::WAVE_RECTANGLE:
            value = phase < wave._dutyCycle ? T{1} : T{-1};
            break;
        case WaveTypes::WAVE_SAWTOOTH:
            value = T{2} * phase - T{1};
            break;
        case WaveTypes::WAVE_TRIANGLE:
            if (phase < wave._dutyCycle)
            {
                value = T{2} * phase / wave._dutyCycle - T{1};
            }
            else
            {
                value = T{1} - T{2} * (phase - wave._dutyCycle) / (T{1} - wave._dutyCycle);
            }
            break;
        default:
            break;
        }

        result += wave._offset + wave._amplitude * value;
    }
    return result;
}
template<fge::SignalValueType T>
fge::Signal<T> SignalGenerator<T>::generateSamples(std::size_t count, T sampleRate) const
{
    if (!(sampleRate > T{0}))
    {
        throw fge::Exception("SignalGenerator : sample rate must be positive");
    }
    return this->generateImpl(count, T{1} / sampleRate);
}
template<fge::SignalValueType T>
fge::Signal<T> SignalGenerator<T>::generate(T duration, T sampleInterval) const
{
    if (!(duration >= T{0}))
    {
        throw fge::Exception("SignalGenerator : duration must not be negative");
    }
    if (!(sampleInterval > T{0}))
    {
        throw fge::Exception("SignalGenerator : sample interval must be positive");
    }

    auto const count = static_cast<std::size_t>(std::round(duration / sampleInterval)) + 1;
    return this->generateImpl(count, sampleInterval);
}

template<fge::SignalValueType T>
typename SignalGenerator<T>::Wave SignalGenerator<T>::sanitizeWave(Wave wave)
{
    wave._dutyCycle = std::clamp(wave._dutyCycle, T{0}, T{1});
    return wave;
}
template<fge::SignalValueType T>
fge::Signal<T> SignalGenerator<T>::generateImpl(std::size_t count, T sampleInterval) const
{
    fge::Signal<T> signal;
    signal.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
    {
        T const time = static_cast<T>(i) * sampleInterval;
        signal.addSample(time, this->getValue(time));
    }
    return signal;
}

} // namespace fge

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

#ifndef _FGE_EXT_C_SIGNALGENERATOR_HPP_INCLUDED
#define _FGE_EXT_C_SIGNALGENERATOR_HPP_INCLUDED

#include "FastEngine/extensions/C_signal.hpp"
#include <vector>

namespace fge
{

/**
 * \class SignalGenerator
 * \ingroup extensions
 * \brief Generate a signal from a sum of waveforms
 *
 * Waves are added with addWave(), the generated value at a given time
 * is the sum of every stored wave.
 *
 * A signal can be generated with:
 * - generate() for a duration and a sample interval
 * - generateSamples() for an exact number of samples and a sample rate
 *
 * The value can also be evaluated at any time with getValue(),
 * useful to drive something in real time without generating a whole signal.
 *
 * \tparam T The data type, float or double
 */
template<fge::SignalValueType T>
class SignalGenerator
{
public:
    using ValueType = T;

    enum class WaveTypes
    {
        WAVE_SINUSOID,  ///< Sine wave
        WAVE_RECTANGLE, ///< Two-level wave, the high-time fraction is the duty cycle
        WAVE_SAWTOOTH,  ///< Linear rising ramp with an abrupt reset
        WAVE_TRIANGLE,  ///< Linear rise and fall, the rise-time fraction is the duty cycle
        WAVE_CONTINUOUS ///< Constant value equal to the offset
    };

    /**
     * \brief Parameters of one waveform
     *
     * WAVE_CONTINUOUS only uses the offset.
     */
    struct Wave
    {
        WaveTypes _type{WaveTypes::WAVE_SINUSOID};
        T _amplitude{1};    ///< Scale of the waveform around the offset
        T _frequency{1};    ///< Frequency in hertz
        T _phaseDegrees{0}; ///< Phase shift in degrees
        /**
         * \brief Rectangle high-time fraction or triangle rise-time fraction
         *
         * The value is clamped in [0, 1].
         * A triangle is symmetric at 0.5.
         *
         * Ignored by the other waveforms.
         */
        T _dutyCycle{0.5};
        T _offset{0}; ///< Additive offset, the constant value for WAVE_CONTINUOUS
    };

    SignalGenerator() = default;

    //Waves

    /**
     * \brief Add a waveform
     *
     * The duty cycle is clamped in [0, 1].
     *
     * \param wave The waveform parameters
     * \return The index of the added wave
     */
    std::size_t addWave(Wave const& wave);
    /**
     * \brief Replace an existing waveform
     *
     * The duty cycle is clamped in [0, 1].
     *
     * \param index The index of the wave
     * \param wave The new waveform parameters
     */
    void setWave(std::size_t index, Wave const& wave);
    /**
     * \brief Remove a waveform
     *
     * Next waves indices are shifted down by one.
     *
     * \param index The index of the wave
     */
    void removeWave(std::size_t index);
    /**
     * \brief Remove every waveforms
     */
    void clearWaves();

    [[nodiscard]] std::size_t getWaveCount() const;
    [[nodiscard]] Wave const& getWave(std::size_t index) const;
    [[nodiscard]] std::vector<Wave> const& getWaves() const;

    //Generation

    /**
     * \brief Compute the sum of every waves at a given time
     *
     * Without any wave, the value is 0.
     *
     * \param time The time in seconds
     * \return The value
     */
    [[nodiscard]] T getValue(T time) const;
    /**
     * \brief Generate an exact number of samples
     *
     * The first sample is at time 0,
     * the next ones are spaced by 1 / sampleRate.
     *
     * A fge::Exception is thrown if the sample rate is not positive.
     *
     * \param count The number of samples
     * \param sampleRate The sample rate in samples per second
     * \return The generated signal
     */
    [[nodiscard]] fge::Signal<T> generateSamples(std::size_t count, T sampleRate) const;
    /**
     * \brief Generate a signal for a duration
     *
     * The first sample is at time 0 and the last one at the duration
     * (rounded to the nearest sample interval).
     * A duration of 0 returns one sample.
     *
     * A fge::Exception is thrown if the duration is negative
     * or if the sample interval is not positive.
     *
     * \param duration The duration in seconds
     * \param sampleInterval The time between two samples in seconds
     * \return The generated signal
     */
    [[nodiscard]] fge::Signal<T> generate(T duration, T sampleInterval) const;

private:
    [[nodiscard]] static Wave sanitizeWave(Wave wave);
    [[nodiscard]] fge::Signal<T> generateImpl(std::size_t count, T sampleInterval) const;

    std::vector<Wave> g_waves;
};

using SignalGeneratorF = SignalGenerator<float>;
using SignalGeneratorD = SignalGenerator<double>;

} // namespace fge

#include "C_signalGenerator.inl"

#endif // _FGE_EXT_C_SIGNALGENERATOR_HPP_INCLUDED

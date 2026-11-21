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

#ifndef _FGE_EXT_C_SIGNAL_HPP_INCLUDED
#define _FGE_EXT_C_SIGNAL_HPP_INCLUDED

#include "FastEngine/C_vector.hpp"
#include "FastEngine/fge_except.hpp"
#include <concepts>
#include <filesystem>
#include <vector>

#define FGE_SIGNAL_CSV_HEADER "time_s,data"
#define FGE_SIGNAL_KEEP_AVERAGE_MAX_ITERATIONS 256
#define FGE_SIGNAL_KEEP_AVERAGE_TOLERANCE 8

namespace fge
{

/**
 * \brief The data types supported by Signal and SignalGenerator
 */
template<class T>
concept SignalValueType = std::same_as<T, float> || std::same_as<T, double>;

/**
 * \class Signal
 * \ingroup extensions
 * \brief A sampled signal, a list of (time, value) samples
 *
 * Each sample is stored as a fge::Vector2<T> where:
 * - \b x is the time in seconds
 * - \b y is the value
 *
 * Times are always strictly increasing,
 * this is checked every time a sample is added.
 * The time spacing between samples doesn't need to be constant.
 *
 * Every processing method (derivative(), integral(), softLimit(), ...)
 * returns a new Signal and keeps the original one untouched.
 *
 * The samples can be directly given to a fge::ObjChart:
 * \code
 * chart->setPoints(signal.getSamples());
 * \endcode
 *
 * \tparam T The data type, float or double
 */
template<fge::SignalValueType T>
class Signal
{
public:
    using ValueType = T;
    using Sample = fge::Vector2<T>;
    using const_iterator = typename std::vector<Sample>::const_iterator;

    Signal() = default;
    /**
     * \brief Construct a signal from a list of samples
     *
     * If the times are not strictly increasing,
     * a fge::Exception is thrown.
     *
     * \param samples The samples, x is the time and y the value
     */
    explicit Signal(std::vector<Sample> samples);

    //Samples

    /**
     * \brief Add a sample at the end of the signal
     *
     * The time must be strictly greater than the time of the last sample,
     * otherwise a fge::Exception is thrown.
     *
     * \param time The time in seconds
     * \param value The value
     */
    void addSample(T time, T value);
    /**
     * \brief Add a sample at the end of the signal
     *
     * \see addSample(T, T)
     *
     * \param sample The sample, x is the time and y the value
     */
    void addSample(Sample const& sample);
    /**
     * \brief Change the value of an existing sample
     *
     * The time of the sample is not modified.
     *
     * \param index The index of the sample
     * \param value The new value
     */
    void setValue(std::size_t index, T value);
    /**
     * \brief Get the value at a given time
     *
     * If the time is outside the signal, it loops:
     * the time is wrapped inside [start time, end time].
     *
     * Without interpolation, the value of the last sample
     * at or before the time is returned (sample and hold).
     * With interpolation, the value is linearly interpolated
     * between the two surrounding samples.
     *
     * The sample is found by a binary search.
     * An empty signal returns 0.
     *
     * \param time The time in seconds
     * \param interpolate \b true to linearly interpolate between samples
     * \return The value
     */
    [[nodiscard]] T getValue(T time, bool interpolate = false) const;
    /**
     * \brief Reserve memory for a number of samples
     *
     * \param count The number of samples
     */
    void reserve(std::size_t count);
    /**
     * \brief Remove every samples
     */
    void clear();

    [[nodiscard]] std::size_t getSampleCount() const;
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] Sample const& getSample(std::size_t index) const;
    [[nodiscard]] std::vector<Sample> const& getSamples() const;

    /**
     * \brief Get the time of the first sample
     *
     * \return The start time, or 0 if the signal is empty
     */
    [[nodiscard]] T getStartTime() const;
    /**
     * \brief Get the time of the last sample
     *
     * \return The end time, or 0 if the signal is empty
     */
    [[nodiscard]] T getEndTime() const;
    /**
     * \brief Get the time between the first and the last sample
     *
     * \return The duration in seconds
     */
    [[nodiscard]] T getDuration() const;
    /**
     * \brief Get the time-weighted average value of the signal
     *
     * The average is computed with the trapezoidal rule,
     * this is the integral of the signal divided by its duration.
     *
     * An empty signal returns 0.
     * A signal with only one sample returns the value of this sample.
     *
     * \return The average value
     */
    [[nodiscard]] T getAverage() const;

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;

    //Processing

    /**
     * \brief Resample the signal with a fixed sample rate
     *
     * New samples are placed at: start + i / sampleRate,
     * values are linearly interpolated from the original samples.
     *
     * The last original sample is only kept if it falls on the new time grid,
     * values are never extrapolated.
     *
     * \warning When downsampling, no anti-aliasing filter is applied.
     *
     * A fge::Exception is thrown if the sample rate is not positive.
     *
     * \param sampleRate The new sample rate in samples per second
     * \return The resampled signal
     */
    [[nodiscard]] fge::Signal<T> resample(T sampleRate) const;
    /**
     * \brief Compute the derivative of the signal
     *
     * A three-point derivative is used, boundaries included,
     * an uneven time spacing is supported.
     *
     * With only two samples, the simple slope is used.
     * With only one sample, the derivative is 0.
     *
     * \return The derivative, with the original times
     */
    [[nodiscard]] fge::Signal<T> derivative() const;
    /**
     * \brief Compute the cumulative integral of the signal
     *
     * The trapezoidal rule is used,
     * the integral starts at the initial value on the first sample.
     *
     * The initial value is the integration constant,
     * for example, when integrating an acceleration into a velocity,
     * it is the initial velocity.
     * Without it, the result is shifted by a constant
     * and a second integration drifts linearly.
     *
     * \param initialValue The value of the integral on the first sample
     * \return The integral, with the original times
     */
    [[nodiscard]] fge::Signal<T> integral(T initialValue = T{0}) const;
    /**
     * \brief Smoothly compress the values toward the provided bounds
     *
     * With:
     * - c = (lower + upper) / 2
     * - r = upper - c
     * - p = smoothness
     *
     * every value x becomes:
     * c + (x - c) / pow(1 + pow(abs(x - c) / r, p), 1 / p)
     *
     * A bigger smoothness gets closer to a hard clipping.
     *
     * A fge::Exception is thrown if lower is not smaller than upper
     * or if the smoothness is not positive.
     *
     * \param lower The lower bound
     * \param upper The upper bound
     * \param smoothness The smoothness exponent, must be positive
     * \return The limited signal, with the original times
     */
    [[nodiscard]] fge::Signal<T> softLimit(T lower, T upper, T smoothness) const;
    /**
     * \brief Same as softLimit() but keep the average value of the signal
     *
     * This is a shortcut for:
     * \code
     * signal.softLimitWithAverage(signal.getAverage(), lower, upper, smoothness);
     * \endcode
     *
     * \see softLimitWithAverage()
     *
     * \param lower The lower bound
     * \param upper The upper bound
     * \param smoothness The smoothness exponent, must be positive
     * \return The limited signal, with the original times and the same average value
     */
    [[nodiscard]] fge::Signal<T> softLimitKeepAverage(T lower, T upper, T smoothness) const;
    /**
     * \brief Same as softLimit() but the result has the requested average value
     *
     * A soft limiter compresses the peaks, so the signal average can move.
     * Here, the signal is shifted internally (by a constant bias found by bisection)
     * before being limited, in order to reach the requested average value
     * and so the requested area under the curve.
     *
     * This is useful to keep the average of another signal,
     * for example, when a velocity was recomputed from a limited acceleration,
     * limiting it with the average of the original velocity
     * keeps the final position of the trajectory:
     * \code
     * auto const v = x.derivative();
     * auto const a = v.derivative();
     *
     * auto const vLimited = a.softLimitKeepAverage(-aMax, aMax, smoothness)
     *                               .integral(v.getSample(0).y)
     *                               .softLimitWithAverage(v.getAverage(), -vMax, vMax, smoothness);
     * auto const xLimited = vLimited.integral(x.getSample(0).y);
     * \endcode
     *
     * The average can only be reached if it is strictly between lower and upper.
     * If it can't be reached, or if the wanted precision can't be reached,
     * the closest result found is returned.
     *
     * \see softLimit()
     *
     * \param average The requested average value
     * \param lower The lower bound
     * \param upper The upper bound
     * \param smoothness The smoothness exponent, must be positive
     * \return The limited signal, with the original times and the requested average value
     */
    [[nodiscard]] fge::Signal<T> softLimitWithAverage(T average, T lower, T upper, T smoothness) const;
    /**
     * \brief Scale the elapsed time from the first sample
     *
     * Values are not modified.
     * - A factor greater than 1 stretches the signal
     * - A factor smaller than 1 compresses the signal
     *
     * A fge::Exception is thrown if the factor is not positive.
     *
     * \param factor The time factor, must be positive
     * \return The stretched signal
     */
    [[nodiscard]] fge::Signal<T> timeStretch(T factor) const;

    //File

    /**
     * \brief Save the signal in a two columns CSV file
     *
     * The first line is the header: "time_s,data".
     * An existing file is overwritten.
     *
     * \param path The path of the file
     * \return \b true if successful, \b false otherwise
     */
    bool saveToFile(std::filesystem::path const& path) const;
    /**
     * \brief Load a signal from a two columns CSV file
     *
     * A header on the first line is skipped.
     * Times must be strictly increasing.
     *
     * On failure, the signal is not modified.
     *
     * \param path The path of the file
     * \return \b true if successful, \b false otherwise
     */
    bool loadFromFile(std::filesystem::path const& path);

private:
    /**
     * \brief Apply the soft limiter formula on the values of this signal into the result
     *
     * The bias is subtracted from every value before applying the formula.
     * The result must have the same samples count as this signal.
     */
    void applySoftLimit(fge::Signal<T>& result, T lower, T upper, T smoothness, T bias) const;

    std::vector<Sample> g_samples;
};

using SignalF = Signal<float>;
using SignalD = Signal<double>;

} // namespace fge

#include "C_signal.inl"

#endif // _FGE_EXT_C_SIGNAL_HPP_INCLUDED

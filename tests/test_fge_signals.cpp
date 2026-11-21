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

#include "FastEngine/extensions/C_signalGenerator.hpp"
#include "doctest/doctest.h"
#include <cmath>
#include <filesystem>

TEST_CASE("testing signal samples")
{
    fge::SignalD signal;
    REQUIRE(signal.isEmpty());
    REQUIRE(signal.getAverage() == 0.0);

    signal.addSample(0.0, 1.0);
    signal.addSample(1.0, 3.0);
    signal.addSample({3.0, 3.0});
    REQUIRE(signal.getSampleCount() == 3);
    REQUIRE(signal.getStartTime() == 0.0);
    REQUIRE(signal.getEndTime() == 3.0);
    REQUIRE(signal.getDuration() == 3.0);

    //(2 * 1 + 3 * 2) / 3
    CHECK(signal.getAverage() == doctest::Approx(8.0 / 3.0));

    SUBCASE("times must be strictly increasing")
    {
        CHECK_THROWS_AS(signal.addSample(3.0, 0.0), fge::Exception);
        CHECK_THROWS_AS(signal.addSample(2.0, 0.0), fge::Exception);
        CHECK_THROWS_AS(fge::SignalD({{0.0, 0.0}, {0.0, 1.0}}), fge::Exception);
        REQUIRE(signal.getSampleCount() == 3);
    }

    SUBCASE("get value")
    {
        //Samples: (0, 1), (1, 3), (3, 3)
        CHECK(signal.getValue(0.0) == 1.0);
        CHECK(signal.getValue(0.5) == 1.0);
        CHECK(signal.getValue(1.0) == 3.0);
        CHECK(signal.getValue(3.0) == 3.0);
        CHECK(signal.getValue(0.5, true) == doctest::Approx(2.0));
        CHECK(signal.getValue(1.0, true) == 3.0);

        //Looping, (end, end + duration] maps to (start, end]
        CHECK(signal.getValue(3.5, true) == doctest::Approx(2.0));
        CHECK(signal.getValue(6.0) == 3.0);
        CHECK(signal.getValue(-2.5, true) == doctest::Approx(2.0));
        CHECK(signal.getValue(-3.0) == 1.0);
        CHECK(signal.getValue(3.0e6 + 0.5, true) == doctest::Approx(2.0));

        CHECK(fge::SignalD{}.getValue(1.0) == 0.0);
        CHECK(fge::SignalD({{2.0, 7.0}}).getValue(-10.0, true) == 7.0);
    }

    SUBCASE("set value")
    {
        signal.setValue(1, -2.0);
        CHECK(signal.getSample(1) == fge::SignalD::Sample{1.0, -2.0});
    }
}

TEST_CASE("testing signal generator")
{
    using Generator = fge::SignalGeneratorD;
    using WaveTypes = Generator::WaveTypes;

    Generator generator;
    REQUIRE(generator.getValue(0.3) == 0.0);

    SUBCASE("sample count and times")
    {
        auto const signal = generator.generate(1.0, 0.1);
        REQUIRE(signal.getSampleCount() == 11);
        CHECK(signal.getEndTime() == doctest::Approx(1.0));

        auto const samples = generator.generateSamples(5, 10.0);
        REQUIRE(samples.getSampleCount() == 5);
        CHECK(samples.getEndTime() == doctest::Approx(0.4));

        CHECK(generator.generate(0.0, 0.1).getSampleCount() == 1);
        CHECK(generator.generateSamples(0, 10.0).isEmpty());

        CHECK_THROWS_AS((void) generator.generate(-1.0, 0.1), fge::Exception);
        CHECK_THROWS_AS((void) generator.generate(1.0, 0.0), fge::Exception);
        CHECK_THROWS_AS((void) generator.generateSamples(5, -1.0), fge::Exception);
    }

    SUBCASE("sinusoid")
    {
        generator.addWave({WaveTypes::WAVE_SINUSOID, 2.0, 1.0, 0.0, 0.5, 0.0});
        auto const signal = generator.generateSamples(4, 4.0);
        CHECK(signal.getSample(0).y == doctest::Approx(0.0));
        CHECK(signal.getSample(1).y == doctest::Approx(2.0));
        CHECK(signal.getSample(2).y == doctest::Approx(0.0));
        CHECK(signal.getSample(3).y == doctest::Approx(-2.0));

        //90 degrees phase shift
        generator.setWave(0, {WaveTypes::WAVE_SINUSOID, 1.0, 1.0, 90.0, 0.5, 0.0});
        CHECK(generator.getValue(0.0) == doctest::Approx(1.0));
    }

    SUBCASE("rectangle")
    {
        generator.addWave({WaveTypes::WAVE_RECTANGLE, 1.0, 1.0, 0.0, 0.25, 0.0});
        CHECK(generator.getValue(0.1) == 1.0);
        CHECK(generator.getValue(0.3) == -1.0);
    }

    SUBCASE("sawtooth and triangle")
    {
        generator.addWave({WaveTypes::WAVE_SAWTOOTH, 1.0, 1.0, 0.0, 0.5, 0.0});
        CHECK(generator.getValue(0.25) == doctest::Approx(-0.5));

        generator.setWave(0, {WaveTypes::WAVE_TRIANGLE, 1.0, 1.0, 0.0, 0.5, 0.0});
        CHECK(generator.getValue(0.0) == doctest::Approx(-1.0));
        CHECK(generator.getValue(0.5) == doctest::Approx(1.0));
        CHECK(generator.getValue(0.75) == doctest::Approx(0.0));
    }

    SUBCASE("continuous and sum of waves")
    {
        generator.addWave({WaveTypes::WAVE_CONTINUOUS, 10.0, 10.0, 0.0, 0.5, 0.5});
        generator.addWave({WaveTypes::WAVE_SAWTOOTH, 1.0, 1.0, 0.0, 0.5, 0.0});
        CHECK(generator.getValue(0.25) == doctest::Approx(0.0));
        REQUIRE(generator.getWaveCount() == 2);

        generator.removeWave(1);
        REQUIRE(generator.getWaveCount() == 1);
        CHECK(generator.getValue(0.25) == doctest::Approx(0.5));

        generator.clearWaves();
        REQUIRE(generator.getWaveCount() == 0);
    }

    SUBCASE("duty cycle is clamped")
    {
        generator.addWave({WaveTypes::WAVE_TRIANGLE, 1.0, 1.0, 0.0, 2.0, 0.0});
        CHECK(generator.getWave(0)._dutyCycle == 1.0);
        generator.setWave(0, {WaveTypes::WAVE_TRIANGLE, 1.0, 1.0, 0.0, -1.0, 0.0});
        CHECK(generator.getWave(0)._dutyCycle == 0.0);
    }
}

TEST_CASE("testing signal processing")
{
    SUBCASE("derivative")
    {
        //Exact for a quadratic, even with uneven time spacing
        fge::SignalD signal({{0.0, 0.0}, {0.5, 0.25}, {1.5, 2.25}, {2.0, 4.0}, {3.5, 12.25}});
        auto const derivative = signal.derivative();
        REQUIRE(derivative.getSampleCount() == signal.getSampleCount());
        for (auto const& sample: derivative)
        {
            CHECK(sample.y == doctest::Approx(2.0 * sample.x));
        }

        CHECK(fge::SignalD{}.derivative().isEmpty());
        CHECK(fge::SignalD({{1.0, 5.0}}).derivative().getSample(0).y == 0.0);
        CHECK(fge::SignalD({{0.0, 1.0}, {2.0, 5.0}}).derivative().getSample(1).y == doctest::Approx(2.0));
    }

    SUBCASE("integral")
    {
        fge::SignalD signal({{0.0, 2.0}, {1.0, 2.0}, {3.0, 2.0}});
        auto const integral = signal.integral();
        CHECK(integral.getSample(0).y == 0.0);
        CHECK(integral.getSample(1).y == doctest::Approx(2.0));
        CHECK(integral.getSample(2).y == doctest::Approx(6.0));

        auto const integralWithInitial = signal.integral(10.0);
        CHECK(integralWithInitial.getSample(0).y == 10.0);
        CHECK(integralWithInitial.getSample(2).y == doctest::Approx(16.0));
    }

    SUBCASE("resample")
    {
        fge::SignalD signal({{0.0, 0.0}, {1.0, 10.0}, {3.0, 30.0}});
        auto const resampled = signal.resample(2.0);
        REQUIRE(resampled.getSampleCount() == 7);
        CHECK(resampled.getSample(1).y == doctest::Approx(5.0));
        CHECK(resampled.getSample(5).y == doctest::Approx(25.0));
        CHECK(resampled.getEndTime() == doctest::Approx(3.0));

        CHECK(fge::SignalD({{0.0, 1.0}, {1.0, 2.0}}).resample(3.0).getSampleCount() == 4);
        CHECK(fge::SignalD{}.resample(10.0).isEmpty());
        CHECK_THROWS_AS((void) signal.resample(0.0), fge::Exception);
    }

    SUBCASE("soft limit")
    {
        fge::SignalD signal({{0.0, -10.0}, {1.0, -0.5}, {2.0, 0.0}, {3.0, 0.5}, {4.0, 10.0}});
        auto const limited = signal.softLimit(-1.0, 1.0, 4.0);
        for (auto const& sample: limited)
        {
            CHECK(sample.y >= -1.0);
            CHECK(sample.y <= 1.0);
        }
        CHECK(limited.getSample(2).y == 0.0);
        CHECK(limited.getSample(1).y < limited.getSample(3).y);

        CHECK_THROWS_AS((void) signal.softLimit(1.0, -1.0, 4.0), fge::Exception);
        CHECK_THROWS_AS((void) signal.softLimit(-1.0, 1.0, 0.0), fge::Exception);
    }

    SUBCASE("soft limit keep average")
    {
        fge::SignalGeneratorD generator;
        generator.addWave({fge::SignalGeneratorD::WaveTypes::WAVE_RECTANGLE, 3.0, 2.0, 0.0, 0.4, 0.0});
        generator.addWave({fge::SignalGeneratorD::WaveTypes::WAVE_CONTINUOUS, 0.0, 0.0, 0.0, 0.5, 0.3});
        auto const signal = generator.generate(2.0, 0.01);

        auto const limited = signal.softLimit(-1.0, 1.0, 3.0);
        auto const limitedKeep = signal.softLimitKeepAverage(-1.0, 1.0, 3.0);
        CHECK(limited.getAverage() != doctest::Approx(signal.getAverage()));
        CHECK(limitedKeep.getAverage() == doctest::Approx(signal.getAverage()));
        for (auto const& sample: limitedKeep)
        {
            CHECK(sample.y >= -1.0);
            CHECK(sample.y <= 1.0);
        }

        //The average can't be reached, the closest result is returned
        auto const saturated = signal.softLimitKeepAverage(0.5, 1.0, 3.0);
        CHECK(saturated.getAverage() == doctest::Approx(0.5).epsilon(0.001));
    }

    SUBCASE("soft limit with average")
    {
        fge::SignalGeneratorD generator;
        generator.addWave({fge::SignalGeneratorD::WaveTypes::WAVE_SAWTOOTH, 2.0, 1.5, 0.0, 0.5, 0.0});
        auto const signal = generator.generate(2.0, 0.01);

        for (double const average: {-0.6, 0.0, 0.4})
        {
            auto const limited = signal.softLimitWithAverage(average, -1.0, 1.0, 4.0);
            CHECK(limited.getAverage() == doctest::Approx(average));
            for (auto const& sample: limited)
            {
                CHECK(sample.y >= -1.0);
                CHECK(sample.y <= 1.0);
            }
        }

        auto const keep = signal.softLimitKeepAverage(-1.0, 1.0, 4.0);
        auto const with = signal.softLimitWithAverage(signal.getAverage(), -1.0, 1.0, 4.0);
        CHECK(keep.getSamples() == with.getSamples());

        CHECK(fge::SignalD{}.softLimitWithAverage(0.5, -1.0, 1.0, 4.0).isEmpty());
    }

    SUBCASE("limited trajectory keeps its final position")
    {
        fge::SignalGeneratorD generator;
        generator.addWave({fge::SignalGeneratorD::WaveTypes::WAVE_SINUSOID, 3.0, 0.5, 0.0, 0.5, 0.0});
        generator.addWave({fge::SignalGeneratorD::WaveTypes::WAVE_SINUSOID, 0.5, 3.0, 30.0, 0.5, 0.0});
        auto const x = generator.generate(4.0, 0.01);
        auto const v = x.derivative();
        auto const a = v.derivative();

        double constexpr aMax = 120.0;
        double constexpr vMax = 15.0;
        double constexpr smoothness = 6.0;

        auto const vLimited = a.softLimitKeepAverage(-aMax, aMax, smoothness)
                                      .integral(v.getSample(0).y)
                                      .softLimitWithAverage(v.getAverage(), -vMax, vMax, smoothness);
        auto const xLimited = vLimited.integral(x.getSample(0).y);

        for (auto const& sample: vLimited)
        {
            CHECK(std::abs(sample.y) <= vMax);
        }
        CHECK(xLimited.getSample(0).y == x.getSample(0).y);
        auto const last = x.getSampleCount() - 1;
        CHECK(std::abs(xLimited.getSample(last).y - x.getSample(last).y) < 0.01);

        //Without the integration constants and the original average, the trajectory drifts
        auto const xDrifting =
                a.softLimit(-aMax, aMax, smoothness).integral().softLimit(-vMax, vMax, smoothness).integral();
        CHECK(std::abs(xDrifting.getSample(last).y - x.getSample(last).y) > 10.0);
    }

    SUBCASE("scale limit")
    {
        fge::SignalD signal({{0.0, -4.0}, {1.0, 0.0}, {2.0, 10.0}, {3.0, 2.0}});

        //The max is limiting: k = 5 / 10
        auto const scaled = signal.scaleLimit(-5.0, 5.0);
        CHECK(scaled.getSample(0).y == doctest::Approx(-2.0));
        CHECK(scaled.getSample(1).y == 0.0);
        CHECK(scaled.getSample(2).y == 5.0);
        CHECK(scaled.getSample(3).y == doctest::Approx(1.0));
        CHECK(scaled.getSample(2).x == 2.0);

        //The min is limiting: k = -1 / -4
        auto const scaledMin = signal.scaleLimit(-1.0, 20.0);
        CHECK(scaledMin.getSample(0).y == -1.0);
        CHECK(scaledMin.getSample(2).y == doctest::Approx(2.5));

        //Already inside the bounds, not modified
        CHECK(signal.scaleLimit(-10.0, 10.0).getSamples() == signal.getSamples());

        //Bounds can touch 0
        auto const positive = signal.scaleLimit(0.0, 5.0);
        CHECK(positive.getSample(0).y == 0.0);
        CHECK(positive.getSample(2).y == 0.0);

        //Always inside the bounds, even with floating-point rounding
        fge::SignalGeneratorD generator;
        generator.addWave({fge::SignalGeneratorD::WaveTypes::WAVE_SINUSOID, 3.7, 1.3, 17.0, 0.5, 0.4});
        for (auto const& sample: generator.generate(3.0, 0.01).scaleLimit(-0.3, 1.1))
        {
            CHECK(sample.y >= -0.3);
            CHECK(sample.y <= 1.1);
        }

        CHECK(fge::SignalD{}.scaleLimit(-1.0, 1.0).isEmpty());
        CHECK_THROWS_AS((void) signal.scaleLimit(1.0, 5.0), fge::Exception);
        CHECK_THROWS_AS((void) signal.scaleLimit(-5.0, -1.0), fge::Exception);
    }

    SUBCASE("time stretch")
    {
        fge::SignalD signal({{1.0, 4.0}, {2.0, 5.0}, {4.0, 6.0}});
        auto const stretched = signal.timeStretch(2.0);
        CHECK(stretched.getSample(0) == fge::SignalD::Sample{1.0, 4.0});
        CHECK(stretched.getSample(1) == fge::SignalD::Sample{3.0, 5.0});
        CHECK(stretched.getSample(2) == fge::SignalD::Sample{7.0, 6.0});

        CHECK_THROWS_AS((void) signal.timeStretch(0.0), fge::Exception);
    }
}

TEST_CASE("testing signal file")
{
    fge::SignalGeneratorF generator;
    generator.addWave({});
    auto const signal = generator.generate(1.0f, 0.01f);

    auto const path = std::filesystem::temp_directory_path() / "fge_test_signal.csv";
    REQUIRE(signal.saveToFile(path));

    fge::SignalF loaded;
    REQUIRE(loaded.loadFromFile(path));
    REQUIRE(loaded.getSamples() == signal.getSamples());

    std::filesystem::remove(path);
    CHECK_FALSE(loaded.loadFromFile(path));
    CHECK(loaded.getSampleCount() == signal.getSampleCount());
}

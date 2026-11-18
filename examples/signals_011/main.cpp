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

#include "FastEngine/C_clock.hpp"
#include "FastEngine/C_random.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extensions/C_objChart.hpp"
#include "FastEngine/extensions/C_signalGenerator.hpp"
#include "FastEngine/graphic/C_renderWindow.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objShape.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_objWindow.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define SIGNAL_DURATION 2.0
#define SIGNAL_SAMPLE_INTERVAL 0.01
#define SIGNAL_LIVE_DURATION 3.0
#define SIGNAL_LIVE_SAMPLE_INTERVAL (1.0 / 30.0)
#define SIGNAL_SOFT_LIMIT_LOWER (-1.0)
#define SIGNAL_SOFT_LIMIT_UPPER 1.0
#define SIGNAL_SOFT_LIMIT_SMOOTHNESS 4.0
#define SIGNAL_RESAMPLE_RATE 20.0
#define SIGNAL_TIME_STRETCH_FACTOR 0.5
#define SIGNAL_CSV_FILE "signal.csv"
#define SIGNAL_MAX_DISPLAYED_WAVES 4

using WaveTypes = fge::SignalGeneratorD::WaveTypes;

enum class Processings
{
    PROCESS_DERIVATIVE,
    PROCESS_INTEGRAL,
    PROCESS_SOFT_LIMIT,
    PROCESS_SOFT_LIMIT_KEEP_AVERAGE,
    PROCESS_RESAMPLE,
    PROCESS_TIME_STRETCH,

    PROCESS_COUNT
};

char const* GetProcessingName(Processings processing)
{
    switch (processing)
    {
    case Processings::PROCESS_DERIVATIVE:
        return "derivative()";
    case Processings::PROCESS_INTEGRAL:
        return "integral()";
    case Processings::PROCESS_SOFT_LIMIT:
        return "softLimit() in [-1, 1]";
    case Processings::PROCESS_SOFT_LIMIT_KEEP_AVERAGE:
        return "softLimitKeepAverage() in [-1, 1]";
    case Processings::PROCESS_RESAMPLE:
        return "resample() at 20 Hz";
    case Processings::PROCESS_TIME_STRETCH:
        return "timeStretch() x0.5";
    default:
        return "unknown";
    }
}

char const* GetWaveName(WaveTypes type)
{
    switch (type)
    {
    case WaveTypes::WAVE_SINUSOID:
        return "sinusoid";
    case WaveTypes::WAVE_RECTANGLE:
        return "rectangle";
    case WaveTypes::WAVE_SAWTOOTH:
        return "sawtooth";
    case WaveTypes::WAVE_TRIANGLE:
        return "triangle";
    case WaveTypes::WAVE_CONTINUOUS:
        return "continuous";
    default:
        return "unknown";
    }
}

fge::SignalGeneratorD::Wave CreateRandomWave(WaveTypes type)
{
    fge::SignalGeneratorD::Wave wave;
    wave._type = type;
    wave._amplitude = fge::_random.range(0.2, 1.0);
    wave._frequency = fge::_random.range(0.5, 4.0);
    wave._phaseDegrees = fge::_random.range(0.0, 360.0);
    wave._dutyCycle = fge::_random.range(0.1, 0.9);
    if (type == WaveTypes::WAVE_CONTINUOUS)
    {
        wave._offset = fge::_random.range(-0.5, 0.5);
    }
    return wave;
}

fge::SignalD ApplyProcessing(fge::SignalD const& signal, Processings processing)
{
    switch (processing)
    {
    case Processings::PROCESS_DERIVATIVE:
        return signal.derivative();
    case Processings::PROCESS_INTEGRAL:
        return signal.integral();
    case Processings::PROCESS_SOFT_LIMIT:
        return signal.softLimit(SIGNAL_SOFT_LIMIT_LOWER, SIGNAL_SOFT_LIMIT_UPPER, SIGNAL_SOFT_LIMIT_SMOOTHNESS);
    case Processings::PROCESS_SOFT_LIMIT_KEEP_AVERAGE:
        return signal.softLimitKeepAverage(SIGNAL_SOFT_LIMIT_LOWER, SIGNAL_SOFT_LIMIT_UPPER,
                                           SIGNAL_SOFT_LIMIT_SMOOTHNESS);
    case Processings::PROCESS_RESAMPLE:
        return signal.resample(SIGNAL_RESAMPLE_RATE);
    case Processings::PROCESS_TIME_STRETCH:
        return signal.timeStretch(SIGNAL_TIME_STRETCH_FACTOR);
    default:
        return signal;
    }
}

//Create the MainScene class
class MainScene : public fge::Scene
{
public:
    void start(fge::RenderWindow& renderWindow)
    {
        fge::Event event(renderWindow);
        fge::GuiElementHandler guiElementHandler(event, renderWindow);
        guiElementHandler.setEventCallback();

        this->setLinkedRenderTarget(&renderWindow);

        //Set default callback context
        this->setCallbackContext({&event, &guiElementHandler});

        //Init texture manager
        if (!fge::texture::gManager.initialize())
        {
            std::cout << "Failed to initialize texture manager" << std::endl;
            return;
        }
        //Init font manager
        if (!fge::font::gManager.initialize())
        {
            std::cout << "Failed to initialize font manager" << std::endl;
            return;
        }

        //Load texture
        fge::texture::gManager.loadFromFile("close", "resources/images/window/close.png");
        fge::texture::gManager.loadFromFile("minimize", "resources/images/window/minimize.png");
        fge::texture::gManager.loadFromFile("resize", "resources/images/window/resize.png");
        fge::texture::gManager.loadFromFile("window", "resources/images/window/window.png");

        //Load font
        fge::font::gManager.loadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto* explainText = this->newObject<fge::ObjText>({FGE_SCENE_PLAN_HIGH_TOP + 1}, "", "base",
                                                          fge::Vector2f{10.0f, 5.0f}, 16);
        explainText->setFillColor(fge::Color::Black);

        //Create a window containing a chart that fill it
        auto const createChart = [this](fge::Vector2f const& position, fge::Vector2f const& size, fge::Color color) {
            auto* objWindow = this->newObject<fge::ObjWindow>({FGE_SCENE_PLAN_HIGH_TOP});
            objWindow->setTextureClose("close");
            objWindow->setTextureMinimize("minimize");
            objWindow->setTextureResize("resize");
            objWindow->setTexture("window");
            objWindow->setSize(size);
            objWindow->setPosition(position);
            objWindow->showExitButton(false);

            auto* objChart = objWindow->_windowScene.newObject<fge::ObjChartD>();
            objChart->setFont("base");
            objChart->setDynamicSize(
                    {{0.0f, 0.0f}, {fge::DynamicSize::SizeModes::SIZE_AUTO, fge::DynamicSize::SizeModes::SIZE_AUTO}});
            objChart->setPointSize(3.0f);
            objChart->setDefaultPointColor(color);
            objChart->setLineColor(color);
            return objChart;
        };

        auto* generatedChart = createChart({20.0f, 170.0f}, {570.0f, 300.0f}, fge::Color{30, 110, 220});
        auto* processedChart = createChart({610.0f, 170.0f}, {570.0f, 300.0f}, fge::Color{220, 70, 50});
        auto* liveChart = createChart({20.0f, 490.0f}, {1160.0f, 210.0f}, fge::Color{40, 160, 80});
        liveChart->enableAnimation(false);

        //Create the generator with some default waves
        fge::SignalGeneratorD generator;
        generator.addWave({WaveTypes::WAVE_SINUSOID, 1.0, 1.0, 0.0, 0.5, 0.0});
        generator.addWave({WaveTypes::WAVE_SINUSOID, 0.2, 6.0, 0.0, 0.5, 0.0});

        auto processing = Processings::PROCESS_SOFT_LIMIT_KEEP_AVERAGE;
        fge::SignalD signal;
        std::string processingError;
        std::string saveStatus;

        auto const updateExplainText = [&]() {
            std::ostringstream waves;
            waves << std::fixed << std::setprecision(2);
            for (std::size_t i = 0; i < generator.getWaveCount() && i < SIGNAL_MAX_DISPLAYED_WAVES; ++i)
            {
                auto const& wave = generator.getWave(i);
                waves << (i == 0 ? "" : ", ") << GetWaveName(wave._type);
                if (wave._type == WaveTypes::WAVE_CONTINUOUS)
                {
                    waves << ' ' << wave._offset;
                }
                else
                {
                    waves << ' ' << wave._frequency << "Hz";
                }
            }
            if (generator.getWaveCount() > SIGNAL_MAX_DISPLAYED_WAVES)
            {
                waves << ", ...";
            }

            explainText->setString(
                    std::string{"Top left: generated signal, top right: processed signal, bottom: live value with "
                                "getValue()\n"
                                "1-5: add a random wave (sinusoid, rectangle, sawtooth, triangle, continuous)\n"
                                "BACKSPACE: remove the last wave, C: clear the waves\n"
                                "P: change the processing ("} +
                    GetProcessingName(processing) +
                    ")\n"
                    "S: save the generated signal in " SIGNAL_CSV_FILE +
                    saveStatus + "\nWaves (" + std::to_string(generator.getWaveCount()) + "): " + waves.str() +
                    (processingError.empty() ? "" : "\nProcessing error: " + processingError));
        };

        //Generate the signal, process it and update the charts
        auto const refresh = [&]() {
            signal = generator.generate(SIGNAL_DURATION, SIGNAL_SAMPLE_INTERVAL);
            generatedChart->setPoints(signal.getSamples());

            processingError.clear();
            try
            {
                processedChart->setPoints(ApplyProcessing(signal, processing).getSamples());
            }
            catch (fge::Exception const& e)
            {
                processingError = e.what();
                processedChart->clearPoints();
            }

            updateExplainText();
        };
        refresh();

        event._onKeyDown.addLambda([&]([[maybe_unused]] fge::Event const& evt, SDL_KeyboardEvent const& keyEvent) {
            saveStatus.clear();

            switch (keyEvent.key)
            {
            case SDLK_1:
                generator.addWave(CreateRandomWave(WaveTypes::WAVE_SINUSOID));
                break;
            case SDLK_2:
                generator.addWave(CreateRandomWave(WaveTypes::WAVE_RECTANGLE));
                break;
            case SDLK_3:
                generator.addWave(CreateRandomWave(WaveTypes::WAVE_SAWTOOTH));
                break;
            case SDLK_4:
                generator.addWave(CreateRandomWave(WaveTypes::WAVE_TRIANGLE));
                break;
            case SDLK_5:
                generator.addWave(CreateRandomWave(WaveTypes::WAVE_CONTINUOUS));
                break;
            case SDLK_BACKSPACE:
                if (generator.getWaveCount() > 0)
                {
                    generator.removeWave(generator.getWaveCount() - 1);
                }
                break;
            case SDLK_C:
                generator.clearWaves();
                break;
            case SDLK_P:
                processing = static_cast<Processings>((static_cast<int>(processing) + 1) %
                                                      static_cast<int>(Processings::PROCESS_COUNT));
                break;
            case SDLK_S:
                saveStatus = signal.saveToFile(SIGNAL_CSV_FILE) ? " (saved)" : " (failed to save)";
                updateExplainText();
                return;
            default:
                return;
            }
            refresh();
        });

        fge::Clock liveClock;
        double lastLiveTime = -SIGNAL_LIVE_SAMPLE_INTERVAL;

        //Begin loop
        bool running = true;
        while (running)
        {
            //Update event
            event.process();
            if (event.isEventType(SDL_EVENT_QUIT))
            {
                running = false;
            }
            if (event.isEventType(SDL_EVENT_WINDOW_RESIZED))
            {
                auto view = renderWindow.getView();
                view.resizeFixCenter({event.getWindowSize().x, event.getWindowSize().y});
                renderWindow.setView(view);
                guiElementHandler.checkViewSize();
            }

            //Evaluate the generator in real time, keeping only the last seconds
            double const liveTime = std::chrono::duration<double>(liveClock.getElapsedTime()).count();
            if (liveTime - lastLiveTime >= SIGNAL_LIVE_SAMPLE_INTERVAL)
            {
                lastLiveTime = liveTime;
                liveChart->addPoint({liveTime, generator.getValue(liveTime)});
                while (liveChart->getPointCount() > 0 && liveChart->getPoint(0).x < liveTime - SIGNAL_LIVE_DURATION)
                {
                    liveChart->removePoint(0);
                }
            }

            //Update scene
            auto deltaTick = tick.restart();
            this->update(renderWindow, event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTick));

            //Drawing
            auto imageIndex = renderWindow.prepareNextFrame(nullptr, FGE_RENDER_TIMEOUT_BLOCKING);
            if (imageIndex != FGE_RENDER_BAD_IMAGE_INDEX)
            {
                fge::vulkan::GetActiveContext()._garbageCollector.setCurrentFrame(renderWindow.getCurrentFrame());

                renderWindow.beginRenderPass(imageIndex);

                this->draw(renderWindow);

                renderWindow.endRenderPass();

                renderWindow.display(imageIndex);
            }
        }

        fge::vulkan::GetActiveContext().waitIdle();

        fge::vulkan::GetActiveContext()._garbageCollector.enable(false);
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    using namespace fge::vulkan;

    auto instance = Context::init(SDL_INIT_VIDEO | SDL_INIT_EVENTS, "example 011: signals");
    Context::enumerateExtensions();

    SurfaceSDLWindow window(instance, FGE_WINDOWPOS_CENTERED, {1200, 720}, SDL_WINDOW_RESIZABLE);

    // Check that the window was successfully created
    if (!window.isCreated())
    {
        // In the case that the window could not be made...
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    Context vulkanContext(window);
    vulkanContext._garbageCollector.enable(true);

    if (!fge::shader::gManager.initialize())
    {
        std::cout << "Could not initialize shader manager" << std::endl;
        return 1;
    }
    fge::shader::gManager.loadFromFile(FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX,
                                       "resources/shaders/objShapeInstances_vertex.vert", Shader::Type::SHADER_VERTEX,
                                       fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::gManager.loadFromFile(FGE_OBJSPRITEBATCHES_SHADER_FRAGMENT,
                                       "resources/shaders/objSpriteBatches_fragment.frag",
                                       Shader::Type::SHADER_FRAGMENT, fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::gManager.loadFromFile(FGE_OBJSPRITEBATCHES_SHADER_VERTEX,
                                       "resources/shaders/objSpriteBatches_vertex.vert", Shader::Type::SHADER_VERTEX,
                                       fge::shader::ShaderInputTypes::SHADER_GLSL);

    fge::RenderWindow renderWindow(vulkanContext, window);
    renderWindow.setClearColor(fge::Color::White);

    std::unique_ptr<MainScene> scene = std::make_unique<MainScene>();
    scene->start(renderWindow);
    scene.reset();

    fge::texture::gManager.uninitialize();
    fge::font::gManager.uninitialize();
    fge::shader::gManager.uninitialize();

    renderWindow.destroy();

    vulkanContext.destroy();

    window.destroy();
    instance.destroy();
    SDL_Quit();

    return 0;
}

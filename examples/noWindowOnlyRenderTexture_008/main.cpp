/*
 * Copyright 2025 Guillaume Guillet
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
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/graphic/C_drawable.hpp"
#include "FastEngine/graphic/C_renderTexture.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objRectangleShape.hpp"
#include "FastEngine/object/C_objSlider.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "SDL.h"
#include <cmath>
#include <iostream>

//Create the MainScene class
class MainScene : public fge::Scene
{
public:
    void start(fge::RenderTexture& renderTexture)
    {
        fge::Event event;

        this->setLinkedRenderTarget(&renderTexture);

        //Init texture manager
        fge::texture::gManager.initialize();
        //Init font manager
        fge::font::gManager.initialize();

        //Load font
        fge::font::gManager.loadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto* explainText = this->newObject<fge::ObjText>(
                {FGE_SCENE_PLAN_HIGH_TOP + 1}, "All of this scene should be rendered inside a texture in the GPU",
                "base", fge::Vector2f{}, 18);
        explainText->setFillColor(fge::Color::Black);

        //Add a text with characters that will be moved
        auto* movingText = this->newObject<fge::ObjText>("hello world, I'm a super text !\ttab\nnewLine", "base",
                                                         fge::Vector2f{200.0f, 200.0f});
        movingText->setFillColor(fge::Color::Black);
        movingText->setOutlineThickness(2.0f);
        movingText->setOutlineColor(fge::Color::Yellow);
        movingText->setStyle(fge::ObjText::Style::Italic | fge::ObjText::Style::StrikeThrough |
                             fge::ObjText::Style::Bold | fge::ObjText::Style::Underlined);

        //Add a rectangle representing the bounds of the moving text
        auto* rectText = this->newObject<fge::ObjRectangleShape>();

        auto rect = movingText->getGlobalBounds();
        rectText->setPosition(rect.getPosition());
        rectText->setSize(rect.getSize());
        rectText->setFillColor(fge::Color::Transparent);
        rectText->setOutlineColor(fge::Color::Red);
        rectText->setOutlineThickness(2.0f);

        //Update scene
        auto deltaTick = tick.restart();
        this->update(renderTexture, event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTick));

        //Update text characters
        auto& characters = movingText->getCharacters();
        for (auto& c: characters)
        {
            c.setFillColor(fge::_random.randColor());
            c.setOutlineColor(fge::_random.randColor());
        }

        //Drawing
        auto imageIndex = renderTexture.prepareNextFrame(nullptr, FGE_RENDER_TIMEOUT_BLOCKING);
        if (imageIndex != FGE_RENDER_BAD_IMAGE_INDEX)
        {
            fge::vulkan::GetActiveContext()._garbageCollector.setCurrentFrame(renderTexture.getCurrentFrame());

            renderTexture.beginRenderPass(imageIndex);

            this->draw(renderTexture);

            renderTexture.endRenderPass();

            renderTexture.display(imageIndex);
        }

        fge::vulkan::GetActiveContext().waitIdle();

        fge::vulkan::GetActiveContext()._garbageCollector.enable(false);

        fge::Surface const surface{renderTexture.getTextureImage().copyToSurface()};
        if (surface.saveToFile("output.png"))
        {
            std::cout << "file is successfully saved to ./output.png" << std::endl;
        }
        else
        {
            std::cout << "error saving file" << std::endl;
        }
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    using namespace fge::vulkan;

    auto instance = Context::init(SDL_INIT_VIDEO | SDL_INIT_EVENTS, "example 008: noWindowOnlyRenderTexture");
    Context::enumerateExtensions();

    Context vulkanContext;
    vulkanContext.initVulkanSurfaceless(instance);
    vulkanContext._garbageCollector.enable(true);

    fge::shader::gManager.initialize();
    fge::shader::gManager.loadFromFile(
            FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX, "resources/shaders/objShapeInstances_vertex.vert",
            fge::vulkan::Shader::Type::SHADER_VERTEX, fge::shader::ShaderInputTypes::SHADER_GLSL);

    fge::RenderTexture renderTexture({800, 600}, vulkanContext);
    renderTexture.setClearColor(fge::Color::White);

    std::unique_ptr<MainScene> scene = std::make_unique<MainScene>();
    scene->start(renderTexture);
    scene.reset();

    fge::texture::gManager.uninitialize();
    fge::font::gManager.uninitialize();
    fge::shader::gManager.uninitialize();

    renderTexture.destroy();

    vulkanContext.destroy();

    instance.destroy();
    SDL_Quit();

    return 0;
}
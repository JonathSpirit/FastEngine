#include <iostream>
#include <vector>
#include <map>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include <cmath>
#include "volk.h"
#include <glm/ext.hpp>
#include "SDL.h"
#include "SDL_video.h"
#include "SDL_vulkan.h"
#include "FastEngine/graphic/C_color.hpp"

#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/graphic/C_surface.hpp"
#include "FastEngine/graphic/C_rect.hpp"
#include "FastEngine/graphic/C_renderWindow.hpp"
#include "FastEngine/graphic/C_renderTexture.hpp"
#include "FastEngine/graphic/C_transformable.hpp"

const std::vector<fge::vulkan::Vertex> verticesTexture = {
        {{0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{512.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{512.0f, 512.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        {{0.0f, 512.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};
const std::vector<uint16_t> indicesTexture = {
        0, 1, 2, 2, 3, 0
};

const std::vector<fge::vulkan::Vertex> vertices = {
        {{-50.0f, -50.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{50.0f, -50.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{50.0f, 50.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-50.0f, 50.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};
const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
};

const std::vector<fge::vulkan::Vertex> vertices2 = {
        {{-20.0f, -20.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{20.0f, -20.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{20.0f, 20.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-20.0f, 20.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};
const std::vector<uint16_t> indices2 = {
        0, 1, 2, 2, 3, 0
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window* window = SDL_CreateWindow("SDL Vulkan Sample", SDL_WINDOWPOS_CENTERED,  SDL_WINDOWPOS_CENTERED,
                                          640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    // Check that the window was successfully created
    if (window == nullptr)
    {
        // In the case that the window could not be made...
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    fge::vulkan::Context vulkanContext{};
    fge::vulkan::Context::initVolk();
    fge::vulkan::Context::enumerateExtensions();
    vulkanContext.initVulkan(window);

    fge::vulkan::GlobalContext = &vulkanContext;

    fge::RenderWindow renderWindow(vulkanContext);

    fge::Surface surface1;
    surface1.loadFromFile("textures/texture.jpg");
    fge::Surface surface2;
    surface2.loadFromFile("textures/texture2.jpg");

    fge::vulkan::TextureImage texture1;
    texture1.create(vulkanContext, surface1.get());
    fge::vulkan::TextureImage texture2;
    texture2.create(vulkanContext, surface2.get());

    fge::Surface testSurface;
    testSurface.create(32, 32, {255,0,0,255});

    texture1.update(testSurface.get(), {33,0});

    fge::RenderTexture renderTexture;
    renderTexture.create(vulkanContext, {200,200});

    std::cout << "Hello, World!" << std::endl;

    SDL_Event evt;

    fge::vulkan::Shader vertShader;
    vertShader.loadFromFile(vulkanContext.getLogicalDevice(), "shaders/vertex.spv", fge::vulkan::Shader::Type::SHADER_VERTEX);
    fge::vulkan::Shader fragShader;
    fragShader.loadFromFile(vulkanContext.getLogicalDevice(), "shaders/fragmentTexture.spv", fge::vulkan::Shader::Type::SHADER_FRAGMENT);

    const VkExtent2D extent2D{renderWindow.getSize().x, renderWindow.getSize().y};

    fge::vulkan::GraphicPipeline graphicPipeline;
    graphicPipeline.setViewport(extent2D);
    graphicPipeline.setShader(vertShader);
    graphicPipeline.setShader(fragShader);
    fge::vulkan::VertexBuffer vertexBuffer;
    vertexBuffer.create(vulkanContext, vertices.size(), indices.size(), true);
    memcpy(vertexBuffer.getVertices(), vertices.data(), sizeof(fge::vulkan::Vertex)*vertices.size());
    memcpy(vertexBuffer.getIndices(), indices.data(), sizeof(uint16_t)*indices.size());
    vertexBuffer.mapVertices();
    vertexBuffer.mapIndices();
    graphicPipeline.setVertexBuffer(&vertexBuffer);

    fge::vulkan::GraphicPipeline graphicPipeline2;
    graphicPipeline2.setViewport(extent2D);
    graphicPipeline2.setShader(vertShader);
    graphicPipeline2.setShader(fragShader);
    fge::vulkan::VertexBuffer vertexBuffer2;
    vertexBuffer2.create(vulkanContext, vertices2.size(), indices2.size(), true);
    memcpy(vertexBuffer2.getVertices(), vertices2.data(), sizeof(fge::vulkan::Vertex)*vertices2.size());
    memcpy(vertexBuffer2.getIndices(), indices2.data(), sizeof(uint16_t)*indices2.size());
    vertexBuffer2.mapVertices();
    vertexBuffer2.mapIndices();
    graphicPipeline2.setVertexBuffer(&vertexBuffer2);

    fge::vulkan::GraphicPipeline graphicPipeline3;
    graphicPipeline3.setViewport(extent2D);
    graphicPipeline3.setShader(vertShader);
    graphicPipeline3.setShader(fragShader);
    fge::vulkan::VertexBuffer vertexBuffer3;
    vertexBuffer3.create(vulkanContext, vertices2.size(), indices2.size(), true);
    memcpy(vertexBuffer3.getVertices(), vertices2.data(), sizeof(fge::vulkan::Vertex)*vertices2.size());
    memcpy(vertexBuffer3.getIndices(), indices2.data(), sizeof(uint16_t)*indices2.size());
    vertexBuffer3.mapVertices();
    vertexBuffer3.mapIndices();
    graphicPipeline3.setVertexBuffer(&vertexBuffer3);

    fge::vulkan::GraphicPipeline graphicPipeline4;
    graphicPipeline4.setViewport(extent2D);
    graphicPipeline4.setShader(vertShader);
    graphicPipeline4.setShader(fragShader);
    fge::vulkan::VertexBuffer vertexBuffer4;
    vertexBuffer4.create(vulkanContext, verticesTexture.size(), indicesTexture.size(), true);
    memcpy(vertexBuffer4.getVertices(), verticesTexture.data(), sizeof(fge::vulkan::Vertex)*verticesTexture.size());
    memcpy(vertexBuffer4.getIndices(), indicesTexture.data(), sizeof(uint16_t)*indicesTexture.size());
    vertexBuffer4.mapVertices();
    vertexBuffer4.mapIndices();
    graphicPipeline4.setVertexBuffer(&vertexBuffer4);
    fge::Transformable transformable4;
    transformable4.setScale({0.3f, 0.2f});
    transformable4.setRotation(-8.8f);
    transformable4.setOrigin({80.0f, -30.0f});
    transformable4.setPosition({-20.0f, 18.3f});

    fge::Transformable transformable3;

    fge::Transformable transformable2;
    fge::Transformable transformable1;
    transformable2.setPosition({200.0f, -200.0f});
    transformable1.setPosition({200.0f, -200.0f});

    fge::Surface testCopySurface(texture1.copyToSurface());
    testCopySurface.saveToFile("myCopiedSurface.png");

    float t = 0.0f;

    auto view = renderWindow.getView();
    view.rotate(-20.0f);
    view.move({-360.0f, -280.0f});
    renderWindow.setView(view);

    bool running = true;
    while (running)
    {
        while (SDL_PollEvent(&evt) != 0)
        {
            if (evt.type == SDL_QUIT)
            {
                running = false;
            }
        }

        auto imageIndex = renderWindow.prepareNextFrame(nullptr);
        if (imageIndex != BAD_IMAGE_INDEX)
        {
            renderWindow.beginRenderPass(imageIndex);

            auto inheritanceInfo = renderWindow.getInheritanceInfo(imageIndex);

            (void)renderTexture.prepareNextFrame(&inheritanceInfo);
            renderTexture.beginRenderPass(0);
            renderTexture.draw(graphicPipeline3, fge::RenderStates{transformable3.getTransform(), &transformable3, &texture1});
            renderTexture.endRenderPass();
            renderTexture.display(0, nullptr, 0);

            ///renderTexture.executeCommands(renderWindow.getCommandBuffer());

            renderWindow.draw(graphicPipeline, fge::RenderStates{transformable1.getTransform(), &transformable1, &renderTexture.getTextureImage()});
            renderWindow.draw(graphicPipeline2, fge::RenderStates{transformable2.getTransform(), &transformable2, &texture2});
            renderWindow.draw(graphicPipeline4, fge::RenderStates{transformable4.getTransform(), &transformable4, &texture1});
            renderWindow.endRenderPass();

            auto extraCommandBuffer = renderTexture.getCommandBuffer();
            renderWindow.display(imageIndex, &extraCommandBuffer, 1);

            //uboTest._shift.y = std::sin(2.0f*3.14f*500.0f*t);
            //renderWindow.getUniformBuffers()[0].copyData(&uboTest, sizeof(uboTest));
            //renderWindow.getUniformBuffers()[0].copyData(glm::value_ptr(test), sizeof(test));
        }

        t = fmodf(t+0.01f, 100.0f);
        SDL_Delay(33);
    }

    vulkanContext.waitIdle();

    texture1.destroy();
    texture2.destroy();

    transformable1.destroy();
    transformable2.destroy();
    transformable4.destroy();
    transformable3.destroy();

    renderTexture.destroy();

    renderWindow.destroy();

    vertShader.destroy();
    fragShader.destroy();

    vertexBuffer.destroy();
    vertexBuffer2.destroy();
    vertexBuffer3.destroy();
    vertexBuffer4.destroy();

    graphicPipeline.destroy();
    graphicPipeline2.destroy();
    graphicPipeline3.destroy();
    graphicPipeline4.destroy();

    vulkanContext.destroy();

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;
}

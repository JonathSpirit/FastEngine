/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/manager/crash_manager.hpp"
#include <exception>

namespace fge::crash
{

namespace
{

signed char const _computerCrash[] = {
        -119, 80,   78,   71,   13,  10,  26,   10,  0,    0,    0,   13,   73,   72,   68,   82,   0,    0,    0,
        16,   0,    0,    0,    16,  8,   6,    0,   0,    0,    31,  -13,  -1,   97,   0,    0,    0,    6,    98,
        75,   71,   68,   0,    0,   0,   0,    0,   0,    -7,   67,  -69,  127,  0,    0,    0,    9,    112,  72,
        89,   115,  0,    0,    11,  19,  0,    0,   11,   19,   1,   0,    -102, -100, 24,   0,    0,    0,    7,
        116,  73,   77,   69,   7,   -31, 9,    21,  17,   39,   5,   -52,  80,   40,   -10,  0,    0,    0,    29,
        105,  84,   88,   116,  67,  111, 109,  109, 101,  110,  116, 0,    0,    0,    0,    0,    67,   114,  101,
        97,   116,  101,  100,  32,  119, 105,  116, 104,  32,   71,  73,   77,   80,   100,  46,   101,  7,    0,
        0,    0,    126,  73,   68,  65,  84,   56,  -53,  -91,  83,  65,   14,   -64,  32,   8,    107,  -119, -57,
        -19,  -1,   15,   -11,  1,   -35, -55,  5,   -55,  68,   -89, 92,   -116, 82,   -79,  -108, 74,   92,   -126,
        42,   -124, -115, -32,  13,  82,  -110, 120, -125, 59,   5,   84,   33,   -13,  27,   -65,  -114, 46,   68,
        -116, 121,  58,   -86,  72,  -39, 124,  97,  58,   6,    13,  -112, 49,   -120, -104, 84,   -125, 6,    -52,
        -14,  101,  -90,  -14,  76,  72,  -61,  97,  -108, -116, -6,  10,   35,   74,   -22,  -6,   -52,  38,   -31,
        115,  -81,  62,   -83,  -64, 106, -49,  67,  31,   -52,  76,  52,   -62,  88,   124,  121,  -59,  -119, -2,
        -114, -59,  -125, -103, 19,  35,  -58,  -2,  -50,  61,   98,  120,  -6,   -99,  31,   114,  -58,  90,   52,
        -97,  -8,   91,   29,   0,   0,   0,    0,   73,   69,   78,  68,   -82,  66,   96,   -126};

char _crashInfoText[200] = "I'm sorry but your app crash :(";
sf::RenderWindow* _crashScreen = nullptr;
sf::Font* _crashFont = nullptr;

void CrashFunction()
{
    _crashScreen->close();
    _crashScreen->create(sf::VideoMode(400, 400), "Crash !", sf::Style::Default);

    std::string textError = "Unknown error";

    if (auto exc = std::current_exception())
    {
        try
        {
            std::rethrow_exception(exc);
        }
        catch (std::exception const& e)
        {
            textError = e.what();
        }
        catch (std::string const& e)
        {
            textError = e;
        }
        catch (char const*& e)
        {
            textError = e;
        }
        catch (...)
        {}
    }
    if (textError.size() > 50)
    {
        textError.insert(textError.begin() + 50, '\n');
    }

    sf::Text txtInfo;
    sf::Text errMsg;
    sf::Sprite sptDeadComputer;
    sf::Texture tDeadComputer;

    tDeadComputer.loadFromMemory(_computerCrash, sizeof(_computerCrash));

    sptDeadComputer.setTexture(tDeadComputer);
    sptDeadComputer.setOrigin(8, 8);
    sptDeadComputer.setScale(3, 3);
    sptDeadComputer.setPosition(200, 60);

    txtInfo.setFont(*_crashFont);
    txtInfo.setFillColor(sf::Color::White);
    txtInfo.setPosition(10, 120);
    txtInfo.setCharacterSize(18);
    txtInfo.setString(_crashInfoText);

    errMsg.setFont(*_crashFont);
    errMsg.setFillColor(sf::Color::White);
    errMsg.setPosition(10, 180);
    errMsg.setCharacterSize(14);
    errMsg.setString(textError);

    while (_crashScreen->isOpen())
    {
        sf::Event event{};
        while (_crashScreen->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                _crashScreen->close();
            }
        }
        _crashScreen->clear(sf::Color::Blue);

        _crashScreen->draw(sptDeadComputer);
        _crashScreen->draw(txtInfo);
        _crashScreen->draw(errMsg);

        _crashScreen->display();
    }

    std::abort();
}

} // namespace

void Init(sf::RenderWindow& screen, sf::Font& font)
{
    _crashScreen = &screen;
    _crashFont = &font;

    std::set_terminate(CrashFunction);
}

void SetInfoText(std::string const& txt)
{
    for (std::size_t i = 0; i < txt.size(); ++i)
    {
        _crashInfoText[i] = txt[i];
    }
    _crashInfoText[txt.size()] = '\0';
}

} // namespace fge::crash

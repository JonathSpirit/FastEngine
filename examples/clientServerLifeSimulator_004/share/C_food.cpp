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

#include "C_food.hpp"
#include "FastEngine/C_random.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace ls
{

Food::Food(fge::Vector2f const& pos)
{
    this->setPosition(pos);
}

void Food::first([[maybe_unused]] fge::Scene& scene)
{
    this->_nutrition = fge::_random.range(1, 20);
    this->setOrigin({24, 19});

    this->networkRegister();

    this->g_circleShape.setRadius(8.0f);
    this->g_circleShape.setFillColor(fge::Color::Green);
    this->g_circleShape.setOutlineColor(fge::Color::Black);
    this->g_circleShape.setOutlineThickness(1.0f);
    this->g_circleShape.setOrigin({8, 8});
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(Food)
{
    auto copyStates = states.copy();
    copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));
    this->g_circleShape.draw(target, copyStates);
}
#endif

void Food::networkRegister()
{
    this->_netList.clear();
    this->_netList.pushTrivial<fge::Vector2f>(&this->getPosition(),
                                              [&](fge::Vector2f const& pos) { this->setPosition(pos); });
}

void Food::save(nlohmann::json& jsonObject)
{
    fge::Object::save(jsonObject);
}
void Food::load(nlohmann::json& jsonObject, std::filesystem::path const& filePath)
{
    fge::Object::load(jsonObject, filePath);
}
void Food::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);
    pck << this->_nutrition;
}
void Food::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);
    pck >> this->_nutrition;
}

char const* Food::getClassName() const
{
    return "LS:OBJ:FOOD";
}
char const* Food::getReadableClassName() const
{
    return "food";
}

} // namespace ls

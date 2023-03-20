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

#include "C_drink.hpp"
#include "FastEngine/C_random.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace ls
{

Drink::Drink(const fge::Vector2f& pos)
{
    this->setPosition(pos);
}

void Drink::first([[maybe_unused]] fge::Scene* scene)
{
    this->_nutrition = fge::_random.range(1, 20);
    this->setOrigin({24, 19});

    this->networkRegister();

    this->g_circleShape.setRadius(8.0f);
    this->g_circleShape.setFillColor(fge::Color::Blue);
    this->g_circleShape.setOutlineColor(fge::Color::Black);
    this->g_circleShape.setOutlineThickness(1.0f);
    this->g_circleShape.setOrigin({8, 8});
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(Drink)
{
    auto copyStates = states.copy(this->_transform.start(*this, states._resTransform.get()));
    target.draw(this->g_circleShape, copyStates);
}
#endif

void Drink::networkRegister()
{
    this->_netList.clear();
    this->_netList.push(new fge::net::NetworkType<fge::Vector2f>(
            {&this->getPosition(), [&](const fge::Vector2f& pos) { this->setPosition(pos); }}));
}

void Drink::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);
}
void Drink::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);
}
void Drink::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);
    pck << this->_nutrition;
}
void Drink::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);
    pck >> this->_nutrition;
}

const char* Drink::getClassName() const
{
    return "LS:OBJ:DRINK";
}
const char* Drink::getReadableClassName() const
{
    return "drink";
}

} // namespace ls

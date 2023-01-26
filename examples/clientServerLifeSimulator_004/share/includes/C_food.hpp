/*
 * Copyright 2022 Guillaume Guillet
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

#ifndef _EXFGE_C_FOOD_HPP_INCLUDED
#define _EXFGE_C_FOOD_HPP_INCLUDED

#include "C_customObject.hpp"
#include "FastEngine/graphic/C_circleShape.hpp"

namespace ls
{

class Food : public ls::CustomObject
{
public:
    Food() = default;
    explicit Food(const fge::Vector2f& pos);
    ~Food() override = default;

    void first(fge::Scene* scene) override;
    FGE_OBJ_DRAW_DECLARE
    void networkRegister() override;

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    uint8_t _nutrition{0};

private:
    fge::CircleShape g_circleShape;
};

} // namespace ls

#endif // _EXFGE_C_FOOD_HPP_INCLUDED

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

#ifndef _EXFGE_C_CREATURE_HPP_INCLUDED
#define _EXFGE_C_CREATURE_HPP_INCLUDED

#include "C_customObject.hpp"
#include "FastEngine/C_animation.hpp"
#include "FastEngine/C_clock.hpp"
#include "FastEngine/C_font.hpp"
#include "FastEngine/C_soundBuffer.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/network/C_packet.hpp"
#include "FastEngine/object/C_objCircleShape.hpp"
#include "FastEngine/object/C_objRectangleShape.hpp"
#include "FastEngine/object/C_objSprite.hpp"
#include "FastEngine/object/C_objTextList.hpp"
#include <queue>

namespace ls
{

enum class CreatureGender : uint8_t
{
    GENDER_MALE,
    GENDER_FEMALE
};

class Creature;

struct CreatureData
{
    CreatureData();

    void networkRegister(fge::net::NetworkTypeContainer& netList, Creature* creature, void (Creature::*callback)());

    uint8_t _lifePoint;

    CreatureGender _gender;

    uint8_t _hunger;
    uint8_t _thirst;

    uint8_t _libido;
    uint8_t _libidoAdd;
    bool _pregnant;

    uint8_t _energy;

    uint8_t _height;
    uint8_t _muscularMass;
    uint8_t _bodyFat;

    float _sightRadius;
};

struct Action
{
    enum class Types : uint8_t
    {
        ACTION_EAT,
        ACTION_DRINK,
        ACTION_MAKEBABY
    };

    Action::Types _type;
    fge::ObjectSid _target;
};

fge::net::Packet& operator<<(fge::net::Packet& pck, CreatureData const& data);
fge::net::Packet const& operator>>(fge::net::Packet const& pck, CreatureData& data);

class Creature : public ls::CustomObject, public fge::Subscriber
{
public:
    Creature() = default;
    explicit Creature(fge::Vector2f const& pos);
    ~Creature() override = default;

    void first(fge::Scene* scene) override;
    bool worldTick() override;
    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE
    void networkRegister() override;

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet const& pck) override;

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

    fge::Font _font;

#ifndef FGE_DEF_SERVER
    fge::Clock _speakClock;
    fge::SoundBuffer _speakSound;
    std::chrono::milliseconds _speakDelay;

    fge::Animation _anim;
#endif // FGE_DEF_SERVER

    std::chrono::microseconds _timeAnimation{0};
    std::chrono::microseconds _timeRandomMove{0};
    std::chrono::microseconds _timePregnant{0};
    std::queue<Action> _actionQueue;

    CreatureData _data;

private:
    fge::ObjRectangleShape g_rectInfoBox;
    fge::ObjRectangleShape g_rectBarLife;
    fge::ObjRectangleShape g_rectBarHunger;
    fge::ObjRectangleShape g_rectBarThirst;
    fge::ObjRectangleShape g_rectBarLibido;

    void refreshStats();

    mutable fge::ObjSprite g_spriteCreature;
    fge::texture::TextureDataPtr g_animTexture;

    fge::ObjCircleShape g_circleSight;

    fge::ObjText g_txtGender;
};

} // namespace ls

#endif // _EXFGE_C_CREATURE_HPP_INCLUDED

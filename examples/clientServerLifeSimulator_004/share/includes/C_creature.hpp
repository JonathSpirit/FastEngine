#ifndef _EXFGE_C_CREATURE_HPP_INCLUDED
#define _EXFGE_C_CREATURE_HPP_INCLUDED

#include <C_customObject.hpp>
#include <FastEngine/C_animation.hpp>
#include <FastEngine/C_font.hpp>
#include <FastEngine/C_packet.hpp>
#include <queue>
#include <SFML/Audio.hpp>

namespace ls
{

enum class CreatureGender : uint8_t
{
    GENDER_MALE,
    GENDER_FEMALE
};

struct CreatureData
{
    CreatureData();

    void networkRegister(fge::net::NetworkTypeContainer& netList);

    uint8_t _lifePoint;

    CreatureGender _gender;

    uint8_t _hunger;
    uint8_t _thirst;

    uint8_t _libido;
    uint8_t _libidoAdd;

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

fge::net::Packet& operator<<(fge::net::Packet& pck, const CreatureData& data);
const fge::net::Packet& operator>>(const fge::net::Packet& pck, CreatureData& data);

class Creature : public ls::CustomObject
{
public:
    Creature() = default;
    explicit Creature(const sf::Vector2f& pos);
    ~Creature() override = default;

    void first(fge::Scene* scene) override;
    bool worldTick() override;
    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE
    void networkRegister() override;

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

#ifndef FGE_DEF_SERVER
    fge::Font _font;

    sf::Clock _speakClock;
    sf::Sound _speakSound;
    sf::Time _speakDelay;

    fge::Animation _anim;
#endif // FGE_DEF_SERVER

    std::chrono::milliseconds _timeAnimation{0};
    std::chrono::milliseconds _timeRandomMove{0};
    std::queue<Action> _actionQueue;

    CreatureData _data;
};

}//end ls

#endif // _EXFGE_C_CREATURE_HPP_INCLUDED
#ifndef _EXFGE_C_DRINK_HPP_INCLUDED
#define _EXFGE_C_DRINK_HPP_INCLUDED

#include <C_customObject.hpp>

namespace ls
{

class Drink : public ls::CustomObject
{
public:
    Drink() = default;
    explicit Drink(const sf::Vector2f& pos);
    ~Drink() override = default;

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
};

}//end ls

#endif // _EXFGE_C_DRINK_HPP_INCLUDED

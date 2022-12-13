#ifndef _EXFGE_C_FOOD_HPP_INCLUDED
#define _EXFGE_C_FOOD_HPP_INCLUDED

#include <C_customObject.hpp>

namespace ls
{

class Food : public ls::CustomObject
{
public:
    Food() = default;
    explicit Food(const sf::Vector2f& pos);
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
};

} // namespace ls

#endif // _EXFGE_C_FOOD_HPP_INCLUDED

#ifndef _FGE_C_OBJSWITCH_HPP_INCLUDED
#define _FGE_C_OBJSWITCH_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_flag.hpp>
#include <FastEngine/C_objSprite.hpp>

#define FGE_OBJSWITCH_CLASSNAME "FGE:OBJ:SWITCH"

namespace fge
{

class FGE_API ObjSwitch : public fge::Object
{
public:
    ObjSwitch();
    ObjSwitch(const fge::Texture& t_on, const fge::Texture& t_off, const sf::Vector2f& pos=sf::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSwitch)

    const fge::Texture& getTextureOn() const;
    const fge::Texture& getTextureOff() const;
    void setTextureOn(const fge::Texture& t_on);
    void setTextureOff(const fge::Texture& t_off);

    void setColor(const sf::Color& color);

    void setActiveStat(bool active);
    bool getActiveStat() const;

    void update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr) override;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void save(nlohmann::json& jsonObject, fge::Scene* scene_ptr) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene_ptr) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    std::string getClassName() const override;
    std::string getReadableClassName() const override;

    sf::FloatRect getGlobalBounds() const override;
    sf::FloatRect getLocalBounds() const override;

private:
    mutable fge::ObjSprite g_sprite;

    fge::Texture g_textureOn;
    fge::Texture g_textureOff;

    sf::Color g_color;

    bool g_statMouseOn = false;
    bool g_statActive = false;

    fge::Flag g_flag = false;
};

}//end fge

#endif // _FGE_C_OBJSWITCH_HPP_INCLUDED

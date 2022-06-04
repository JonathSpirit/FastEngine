#ifndef _FGE_C_OBJANIM_HPP_INCLUDED
#define _FGE_C_OBJANIM_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_animation.hpp>
#include <chrono>
#include <string>

#define FGE_OBJANIM_DEFAULT_TICKDURATION_MS 10
#define FGE_OBJANIM_CLASSNAME "FGE:OBJ:ANIM"

namespace fge
{

class FGE_API ObjAnimation : public fge::Object
{
public:
    ObjAnimation();
    explicit ObjAnimation(const fge::Animation& animation, const sf::Vector2f& position=sf::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjAnimation)

    void setAnimation(const fge::Animation& animation);
    void setTextureRect(const sf::IntRect& rectangle);

    void setColor(const sf::Color& color);

    void setPause(bool flag);
    bool isPaused() const;

    void setTickDuration(const std::chrono::milliseconds& tms);
    const std::chrono::milliseconds& getTickDuration() const;

    const fge::Animation& getAnimation() const;
    fge::Animation& getAnimation();
    const sf::IntRect& getTextureRect() const;

    const sf::Color& getColor() const;

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

    void updatePositions();
    void updateTexCoords();

    sf::Vertex g_vertices[4];
    fge::Animation g_animation;
    sf::IntRect g_textureRect;

    std::chrono::milliseconds g_tickDuration;
    std::chrono::milliseconds g_nextFrameTime;

    bool g_paused;
};

}//end fge

#endif // _FGE_C_OBJANIM_HPP_INCLUDED

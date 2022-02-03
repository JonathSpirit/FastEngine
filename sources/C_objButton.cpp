#include "FastEngine/C_objButton.hpp"
#include "FastEngine/extra_function.hpp"

namespace fge
{

FGE_API ObjButton::ObjButton() :
    g_color(sf::Color::White)
{
}
FGE_API ObjButton::ObjButton(const fge::Texture& t_on, const fge::Texture& t_off, const sf::Vector2f& pos) :
    g_textureOn(t_on),
    g_textureOff(t_off),
    g_color(sf::Color::White)
{
    this->setPosition(pos);
    this->g_sprite.setTexture(t_off);
}

const fge::Texture& FGE_API ObjButton::getTextureOn() const
{
    return this->g_textureOn;
}
const fge::Texture& FGE_API ObjButton::getTextureOff() const
{
    return this->g_textureOff;
}
void FGE_API ObjButton::setTextureOn(const fge::Texture& t_on)
{
    this->g_textureOn = t_on;
}
void FGE_API ObjButton::setTextureOff(const fge::Texture& t_off)
{
    this->g_textureOff = t_off;
}

void FGE_API ObjButton::setColor(const sf::Color& color)
{
    this->g_color = color;
}

void FGE_API ObjButton::setActiveStat(bool active)
{
    this->g_statActive = active;
    this->g_sprite.setTexture(this->g_statActive ? this->g_textureOn : this->g_textureOff);
}
bool FGE_API ObjButton::getActiveStat() const
{
    return this->g_statActive;
}

void FGE_API ObjButton::update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr)
{
    this->g_statMouseOn = fge::IsMouseOn( screen.mapPixelToCoords(event.getMousePixelPos()), this->getGlobalBounds() );

    if ( event.isMouseButtonPressed( sf::Mouse::Left ) )
    {
        if ( !this->g_flag )
        {
            this->g_flag = true;
            this->g_statActive = this->g_statMouseOn;
            this->g_sprite.setTexture(this->g_statActive ? this->g_textureOn : this->g_textureOff);
        }
    }
    else
    {
        this->g_flag = false;
        this->g_statActive = false;
        this->g_sprite.setTexture(this->g_textureOff);
    }

    if ( !this->g_statMouseOn )
    {
        this->g_statActive = false;
    }
}
void FGE_API ObjButton::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= this->getTransform();
    this->g_sprite.setColor(this->g_statMouseOn ? (this->g_color - sf::Color(50,50,50,0)) : this->g_color);
    target.draw(this->g_sprite, states);
}

void FGE_API ObjButton::save(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::save(jsonObject, scene_ptr);

    jsonObject["color"] = this->g_color.toInteger();

    jsonObject["textureOn"] = this->g_textureOn;
    jsonObject["textureOff"] = this->g_textureOff;

    jsonObject["statMouseOn"] = this->g_statMouseOn;
    jsonObject["statActive"] = this->g_statActive;
}
void FGE_API ObjButton::load(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::load(jsonObject, scene_ptr);

    this->g_color = sf::Color( jsonObject.value<uint32_t>("color", 0) );

    this->g_textureOn = jsonObject.value<std::string>("textureOn", FGE_TEXTURE_BAD);
    this->g_textureOff = jsonObject.value<std::string>("textureOff", FGE_TEXTURE_BAD);

    this->g_statMouseOn = jsonObject.value<bool>("statMouseOn", false);
    this->g_statActive = jsonObject.value<bool>("statActive", false);

    this->g_sprite.setTexture(this->g_statActive ? this->g_textureOn : this->g_textureOff);
}

void FGE_API ObjButton::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_color << this->g_textureOn << this->g_textureOff << this->g_statMouseOn << this->g_statActive;
}
void FGE_API ObjButton::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_color >> this->g_textureOn >> this->g_textureOff >> this->g_statMouseOn >> this->g_statActive;
}

std::string FGE_API ObjButton::getClassName() const
{
    return FGE_OBJBUTTON_CLASSNAME;
}
std::string FGE_API ObjButton::getReadableClassName() const
{
    return "button";
}

sf::FloatRect FGE_API ObjButton::getGlobalBounds() const
{
    return this->getTransform().transformRect( this->g_sprite.getLocalBounds() );
}
sf::FloatRect FGE_API ObjButton::getLocalBounds() const
{
    return this->g_sprite.getLocalBounds();
}

}//end fge

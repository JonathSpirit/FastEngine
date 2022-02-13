#include "FastEngine/C_objSprite.hpp"

namespace fge
{

ObjSprite::ObjSprite()
{
}
ObjSprite::ObjSprite(const fge::Texture& texture, const sf::Vector2f& position)
{
    this->setTexture(texture);
    this->setPosition(position);
}
ObjSprite::ObjSprite(const fge::Texture& texture, const sf::IntRect& rectangle, const sf::Vector2f& position)
{
    this->setTexture(texture);
    this->setTextureRect(rectangle);
    this->setPosition(position);
}

void ObjSprite::setTexture(const fge::Texture& texture, bool resetRect)
{
    // Recompute the texture area if requested, or if there was no valid texture & rect before
    if ( resetRect || !this->g_texture.valid() )
    {
        this->setTextureRect( sf::IntRect(0, 0, texture.getTextureSize().x, texture.getTextureSize().y) );
    }

    // Assign the new texture
    this->g_texture = texture;
}
void ObjSprite::setTextureRect(const sf::IntRect& rectangle)
{
    if (rectangle != this->g_textureRect)
    {
        this->g_textureRect = rectangle;
        this->updatePositions();
        this->updateTexCoords();
    }
}

void ObjSprite::setColor(const sf::Color& color)
{
    this->g_vertices[0].color = color;
    this->g_vertices[1].color = color;
    this->g_vertices[2].color = color;
    this->g_vertices[3].color = color;
}

const fge::Texture& ObjSprite::getTexture() const
{
    return this->g_texture;
}
const sf::IntRect& ObjSprite::getTextureRect() const
{
    return this->g_textureRect;
}

const sf::Color& ObjSprite::getColor() const
{
    return this->g_vertices[0].color;
}

void ObjSprite::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= this->getTransform();
    states.texture = static_cast<const sf::Texture*>(this->g_texture);
    target.draw(this->g_vertices, 4, sf::TriangleStrip, states);
}

void ObjSprite::save(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::save(jsonObject, scene_ptr);

    jsonObject["color"] = this->g_vertices[0].color.toInteger();
    jsonObject["texture"] = this->g_texture;
}
void ObjSprite::load(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::load(jsonObject, scene_ptr);

    this->setColor( sf::Color( jsonObject.value<uint32_t>("color", 0) ) );
    this->g_texture = jsonObject.value<std::string>("texture", FGE_TEXTURE_BAD);
    this->setTexture(this->g_texture, true);
}

void ObjSprite::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_vertices[0].color << this->g_texture;
}
void ObjSprite::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    sf::Color color;
    pck >> color >> this->g_texture;
    this->setColor(color);
}

std::string ObjSprite::getClassName() const
{
    return FGE_OBJSPRITE_CLASSNAME;
}
std::string ObjSprite::getReadableClassName() const
{
    return "sprite";
}

sf::FloatRect ObjSprite::getGlobalBounds() const
{
    return this->getTransform().transformRect(this->getLocalBounds());
}
sf::FloatRect ObjSprite::getLocalBounds() const
{
    float width = static_cast<float>( std::abs(this->g_textureRect.width) );
    float height = static_cast<float>( std::abs(this->g_textureRect.height) );

    return sf::FloatRect(0.f, 0.f, width, height);
}

void ObjSprite::updatePositions()
{
    sf::FloatRect bounds = this->getLocalBounds();

    this->g_vertices[0].position = sf::Vector2f(0, 0);
    this->g_vertices[1].position = sf::Vector2f(0, bounds.height);
    this->g_vertices[2].position = sf::Vector2f(bounds.width, 0);
    this->g_vertices[3].position = sf::Vector2f(bounds.width, bounds.height);
}

void ObjSprite::updateTexCoords()
{
    float left   = static_cast<float>(this->g_textureRect.left);
    float right  = left + this->g_textureRect.width;
    float top    = static_cast<float>(this->g_textureRect.top);
    float bottom = top + this->g_textureRect.height;

    this->g_vertices[0].texCoords = sf::Vector2f(left, top);
    this->g_vertices[1].texCoords = sf::Vector2f(left, bottom);
    this->g_vertices[2].texCoords = sf::Vector2f(right, top);
    this->g_vertices[3].texCoords = sf::Vector2f(right, bottom);
}

}//end fge

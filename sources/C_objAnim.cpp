#include "FastEngine/C_objAnim.hpp"

namespace fge
{

FGE_API ObjAnimation::ObjAnimation() :
    g_tickDuration(FGE_OBJANIM_DEFAULT_TICKDURATION),

    g_paused(false)
{
    sf::Texture* buffTexture = static_cast<sf::Texture*>(this->g_animation);
    this->setTextureRect( sf::IntRect(0, 0, buffTexture->getSize().x, buffTexture->getSize().y) );
}
FGE_API ObjAnimation::ObjAnimation(const fge::Animation& animation, const sf::Vector2f& position) :
    g_animation(animation),
    g_tickDuration(FGE_OBJANIM_DEFAULT_TICKDURATION),

    g_paused(false)
{
    this->setPosition(position);

    sf::Texture* buffTexture = static_cast<sf::Texture*>(this->g_animation);
    this->setTextureRect( sf::IntRect(0, 0, buffTexture->getSize().x, buffTexture->getSize().y) );
}

void FGE_API ObjAnimation::setAnimation(const fge::Animation& animation)
{
    this->g_animation = animation;

    sf::Texture* buffTexture = static_cast<sf::Texture*>(this->g_animation);
    this->setTextureRect( sf::IntRect(0, 0, buffTexture->getSize().x, buffTexture->getSize().y) );
}
void FGE_API ObjAnimation::setTextureRect(const sf::IntRect& rectangle)
{
    if (rectangle != this->g_textureRect)
    {
        this->g_textureRect = rectangle;
        this->updatePositions();
        this->updateTexCoords();
    }
}

void FGE_API ObjAnimation::setColor(const sf::Color& color)
{
    this->g_vertices[0].color = color;
    this->g_vertices[1].color = color;
    this->g_vertices[2].color = color;
    this->g_vertices[3].color = color;
}

void FGE_API ObjAnimation::setPause(bool flag)
{
    this->g_paused = flag;
}
bool FGE_API ObjAnimation::isPaused() const
{
    return this->g_paused;
}

void FGE_API ObjAnimation::setTickDuration(const std::chrono::milliseconds& tms)
{
    this->g_tickDuration = tms;
}
const std::chrono::milliseconds& FGE_API ObjAnimation::getTickDuration() const
{
    return this->g_tickDuration;
}

const fge::Animation& FGE_API ObjAnimation::getAnimation() const
{
    return this->g_animation;
}
fge::Animation& FGE_API ObjAnimation::getAnimation()
{
    return this->g_animation;
}
const sf::IntRect& FGE_API ObjAnimation::getTextureRect() const
{
    return this->g_textureRect;
}

const sf::Color& FGE_API ObjAnimation::getColor() const
{
    return this->g_vertices[0].color;
}

void FGE_API ObjAnimation::update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr)
{
    if (!this->g_paused)
    {
        fge::anim::AnimationFrame* frame = this->g_animation.getFrame();
        if ( frame != nullptr )
        {
            if ( std::chrono::duration_cast<std::chrono::milliseconds>(this->g_clock.getElapsedTime()) >= this->g_tickDuration*frame->_ticks )
            {
                frame = this->g_animation.getFrame(this->g_animation.nextFrame());
                this->setTextureRect( sf::IntRect(0, 0, frame->_texture->getSize().x, frame->_texture->getSize().y) );
                this->g_clock.restart();
            }
        }
        else
        {
            this->g_animation.setFrame(0);
            this->g_clock.restart();
        }
    }
}
void FGE_API ObjAnimation::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= this->getTransform();
    states.texture = static_cast<const sf::Texture*>(this->g_animation);
    target.draw(this->g_vertices, 4, sf::TriangleStrip, states);
}

void FGE_API ObjAnimation::save(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::save(jsonObject, scene_ptr);

    jsonObject["color"] = this->g_vertices[0].color.toInteger();
    jsonObject["animation"] = this->g_animation;
    jsonObject["animationGroup"] = this->g_animation.getGroupIndex();
    jsonObject["animationFrame"] = this->g_animation.getFrameIndex();
    jsonObject["animationLoop"] = this->g_animation.isLoop();
    jsonObject["tickDuration"] = static_cast<uint16_t>(this->g_tickDuration.count());
}
void FGE_API ObjAnimation::load(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::load(jsonObject, scene_ptr);

    this->setColor( sf::Color( jsonObject.value<uint32_t>("color", 0) ) );
    this->g_animation = jsonObject.value<std::string>("animation", FGE_ANIM_BAD);
    this->g_animation.setGroup( jsonObject.value<std::size_t>("animationGroup", 0) );
    this->g_animation.setFrame( jsonObject.value<std::size_t>("animationFrame", 0) );
    this->g_animation.setLoop( jsonObject.value<bool>("animationLoop", false) );
    this->g_tickDuration = std::chrono::milliseconds( jsonObject.value<uint16_t>("tickDuration", FGE_OBJANIM_DEFAULT_TICKDURATION) );

    sf::Texture* buffTexture = static_cast<sf::Texture*>(this->g_animation);
    this->setTextureRect( sf::IntRect(0, 0, buffTexture->getSize().x, buffTexture->getSize().y) );
}
void FGE_API ObjAnimation::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_vertices[0].color << this->g_animation;
    pck << static_cast<uint32_t>(this->g_animation.getGroupIndex()) << static_cast<uint32_t>(this->g_animation.getFrameIndex()) << this->g_animation.isLoop();
    pck << static_cast<uint16_t>(this->g_tickDuration.count());
}
void FGE_API ObjAnimation::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    sf::Color color;
    pck >> color >> this->g_animation;
    this->setColor(color);
    uint32_t group=0, frame=0; bool loop=false;
    pck >> group >> frame >> loop;
    this->g_animation.setGroup(group);
    this->g_animation.setFrame(frame);
    this->g_animation.setLoop(loop);

    uint16_t tmpTick = FGE_OBJANIM_DEFAULT_TICKDURATION;
    pck >> tmpTick;
    this->g_tickDuration = std::chrono::milliseconds(tmpTick);

    sf::Texture* buffTexture = static_cast<sf::Texture*>(this->g_animation);
    this->setTextureRect( sf::IntRect(0, 0, buffTexture->getSize().x, buffTexture->getSize().y) );
}

std::string FGE_API ObjAnimation::getClassName() const
{
    return FGE_OBJANIM_CLASSNAME;
}
std::string FGE_API ObjAnimation::getReadableClassName() const
{
    return "animation";
}

sf::FloatRect FGE_API ObjAnimation::getGlobalBounds() const
{
    return this->getTransform().transformRect(this->getLocalBounds());
}
sf::FloatRect FGE_API ObjAnimation::getLocalBounds() const
{
    float width = static_cast<float>( std::abs(this->g_textureRect.width) );
    float height = static_cast<float>( std::abs(this->g_textureRect.height) );

    return sf::FloatRect(0.f, 0.f, width, height);
}

void FGE_API ObjAnimation::updatePositions()
{
    sf::FloatRect bounds = this->getLocalBounds();

    this->g_vertices[0].position = sf::Vector2f(0, 0);
    this->g_vertices[1].position = sf::Vector2f(0, bounds.height);
    this->g_vertices[2].position = sf::Vector2f(bounds.width, 0);
    this->g_vertices[3].position = sf::Vector2f(bounds.width, bounds.height);
}
void FGE_API ObjAnimation::updateTexCoords()
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

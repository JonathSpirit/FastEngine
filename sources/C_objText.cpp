#include "FastEngine/C_objText.hpp"
#include "FastEngine/font_manager.hpp"

namespace fge
{

FGE_API ObjText::ObjText()
{
}
FGE_API ObjText::ObjText(const fge::Font& font, const sf::Vector2f& position) :
    g_font(font)
{
    this->g_text.setFont(this->g_font);
    this->setPosition(position);
}
FGE_API ObjText::ObjText(const fge::Font& font, const std::string& txt, const sf::Vector2f& position) :
    g_font(font)
{
    this->g_text.setFont(this->g_font);
    this->g_text.setString(txt);
    this->setPosition(position);
}
FGE_API ObjText::ObjText(const fge::Font& font, const char* txt, const sf::Vector2f& position) :
    g_font(font)
{
    this->g_text.setFont(this->g_font);
    this->g_text.setString(txt);
    this->setPosition(position);
}

void FGE_API ObjText::setFont(const fge::Font& font)
{
    this->g_font = font;
    this->g_text.setFont(this->g_font);
}
const fge::Font& FGE_API ObjText::getFont() const
{
    return this->g_font;
}

void FGE_API ObjText::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= this->getTransform();
    this->g_text.setFont( this->g_font );
    target.draw(this->g_text, states);
}

void FGE_API ObjText::save(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::save(jsonObject, scene_ptr);

    jsonObject["font"] = this->g_font;

    std::basic_string<uint32_t> tmpString = this->g_text.getString().toUtf32();
    jsonObject["string"] = tmpString;

    jsonObject["characterSize"] = static_cast<uint16_t>(this->g_text.getCharacterSize());
    jsonObject["letterSpacing"] = this->g_text.getLetterSpacing();
    jsonObject["lineSpacing"] = this->g_text.getLineSpacing();
    jsonObject["style"] = static_cast<uint32_t>(this->g_text.getStyle());
    jsonObject["fillColor"] = static_cast<uint32_t>(this->g_text.getFillColor().toInteger());
    jsonObject["outlineColor"] = static_cast<uint32_t>(this->g_text.getOutlineColor().toInteger());
    jsonObject["outlineThickness"] = this->g_text.getOutlineThickness();
}
void FGE_API ObjText::load(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::load(jsonObject, scene_ptr);

    this->g_font = jsonObject.value<std::string>("font", FGE_FONT_BAD);

    std::basic_string<uint32_t> tmpString = jsonObject.value<std::basic_string<uint32_t> >("string", std::basic_string<uint32_t>());
    this->g_text.setString( sf::String(tmpString) );

    this->g_text.setCharacterSize( jsonObject.value<uint16_t>("characterSize", this->g_text.getCharacterSize()) );
    this->g_text.setLetterSpacing( jsonObject.value<float>("letterSpacing", this->g_text.getLetterSpacing()) );
    this->g_text.setLineSpacing( jsonObject.value<float>("lineSpacing", this->g_text.getLineSpacing()) );
    this->g_text.setStyle( jsonObject.value<uint32_t>("style", this->g_text.getStyle()) );
    this->g_text.setFillColor( sf::Color(jsonObject.value<uint32_t>("fillColor", 0)) );
    this->g_text.setOutlineColor( sf::Color(jsonObject.value<uint32_t>("outlineColor", 0)) );
    this->g_text.setOutlineThickness( jsonObject.value<float>("outlineThickness", 0.0f) );
}

void FGE_API ObjText::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_font;

    pck << this->g_text.getString();
    pck << static_cast<uint16_t>(this->g_text.getCharacterSize());
    pck << this->g_text.getLetterSpacing();
    pck << this->g_text.getLineSpacing();
    pck << static_cast<uint32_t>(this->g_text.getStyle());
    pck << this->g_text.getFillColor();
    pck << this->g_text.getOutlineColor();
    pck << this->g_text.getOutlineThickness();
}
void FGE_API ObjText::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    sf::String tmpString;
    uint16_t tmpUint16=0;
    uint32_t tmpUint32=0;
    float tmpFloat=0;
    sf::Color tmpColor;

    pck >> this->g_font;

    pck >> tmpString;
    this->g_text.setString(tmpString);
    pck >> tmpUint16;
    this->g_text.setCharacterSize(tmpUint16);
    pck >> tmpFloat;
    this->g_text.setLetterSpacing(tmpFloat);
    pck >> tmpFloat;
    this->g_text.setLineSpacing(tmpFloat);
    pck >> tmpUint32;
    this->g_text.setStyle(tmpUint32);
    pck >> tmpColor;
    this->g_text.setFillColor(tmpColor);
    pck >> tmpColor;
    this->g_text.setOutlineColor(tmpColor);
    pck >> tmpFloat;
    this->g_text.setOutlineThickness(tmpFloat);
}

std::string FGE_API ObjText::getClassName() const
{
    return FGE_OBJTEXT_CLASSNAME;
}
std::string FGE_API ObjText::getReadableClassName() const
{
    return "text";
}

sf::FloatRect FGE_API ObjText::getGlobalBounds() const
{
    return this->getTransform().transformRect( this->g_text.getLocalBounds() );
}
sf::FloatRect FGE_API ObjText::getLocalBounds() const
{
    return this->g_text.getLocalBounds();
}

}//end fge

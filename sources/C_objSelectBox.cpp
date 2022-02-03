#include "FastEngine/C_objSelectBox.hpp"
#include <FastEngine/extra_function.hpp>

namespace fge
{

FGE_API ObjSelectBox::ObjSelectBox()
{
    this->g_text.setFillColor(this->g_colorText);
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_box.setOutlineThickness(1.0f);
    this->g_box.setSize(this->g_boxSize);

    this->g_text.setCharacterSize(12);
}
FGE_API ObjSelectBox::ObjSelectBox(const fge::Font& font, const sf::Vector2f& pos) :
    g_text(font)
{
    this->g_text.setFillColor(this->g_colorText);
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_box.setOutlineThickness(1.0f);
    this->g_box.setSize(this->g_boxSize);

    this->g_text.setCharacterSize(12);

    this->setPosition(pos);
}

std::vector<sf::String>& FGE_API ObjSelectBox::getTextList()
{
    return this->g_textList;
}
const std::vector<sf::String>& FGE_API ObjSelectBox::getTextList() const
{
    return this->g_textList;
}

void FGE_API ObjSelectBox::setSelectedText(const sf::String& string)
{
    this->g_textSelected = string;
}
const sf::String& FGE_API ObjSelectBox::getSelectedText() const
{
    return this->g_textSelected;
}

void FGE_API ObjSelectBox::setCharacterSize(unsigned int size)
{
    this->g_text.setCharacterSize(size);
}

void FGE_API ObjSelectBox::setActiveStat(bool active)
{
    this->g_statActive = active;
}

void FGE_API ObjSelectBox::setBoxSize(const sf::Vector2f& size)
{
    this->g_boxSize = size;
    this->g_box.setSize(this->g_boxSize);
}
void FGE_API ObjSelectBox::setBoxSize(float w, float h)
{
    this->g_boxSize.x = w;
    this->g_boxSize.y = h;
    this->g_box.setSize(this->g_boxSize);
}

void FGE_API ObjSelectBox::setBoxColor(const sf::Color& color)
{
    this->g_colorBox = color;
    this->g_box.setFillColor(color);
}
void FGE_API ObjSelectBox::setBoxOutlineColor(const sf::Color& color)
{
    this->g_colorBoxOutline = color;
    this->g_box.setOutlineColor(color);
}
void FGE_API ObjSelectBox::setTextColor(const sf::Color& color)
{
    this->g_colorText = color;
    this->g_text.setFillColor(color);
}

unsigned int FGE_API ObjSelectBox::getCharacterSize() const
{
    return this->g_text.getCharacterSize();
}

bool FGE_API ObjSelectBox::getActiveStat() const
{
    return this->g_statActive;
}

const sf::Vector2f& FGE_API ObjSelectBox::getBoxSize() const
{
    return this->g_boxSize;
}

const sf::Color& FGE_API ObjSelectBox::getBoxColor() const
{
    return this->g_colorBox;
}
const sf::Color& FGE_API ObjSelectBox::getBoxOutlineColor() const
{
    return this->g_colorBoxOutline;
}
const sf::Color& FGE_API ObjSelectBox::getTextColor() const
{
    return this->g_colorText;
}

void FGE_API ObjSelectBox::update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr)
{
    sf::Vector2f mousePosition = screen.mapPixelToCoords(event.getMousePixelPos());
    sf::FloatRect bounds = this->getGlobalBounds();
    this->g_statMouseOn = fge::IsMouseOn( mousePosition, bounds );

    if ( this->g_statActive )
    {
        for ( std::size_t i=0; i<this->g_textList.size(); ++i )
        {
            bounds.top += this->g_boxSize.y;

            if ( fge::IsMouseOn( mousePosition, bounds ) )
            {
                this->g_textCursor = &this->g_textList[i];
                if ( this->g_flag.check(event.isMouseButtonPressed(sf::Mouse::Left)) )
                {
                    this->g_textSelected = this->g_textList[i];
                    this->g_statActive = false;
                }
                break;
            }
        }

        if ( this->g_flag.check(event.isMouseButtonPressed(sf::Mouse::Left)) )
        {
            this->g_statActive = false;
        }
    }
    else
    {
        if ( this->g_flag.check(event.isMouseButtonPressed(sf::Mouse::Left))  )
        {
            if ( this->g_statMouseOn )
            {
                this->g_statActive = true;
            }
        }
    }
}
void FGE_API ObjSelectBox::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    this->g_text.setString( this->g_textSelected );

    this->g_box.setFillColor( this->g_colorBox );

    states.transform *= this->getTransform();
    target.draw(this->g_box, states);
    target.draw(this->g_text, states);

    if ( this->g_statActive )
    {
        for ( std::size_t i=0; i<this->g_textList.size(); ++i )
        {
            states.transform.translate(0, this->g_boxSize.y);

            this->g_box.setFillColor( (&this->g_textList[i] == this->g_textCursor) ? this->g_colorBox-sf::Color(100,100,0,0) : this->g_colorBox );

            target.draw(this->g_box, states);
            this->g_text.setString(this->g_textList[i]);
            target.draw(this->g_text, states);
        }
    }
}

void FGE_API ObjSelectBox::save(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::save(jsonObject, scene_ptr);

    jsonObject["colorBox"] = this->g_colorBox.toInteger();
    jsonObject["colorBoxOutline"] = this->g_colorBoxOutline.toInteger();
    jsonObject["colorText"] = this->g_colorText.toInteger();

    std::basic_string<uint32_t> tmpString = this->g_textSelected.toUtf32();
    jsonObject["textSelected"] = tmpString;
    jsonObject["texts"] = this->g_textList;

    jsonObject["characterSize"] = this->g_text.getCharacterSize();
    jsonObject["font"] = this->g_text.getFont();

    jsonObject["boxSizeX"] = this->g_boxSize.x;
    jsonObject["boxSizeY"] = this->g_boxSize.y;

    jsonObject["statActive"] = this->g_statActive;
    jsonObject["statMouseOn"] = this->g_statMouseOn;
}
void FGE_API ObjSelectBox::load(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::load(jsonObject, scene_ptr);

    this->g_colorBox = sf::Color( jsonObject.value<uint32_t>("colorBox", 0xFFFFFFFF) );
    this->g_colorBoxOutline = sf::Color( jsonObject.value<uint32_t>("colorBoxOutline", 0) );
    this->g_colorText = sf::Color( jsonObject.value<uint32_t>("colorText", 0) );
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_text.setFillColor(this->g_colorText);

    std::basic_string<uint32_t> tmpString = jsonObject.value<std::basic_string<uint32_t> >("textSelected", std::basic_string<uint32_t>());
    this->g_textSelected = tmpString;
    std::vector<std::string> tmpStrings = jsonObject.value<std::vector<std::string> >("texts", std::vector<std::string>());
    this->g_textList.clear();
    for (std::size_t i=0; i<tmpStrings.size(); ++i)
    {
        this->g_textList.push_back(tmpStrings[i]);
    }

    unsigned int charSize = jsonObject.value<uint32_t>("characterSize", 12);
    this->g_text.setCharacterSize(charSize);
    this->g_text.setFont( jsonObject.value<std::string>("font", FGE_FONT_BAD) );

    this->g_boxSize.x = jsonObject.value<float>("boxSizeX", 120);
    this->g_boxSize.y = jsonObject.value<float>("boxSizeY", 18);
    this->g_box.setSize(this->g_boxSize);

    this->g_statActive = jsonObject.value<bool>("statActive", false);
    this->g_statMouseOn = jsonObject.value<bool>("statMouseOn", false);
}

void FGE_API ObjSelectBox::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_colorBox << this->g_colorBoxOutline << this->g_colorText;

    pck << this->g_textSelected;

    pck << static_cast<uint16_t>(this->g_text.getCharacterSize()) << this->g_text.getFont();
    pck << this->g_textList;

    pck << this->g_boxSize;

    pck << this->g_statActive << this->g_statMouseOn;
}
void FGE_API ObjSelectBox::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_colorBox >> this->g_colorBoxOutline >> this->g_colorText;
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_text.setFillColor(this->g_colorText);

    pck >> this->g_textSelected;

    uint16_t tmpCharSize=12; fge::Font tmpFont;
    pck >> tmpCharSize >> tmpFont;

    this->g_text.setCharacterSize(tmpCharSize);
    this->g_text.setFont(tmpFont);

    pck >> this->g_textList;

    pck >> this->g_boxSize;
    this->g_box.setSize(this->g_boxSize);

    pck >> this->g_statActive >> this->g_statMouseOn;
}

std::string FGE_API ObjSelectBox::getClassName() const
{
    return FGE_OBJSELECTBOX_CLASSNAME;
}
std::string FGE_API ObjSelectBox::getReadableClassName() const
{
    return "selection box";
}

sf::FloatRect FGE_API ObjSelectBox::getGlobalBounds() const
{
    return this->getTransform().transformRect( this->g_box.getLocalBounds() );
}
sf::FloatRect FGE_API ObjSelectBox::getLocalBounds() const
{
    return this->g_box.getLocalBounds();
}

}//end fge

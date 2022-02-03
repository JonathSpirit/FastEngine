#include "FastEngine/C_object.hpp"
#include "FastEngine/extra_string.hpp"
#include "FastEngine/reg_manager.hpp"

#include <fstream>
#include <iomanip>

namespace fge
{

void FGE_API Object::first(fge::Scene* scene_ptr)
{
}
void FGE_API Object::callbackRegister(fge::Event& event)
{
}
void FGE_API Object::update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr)
{
}
void FGE_API Object::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}
void FGE_API Object::networkRegister()
{
}
void FGE_API Object::removed(fge::Scene* scene_ptr)
{
}

fge::Object* FGE_API Object::copy()
{
    return fge::reg::Duplicate(this);
}

void FGE_API Object::save(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    jsonObject["_posX"] = this->getPosition().x;
    jsonObject["_posY"] = this->getPosition().y;

    jsonObject["_rotation"] = this->getRotation();

    jsonObject["_scaleX"] = this->getScale().x;
    jsonObject["_scaleY"] = this->getScale().y;

    jsonObject["_originX"] = this->getOrigin().x;
    jsonObject["_originY"] = this->getOrigin().y;

    jsonObject["tags"] = nlohmann::json::array();
    for ( fge::TagList::TagListType::const_iterator it=this->_tags.cbegin(); it!=this->_tags.cend(); ++it )
    {
        jsonObject["tags"] += (*it);
    }
}
void FGE_API Object::load(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    this->setPosition( jsonObject["_posX"].get<float>(), jsonObject["_posY"].get<float>() );

    this->setRotation( jsonObject["_rotation"].get<float>() );

    this->setScale( jsonObject["_scaleX"].get<float>(), jsonObject["_scaleY"].get<float>() );

    this->setOrigin( jsonObject["_originX"].get<float>(), jsonObject["_originY"].get<float>() );

    this->_tags.clear();

    for (nlohmann::json::iterator it = jsonObject["tags"].begin(); it != jsonObject["tags"].end(); ++it)
    {
        this->_tags.add( (*it).get<std::string>() );
    }
}

void FGE_API Object::pack(fge::net::Packet& pck)
{
    pck << this->getPosition() << this->getRotation() << this->getScale() << this->getOrigin();

    pck << static_cast<uint32_t>(this->_tags.getSize());
    for ( fge::TagList::TagListType::const_iterator it=this->_tags.cbegin(); it!=this->_tags.cend(); ++it )
    {
        pck << *it;
    }
}
void FGE_API Object::unpack(fge::net::Packet& pck)
{
    sf::Vector2f buffVec2f;
    float buffFloat = 0;

    pck >> buffVec2f;
    this->setPosition(buffVec2f);
    pck >> buffFloat;
    this->setRotation(buffFloat);
    pck >> buffVec2f;
    this->setScale(buffVec2f);
    pck >> buffVec2f;
    this->setOrigin(buffVec2f);

    uint32_t tagSize=0;
    pck >> tagSize;
    this->_tags.clear();
    for ( uint32_t i=0; i<tagSize; ++i )
    {
        std::string str;
        pck >> str;
        this->_tags.add(str);
    }
}

std::string FGE_API Object::getClassName() const
{
    return FGE_OBJ_BADCLASSNAME;
}
std::string FGE_API Object::getReadableClassName() const
{
    return FGE_OBJ_BADCLASSNAME;
}

sf::FloatRect FGE_API Object::getGlobalBounds() const
{
    return this->getTransform().transformRect( this->getLocalBounds() );
}
sf::FloatRect FGE_API Object::getLocalBounds() const
{
    return sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f);
}

bool FGE_API Object::saveInFile(const std::string& path)
{
    nlohmann::json objNewJson = nlohmann::json::object();
    nlohmann::json& objJson = objNewJson[this->getClassName()];

    objJson = nlohmann::json::object();

    this->save( objJson );

    std::ofstream outFile(path);
    if ( outFile )
    {
        outFile << std::setw(2) << objNewJson << std::endl;
        outFile.close();
        return true;
    }
    outFile.close();
    return false;
}
bool FGE_API Object::loadFromFile(const std::string& path)
{
    std::ifstream inFile(path);
    if ( !inFile )
    {
        inFile.close();
        return false;
    }

    nlohmann::json inputJson;
    inFile >> inputJson;
    inFile.close();

    std::string className = inputJson.begin().key();

    if (className == this->getClassName())
    {
        nlohmann::json& objJson = inputJson.begin().value();

        this->load( objJson );
        return true;
    }
    return false;
}
fge::Object* FGE_API Object::LoadFromFile(const std::string& path)
{
    std::ifstream inFile(path);
    if ( !inFile )
    {
        inFile.close();
        return nullptr;
    }

    nlohmann::json inputJson;
    inFile >> inputJson;

    fge::Object* buffObj = fge::reg::GetNewClassOf( inputJson.begin().key() );
    if ( buffObj )
    {
        nlohmann::json& objJson = inputJson.begin().value();

        buffObj->load( objJson );
        return buffObj;
    }
    return nullptr;
}

}//end fge

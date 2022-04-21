#include "FastEngine/C_object.hpp"
#include "FastEngine/extra_string.hpp"
#include "FastEngine/reg_manager.hpp"

#include <fstream>
#include <iomanip>

namespace fge
{

void Object::first(fge::Scene* scene_ptr)
{
}
void Object::callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
}
void Object::update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr)
{
}
void Object::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}
void Object::networkRegister()
{
}
void Object::removed(fge::Scene* scene_ptr)
{
}

fge::Object* Object::copy()
{
    return fge::reg::Duplicate(this);
}

void Object::save(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    jsonObject["_posX"] = this->getPosition().x;
    jsonObject["_posY"] = this->getPosition().y;

    jsonObject["_rotation"] = this->getRotation();

    jsonObject["_scaleX"] = this->getScale().x;
    jsonObject["_scaleY"] = this->getScale().y;

    jsonObject["_originX"] = this->getOrigin().x;
    jsonObject["_originY"] = this->getOrigin().y;

    jsonObject["tags"] = nlohmann::json::array();
    for (const auto& tag : this->_tags)
    {
        jsonObject["tags"] += tag;
    }
}
void Object::load(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    this->setPosition( jsonObject["_posX"].get<float>(), jsonObject["_posY"].get<float>() );

    this->setRotation( jsonObject["_rotation"].get<float>() );

    this->setScale( jsonObject["_scaleX"].get<float>(), jsonObject["_scaleY"].get<float>() );

    this->setOrigin( jsonObject["_originX"].get<float>(), jsonObject["_originY"].get<float>() );

    this->_tags.clear();

    for (auto& it : jsonObject["tags"])
    {
        this->_tags.add( it.get<std::string>() );
    }
}

void Object::pack(fge::net::Packet& pck)
{
    pck << this->getPosition() << this->getRotation() << this->getScale() << this->getOrigin();
}
void Object::unpack(fge::net::Packet& pck)
{
    sf::Vector2f buffVec2f;
    float buffFloat{0.0f};

    pck >> buffVec2f;
    this->setPosition(buffVec2f);
    pck >> buffFloat;
    this->setRotation(buffFloat);
    pck >> buffVec2f;
    this->setScale(buffVec2f);
    pck >> buffVec2f;
    this->setOrigin(buffVec2f);
}

std::string Object::getClassName() const
{
    return FGE_OBJ_BADCLASSNAME;
}
std::string Object::getReadableClassName() const
{
    return FGE_OBJ_BADCLASSNAME;
}

sf::FloatRect Object::getGlobalBounds() const
{
    return this->getTransform().transformRect( this->getLocalBounds() );
}
sf::FloatRect Object::getLocalBounds() const
{
    return {0.0f, 0.0f, 1.0f, 1.0f};
}

bool Object::saveInFile(const std::string& path)
{
    nlohmann::json objNewJson = nlohmann::json::object();
    nlohmann::json& objJson = objNewJson[this->getClassName()];

    objJson = nlohmann::json::object();

    this->save(objJson, FGE_OBJ_NOSCENE);

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
bool Object::loadFromFile(const std::string& path)
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

        this->load(objJson, FGE_OBJ_NOSCENE);
        return true;
    }
    return false;
}
fge::Object* Object::LoadFromFile(const std::string& path)
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

        buffObj->load(objJson, FGE_OBJ_NOSCENE);
        return buffObj;
    }
    return nullptr;
}

}//end fge

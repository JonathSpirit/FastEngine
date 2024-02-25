/*
 * Copyright 2024 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FastEngine/object/C_object.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/arbitraryJsonTypes.hpp"
#include "FastEngine/extra/extra_string.hpp"
#include "FastEngine/manager/reg_manager.hpp"

#include <fstream>
#include <iomanip>

namespace fge
{

Object::Object() :
        fge::Anchor(this)
{}
Object::Object(Object const& r) :
        fge::Transformable(r),
        fge::Anchor(this, r)
{}
Object::Object(Object&& r) noexcept :
        fge::Transformable(std::move(r)),
        fge::Anchor(this, r)
{}

void Object::first([[maybe_unused]] fge::Scene* scene) {}
void Object::callbackRegister([[maybe_unused]] fge::Event& event,
                              [[maybe_unused]] fge::GuiElementHandler* guiElementHandlerPtr)
{}
#ifdef FGE_DEF_SERVER
void Object::update([[maybe_unused]] fge::Event& event,
                    [[maybe_unused]] std::chrono::microseconds const& deltaTime,
                    [[maybe_unused]] fge::Scene* scene)
#else
void Object::update([[maybe_unused]] fge::RenderWindow& screen,
                    [[maybe_unused]] fge::Event& event,
                    [[maybe_unused]] std::chrono::microseconds const& deltaTime,
                    [[maybe_unused]] fge::Scene* scene)
#endif //FGE_DEF_SERVER
{}

#ifndef FGE_DEF_SERVER
void Object::draw([[maybe_unused]] fge::RenderTarget& target, [[maybe_unused]] fge::RenderStates const& states) const {}
#endif //FGE_DEF_SERVER
void Object::networkRegister() {}
void Object::removed([[maybe_unused]] fge::Scene* scene) {}

fge::Object* Object::copy()
{
    return fge::reg::Duplicate(this);
}

void Object::save(nlohmann::json& jsonObject, [[maybe_unused]] fge::Scene* scene)
{
    jsonObject["_pos"] = this->getPosition();
    jsonObject["_rotation"] = this->getRotation();
    jsonObject["_scale"] = this->getScale();
    jsonObject["_origin"] = this->getOrigin();

    jsonObject["tags"] = nlohmann::json::array();
    for (auto const& tag: this->_tags)
    {
        jsonObject["tags"] += tag;
    }
}
void Object::load(nlohmann::json& jsonObject, [[maybe_unused]] fge::Scene* scene)
{
    this->setPosition(jsonObject["_pos"].get<fge::Vector2f>());
    this->setRotation(jsonObject["_rotation"].get<float>());
    this->setScale(jsonObject["_scale"].get<fge::Vector2f>());
    this->setOrigin(jsonObject["_origin"].get<fge::Vector2f>());

    this->_tags.clear();

    for (auto& it: jsonObject["tags"])
    {
        this->_tags.add(it.get<std::string>());
    }
}

void Object::pack(fge::net::Packet& pck)
{
    pck << this->getPosition() << this->getRotation() << this->getScale() << this->getOrigin();
}
void Object::unpack(fge::net::Packet const& pck)
{
    fge::Vector2f buffVec2f;
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

char const* Object::getClassName() const
{
    return FGE_OBJ_BADCLASSNAME;
}
char const* Object::getReadableClassName() const
{
    return FGE_OBJ_BADCLASSNAME;
}

fge::RectFloat Object::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::Quad Object::getGlobalQuad() const
{
    return this->getTransform() * this->getLocalQuad();
}
fge::RectFloat Object::getLocalBounds() const
{
    return {{0.0f, 0.0f}, {1.0f, 1.0f}};
}
fge::Quad Object::getLocalQuad() const
{
    return fge::Quad(this->getLocalBounds());
}

bool Object::saveInFile(std::string const& path)
{
    nlohmann::json objNewJson = nlohmann::json::object();
    nlohmann::json& objJson = objNewJson[this->getClassName()];

    objJson = nlohmann::json::object();

    this->save(objJson, FGE_OBJ_NOSCENE);

    std::ofstream outFile(path);
    if (outFile)
    {
        outFile << std::setw(2) << objNewJson << std::endl;
        outFile.close();
        return true;
    }
    outFile.close();
    return false;
}
bool Object::loadFromFile(std::string const& path)
{
    std::ifstream inFile(path);
    if (!inFile)
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
fge::Object* Object::LoadFromFile(std::string const& path)
{
    std::ifstream inFile(path);
    if (!inFile)
    {
        inFile.close();
        return nullptr;
    }

    nlohmann::json inputJson;
    inFile >> inputJson;

    fge::Object* buffObj = fge::reg::GetNewClassOf(inputJson.begin().key());
    if (buffObj != nullptr)
    {
        nlohmann::json& objJson = inputJson.begin().value();

        buffObj->load(objJson, FGE_OBJ_NOSCENE);
        return buffObj;
    }
    return nullptr;
}

fge::GuiElement* Object::getGuiElement()
{
    return nullptr;
}

glm::mat4 Object::getParentsTransform() const
{
    glm::mat4 parentsTransform(1.0f);
    if (auto myObject = this->_myObjectData.lock())
    {
        auto parent = myObject->getParent().lock();
        while (parent)
        {
            parentsTransform *= parent->getObject()->getTransform();
            parent = parent->getParent().lock();
        }
    }
    return parentsTransform;
}
fge::Vector2f Object::getParentsScale() const
{
    fge::Vector2f parentsScale{1.0f, 1.0f};
    if (auto myObject = this->_myObjectData.lock())
    {
        auto parent = myObject->getParent().lock();
        while (parent)
        {
            parentsScale.x *= parent->getObject()->getScale().x;
            parentsScale.y *= parent->getObject()->getScale().y;
            parent = parent->getParent().lock();
        }
    }
    return parentsScale;
}

} // namespace fge

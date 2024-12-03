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
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/manager/reg_manager.hpp"
#include "FastEngine/network/C_packet.hpp"

namespace fge
{

Object::Object() :
        fge::Anchor(this),
        _children(this)
{}
Object::Object(Object const& r) :
        fge::Transformable(r),
        fge::Anchor(this, r),
        _children(this)
{}
Object::Object(Object&& r) noexcept :
        fge::Transformable(std::move(r)),
        fge::Anchor(this, r),
        _children(this)
{}

void Object::first([[maybe_unused]] fge::Scene& scene) {}
void Object::transfered([[maybe_unused]] fge::Scene& oldScene, [[maybe_unused]] fge::Scene& newScene) {}
void Object::callbackRegister([[maybe_unused]] fge::Event& event,
                              [[maybe_unused]] fge::GuiElementHandler* guiElementHandlerPtr)
{}
#ifdef FGE_DEF_SERVER
void Object::update([[maybe_unused]] fge::Event& event,
                    [[maybe_unused]] fge::DeltaTime const& deltaTime,
                    [[maybe_unused]] fge::Scene& scene)
{}
void Object::update(fge::Event& event, fge::DeltaTime const& deltaTime)
{
    if (auto myObject = this->_myObjectData.lock())
    {
        this->update(event, deltaTime, *myObject->getScene());
    }
}
#else
void Object::update([[maybe_unused]] fge::RenderTarget& target,
                    [[maybe_unused]] fge::Event& event,
                    [[maybe_unused]] fge::DeltaTime const& deltaTime,
                    [[maybe_unused]] fge::Scene& scene)
{}
void Object::update(fge::RenderTarget& target, fge::Event& event, fge::DeltaTime const& deltaTime)
{
    if (auto myObject = this->_myObjectData.lock())
    {
        this->update(target, event, deltaTime, *myObject->getScene());
    }
}
#endif //FGE_DEF_SERVER

#ifndef FGE_DEF_SERVER
void Object::draw([[maybe_unused]] fge::RenderTarget& target, [[maybe_unused]] fge::RenderStates const& states) const {}
#endif //FGE_DEF_SERVER
void Object::networkRegister() {}
void Object::netSignaled([[maybe_unused]] int8_t signal) {}
void Object::removed([[maybe_unused]] fge::Scene& scene) {}

fge::Object* Object::copy()
{
    return fge::reg::Duplicate(this);
}

void Object::save(nlohmann::json& jsonObject)
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
void Object::load(nlohmann::json& jsonObject)
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

bool Object::saveInFile(std::filesystem::path const& path, int fieldWidth, bool saveClassName)
{
    nlohmann::json objNewJson = nlohmann::json::object();

    if (saveClassName)
    {
        nlohmann::json& objJson = objNewJson[this->getClassName()];
        objJson = nlohmann::json::object();
        this->save(objJson);
    }
    else
    {
        this->save(objNewJson);
    }
    return SaveJsonToFile(path, objNewJson, fieldWidth);
}
bool Object::loadFromFile(std::filesystem::path const& path, bool loadClassName)
{
    nlohmann::json inputJson;
    if (!LoadJsonFromFile(path, inputJson))
    {
        return false;
    }

    if (!inputJson.is_object() || inputJson.size() == 0)
    {
        return false;
    }

    if (loadClassName)
    {
        if (inputJson.begin().key() != this->getClassName())
        {
            return false;
        }

        nlohmann::json& objJson = inputJson.begin().value();

        this->load(objJson);
        return true;
    }

    this->load(inputJson);
    return true;
}
std::unique_ptr<fge::Object> Object::LoadFromFile(std::filesystem::path const& path)
{
    nlohmann::json inputJson;
    if (!LoadJsonFromFile(path, inputJson))
    {
        return nullptr;
    }

    if (!inputJson.is_object() || inputJson.size() == 0)
    {
        return nullptr;
    }

    auto const& className = inputJson.begin().key();

    auto newObject = std::unique_ptr<fge::Object>(fge::reg::GetNewClassOf(className));
    if (newObject)
    {
        nlohmann::json& objJson = inputJson.begin().value();

        newObject->load(objJson);
        return newObject;
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

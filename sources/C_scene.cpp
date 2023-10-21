/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/C_scene.hpp"
#include "FastEngine/C_clientList.hpp"
#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/C_random.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/manager/network_manager.hpp"
#include "FastEngine/manager/reg_manager.hpp"

#include <fstream>
#include <iomanip>
#include <memory>

namespace fge
{

Scene::Scene() :
        g_name(),

        g_perClientSyncs(),
        g_enableNetworkEventsFlag(false),

        g_customView(),
        g_linkedRenderTarget(nullptr),

        g_updateCount(0),
        g_deleteMe(false),
        g_updatedObjectIterator(),

        g_data(),
        g_dataMap(),
        g_planDataMap(),

        g_callbackContext({nullptr, nullptr})
{
    this->g_updatedObjectIterator = this->g_data.end();
}
Scene::Scene(std::string sceneName) :
        g_name(std::move(sceneName)),

        g_perClientSyncs(),
        g_enableNetworkEventsFlag(false),

        g_customView(),
        g_linkedRenderTarget(nullptr),

        g_updateCount(0),
        g_deleteMe(false),
        g_updatedObjectIterator(),

        g_data(),
        g_dataMap(),
        g_planDataMap(),

        g_callbackContext({nullptr, nullptr})
{
    this->g_updatedObjectIterator = this->g_data.end();
}
Scene::Scene(Scene const& r) :
        _netList(),
        _properties(r._properties),

        g_name(r.g_name),

        g_perClientSyncs(),
        g_enableNetworkEventsFlag(r.g_enableNetworkEventsFlag),

        g_customView(r.g_customView),
        g_linkedRenderTarget(r.g_linkedRenderTarget),

        g_updateCount(r.g_updateCount),
        g_deleteMe(false),
        g_updatedObjectIterator(),

        g_data(),
        g_dataMap(),
        g_planDataMap(),

        g_callbackContext(r.g_callbackContext)
{
    this->g_updatedObjectIterator = this->g_data.end();

    for (auto const& objectData: r.g_data)
    {
        this->newObject(FGE_NEWOBJECT_PTR(objectData->g_object->copy()), objectData->g_plan, objectData->g_sid,
                        objectData->g_type);
    }
    //TODO: make sure to copy object parent too
}

Scene& Scene::operator=(Scene const& r)
{
    this->clear();

    this->_properties = r._properties;

    this->g_name = r.g_name;

    this->g_perClientSyncs.clear();
    this->g_enableNetworkEventsFlag = r.g_enableNetworkEventsFlag;

    this->g_customView = r.g_customView;
    this->g_linkedRenderTarget = r.g_linkedRenderTarget;

    this->g_updateCount = r.g_updateCount;
    this->g_deleteMe = false;
    this->g_updatedObjectIterator = this->g_data.end();

    this->g_callbackContext = r.g_callbackContext;

    for (auto const& objectData: r.g_data)
    {
        this->newObject(FGE_NEWOBJECT_PTR(objectData->g_object->copy()), objectData->g_plan, objectData->g_sid,
                        objectData->g_type);
    }
    //TODO: make sure to copy object parent too
    return *this;
}

/** Scene **/
#ifdef FGE_DEF_SERVER
void Scene::update(fge::Event& event,
                   std::chrono::microseconds const& deltaTime,
                   std::underlying_type_t<UpdateFlags> flags)
#else
void Scene::update(fge::RenderWindow& screen,
                   fge::Event& event,
                   std::chrono::microseconds const& deltaTime,
                   std::underlying_type_t<UpdateFlags> flags)
#endif //FGE_DEF_SERVER
{
    for (this->g_updatedObjectIterator = this->g_data.begin(); this->g_updatedObjectIterator != this->g_data.end();
         ++this->g_updatedObjectIterator)
    {
        if ((*this->g_updatedObjectIterator)->g_object->isNeedingAnchorUpdate())
        {
            (*this->g_updatedObjectIterator)->g_object->updateAnchor();
        }

#ifdef FGE_DEF_SERVER
        (*this->g_updatedObjectIterator)->g_object->update(event, deltaTime, this);
#else
        (*this->g_updatedObjectIterator)->g_object->update(screen, event, deltaTime, this);
#endif //FGE_DEF_SERVER
        if (this->g_deleteMe)
        {
            this->g_deleteMe = false;
            if (this->g_enableNetworkEventsFlag)
            {
                this->pushEvent({fge::SceneNetEvent::SEVT_DELOBJECT, (*this->g_updatedObjectIterator)->g_sid});
            }

            (*this->g_updatedObjectIterator)->g_object->removed(this);
            this->_onRemoveObject.call(this, *this->g_updatedObjectIterator);
            (*this->g_updatedObjectIterator)->g_linkedScene = nullptr;
            (*this->g_updatedObjectIterator)->g_object->_myObjectData.reset();
            auto objectPlan = (*this->g_updatedObjectIterator)->g_plan;
            this->hash_updatePlanDataMap(objectPlan, this->g_updatedObjectIterator, true);
            this->g_updatedObjectIterator = --this->g_data.erase(this->g_updatedObjectIterator);

            this->_onPlanUpdate.call(this, objectPlan);
        }
    }

    if ((flags & UpdateFlags::INCREMENT_UPDATE_COUNT) > 0)
    {
        ++this->g_updateCount;
    }
}
uint16_t Scene::getUpdateCount() const
{
    return this->g_updateCount;
}

#ifndef FGE_DEF_SERVER
void Scene::draw(fge::RenderTarget& target, fge::RenderStates const& states) const
{
    this->_onDraw.call(this, target);

    fge::RectFloat const screenBounds = fge::GetScreenRect(target);

    fge::View const backupView = target.getView();
    if (this->g_customView)
    {
        target.setView(*this->g_customView);
    }

    fge::ObjectPlanDepth depthCount = 0;
    auto planDataMapIt = this->g_planDataMap.begin();

    for (auto objectIt = this->g_data.begin(); objectIt != this->g_data.end(); ++objectIt)
    {
        //Check plan depth
        if (planDataMapIt != this->g_planDataMap.end())
        {
            if (objectIt == planDataMapIt->second)
            { //New plan, we reset depth count
                depthCount = 0;
                ++planDataMapIt; //go next plan ...
            }
        }

        (*objectIt)->g_planDepth = depthCount++;

        fge::Object* object = (*objectIt)->g_object.get();

        if (object->_drawMode == fge::Object::DrawModes::DRAW_ALWAYS_HIDDEN)
        {
            continue;
        }

        if (object->_drawMode == fge::Object::DrawModes::DRAW_IF_ON_TARGET)
        {
            fge::RectFloat objectBounds = object->getGlobalBounds();
            if (objectBounds._width == 0.0f)
            {
                ++objectBounds._width;
            }
            if (objectBounds._height == 0.0f)
            {
                ++objectBounds._height;
            }

            if (!objectBounds.findIntersection(screenBounds))
            {
                continue;
            }
        }

        target.draw(*object, states);
    }

    target.setView(backupView);
}
#endif //FGE_DEF_SERVER

fge::ObjectPlanDepth Scene::updatePlanDepth(fge::ObjectSid sid)
{
    auto it = this->g_dataMap.find(sid);

    if (it != this->g_dataMap.end())
    {
        auto plan = it->second->get()->g_plan;
        auto firstPlanObjectIt = this->g_planDataMap.find(plan)->second;

        fge::ObjectPlanDepth const planDepth = std::distance(firstPlanObjectIt, it->second);
        it->second->get()->g_planDepth = planDepth;

        this->_onPlanUpdate.call(this, plan);
        return planDepth;
    }
    return FGE_SCENE_BAD_PLANDEPTH;
}
void Scene::updateAllPlanDepth(fge::ObjectPlan plan)
{
    auto it = this->g_planDataMap.find(plan);

    if (it != this->g_planDataMap.end())
    {
        fge::ObjectPlanDepth depthCount = 0;

        for (auto objectIt = it->second; objectIt != this->g_data.end(); ++objectIt)
        {
            if (objectIt->get()->g_plan != plan)
            {
                break;
            }
            (*objectIt)->g_planDepth = depthCount++;
        }

        this->_onPlanUpdate.call(this, plan);
    }
}
void Scene::updateAllPlanDepth()
{
    fge::ObjectPlanDepth depthCount = 0;
    auto planDataMapIt = this->g_planDataMap.begin();

    for (auto objectIt = this->g_data.begin(); objectIt != this->g_data.end(); ++objectIt)
    {
        //Check plan depth
        if (planDataMapIt != this->g_planDataMap.end())
        {
            if (objectIt == planDataMapIt->second)
            { //New plan, we reset depth count
                depthCount = 0;
                ++planDataMapIt; //go next plan ...
            }
        }

        (*objectIt)->g_planDepth = depthCount++;
    }

    this->_onPlanUpdate.call(this, FGE_SCENE_BAD_PLAN);
}

void Scene::clear()
{
    this->delAllObject(false);
    this->_properties.delAllProperties();
    this->_netList.clear();
}

/** Object **/
fge::ObjectDataShared Scene::newObject(fge::ObjectPtr&& newObject,
                                       fge::ObjectPlan plan,
                                       fge::ObjectSid sid,
                                       fge::ObjectType type,
                                       bool silent)
{
    if (newObject == nullptr)
    {
        return nullptr;
    }
    fge::ObjectSid generatedSid = this->generateSid(sid);
    if (generatedSid == FGE_SCENE_BAD_SID)
    {
        return nullptr;
    }
    if (this->g_enableNetworkEventsFlag)
    {
        this->pushEvent({fge::SceneNetEvent::SEVT_NEWOBJECT, generatedSid});
    }

    auto it = this->hash_getInsertionIteratorFromPlanDataMap(plan);

    it = this->g_data.insert(it,
                             std::make_shared<fge::ObjectData>(this, std::move(newObject), generatedSid, plan, type));
    this->g_dataMap[generatedSid] = it;
    (*it)->g_object->_myObjectData = *it;
    this->hash_updatePlanDataMap(plan, it, false);
    if ((this->g_updatedObjectIterator != this->g_data.end()) && (*it)->g_parent.expired())
    { //An object is created inside another object and orphan, make it parent
        (*it)->g_parent = *this->g_updatedObjectIterator;
    }
    if (!silent)
    {
        (*it)->g_object->first(this);
    }

    if ((*it)->g_object->_callbackContextMode == fge::Object::CallbackContextModes::CONTEXT_AUTO &&
        this->g_callbackContext._event != nullptr && !silent)
    {
        (*it)->g_object->callbackRegister(*this->g_callbackContext._event, this->g_callbackContext._guiElementHandler);
    }

    this->_onNewObject.call(this, *it);
    this->_onPlanUpdate.call(this, plan);

    return *it;
}
fge::ObjectDataShared Scene::newObject(fge::ObjectDataShared const& objectData, bool silent)
{
    fge::ObjectSid generatedSid = this->generateSid(objectData->g_sid);
    if (generatedSid == FGE_SCENE_BAD_SID)
    {
        return nullptr;
    }
    if (this->g_enableNetworkEventsFlag)
    {
        this->pushEvent({fge::SceneNetEvent::SEVT_NEWOBJECT, generatedSid});
    }

    objectData->g_sid = generatedSid;

    auto it = this->hash_getInsertionIteratorFromPlanDataMap(objectData->g_plan);

    it = this->g_data.insert(it, objectData);
    this->g_dataMap[generatedSid] = it;
    objectData->g_linkedScene = this;
    objectData->g_object->_myObjectData = objectData;
    this->hash_updatePlanDataMap(objectData->g_plan, it, false);
    if ((this->g_updatedObjectIterator != this->g_data.end()) && objectData->g_parent.expired())
    { //An object is created inside another object and orphan, make it parent
        objectData->g_parent = *this->g_updatedObjectIterator;
    }
    if (!silent)
    {
        objectData->g_object->first(this);
    }

    if (objectData->g_object->_callbackContextMode == fge::Object::CallbackContextModes::CONTEXT_AUTO &&
        this->g_callbackContext._event != nullptr && !silent)
    {
        objectData->g_object->callbackRegister(*this->g_callbackContext._event,
                                               this->g_callbackContext._guiElementHandler);
    }

    this->_onNewObject.call(this, objectData);
    this->_onPlanUpdate.call(this, objectData->g_plan);

    return objectData;
}

fge::ObjectDataShared Scene::duplicateObject(fge::ObjectSid sid, fge::ObjectSid newSid)
{
    auto it = this->g_dataMap.find(sid);

    if (it != this->g_dataMap.end())
    {
        fge::ObjectDataShared object = *it->second;
        fge::ObjectDataShared newObject = std::make_shared<fge::ObjectData>();
        newObject->g_object.reset(object->g_object->copy());
        newObject->g_plan = object->g_plan;
        newObject->g_sid = newSid;
        newObject->g_type = object->g_type;

        return this->newObject(newObject);
    }

    return nullptr;
}

fge::ObjectDataShared Scene::transferObject(fge::ObjectSid sid, fge::Scene& newScene)
{
    auto it = this->g_dataMap.find(sid);

    if (it != this->g_dataMap.end())
    {
        if (!newScene.isValid(sid))
        {
            fge::ObjectDataShared buff = std::move(*it->second);
            buff->g_object->removed(this);
            this->_onRemoveObject.call(this, buff);
            this->hash_updatePlanDataMap(buff->g_plan, it->second, true);
            this->g_data.erase(it->second);
            this->g_dataMap.erase(it);

            this->_onPlanUpdate.call(this, buff->g_plan);

            if (this->g_enableNetworkEventsFlag)
            {
                this->pushEvent({fge::SceneNetEvent::SEVT_DELOBJECT, sid});
            }

            return newScene.newObject(buff);
        }
    }
    return nullptr;
}

void Scene::delUpdatedObject()
{
    this->g_deleteMe = true;
}
bool Scene::delObject(fge::ObjectSid sid)
{
    auto it = this->g_dataMap.find(sid);

    if (it != this->g_dataMap.end())
    {
        if (this->g_enableNetworkEventsFlag)
        {
            this->pushEvent({fge::SceneNetEvent::SEVT_DELOBJECT, (*it->second)->g_sid});
        }

        (*it->second)->g_object->removed(this);
        this->_onRemoveObject.call(this, *it->second);
        (*it->second)->g_linkedScene = nullptr;
        (*it->second)->g_object->_myObjectData.reset();
        auto objectPlan = (*it->second)->g_plan;
        this->hash_updatePlanDataMap(objectPlan, it->second, true);
        this->g_data.erase(it->second);
        this->g_dataMap.erase(it);

        this->_onPlanUpdate.call(this, objectPlan);

        return true;
    }
    return false;
}
std::size_t Scene::delAllObject(bool ignoreGuiObject)
{
    if (this->g_enableNetworkEventsFlag)
    {
        this->clearNetEventsQueue();
        this->pushEvent({fge::SceneNetEvent::SEVT_DELOBJECT, FGE_SCENE_BAD_SID});
    }

    std::size_t buffSize = this->g_data.size();
    for (auto it = this->g_data.begin(); it != this->g_data.end(); ++it)
    {
        if (ignoreGuiObject)
        {
            if ((*it)->g_type == fge::ObjectType::TYPE_GUI)
            {
                --buffSize;
                continue;
            }
        }

        (*it)->g_object->removed(this);
        this->_onRemoveObject.call(this, *it);
        (*it)->g_linkedScene = nullptr;
        (*it)->g_object->_myObjectData.reset();
        this->hash_updatePlanDataMap((*it)->g_plan, it, true);

        this->g_dataMap.erase((*it)->g_sid);
        it = --this->g_data.erase(it);
    }

    this->_onPlanUpdate.call(this, FGE_SCENE_BAD_PLAN);
    return buffSize;
}

bool Scene::setObjectSid(fge::ObjectSid sid, fge::ObjectSid newSid)
{
    if (newSid == FGE_SCENE_BAD_SID)
    {
        return false;
    }
    if (sid == newSid)
    {
        return true;
    }

    auto it = this->g_dataMap.find(newSid);

    if (it == this->g_dataMap.end())
    {
        it = this->g_dataMap.find(sid);

        if (it != this->g_dataMap.end())
        {
            if (this->g_enableNetworkEventsFlag)
            {
                this->pushEvent({fge::SceneNetEvent::SEVT_DELOBJECT, (*it->second)->g_sid});
                this->pushEvent({fge::SceneNetEvent::SEVT_NEWOBJECT, newSid});
            }

            (*it->second)->g_sid = newSid;
            this->g_dataMap[newSid] = std::move(it->second);
            this->g_dataMap.erase(it);
            return true;
        }
    }
    return false;
}
bool Scene::setObject(fge::ObjectSid sid, fge::ObjectPtr&& newObject)
{
    if (newObject == nullptr)
    {
        return false;
    }

    auto it = this->g_dataMap.find(sid);

    if (it != this->g_dataMap.end())
    {
        if (this->g_enableNetworkEventsFlag)
        {
            this->pushEvent({fge::SceneNetEvent::SEVT_NEWOBJECT, (*it->second)->g_sid});
        }

        (*it->second)->g_object->removed(this);
        (*it->second)->g_linkedScene = nullptr;
        (*it->second)->g_object->_myObjectData.reset();

        (*it->second) = std::make_shared<fge::ObjectData>(this, std::move(newObject), (*it->second)->g_sid,
                                                          (*it->second)->g_plan, (*it->second)->g_type);
        (*it->second)->g_object->_myObjectData = *it->second;
        (*it->second)->g_object->first(this);

        if ((*it->second)->g_object->_callbackContextMode == fge::Object::CallbackContextModes::CONTEXT_AUTO &&
            this->g_callbackContext._event != nullptr)
        {
            (*it->second)
                    ->g_object->callbackRegister(*this->g_callbackContext._event,
                                                 this->g_callbackContext._guiElementHandler);
        }
        return true;
    }
    return false;
}
bool Scene::setObjectPlan(fge::ObjectSid sid, fge::ObjectPlan newPlan)
{
    auto it = this->g_dataMap.find(sid);

    if (it != this->g_dataMap.end())
    {
        this->hash_updatePlanDataMap((*it->second)->g_plan, it->second, true);

        auto newPosIt = this->hash_getInsertionIteratorFromPlanDataMap(newPlan);

        auto oldPlan = (*it->second)->g_plan;
        (*it->second)->g_plan = newPlan;

        this->g_data.splice(newPosIt, this->g_data, it->second);
        this->hash_updatePlanDataMap(newPlan, it->second, false);

        if (oldPlan != newPlan)
        {
            this->_onPlanUpdate.call(this, oldPlan);
        }
        this->_onPlanUpdate.call(this, newPlan);
        return true;
    }
    return false;
}
bool Scene::setObjectPlanTop(fge::ObjectSid sid)
{
    auto it = this->g_dataMap.find(sid);

    if (it != this->g_dataMap.end())
    {
        auto newPosIt = this->g_planDataMap.find((*it->second)->g_plan);

        if (it->second == newPosIt->second)
        { //already on top
            return true;
        }

        this->g_data.splice(newPosIt->second, this->g_data, it->second);
        this->hash_updatePlanDataMap((*it->second)->g_plan, it->second, false);

        this->_onPlanUpdate.call(this, (*it->second)->g_plan);
        return true;
    }
    return false;
}
bool Scene::setObjectPlanBot(fge::ObjectSid sid)
{
    auto it = this->g_dataMap.find(sid);

    if (it != this->g_dataMap.end())
    {
        auto plan = (*it->second)->g_plan;
        auto planIt = this->g_planDataMap.find(plan);
        auto planItAfter = planIt;
        ++planItAfter; //Next plan

        bool wasOnTop = false;
        if (it->second == planIt->second)
        { //is on top
            wasOnTop = true;
            this->hash_updatePlanDataMap(plan, it->second, true);
        }

        if (planItAfter == this->g_planDataMap.end())
        { //object can be pushed at the end
            this->g_data.splice(this->g_data.end(), this->g_data, it->second);
            if (wasOnTop)
            {
                this->hash_updatePlanDataMap(plan, it->second, false);
            }
        }
        else
        {
            this->g_data.splice(planItAfter->second, this->g_data, it->second);
            if (wasOnTop)
            {
                this->hash_updatePlanDataMap(plan, it->second, false);
            }
        }

        this->_onPlanUpdate.call(this, plan);

        return true;
    }
    return false;
}

fge::ObjectDataShared Scene::getObject(fge::ObjectSid sid) const
{
    auto it = this->g_dataMap.find(sid);
    if (it != this->g_dataMap.cend())
    {
        return *it->second;
    }
    return nullptr;
}
fge::ObjectDataShared Scene::getObject(fge::Object const* ptr) const
{
    auto it = this->find(ptr);
    if (it != this->g_data.cend())
    {
        return (*it);
    }
    return nullptr;
}
fge::Object* Scene::getObjectPtr(fge::ObjectSid sid) const
{
    auto it = this->g_dataMap.find(sid);
    if (it != this->g_dataMap.cend())
    {
        return (*it->second)->g_object.get();
    }
    return nullptr;
}
fge::ObjectDataShared Scene::getUpdatedObject() const
{
    return *this->g_updatedObjectIterator;
}

/** Search function **/
std::size_t Scene::getAllObj_ByPosition(fge::Vector2f const& pos, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (auto const& data: this->g_data)
    {
        auto objBounds = data->g_object->getGlobalBounds();
        if (objBounds.contains(pos))
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}
std::size_t Scene::getAllObj_ByZone(fge::RectFloat const& zone, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (auto const& data: this->g_data)
    {
        auto objBounds = data->g_object->getGlobalBounds();
        if (objBounds.findIntersection(zone))
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}
#ifndef FGE_DEF_SERVER
std::size_t Scene::getAllObj_ByLocalPosition(fge::Vector2i const& pos,
                                             fge::RenderTarget const& target,
                                             fge::ObjectContainer& buff) const
{
    return this->getAllObj_ByPosition(
            target.mapPixelToCoords(pos, this->g_customView ? *this->g_customView : target.getView()), buff);
}
std::size_t Scene::getAllObj_ByLocalZone(fge::RectInt const& zone,
                                         fge::RenderTarget const& target,
                                         fge::ObjectContainer& buff) const
{
    return this->getAllObj_ByZone(
            fge::PixelToCoordRect(zone, target, this->g_customView ? *this->g_customView : target.getView()), buff);
}
std::size_t Scene::getAllObj_FromLocalPosition(fge::Vector2i const& pos,
                                               fge::RenderTarget const& target,
                                               fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (auto const& data: this->g_data)
    {
        auto objBounds = fge::CoordToPixelRect(data->g_object->getGlobalBounds(), target,
                                               this->g_customView ? *this->g_customView : target.getView());
        if (objBounds.contains(pos))
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}
std::size_t Scene::getAllObj_FromLocalZone(fge::RectInt const& zone,
                                           fge::RenderTarget const& target,
                                           fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (auto const& data: this->g_data)
    {
        auto objBounds = fge::CoordToPixelRect(data->g_object->getGlobalBounds(), target,
                                               this->g_customView ? *this->g_customView : target.getView());
        if (objBounds.findIntersection(zone))
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}
#endif //FGE_DEF_SERVER

std::size_t Scene::getAllObj_ByClass(std::string_view class_name, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (auto const& data: this->g_data)
    {
        if (data->g_object->getClassName() == class_name)
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}
std::size_t Scene::getAllObj_ByTag(std::string_view tag_name, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (auto const& data: this->g_data)
    {
        if (data->g_object->_tags.check(tag_name))
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}

fge::ObjectDataShared Scene::getFirstObj_ByPosition(fge::Vector2f const& pos) const
{
    for (auto const& data: this->g_data)
    {
        fge::ObjectPtr const& buffObj = data->g_object;
        auto objBounds = buffObj->getGlobalBounds();
        if (objBounds.contains(pos))
        {
            return data;
        }
    }
    return nullptr;
}
fge::ObjectDataShared Scene::getFirstObj_ByZone(fge::RectFloat const& zone) const
{
    for (auto const& data: this->g_data)
    {
        fge::ObjectPtr const& buffObj = data->g_object;
        auto objBounds = buffObj->getGlobalBounds();
        if (objBounds.findIntersection(zone))
        {
            return data;
        }
    }
    return nullptr;
}

#ifndef FGE_DEF_SERVER
fge::ObjectDataShared Scene::getFirstObj_ByLocalPosition(fge::Vector2i const& pos,
                                                         fge::RenderTarget const& target) const
{
    return this->getFirstObj_ByPosition(
            target.mapPixelToCoords(pos, this->g_customView ? *this->g_customView : target.getView()));
}
fge::ObjectDataShared Scene::getFirstObj_ByLocalZone(fge::RectInt const& zone, fge::RenderTarget const& target) const
{
    return this->getFirstObj_ByZone(
            fge::PixelToCoordRect(zone, target, this->g_customView ? *this->g_customView : target.getView()));
}
fge::ObjectDataShared Scene::getFirstObj_FromLocalPosition(fge::Vector2i const& pos,
                                                           fge::RenderTarget const& target) const
{
    for (auto const& data: this->g_data)
    {
        fge::ObjectPtr const& buffObj = data->g_object;
        auto objBounds = fge::CoordToPixelRect(buffObj->getGlobalBounds(), target,
                                               this->g_customView ? *this->g_customView : target.getView());
        if (objBounds.contains(pos))
        {
            return data;
        }
    }
    return nullptr;
}
fge::ObjectDataShared Scene::getFirstObj_FromLocalZone(fge::RectInt const& zone, fge::RenderTarget const& target) const
{
    for (auto const& data: this->g_data)
    {
        fge::ObjectPtr const& buffObj = data->g_object;
        auto objBounds = fge::CoordToPixelRect(buffObj->getGlobalBounds(), target,
                                               this->g_customView ? *this->g_customView : target.getView());
        if (objBounds.findIntersection(zone))
        {
            return data;
        }
    }
    return nullptr;
}
#endif //FGE_DEF_SERVER

fge::ObjectDataShared Scene::getFirstObj_ByClass(std::string_view class_name) const
{
    for (auto const& data: this->g_data)
    {
        if (data->g_object->getClassName() == class_name)
        {
            return data;
        }
    }
    return nullptr;
}
fge::ObjectDataShared Scene::getFirstObj_ByTag(std::string_view tag_name) const
{
    for (auto const& data: this->g_data)
    {
        if (data->g_object->_tags.check(tag_name))
        {
            return data;
        }
    }
    return nullptr;
}

/** Static id **/
fge::ObjectSid Scene::getSid(fge::Object const* ptr) const
{
    for (auto const& data: this->g_data)
    {
        if (*data == ptr)
        {
            return data->g_sid;
        }
    }
    return FGE_SCENE_BAD_SID;
}

fge::ObjectSid Scene::generateSid(fge::ObjectSid wanted_sid) const
{
    if (wanted_sid != FGE_SCENE_BAD_SID)
    {
        if (this->g_dataMap.find(wanted_sid) == this->g_dataMap.cend())
        {
            return wanted_sid;
        }
        else
        {
            return FGE_SCENE_BAD_SID;
        }
    }

    fge::ObjectSid new_sid;
    while (true)
    {
        new_sid = fge::_random.range<fge::ObjectSid>(0, FGE_SCENE_BAD_SID);

        if (this->g_dataMap.find(wanted_sid) == this->g_dataMap.cend())
        {
            return new_sid;
        }
    }
}

/** Network **/
void Scene::pack(fge::net::Packet& pck)
{
    //update count
    pck << this->g_updateCount;

    //scene name
    pck << this->g_name;

    //scene data
    for (std::size_t i = 0; i < this->_netList.size(); ++i)
    {
        this->_netList[i]->packData(pck);
    }

    //object size
    pck << static_cast<fge::net::SizeType>(this->g_data.size());
    for (auto const& data: this->g_data)
    {
        if (data->g_type == fge::ObjectType::TYPE_GUI)
        { //Ignore GUI
            pck << static_cast<fge::ObjectSid>(FGE_SCENE_BAD_SID);
            continue;
        }

        //SID
        pck << data->g_sid;
        //CLASS
        pck << fge::reg::GetClassId(data->g_object->getClassName());
        //PLAN
        pck << data->g_plan;
        //TYPE
        pck << static_cast<std::underlying_type<fge::ObjectType>::type>(data->g_type);

        data->g_object->pack(pck);
    }
}
void Scene::pack(fge::net::Packet& pck, fge::net::Identity const& id)
{
    this->g_perClientSyncs.emplace(std::piecewise_construct, std::forward_as_tuple(id),
                                   std::forward_as_tuple(this->g_updateCount));
    this->pack(pck);
}
std::optional<fge::net::Error> Scene::unpack(fge::net::Packet const& pck)
{
    constexpr char const* const func = __func__;
    fge::reg::ClassId buffClass{FGE_REG_BADCLASSID};
    fge::ObjectPlan buffPlan{FGE_SCENE_PLAN_DEFAULT};
    fge::ObjectSid buffSid{FGE_SCENE_BAD_SID};

    using namespace fge::net::rules;

    //update count
    return RValid<decltype(this->g_updateCount)>({pck, &this->g_updateCount})
            .and_then([&](auto& chain) {
        //scene name
        return RValid(RSizeRange<std::string>(0, FGE_SCENE_LIMIT_NAMESIZE, chain.template newChain(&this->g_name)));
    })
            .and_then([&](auto& chain) {
        //scene data
        for (std::size_t i = 0; i < this->_netList.size(); ++i)
        {
            if (!this->_netList[i]->applyData(pck))
            {
                break;
            }
        }

        return chain;
    })
            .and_then([&](auto& chain) {
        //object size
        this->delAllObject(true);
        return RValid(chain.template newChain<fge::net::SizeType>());
    })
            .and_for_each(0, 1, [&](auto& chain, [[maybe_unused]] auto& i) {
        //SID
        pck >> buffSid;
        if (buffSid == FGE_SCENE_BAD_SID)
        {
            return chain.end(std::nullopt);
        }
        //CLASS
        pck >> buffClass;
        if (buffClass == FGE_REG_BADCLASSID)
        {
            return chain.end(
                    net::Error{net::Error::Types::ERR_EXTRACT, pck.getReadPos(), "received bad class ID", func});
        }
        //PLAN
        pck >> buffPlan;
        //TYPE
        return RStrictLess<std::underlying_type_t<fge::ObjectType>>(fge::ObjectType::TYPE_MAX_, pck)
                .and_then([&](auto& chain) {
            fge::ObjectPtr buffObject{fge::reg::GetNewClassOf(buffClass)};
            if (buffObject)
            {
                this->newObject(std::move(buffObject), buffPlan, buffSid, static_cast<fge::ObjectType>(chain.value()))
                        ->g_object->unpack(pck);
            }
            else
            {
                return chain.invalidate(
                        net::Error{net::Error::Types::ERR_EXTRACT, pck.getReadPos(), "unknown class ID", func});
            }

            return chain;
        }).end();
    }).end();
}
void Scene::packModification(fge::net::Packet& pck, fge::net::ClientList& clients, fge::net::Identity const& id)
{
    //update count range
    auto it = this->g_perClientSyncs.find(id);
    if (it != this->g_perClientSyncs.end())
    {
        pck << it->second._lastUpdateCount << this->g_updateCount;
    }
    else
    { //Should not really happen as clientCheckup is generally called before
        pck << this->g_updateCount << this->g_updateCount;
    }

    //scene name
    pck << this->g_name;

    //scene data
    fge::net::SizeType countSceneDataModification = 0;

    std::size_t rewritePos = pck.getDataSize();
    pck.pack(&countSceneDataModification, sizeof(countSceneDataModification)); //Will be rewritten

    for (std::size_t i = 0; i < this->_netList.size(); ++i)
    {
        fge::net::NetworkTypeBase* netType = this->_netList[i];

        netType->clientsCheckup(clients);
        if (netType->checkClient(id))
        {
            pck << static_cast<fge::net::SizeType>(i);
            netType->packData(pck, id);

            ++countSceneDataModification;
        }
    }
    pck.pack(rewritePos, &countSceneDataModification, sizeof(countSceneDataModification)); //Rewriting size

    //object size
    fge::net::SizeType countObject = 0;

    std::size_t countObjectPos = pck.getDataSize();
    pck.pack(&countObject, sizeof(countObject)); //Will be rewritten

    std::size_t dataPos = pck.getDataSize();
    constexpr std::size_t const reservedSize =
            sizeof(fge::ObjectSid) + sizeof(fge::reg::ClassId) + sizeof(fge::ObjectPlan) +
            sizeof(std::underlying_type<fge::ObjectType>::type) + sizeof(fge::net::SizeType);
    pck.append(reservedSize);

    for (auto const& data: this->g_data)
    {
        //MODIF COUNT/OBJECT DATA
        fge::net::SizeType countModification = 0;
        for (std::size_t i = 0; i < data->getObject()->_netList.size(); ++i)
        {
            fge::net::NetworkTypeBase* netType = data->getObject()->_netList[i];

            netType->clientsCheckup(clients);

            if (netType->checkClient(id))
            {
                pck << static_cast<fge::net::SizeType>(i);
                netType->packData(pck, id);

                ++countModification;
            }
        }
        if (countModification > 0)
        {
            //SID
            pck.pack(dataPos, &data->g_sid, sizeof(fge::ObjectSid));
            //CLASS
            fge::reg::ClassId tmpClass = fge::reg::GetClassId(data->getObject()->getClassName());
            pck.pack(dataPos + sizeof(fge::ObjectSid), &tmpClass, sizeof(fge::reg::ClassId));
            //PLAN
            pck.pack(dataPos + sizeof(fge::ObjectSid) + sizeof(fge::reg::ClassId), &data->g_plan,
                     sizeof(fge::ObjectPlan));
            //TYPE
            std::underlying_type<fge::ObjectType>::type tmpType = data->g_type;
            pck.pack(dataPos + sizeof(fge::ObjectSid) + sizeof(fge::reg::ClassId) + sizeof(fge::ObjectPlan), &tmpType,
                     sizeof(tmpType));

            pck.pack(dataPos + sizeof(fge::ObjectSid) + sizeof(fge::reg::ClassId) + sizeof(fge::ObjectPlan) +
                             sizeof(tmpType),
                     &countModification, sizeof(countModification));

            dataPos = pck.getDataSize();
            pck.append(reservedSize);

            ++countObject;
        }
    }

    pck.shrink(reservedSize);
    pck.pack(countObjectPos, &countObject, sizeof(countObject)); //Rewriting size
    clients.clearClientEvent();
}
void Scene::packModification(fge::net::Packet& pck, fge::net::Identity const& id)
{
    //update count range
    auto it = this->g_perClientSyncs.find(id);
    if (it != this->g_perClientSyncs.end())
    {
        pck << it->second._lastUpdateCount << this->g_updateCount;
        it->second._lastUpdateCount = this->g_updateCount; //keep track of last update count for the client
    }
    else
    { //Should not really happen as clientCheckup is generally called before
        pck << this->g_updateCount << this->g_updateCount;
    }

    //scene name
    pck << this->g_name;

    //scene data
    fge::net::SizeType countSceneDataModification = 0;

    std::size_t rewritePos = pck.getDataSize();
    pck.pack(&countSceneDataModification, sizeof(countSceneDataModification)); //Will be rewritten

    for (std::size_t i = 0; i < this->_netList.size(); ++i)
    {
        fge::net::NetworkTypeBase* netType = this->_netList[i];

        if (netType->checkClient(id))
        {
            pck << static_cast<fge::net::SizeType>(i);
            netType->packData(pck, id);

            ++countSceneDataModification;
        }
    }
    pck.pack(rewritePos, &countSceneDataModification, sizeof(countSceneDataModification)); //Rewriting size

    //object size
    fge::net::SizeType countObject = 0;

    std::size_t countObjectPos = pck.getDataSize();
    pck.pack(&countObject, sizeof(countObject)); //Will be rewritten

    std::size_t dataPos = pck.getDataSize();
    constexpr std::size_t const reservedSize =
            sizeof(fge::ObjectSid) + sizeof(fge::reg::ClassId) + sizeof(fge::ObjectPlan) +
            sizeof(std::underlying_type<fge::ObjectType>::type) + sizeof(fge::net::SizeType);
    pck.append(reservedSize);

    for (auto const& data: this->g_data)
    {
        //MODIF COUNT/OBJECT DATA
        fge::net::SizeType countModification = 0;
        for (std::size_t i = 0; i < data->getObject()->_netList.size(); ++i)
        {
            fge::net::NetworkTypeBase* netType = data->getObject()->_netList[i];

            if (netType->checkClient(id))
            {
                pck << static_cast<fge::net::SizeType>(i);
                netType->packData(pck, id);

                ++countModification;
            }
        }
        if (countModification)
        {
            //SID
            pck.pack(dataPos, &data->g_sid, sizeof(fge::ObjectSid));
            //CLASS
            fge::reg::ClassId tmpClass = fge::reg::GetClassId(data->getObject()->getClassName());
            pck.pack(dataPos + sizeof(fge::ObjectSid), &tmpClass, sizeof(fge::reg::ClassId));
            //PLAN
            pck.pack(dataPos + sizeof(fge::ObjectSid) + sizeof(fge::reg::ClassId), &data->g_plan,
                     sizeof(fge::ObjectPlan));
            //TYPE
            std::underlying_type<fge::ObjectType>::type tmpType = data->g_type;
            pck.pack(dataPos + sizeof(fge::ObjectSid) + sizeof(fge::reg::ClassId) + sizeof(fge::ObjectPlan), &tmpType,
                     sizeof(tmpType));

            pck.pack(dataPos + sizeof(fge::ObjectSid) + sizeof(fge::reg::ClassId) + sizeof(fge::ObjectPlan) +
                             sizeof(tmpType),
                     &countModification, sizeof(countModification));

            dataPos = pck.getDataSize();
            pck.append(reservedSize);

            ++countObject;
        }
    }

    pck.shrink(reservedSize);
    pck.pack(countObjectPos, &countObject, sizeof(countObject)); //Rewriting size
}
std::optional<fge::net::Error>
Scene::unpackModification(fge::net::Packet const& pck, UpdateCountRange& updateCountRange, bool isPreExtractedPacket)
{
    constexpr char const* const func = __func__;

    using namespace fge::net::rules;

    //update count range
    if (!isPreExtractedPacket)
    {
        pck >> updateCountRange._last >> updateCountRange._now;
        if (!pck)
        {
            return net::Error{net::Error::Types::ERR_EXTRACT, pck.getReadPos(), "received bad update count range",
                              func};
        }

        //Check if the pack is not old
        if (updateCountRange._last < this->g_updateCount ||
            (updateCountRange._last > this->g_updateCount && updateCountRange._now < this->g_updateCount))
        {
            return net::Error{net::Error::Types::ERR_SCENE_OLD_PACKET, pck.getReadPos(),
                              "old network updates for this scene", func};
        }

        //Check if the pack is continuous
        if (updateCountRange._last != this->g_updateCount)
        {
            return net::Error{net::Error::Types::ERR_SCENE_NEED_CACHING, pck.getReadPos(),
                              "discontinuity in the network updates for this scene", func};
        }
    }

    this->g_updateCount = updateCountRange._now;

    //scene name
    return RValid(RSizeRange<std::string>(0, FGE_SCENE_LIMIT_NAMESIZE, {pck, &this->g_name}))
            .and_then([&](auto& chain) {
        //scene data
        return RLess<fge::net::SizeType>(this->_netList.size(), chain.template newChain<fge::net::SizeType>());
    })
            .and_for_each(0, 1,
                          [&](auto& chain, [[maybe_unused]] auto& i) {
        return RValid(chain.template newChain<fge::net::SizeType>())
                .and_then([&](auto& chain) {
            this->_netList[chain.value()]->applyData(pck);
            return chain;
        }).end();
    })
            .and_then([&](auto& chain) {
        return RValid<fge::net::SizeType>(chain.template newChain<fge::net::SizeType>());
    })
            .and_for_each(0, 1, [&](auto& chain, [[maybe_unused]] auto& i) {
        fge::reg::ClassId buffClass{FGE_REG_BADCLASSID};
        fge::ObjectPlan buffPlan{FGE_SCENE_PLAN_DEFAULT};
        fge::ObjectSid buffSid{FGE_SCENE_BAD_SID};
        std::underlying_type_t<fge::ObjectType> buffType{fge::ObjectType::TYPE_NULL};

        //SID
        pck >> buffSid;
        //CLASS
        pck >> buffClass;
        //PLAN
        pck >> buffPlan;
        //TYPE
        auto err = RStrictLess<std::underlying_type_t<fge::ObjectType>>(fge::ObjectType::TYPE_MAX_, pck)
                           .apply(buffType)
                           .end();

        if (err)
        {
            return err;
        }

        auto buffObject = this->getObject(buffSid);
        if (!buffObject)
        {
            buffObject = this->newObject(fge::ObjectPtr{fge::reg::GetNewClassOf(buffClass)}, buffPlan, buffSid,
                                         static_cast<fge::ObjectType>(buffType));
            if (!buffObject)
            {
                return chain.end(
                        net::Error{net::Error::Types::ERR_EXTRACT, pck.getReadPos(), "unknown class ID / SID", func});
            }
        }

        //MODIF COUNT/OBJECT DATA
        auto& objectNetList = buffObject->g_object->_netList;

        return RLess<fge::net::SizeType>(objectNetList.size(), pck)
                .and_for_each(0, 1, [&](auto& chain, [[maybe_unused]] auto& i) {
            return RLess<fge::net::SizeType>(objectNetList.size(), chain.template newChain<fge::net::SizeType>())
                    .and_then([&](auto& chain) {
                objectNetList[chain.value()]->applyData(pck);
                return chain;
            }).end();
        }).end();
    }).end();
}

void Scene::packNeededUpdate(fge::net::Packet& pck)
{
    fge::net::SizeType countObject = 0;

    std::size_t countObjectPos = pck.getDataSize();
    pck.pack(&countObject, sizeof(countObject)); //Will be rewritten

    for (auto const& data: this->g_data)
    {
        std::size_t dataPos = pck.getDataSize();
        constexpr std::size_t const reservedSize = sizeof(fge::ObjectSid);
        pck.append(reservedSize);

        std::size_t count = data->getObject()->_netList.packNeededUpdate(pck);

        if (count > 0)
        {
            //SID
            pck.pack(dataPos, &data->g_sid, sizeof(fge::ObjectSid));
            ++countObject;
        }
        else
        {
            pck.shrink(reservedSize + sizeof(fge::net::SizeType));
        }
    }

    pck.pack(countObjectPos, &countObject, sizeof(countObject)); //Rewriting size
}
std::optional<fge::net::Error> Scene::unpackNeededUpdate(fge::net::Packet const& pck, fge::net::Identity const& id)
{
    constexpr char const* const func = __func__;

    using namespace fge::net::rules;

    return RValid<fge::net::SizeType>(pck)
            .and_for_each(0, 1, [&](auto& chain, [[maybe_unused]] auto& i) {
        return RMustEqual<fge::ObjectSid, ROutputs::R_INVERTED>(FGE_SCENE_BAD_SID,
                                                                chain.template newChain<fge::ObjectSid>())
                .and_then([&](auto& chain) {
            auto object = this->getObject(chain.value());
            if (object)
            {
                object->g_object->_netList.unpackNeededUpdate(pck, id);
            }
            else
            {
                fge::net::SizeType uselessDataSize{0};
                pck >> uselessDataSize;
                pck.skip(uselessDataSize * sizeof(fge::net::SizeType));
                if (!pck.isValid())
                {
                    return chain.invalidate(
                            net::Error{net::Error::Types::ERR_EXTRACT, pck.getReadPos(), "unattended data size", func});
                }
            }

            return chain;
        }).end();
    }).end();
}

void Scene::forceCheckClient(fge::net::Identity const& id)
{
    this->_netList.forceCheckClient(id);

    for (auto const& data: *this)
    {
        data->getObject()->_netList.forceCheckClient(id);
    }
}
void Scene::forceUncheckClient(fge::net::Identity const& id)
{
    this->_netList.forceUncheckClient(id);

    for (auto const& data: *this)
    {
        data->getObject()->_netList.forceUncheckClient(id);
    }
}

void Scene::clientsCheckup(fge::net::ClientList const& clients)
{
    this->_netList.clientsCheckup(clients);

    for (auto const& data: *this)
    {
        data->getObject()->_netList.clientsCheckup(clients);
    }

    //Remove/Add client
    for (std::size_t i = 0; i < clients.getClientEventSize(); ++i)
    {
        auto const& evt = clients.getClientEvent(i);
        if (evt._event == fge::net::ClientListEvent::CLEVT_DELCLIENT)
        {
            this->g_perClientSyncs.erase(evt._id);
        }
        else
        {
            this->g_perClientSyncs.emplace(std::piecewise_construct, std::forward_as_tuple(evt._id),
                                           std::forward_as_tuple(this->g_updateCount));
        }
    }
}

/** SceneNetEvent **/

void Scene::pushEvent(fge::SceneNetEvent const& netEvent)
{
    for (auto& clientSync: this->g_perClientSyncs)
    {
        clientSync.second._networkEvents.push(netEvent);
    }
}
bool Scene::pushEvent(fge::SceneNetEvent const& netEvent, fge::net::Identity const& id)
{
    auto it = this->g_perClientSyncs.find(id);
    if (it != this->g_perClientSyncs.end())
    {
        it->second._networkEvents.push(netEvent);
        return true;
    }
    return false;
}
void Scene::watchEvent(bool on)
{
    if (!on)
    {
        this->clearNetEventsQueue();
    }
    this->g_enableNetworkEventsFlag = on;
}
bool Scene::isWatchingEvent() const
{
    return this->g_enableNetworkEventsFlag;
}

void Scene::clearNetEventsQueue(fge::net::Identity const& id)
{
    auto it = this->g_perClientSyncs.find(id);
    if (it != this->g_perClientSyncs.end())
    {
        std::queue<fge::SceneNetEvent>().swap(it->second._networkEvents);
    }
}
void Scene::clearNetEventsQueue()
{
    for (auto& clientSync: this->g_perClientSyncs)
    {
        std::queue<fge::SceneNetEvent>().swap(clientSync.second._networkEvents);
    }
}
void Scene::clearPerClientSyncData()
{
    this->g_perClientSyncs.clear();
}

void Scene::packWatchedEvent(fge::net::Packet& pck, fge::net::Identity const& id)
{
    fge::net::SizeType counter = 0;

    std::size_t rewritePos = pck.getDataSize();
    pck.pack(&counter, sizeof(counter)); //will be rewritten

    auto it = this->g_perClientSyncs.find(id);
    if (it == this->g_perClientSyncs.end())
    {
        return;
    }

    auto& events = it->second._networkEvents;

    while (!events.empty())
    {
        if (events.front()._event == fge::SceneNetEvent::SEVT_NEWOBJECT)
        { //New object
            fge::ObjectDataShared data = this->getObject(events.front()._sid);
            if (data)
            {
                pck << static_cast<std::underlying_type<fge::SceneNetEvent::Events>::type>(
                        fge::SceneNetEvent::SEVT_NEWOBJECT);
                //SID
                pck << data->g_sid;
                //CLASS
                pck << fge::reg::GetClassId(data->g_object->getClassName());
                //PLAN
                pck << data->g_plan;
                //TYPE
                pck << static_cast<std::underlying_type<fge::ObjectType>::type>(data->g_type);
                ;

                data->g_object->pack(pck);
                ++counter;
            }
        }
        else
        { //Remove object
            pck << static_cast<std::underlying_type<fge::SceneNetEvent::Events>::type>(
                    fge::SceneNetEvent::SEVT_DELOBJECT);
            //SID
            pck << events.front()._sid;
            ++counter;
        }
        events.pop();
    }

    pck.pack(rewritePos, &counter, sizeof(counter));
}
std::optional<fge::net::Error> Scene::unpackWatchedEvent(fge::net::Packet const& pck)
{
    constexpr char const* const func = __func__;
    fge::ObjectSid buffSid;
    fge::reg::ClassId buffClass;
    fge::ObjectPlan buffPlan;
    std::underlying_type_t<fge::ObjectType> buffType{fge::ObjectType::TYPE_NULL};

    using namespace fge::net::rules;

    return RValid<fge::net::SizeType>(pck)
            .and_for_each(0, 1, [&]([[maybe_unused]] auto& chain, [[maybe_unused]] auto& i) {
        //Event type
        return RStrictLess<std::underlying_type_t<fge::SceneNetEvent::Events>>(fge::SceneNetEvent::Events::SEVT_MAX_,
                                                                               pck)
                .and_then([&](auto& chain) {
            auto event = static_cast<fge::SceneNetEvent::Events>(chain.value());

            if (event == fge::SceneNetEvent::SEVT_NEWOBJECT)
            { //New object
                //SID
                pck >> buffSid;
                //CLASS
                pck >> buffClass;
                //PLAN
                pck >> buffPlan;
                //TYPE
                fge::net::rules::RStrictLess<std::underlying_type_t<fge::ObjectType>>(fge::ObjectType::TYPE_MAX_, pck)
                        .apply(buffType);
                if (!pck.isValid())
                {
                    return chain.invalidate(net::Error{net::Error::Types::ERR_EXTRACT, pck.getReadPos(),
                                                       "unattended object type", func});
                }

                this->delObject(buffSid);
                auto* newObj = fge::reg::GetNewClassOf(buffClass);
                if (newObj == nullptr)
                {
                    return chain.invalidate(
                            net::Error{net::Error::Types::ERR_EXTRACT, pck.getReadPos(), "unknown class ID", func});
                }
                this->newObject(FGE_NEWOBJECT_PTR(newObj), buffPlan, buffSid, static_cast<fge::ObjectType>(buffType))
                        ->g_object->unpack(pck);
            }
            else if (event == fge::SceneNetEvent::SEVT_DELOBJECT)
            { //Remove object
                //SID
                pck >> buffSid;
                if (!pck.isValid())
                {
                    return chain.invalidate(
                            net::Error{net::Error::Types::ERR_EXTRACT, pck.getReadPos(), "unattended data size", func});
                }
                if (buffSid == FGE_SCENE_BAD_SID)
                {
                    this->delAllObject(true);
                }
                else
                {
                    this->delObject(buffSid);
                }
            }

            return chain;
        }).end();
    }).end();
}

/** Custom view **/
void Scene::setCustomView(std::shared_ptr<fge::View> customView)
{
    this->g_customView = std::move(customView);
}
std::shared_ptr<fge::View> const& Scene::getCustomView() const
{
    return this->g_customView;
}
void Scene::delCustomView()
{
    this->g_customView.reset();
}

/** Linked renderTarget **/
void Scene::setLinkedRenderTarget(fge::RenderTarget* target)
{
    this->g_linkedRenderTarget = target;
}
fge::RenderTarget const* Scene::getLinkedRenderTarget() const
{
    return this->g_linkedRenderTarget;
}
fge::RenderTarget* Scene::getLinkedRenderTarget()
{
    return this->g_linkedRenderTarget;
}

fge::View const* Scene::getRelatedView() const
{
    if (this->g_customView != nullptr)
    {
        return this->g_customView.get();
    }
    if (this->g_linkedRenderTarget != nullptr)
    {
        return &this->g_linkedRenderTarget->getView();
    }
    return nullptr;
}

void Scene::setCallbackContext(fge::CallbackContext context)
{
    this->g_callbackContext = context;
}
fge::CallbackContext Scene::getCallbackContext() const
{
    return this->g_callbackContext;
}

/** Save/Load in file **/

bool Scene::saveInFile(std::string const& path)
{
    nlohmann::json outputJson;

    outputJson["SceneInfo"]["name"] = this->getName();

    outputJson["SceneData"] = nlohmann::json::object();
    this->saveCustomData(outputJson["SceneData"]);

    outputJson["Objects"] = nlohmann::json::array();
    for (auto const& data: this->g_data)
    {
        nlohmann::json objNewJson = nlohmann::json::object();
        nlohmann::json& objJson = objNewJson[data->getObject()->getClassName()];

        objJson = nlohmann::json::object();

        objJson["_sid"] = data->getSid();
        objJson["_plan"] = data->getPlan();
        objJson["_type"] = data->getType();

        data->getObject()->save(objJson, this);
        outputJson["Objects"] += objNewJson;
    }

    std::ofstream outFile(path);
    if (outFile)
    {
        outFile << std::setw(2) << outputJson << std::endl;
        outFile.close();
        return true;
    }
    outFile.close();
    return false;
}
bool Scene::loadFromFile(std::string const& path)
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

    this->clear();
    this->setName(inputJson["SceneInfo"]["name"].get<std::string>());

    this->loadCustomData(inputJson["SceneData"]);

    nlohmann::json& jsonObjArray = inputJson["Objects"];
    for (auto& it: jsonObjArray)
    {
        fge::ObjectPtr buffObj{fge::reg::GetNewClassOf(it.begin().key())};

        if (buffObj)
        {
            nlohmann::json& objJson = it.begin().value();

            this->newObject(std::move(buffObj), objJson["_plan"].get<fge::ObjectPlan>(),
                            objJson["_sid"].get<fge::ObjectSid>(), objJson["_type"].get<fge::ObjectType>())
                    ->g_object->load(objJson, this);
        }
        else
        {
            return false;
        }
    }
    return true;
}

/** Iterator **/
fge::ObjectContainer::const_iterator Scene::find(fge::ObjectSid sid) const
{
    auto it = this->g_dataMap.find(sid);
    return (it != this->g_dataMap.cend()) ? static_cast<fge::ObjectContainer::const_iterator>(it->second)
                                          : this->g_data.cend();
}
fge::ObjectContainer::const_iterator Scene::find(fge::Object const* ptr) const
{
    for (auto it = this->g_data.cbegin(); it != this->g_data.cend(); ++it)
    {
        if (*(*it) == ptr)
        {
            return it;
        }
    }
    return this->g_data.cend();
}
fge::ObjectContainer::const_iterator Scene::findPlan(fge::ObjectPlan plan) const
{
    auto it = this->g_planDataMap.find(plan);
    return (it != this->g_planDataMap.cend()) ? static_cast<fge::ObjectContainer::const_iterator>(it->second)
                                              : this->g_data.cend();
}

//Private
void Scene::hash_updatePlanDataMap(fge::ObjectPlan plan, fge::ObjectContainer::iterator whoIterator, bool isLeaving)
{
    /*
     * This function is called when an iterator (whoIterator) HAVE BEEN moved in this plan and must be checked
     * or an object IS ABOUT to leave the plan.
     */

    if (isLeaving)
    {
        auto it = this->g_planDataMap.find(plan);
        if (it->second == whoIterator)
        {
            ++it->second; //go to the next element and check is plan (or end)
            if (it->second != this->g_data.end())
            {
                if ((*it->second)->g_plan == plan)
                {
                    //we already updated the iterator, nothing to do more.
                    return;
                }
            }
            //erase the element
            this->g_planDataMap.erase(it);
        }
    }
    else
    {
        auto pair = this->g_planDataMap.insert(std::make_pair(plan, whoIterator));
        //we try to insert it
        if (!pair.second)
        { //we have to check the object on the left.
            auto itLeft = pair.first->second;
            --itLeft;
            if (itLeft == whoIterator)
            { //the new object is on the left, so we update the stocked iterator
                pair.first->second = whoIterator;
            }
        }
    }
}
fge::ObjectContainer::iterator Scene::hash_getInsertionIteratorFromPlanDataMap(fge::ObjectPlan plan)
{
    auto it = this->g_planDataMap.find(plan);

    if (it == this->g_planDataMap.end())
    {
        for (auto& itPlan: this->g_planDataMap)
        {
            if (plan <= itPlan.first)
            {
                return itPlan.second;
            }
        }
    }
    else
    {
        return it->second;
    }

    return this->g_data.end();
}

} // namespace fge

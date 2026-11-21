/*
 * Copyright 2026 Guillaume Guillet
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
#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/C_random.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/manager/network_manager.hpp"
#include "FastEngine/manager/reg_manager.hpp"
#include "FastEngine/network/C_clientList.hpp"

namespace fge
{

//ObjectContainerHashMap

ObjectContainerHashMap::ObjectContainerHashMap(ObjectStorage& objects)
{
    this->reMap(objects);
}

void ObjectContainerHashMap::clear()
{
    this->g_objectMap.clear();
}

void ObjectContainerHashMap::reMap(ObjectStorage& objects)
{
    this->clear();
    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        this->g_objectMap[it->_data->getSid()] = it;
    }
}

bool ObjectContainerHashMap::newSid(ObjectSid oldSid, ObjectSid newSid)
{
    auto itOld = this->g_objectMap.find(oldSid);
    auto itNew = this->g_objectMap.find(newSid);

    if (itOld == this->g_objectMap.end() || itNew != this->g_objectMap.end())
    {
        return false;
    }

    auto data = itOld->second;
    this->g_objectMap.erase(itOld);
    this->g_objectMap[newSid] = data;
    return true;
}

bool ObjectContainerHashMap::newObject(ObjectSid sid, ObjectStorage::iterator it)
{
    auto itMap = this->g_objectMap.find(sid);

    if (itMap != this->g_objectMap.end())
    {
        return false;
    }

    this->g_objectMap[sid] = it;
    return true;
}

void ObjectContainerHashMap::delObject(ObjectSid sid)
{
    this->g_objectMap.erase(sid);
}

std::optional<ObjectStorage::iterator> ObjectContainerHashMap::find(ObjectSid sid)
{
    auto it = this->g_objectMap.find(sid);

    if (it != this->g_objectMap.end())
    {
        return it->second;
    }
    return std::nullopt;
}

std::optional<ObjectStorage::const_iterator> ObjectContainerHashMap::find(ObjectSid sid) const
{
    auto it = this->g_objectMap.find(sid);

    if (it != this->g_objectMap.end())
    {
        return it->second;
    }
    return std::nullopt;
}
fge::ObjectDataShared ObjectContainerHashMap::retrieve(ObjectSid sid) const
{
    auto it = this->g_objectMap.find(sid);

    if (it != this->g_objectMap.end())
    {
        return it->second->_data;
    }
    return fge::ObjectDataShared{};
}
bool ObjectContainerHashMap::contains(ObjectSid sid) const
{
    return this->g_objectMap.contains(sid);
}

std::size_t ObjectContainerHashMap::size() const
{
    return this->g_objectMap.size();
}

//Scene

Scene::Scene() :
        g_name(),

        g_perClientSyncs(),
        g_enableNetworkEventsFlag(false),

        g_linkedRenderTarget(nullptr),

        g_updateCount(0),
        g_deleteMe(false),

        g_callbackContext({nullptr, nullptr})
{}
Scene::Scene(std::string sceneName) :
        g_name(std::move(sceneName)),

        g_perClientSyncs(),
        g_enableNetworkEventsFlag(false),

        g_linkedRenderTarget(nullptr),

        g_updateCount(0),
        g_deleteMe(false),

        g_callbackContext({nullptr, nullptr})
{}
Scene::Scene(Scene const& r) :
        OwnView(r),
        _netList(),
        _properties(r._properties),

        g_name(r.g_name),

        g_perClientSyncs(),
        g_enableNetworkEventsFlag(r.g_enableNetworkEventsFlag),

        g_linkedRenderTarget(r.g_linkedRenderTarget),

        g_updateCount(r.g_updateCount),
        g_deleteMe(false),

        g_callbackContext(r.g_callbackContext)
{
    for (auto const& objectData: r)
    {
        this->newObjectPlanBack(FGE_NEWOBJECT_PTR(objectData->g_object->copy()), objectData->g_plan, objectData->g_sid,
                                objectData->g_type);
    }
    //TODO: make sure to copy object parent too
}
Scene::~Scene()
{
    this->clear();
}

Scene& Scene::operator=(Scene const& r)
{
    this->clear();

    this->_properties = r._properties;

    this->g_name = r.g_name;

    this->g_perClientSyncs.clear();
    this->g_enableNetworkEventsFlag = r.g_enableNetworkEventsFlag;

    this->g_linkedRenderTarget = r.g_linkedRenderTarget;

    this->g_updateCount = r.g_updateCount;
    this->g_deleteMe = false;

    this->g_callbackContext = r.g_callbackContext;

    for (auto const& objectData: r)
    {
        this->newObjectPlanBack(FGE_NEWOBJECT_PTR(objectData->g_object->copy()), objectData->g_plan, objectData->g_sid,
                                objectData->g_type);
    }
    //TODO: make sure to copy object parent too

    return *this;
}

/** Scene **/
#ifdef FGE_DEF_SERVER
void Scene::update(fge::Event& event, fge::DeltaTime const& deltaTime, std::underlying_type_t<UpdateFlags> flags)
#else
void Scene::update(fge::RenderTarget& target,
                   fge::Event& event,
                   fge::DeltaTime const& deltaTime,
                   std::underlying_type_t<UpdateFlags> flags)
#endif //FGE_DEF_SERVER
{
    //Update order is not guaranteed (unordered storage), only the draw order is.
    for (auto it = this->g_objects.begin(); it != this->g_objects.end();)
    {
        this->g_updatedObject = &(*it);
        auto updatedObject = it->_data;

        if (updatedObject->g_object->isNeedingAnchorUpdate())
        {
            updatedObject->g_object->updateAnchor();
        }

#ifdef FGE_DEF_SERVER
        updatedObject->g_object->update(event, deltaTime, *this);
        if ((updatedObject->g_object->_childrenControlFlags & Object::ChildrenControlFlags::CHILDREN_AUTO_UPDATE) > 0)
        {
            updatedObject->g_object->_children.update(event, deltaTime, *this);
        }
#else
        updatedObject->g_object->update(target, event, deltaTime, *this);
        if ((updatedObject->g_object->_childrenControlFlags & Object::ChildrenControlFlags::CHILDREN_AUTO_UPDATE) > 0)
        {
            updatedObject->g_object->_children.update(target, event, deltaTime, *this);
        }
#endif //FGE_DEF_SERVER

        if (this->g_deleteMe)
        {
            this->g_deleteMe = false;
            if (this->g_enableNetworkEventsFlag)
            {
                this->pushEvent({fge::SceneNetEvent::Events::OBJECT_DELETED, updatedObject->g_sid});
            }

            updatedObject->g_object->removed(*this);
            if ((updatedObject->g_object->_childrenControlFlags &
                 Object::ChildrenControlFlags::CHILDREN_AUTO_CLEAR_ON_REMOVE) > 0)
            {
                updatedObject->g_object->_children.clear();
            }
            updatedObject->g_boundScene = nullptr;
            updatedObject->g_object->_myObjectData.reset();

            auto objectPlan = updatedObject->g_plan;
            this->g_objectsHashMap.delObject(updatedObject->g_sid);
            this->hash_updatePlanDataMap(objectPlan, this->g_updatedObject, true);
            this->drawOrder_unlink(this->g_updatedObject);
            this->g_updatedObject = nullptr;

            this->_onObjectRemoved.call(*this, updatedObject);
            this->_onPlanUpdate.call(*this, objectPlan);

            //The storage element is erased after the callbacks, as they can modify the scene:
            //the iterator of an element stay valid until this element is erased.
            it = this->g_objects.erase(it);
            continue;
        }

        ++it;
    }
    this->g_updatedObject = nullptr;

    if ((flags & UpdateFlags::INCREMENT_UPDATE_COUNT) > 0)
    {
        ++this->g_updateCount;
    }

    this->_onDelayedUpdate.call(*this);
    this->_onDelayedUpdate.clear();
}
uint16_t Scene::getUpdateCount() const
{
    return this->g_updateCount;
}

#ifndef FGE_DEF_SERVER
void Scene::draw(fge::RenderTarget& target, fge::RenderStates const& states) const
{
    this->_onDraw.call(*this, target);

    fge::RectFloat const screenBounds = fge::GetScreenRect(target);

    fge::ObjectPlanDepth depthCount = 0;
    auto planDataMapIt = this->g_planDataMap.begin();

    fge::View const backupView = target.getView();

    for (fge::ObjectNode const* node = this->g_drawHead; node != nullptr; node = node->g_next)
    {
        auto const& objectData = node->_data;

        //Check plan depth
        if (planDataMapIt != this->g_planDataMap.end())
        {
            if (node == planDataMapIt->second)
            { //New plan, we reset depth count
                depthCount = 0;
                ++planDataMapIt; //go next plan ...
            }
        }

        objectData->g_planDepth = depthCount++;

        fge::Object const* object = objectData->g_object.get();

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

        //setting up the view
        target.setView(object->requestView(target, *this));

        if ((object->_childrenControlFlags & Object::ChildrenControlFlags::CHILDREN_AUTO_DRAW) > 0)
        {
            object->_children.draw(target, states);
        }

        if (objectData->g_contextFlags.has(ObjectContextFlags::OBJ_CONTEXT_DETACHED) && !objectData->g_parent.expired())
        {
            auto const parent = objectData->g_parent.lock();
            auto copyStates = states.copy();
            copyStates._resTransform.set(target.requestGlobalTransform(*parent->g_object, states._resTransform));
            object->draw(target, copyStates);
            continue;
        }

        object->draw(target, states);
    }

    target.setView(backupView);
}
#endif //FGE_DEF_SERVER

fge::ObjectPlanDepth Scene::updatePlanDepth(fge::ObjectSid sid)
{
    auto objectIt = this->g_objectsHashMap.find(sid);
    if (!objectIt)
    {
        return FGE_SCENE_BAD_PLANDEPTH;
    }

    auto* const node = &(*objectIt.value());
    auto const plan = node->_data->g_plan;

    fge::ObjectPlanDepth planDepth = 0;
    for (auto const* it = this->g_planDataMap.find(plan)->second; it != node && it != nullptr; it = it->g_next)
    {
        ++planDepth;
    }
    node->_data->g_planDepth = planDepth;

    this->_onPlanUpdate.call(*this, plan);
    return planDepth;
}
void Scene::updateAllPlanDepth(fge::ObjectPlan plan)
{
    auto it = this->g_planDataMap.find(plan);

    if (it != this->g_planDataMap.end())
    {
        fge::ObjectPlanDepth depthCount = 0;

        for (auto const* node = it->second; node != nullptr; node = node->g_next)
        {
            if (node->_data->g_plan != plan)
            {
                break;
            }
            node->_data->g_planDepth = depthCount++;
        }

        this->_onPlanUpdate.call(*this, plan);
    }
}
void Scene::updateAllPlanDepth()
{
    fge::ObjectPlanDepth depthCount = 0;
    auto planDataMapIt = this->g_planDataMap.begin();

    for (auto const* node = this->g_drawHead; node != nullptr; node = node->g_next)
    {
        //Check plan depth
        if (planDataMapIt != this->g_planDataMap.end())
        {
            if (node == planDataMapIt->second)
            { //New plan, we reset depth count
                depthCount = 0;
                ++planDataMapIt; //go next plan ...
            }
        }

        node->_data->g_planDepth = depthCount++;
    }

    this->_onPlanUpdate.call(*this, FGE_SCENE_BAD_PLAN);
}

void Scene::clear()
{
    //Clear all callbacks
    this->_onDelayedUpdate.clear();
    this->_onDraw.clear();
    this->_onObjectAdded.clear();
    this->_onObjectRemoved.clear();
    this->_onPlanUpdate.clear();

    this->delAllObject(false);
    this->_properties.delAllProperties();
    this->_netList.clear();
}

/** Object **/
fge::ObjectDataShared Scene::newObject(fge::ObjectPtr&& newObject,
                                       fge::ObjectPlan plan,
                                       fge::ObjectSid sid,
                                       fge::ObjectTypes type,
                                       bool silent,
                                       fge::EnumFlags<ObjectContextFlags> contextFlags)
{
    auto newObjectData = std::make_shared<fge::ObjectData>(this, std::move(newObject), sid, plan, type);
    newObjectData->g_contextFlags = contextFlags;
    return this->newObject(std::move(newObjectData), silent);
}
fge::ObjectDataShared Scene::newObject(fge::ObjectDataShared const& objectData, bool silent)
{
    return this->insertNewObject(objectData, silent, false);
}
fge::ObjectDataShared Scene::newObjectPlanBack(fge::ObjectPtr&& newObject,
                                               fge::ObjectPlan plan,
                                               fge::ObjectSid sid,
                                               fge::ObjectTypes type,
                                               bool silent,
                                               fge::EnumFlags<ObjectContextFlags> contextFlags)
{
    auto newObjectData = std::make_shared<fge::ObjectData>(this, std::move(newObject), sid, plan, type);
    newObjectData->g_contextFlags = contextFlags;
    return this->insertNewObject(newObjectData, silent, true);
}
fge::ObjectDataShared Scene::insertNewObject(fge::ObjectDataShared const& objectData, bool silent, bool planBack)
{
    fge::ObjectSid generatedSid = this->generateSid(objectData->g_sid, objectData->g_type);
    if (generatedSid == FGE_SCENE_BAD_SID)
    {
        return nullptr;
    }
    if (this->g_enableNetworkEventsFlag)
    {
        this->pushEvent({fge::SceneNetEvent::Events::OBJECT_CREATED, generatedSid});
    }

    objectData->g_sid = generatedSid;

    auto* const insertBefore = this->hash_getInsertionIteratorFromPlanDataMap(objectData->g_plan, planBack);

    auto const it = this->g_objects.emplace(objectData);
    auto* const node = &(*it);
    this->drawOrder_insertBefore(node, insertBefore);

    objectData->g_boundScene = this;
    objectData->g_object->_myObjectData = objectData;
    if (!this->g_objectsHashMap.newObject(generatedSid, it))
    {
        //Something went wrong, we can do a re-map
        this->g_objectsHashMap.reMap(this->g_objects);
    }
    this->hash_updatePlanDataMap(objectData->g_plan, node, false);

    if (this->g_updatedObject != nullptr && objectData->g_parent.expired())
    { //An object is created inside another object and orphan, make it parent
        objectData->g_parent = this->g_updatedObject->_data;
    }
    if (!silent)
    {
        objectData->g_object->first(*this);
        objectData->g_object->_children.sceneUpdate(*this);
    }

    if (objectData->g_object->_callbackContextMode == fge::Object::CallbackContextModes::CONTEXT_AUTO &&
        this->g_callbackContext._event != nullptr && !silent)
    {
        objectData->g_object->callbackRegister(*this->g_callbackContext._event,
                                               this->g_callbackContext._guiElementHandler);
    }

    this->_onObjectAdded.call(*this, objectData);
    this->_onPlanUpdate.call(*this, objectData->g_plan);

    return objectData;
}

fge::ObjectDataShared Scene::duplicateObject(fge::ObjectSid sid, fge::ObjectSid newSid)
{
    auto object = this->g_objectsHashMap.retrieve(sid);
    if (!object)
    {
        return nullptr;
    }

    fge::ObjectDataShared newObject = std::make_shared<fge::ObjectData>();
    newObject->g_object.reset(object->g_object->copy());
    newObject->g_plan = object->g_plan;
    newObject->g_sid = newSid;
    newObject->g_type = object->g_type;

    return this->newObject(newObject);
}

fge::ObjectDataShared Scene::transferObject(fge::ObjectSid sid, fge::Scene& newScene)
{
    auto objectIt = this->g_objectsHashMap.find(sid);
    if (!objectIt)
    {
        return nullptr;
    }

    if (newScene.isValid(sid))
    { //Object already exist in the new scene
        return nullptr;
    }

    auto object = objectIt.value()->_data;
    auto* const node = &(*objectIt.value());

    this->hash_updatePlanDataMap(object->g_plan, node, true);
    this->drawOrder_unlink(node);
    this->g_objects.erase(objectIt.value());
    this->g_objectsHashMap.delObject(object->g_sid);

    this->_onObjectRemoved.call(*this, object);
    this->_onPlanUpdate.call(*this, object->g_plan);

    if (this->g_enableNetworkEventsFlag)
    {
        this->pushEvent({fge::SceneNetEvent::Events::OBJECT_DELETED, sid});
    }

    object = newScene.newObject(std::move(object), true);
    if (object)
    {
        object->g_object->transfered(*this, newScene);
    }
    //If the object is not valid, sadly it will just be deleted at this point
    return object;
}

void Scene::delUpdatedObject()
{
    this->g_deleteMe = true;
}
bool Scene::delObject(fge::ObjectSid sid)
{
    auto objectIt = this->g_objectsHashMap.find(sid);
    if (!objectIt)
    {
        return false;
    }

    auto object = objectIt.value()->_data;

    if (this->g_enableNetworkEventsFlag)
    {
        this->pushEvent({fge::SceneNetEvent::Events::OBJECT_DELETED, object->g_sid});
    }

    object->g_object->removed(*this);
    if ((object->g_object->_childrenControlFlags & Object::ChildrenControlFlags::CHILDREN_AUTO_CLEAR_ON_REMOVE) > 0)
    {
        object->g_object->_children.clear();
    }
    object->g_boundScene = nullptr;
    object->g_object->_myObjectData.reset();

    auto objectPlan = object->g_plan;
    auto* const node = &(*objectIt.value());
    this->hash_updatePlanDataMap(objectPlan, node, true);
    this->drawOrder_unlink(node);
    this->g_objects.erase(objectIt.value());
    this->g_objectsHashMap.delObject(sid);

    this->_onObjectRemoved.call(*this, object);
    this->_onPlanUpdate.call(*this, objectPlan);

    return true;
}
std::size_t Scene::delAllObject(bool ignoreGuiObject)
{
    if (this->g_enableNetworkEventsFlag)
    {
        this->clearNetEventsQueue();
        this->pushEvent({fge::SceneNetEvent::Events::OBJECT_DELETED, FGE_SCENE_BAD_SID});
    }

    std::size_t objectCount = this->g_objects.size();
    for (auto it = this->g_objects.begin(); it != this->g_objects.end();)
    {
        auto object = it->_data;

        if (ignoreGuiObject)
        {
            if (object->g_type == fge::ObjectTypes::GUI)
            {
                --objectCount;
                ++it;
                continue;
            }
        }

        object->g_object->removed(*this);
        if ((object->g_object->_childrenControlFlags & Object::ChildrenControlFlags::CHILDREN_AUTO_CLEAR_ON_REMOVE) > 0)
        {
            object->g_object->_children.clear();
        }
        object->g_boundScene = nullptr;
        object->g_object->_myObjectData.reset();
        auto* const node = &(*it);
        this->hash_updatePlanDataMap(object->g_plan, node, true);
        this->drawOrder_unlink(node);

        this->g_objectsHashMap.delObject(object->g_sid);

        this->_onObjectRemoved.call(*this, object);

        //The storage element is erased after the callback, as it can modify the scene:
        //the iterator of an element stay valid until this element is erased.
        it = this->g_objects.erase(it);
    }

    this->_onPlanUpdate.call(*this, FGE_SCENE_BAD_PLAN);
    return objectCount;
}

bool Scene::setObjectSid(fge::ObjectSid sid, fge::ObjectSid newSid)
{
    if (sid == FGE_SCENE_BAD_SID)
    {
        return false;
    }

    if (sid == newSid)
    {
        return true;
    }

    auto const objectIt = this->g_objectsHashMap.find(sid);
    if (!objectIt)
    {
        return false;
    }
    auto object = objectIt.value()->_data;

    if (newSid == FGE_SCENE_BAD_SID)
    { //Request a new random SID
        newSid = this->generateSid(FGE_SCENE_BAD_SID, object->g_type);
        if (newSid == FGE_SCENE_BAD_SID)
        {
            return false;
        }
    }
    else if (this->g_objectsHashMap.find(newSid))
    {
        return false;
    }

    if (this->g_enableNetworkEventsFlag)
    {
        this->pushEvent({fge::SceneNetEvent::Events::OBJECT_DELETED, objectIt.value()->_data->g_sid});
        this->pushEvent({fge::SceneNetEvent::Events::OBJECT_CREATED, newSid});
    }

    object->g_sid = newSid;
    if (!this->g_objectsHashMap.newSid(sid, newSid))
    {
        //Something went wrong, we can do a re-map
        this->g_objectsHashMap.reMap(this->g_objects);
    }
    return true;
}
bool Scene::setObject(fge::ObjectSid sid, fge::ObjectPtr&& newObject)
{
    if (newObject == nullptr || sid == FGE_SCENE_BAD_SID)
    {
        return false;
    }

    auto objectIt = this->g_objectsHashMap.find(sid);
    if (!objectIt)
    {
        return false;
    }

    auto& node = *objectIt.value();
    auto object = node._data;

    if (this->g_enableNetworkEventsFlag)
    {
        this->pushEvent({fge::SceneNetEvent::Events::OBJECT_CREATED, object->g_sid});
    }

    object->g_object->removed(*this); ///TODO: add this and all instance of this code inside a common method/function
    if ((object->g_object->_childrenControlFlags & Object::ChildrenControlFlags::CHILDREN_AUTO_CLEAR_ON_REMOVE) > 0)
    {
        object->g_object->_children.clear();
    }
    object->g_boundScene = nullptr;
    object->g_object->_myObjectData.reset();

    auto newObjectData = std::make_shared<fge::ObjectData>(this, std::move(newObject), object->g_sid, object->g_plan,
                                                           object->g_type);
    newObjectData->g_planDepth = object->g_planDepth;
    newObjectData->g_parent = object->g_parent;
    newObjectData->g_contextFlags = object->g_contextFlags;

    //The node keep his place in the storage, the hash map and the draw order, only the ObjectData is replaced
    node._data = newObjectData;
    object = std::move(newObjectData);

    object->g_object->_myObjectData = object;
    object->g_object->first(*this);
    object->g_object->_children.sceneUpdate(*this);

    if (object->g_object->_callbackContextMode == fge::Object::CallbackContextModes::CONTEXT_AUTO &&
        this->g_callbackContext._event != nullptr)
    {
        object->g_object->callbackRegister(*this->g_callbackContext._event, this->g_callbackContext._guiElementHandler);
    }
    return true;
}
bool Scene::setObjectPlan(fge::ObjectSid sid, fge::ObjectPlan newPlan)
{
    auto objectIt = this->g_objectsHashMap.find(sid);
    if (!objectIt)
    {
        return false;
    }

    auto object = objectIt.value()->_data;
    auto* const node = &(*objectIt.value());

    this->hash_updatePlanDataMap(object->g_plan, node, true);

    auto* const newPos = this->hash_getInsertionIteratorFromPlanDataMap(newPlan);

    auto oldPlan = object->g_plan;
    object->g_plan = newPlan;

    this->drawOrder_move(node, newPos);
    this->hash_updatePlanDataMap(newPlan, node, false);

    if (oldPlan != newPlan)
    {
        this->_onPlanUpdate.call(*this, oldPlan);
    }
    this->_onPlanUpdate.call(*this, newPlan);
    return true;
}
bool Scene::setObjectPlanTop(fge::ObjectSid sid)
{
    auto objectIt = this->g_objectsHashMap.find(sid);
    if (!objectIt)
    {
        return false;
    }

    auto object = objectIt.value()->_data;
    auto* const node = &(*objectIt.value());

    auto newPosIt = this->g_planDataMap.find(object->g_plan);

    if (node == newPosIt->second)
    { //already on top
        return true;
    }

    this->drawOrder_move(node, newPosIt->second);
    this->hash_updatePlanDataMap(object->g_plan, node, false);

    this->_onPlanUpdate.call(*this, object->g_plan);
    return true;
}
bool Scene::setObjectPlanBot(fge::ObjectSid sid)
{
    auto objectIt = this->g_objectsHashMap.find(sid);
    if (!objectIt)
    {
        return false;
    }

    auto object = objectIt.value()->_data;
    auto* const node = &(*objectIt.value());

    auto plan = object->g_plan;
    auto planIt = this->g_planDataMap.find(plan);
    auto planItAfter = planIt;
    ++planItAfter; //Next plan

    bool wasOnTop = false;
    if (node == planIt->second)
    { //is on top
        wasOnTop = true;
        this->hash_updatePlanDataMap(plan, node, true);
    }

    //object is pushed at the end of his plan (before the next plan or at the end)
    this->drawOrder_move(node, planItAfter == this->g_planDataMap.end() ? nullptr : planItAfter->second);
    if (wasOnTop)
    {
        this->hash_updatePlanDataMap(plan, node, false);
    }

    this->_onPlanUpdate.call(*this, plan);

    return true;
}

fge::ObjectDataShared Scene::getObject(fge::ObjectSid sid) const
{
    return this->g_objectsHashMap.retrieve(sid);
}
fge::ObjectDataShared Scene::getObject(fge::Object const* ptr) const
{
    auto it = this->find(ptr);
    if (it != this->end())
    {
        return *it;
    }
    return nullptr;
}
fge::Object* Scene::getObjectPtr(fge::ObjectSid sid) const
{
    auto object = this->g_objectsHashMap.retrieve(sid);
    return object ? object->g_object.get() : nullptr;
}
fge::ObjectDataShared Scene::getUpdatedObject() const
{
    return this->g_updatedObject != nullptr ? this->g_updatedObject->_data : nullptr;
}

/** Search function **/
std::size_t Scene::getAllObj_ByPosition(fge::Vector2f const& pos, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (auto const& data: *this)
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
    for (auto const& data: *this)
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
    return this->getAllObj_ByPosition(target.mapFramebufferCoordsToWorldSpace(pos, this->requestView(target)), buff);
}
std::size_t Scene::getAllObj_ByLocalZone(fge::RectInt const& zone,
                                         fge::RenderTarget const& target,
                                         fge::ObjectContainer& buff) const
{
    return this->getAllObj_ByZone(target.mapFramebufferRectToWorldSpace(zone, this->requestView(target)), buff);
}
std::size_t Scene::getAllObj_FromLocalPosition(fge::Vector2i const& pos,
                                               fge::RenderTarget const& target,
                                               fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (auto const& data: *this)
    {
        auto objBounds =
                target.mapViewRectToFramebufferSpace(data->g_object->getGlobalBounds(), this->requestView(target));
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
    for (auto const& data: *this)
    {
        auto objBounds =
                target.mapViewRectToFramebufferSpace(data->g_object->getGlobalBounds(), this->requestView(target));
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
    for (auto const& data: *this)
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
    for (auto const& data: *this)
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
    for (auto const& data: *this)
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
    for (auto const& data: *this)
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
    return this->getFirstObj_ByPosition(target.mapFramebufferCoordsToWorldSpace(pos, this->requestView(target)));
}
fge::ObjectDataShared Scene::getFirstObj_ByLocalZone(fge::RectInt const& zone, fge::RenderTarget const& target) const
{
    return this->getFirstObj_ByZone(target.mapFramebufferRectToWorldSpace(zone, this->requestView(target)));
}
fge::ObjectDataShared Scene::getFirstObj_FromLocalPosition(fge::Vector2i const& pos,
                                                           fge::RenderTarget const& target) const
{
    for (auto const& data: *this)
    {
        fge::ObjectPtr const& buffObj = data->g_object;
        auto objBounds = target.mapViewRectToFramebufferSpace(buffObj->getGlobalBounds(), this->requestView(target));
        if (objBounds.contains(pos))
        {
            return data;
        }
    }
    return nullptr;
}
fge::ObjectDataShared Scene::getFirstObj_FromLocalZone(fge::RectInt const& zone, fge::RenderTarget const& target) const
{
    for (auto const& data: *this)
    {
        fge::ObjectPtr const& buffObj = data->g_object;
        auto objBounds = target.mapViewRectToFramebufferSpace(buffObj->getGlobalBounds(), this->requestView(target));
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
    for (auto const& data: *this)
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
    for (auto const& data: *this)
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
    for (auto const& node: this->g_objects)
    {
        auto const& data = node._data;
        if (*data == ptr)
        {
            return data->g_sid;
        }
    }
    return FGE_SCENE_BAD_SID;
}

fge::ObjectSid Scene::generateSid(fge::ObjectSid wanted_sid, fge::ObjectTypes type) const
{
    if (type >= ObjectTypes::_MAX_ || type == ObjectTypes::INVALID)
    {
        return FGE_SCENE_BAD_SID;
    }

    if (wanted_sid != FGE_SCENE_BAD_SID)
    {
        if (!this->g_objectsHashMap.contains(wanted_sid))
        {
            return wanted_sid;
        }
        return FGE_SCENE_BAD_SID;
    }

    while (true) ///TODO: not that great
    {
        auto new_sid = _random.range<ObjectSid>(0, (FGE_SCENE_BAD_SID - 1) &
                                                           ~static_cast<DefaultSIDRanges_t>(DefaultSIDRanges::MASK));

        switch (type)
        {
        case ObjectTypes::OBJECT:
            new_sid |= static_cast<DefaultSIDRanges_t>(DefaultSIDRanges::POS_OBJECT);
            break;
        case ObjectTypes::DECAY:
            new_sid |= static_cast<DefaultSIDRanges_t>(DefaultSIDRanges::POS_DECAY);
            break;
        case ObjectTypes::GUI:
            new_sid |= static_cast<DefaultSIDRanges_t>(DefaultSIDRanges::POS_GUI);
            break;
        default:
            //Should never go here !
            return FGE_SCENE_BAD_SID;
        }

        if (!this->g_objectsHashMap.contains(new_sid))
        {
            return new_sid;
        }
    }
}

/** Network **/
void Scene::signalObject(fge::ObjectSid sid, int8_t signal)
{
    if (!this->g_enableNetworkEventsFlag)
    {
        return;
    }

    if (this->g_objectsHashMap.contains(sid))
    {
        this->pushEvent({fge::SceneNetEvent::Events::OBJECT_SIGNALED, sid, signal});
    }
}

void Scene::pack(fge::net::Packet& pck, fge::net::Identity const& id)
{
    if (id._ip.getType() != net::IpAddress::Types::None && id._port != FGE_ANYPORT && id._port != 0)
    {
        auto result = this->g_perClientSyncs.emplace(std::piecewise_construct, std::forward_as_tuple(id),
                                                     std::forward_as_tuple(this->g_updateCount));
        if (!result.second)
        {
            result.first->second._lastUpdateCount = this->g_updateCount;
            std::queue<SceneNetEvent>().swap(result.first->second._networkEvents);
            this->forceUncheckClient(id);
        }
    }

    //update count
    pck << this->g_updateCount;

    //scene name
    pck << this->g_name;

    //scene data
    //TODO: can't ignore for now the scene net list, if client is inside the ignored list
    for (std::size_t i = 0; i < this->_netList.size(); ++i)
    {
        this->_netList[i]->packData(pck);
    }

    //object size
    net::SizeType objectSize = 0;
    std::size_t const objectSizePos = pck.getDataSize();
    pck.pack(&objectSize, sizeof(objectSize)); //Will be rewritten
    for (auto const& data: *this)
    {
        if (data->g_type == ObjectTypes::GUI)
        { //Ignore GUI
            continue;
        }

        if (data->g_object->_netSyncMode != Object::NetSyncModes::FULL_SYNC)
        {
            continue;
        }

        if (data->g_object->_netList.isIgnored(id))
        {
            continue; //Object is ignored for this client
        }

        //SID
        pck << data->g_sid;
        //CLASS
        pck << reg::GetClassId(data->g_object->getClassName());
        //PLAN
        pck << data->g_plan;
        //TYPE
        pck << data->g_type;

        data->g_object->pack(pck);
        ++objectSize;
    }
    pck.pack(objectSizePos, &objectSize, sizeof(objectSize)); //Rewriting size
}
std::optional<fge::net::Error> Scene::unpack(fge::net::Packet const& pck, bool clearObjects)
{
    constexpr char const* const func = __func__;
    reg::ClassId buffClass{FGE_REG_BADCLASSID};
    ObjectPlan buffPlan{FGE_SCENE_PLAN_DEFAULT};
    ObjectSid buffSid{FGE_SCENE_BAD_SID};

    using namespace fge::net::rules;

    //update count
    return RValid(pck, &this->g_updateCount)
            .and_then([&](auto& chain) {
        //scene name
        return RStringRange(0, FGE_SCENE_LIMIT_NAMESIZE, chain, &this->g_name);
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
        if (clearObjects)
        {
            this->delAllObject(true);
        }
        //object size
        return RValid<net::SizeType>(chain);
    })
            .and_for_each([&](auto& chain, [[maybe_unused]] auto& i) {
        //SID
        pck >> buffSid;
        if (buffSid == FGE_SCENE_BAD_SID)
        {
            return chain.skip();
        }
        //CLASS
        pck >> buffClass;
        if (buffClass == FGE_REG_BADCLASSID)
        {
            return chain.stop("received bad class ID", func);
        }
        //PLAN
        pck >> buffPlan;
        //TYPE
        return RStrictLess<ObjectTypes>(ObjectTypes::_MAX_, pck)
                .and_then([&](auto& chain) {
            auto buffObject = this->getObject(buffSid);
            if (buffObject)
            { //Object already exist
                if (buffObject->g_contextFlags.has(OBJ_CONTEXT_NETWORK))
                {
                    if (fge::reg::GetClassId(buffObject->g_object->getClassName()) != buffClass)
                    { //Class changed
                        this->delObject(buffSid);
                        buffObject =
                                this->newObjectPlanBack(fge::ObjectPtr{fge::reg::GetNewClassOf(buffClass)}, buffPlan,
                                                        buffSid, chain.value(), false, OBJ_CONTEXT_NETWORK);
                    }
                }
                else
                { //The existing object is a client side object, we are going to change it's sid to avoid conflicts
                    this->setObjectSid(buffSid, FGE_SCENE_BAD_SID);
                    buffObject = this->newObjectPlanBack(fge::ObjectPtr{fge::reg::GetNewClassOf(buffClass)}, buffPlan,
                                                         buffSid, chain.value(), false, OBJ_CONTEXT_NETWORK);
                }
            }
            else
            { //Object does not exist
                buffObject = this->newObjectPlanBack(fge::ObjectPtr{fge::reg::GetNewClassOf(buffClass)}, buffPlan,
                                                     buffSid, chain.value(), false, OBJ_CONTEXT_NETWORK);
            }

            if (!buffObject)
            {
                return chain.invalidate("unknown class ID / SID", func);
            }

            buffObject->g_object->unpack(pck);

            return chain;
        }).end();
    }).end();
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

    std::size_t const rewritePos = pck.getDataSize();
    pck.pack(&countSceneDataModification, sizeof(countSceneDataModification)); //Will be rewritten

    if (!this->_netList.isIgnored(id))
    {
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
    }
    pck.pack(rewritePos, &countSceneDataModification, sizeof(countSceneDataModification)); //Rewriting size

    //object size
    fge::net::SizeType countObject = 0;

    std::size_t const countObjectPos = pck.getDataSize();
    pck.pack(&countObject, sizeof(countObject)); //Will be rewritten

    std::size_t dataPos = pck.getDataSize();
    constexpr std::size_t reservedSize = sizeof(fge::ObjectSid) + sizeof(fge::reg::ClassId) + sizeof(fge::ObjectPlan) +
                                         sizeof(std::underlying_type_t<fge::ObjectTypes>) + sizeof(fge::net::SizeType);
    pck.append(reservedSize);

    for (auto const& node: this->g_objects)
    {
        auto const& data = node._data;
        if (data->g_object->_netSyncMode != Object::NetSyncModes::FULL_SYNC &&
            data->g_object->_netSyncMode != Object::NetSyncModes::DELTA_SYNC)
        {
            continue;
        }

        if (data->g_object->_netList.isIgnored(id))
        {
            continue;
        }

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
            fge::ObjectTypes tmpType = data->g_type;
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
Scene::unpackModification(fge::net::Packet const& pck, UpdateCountRange& range, bool ignoreUpdateCount)
{
    constexpr char const* const func = __func__;

    using namespace fge::net::rules;

    //update count range
    pck >> range._last >> range._now;
    if (!pck)
    {
        return net::Error{net::Error::Types::ERR_EXTRACT, pck.getReadPos(), "received bad update count range", func};
    }

    if (!ignoreUpdateCount)
    {
        //Check if the pack is not old
        if (range._last < this->g_updateCount ||
            (range._last > this->g_updateCount && range._now < this->g_updateCount))
        {
            return net::Error{net::Error::Types::ERR_SCENE_OLD_PACKET, pck.getReadPos(),
                              "old network updates for this scene", func};
        }

        this->g_updateCount = range._now;
    }

    //scene name
    return RStringRange(0, FGE_SCENE_LIMIT_NAMESIZE, pck, &this->g_name)
            .and_then([&](auto& chain) {
        //scene data
        return RLess<fge::net::SizeType>(this->_netList.size(), chain);
    })
            .and_for_each([&](auto& chain, [[maybe_unused]] auto& i) {
        return RValid<fge::net::SizeType>(chain)
                .and_then([&](auto& chain) {
            this->_netList[chain.value()]->applyData(pck);
            return chain;
        }).end();
    })
            .and_then([&](auto& chain) { return RValid<fge::net::SizeType>(chain); })
            .and_for_each([&](auto& chain, [[maybe_unused]] auto& i) {
        fge::reg::ClassId buffClass{FGE_REG_BADCLASSID};
        fge::ObjectPlan buffPlan{FGE_SCENE_PLAN_DEFAULT};
        fge::ObjectSid buffSid{FGE_SCENE_BAD_SID};
        fge::ObjectTypes buffType{fge::ObjectTypes::INVALID};

        //SID
        pck >> buffSid;
        //CLASS
        pck >> buffClass;
        //PLAN
        pck >> buffPlan;
        //TYPE
        auto err = RStrictLess<ObjectTypes>(fge::ObjectTypes::_MAX_, chain).apply(buffType).end();

        if (err)
        {
            return err;
        }

        auto buffObject = this->getObject(buffSid);
        if (buffObject)
        { //Object already exist
            if (buffObject->g_contextFlags.has(OBJ_CONTEXT_NETWORK))
            {
                if (fge::reg::GetClassId(buffObject->g_object->getClassName()) != buffClass)
                { //Class changed
                    this->delObject(buffSid);
                    buffObject = this->newObject(fge::ObjectPtr{fge::reg::GetNewClassOf(buffClass)}, buffPlan, buffSid,
                                                 buffType, false, OBJ_CONTEXT_NETWORK);
                }
            }
            else
            { //The existing object is a client side object, we are going to change it's sid to avoid conflicts
                this->setObjectSid(buffSid, FGE_SCENE_BAD_SID);
                buffObject = this->newObject(fge::ObjectPtr{fge::reg::GetNewClassOf(buffClass)}, buffPlan, buffSid,
                                             buffType, false, OBJ_CONTEXT_NETWORK);
            }
        }
        else
        { //Object does not exist
            buffObject = this->newObject(fge::ObjectPtr{fge::reg::GetNewClassOf(buffClass)}, buffPlan, buffSid,
                                         buffType, false, OBJ_CONTEXT_NETWORK);
        }

        if (!buffObject)
        {
            return chain.stop("unknown class ID / SID", func);
        }

        //MODIF COUNT/OBJECT DATA
        auto& objectNetList = buffObject->g_object->_netList;

        return RLess<fge::net::SizeType>(objectNetList.size(), chain)
                .and_for_each([&](auto& chain, [[maybe_unused]] auto& i) {
            return RLess<fge::net::SizeType>(objectNetList.size(), chain)
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

    for (auto const& node: this->g_objects)
    {
        auto const& data = node._data;
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
            .and_for_each([&](auto& chain, [[maybe_unused]] auto& i) {
        return RMustEqual<fge::ObjectSid, ROutputs::R_INVERTED>(FGE_SCENE_BAD_SID, chain)
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
                    return chain.invalidate("unattended data size", func);
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

void Scene::clientsCheckup(fge::net::ClientList const& clients, bool force)
{
    this->_netList.clientsCheckup(clients, force);

    auto const clientsEmpty = clients.getSize() == 0;
    for (auto const& data: *this)
    {
        data->getObject()->_netList.clientsCheckup(clients,
                                                   force || (data->g_requireForceClientsCheckup && !clientsEmpty));
        data->g_requireForceClientsCheckup = false;
    }

    //Remove/Add client
    if (force)
    {
        this->g_perClientSyncs.clear();
        this->g_perClientSyncs.reserve(clients.getSize());

        auto lock = clients.acquireLock();
        for (auto it = clients.begin(lock); it != clients.end(lock); ++it)
        {
            this->g_perClientSyncs.emplace(std::piecewise_construct, std::forward_as_tuple(it->first),
                                           std::forward_as_tuple(this->g_updateCount));
        }
    }
    else
    {
        //Watch ClientList events
        for (std::size_t i = 0; i < clients.getClientEventSize(); ++i)
        {
            auto const& evt = clients.getClientEvent(i);
            if (evt._event == fge::net::ClientList::Event::Types::EVT_DELCLIENT)
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
        auto const& event = events.front();

        switch (event._event)
        {
        case SceneNetEvent::Events::OBJECT_DELETED:
            pck << static_cast<fge::SceneNetEvent::Events_t>(fge::SceneNetEvent::Events::OBJECT_DELETED);
            //SID
            pck << event._sid;
            ++counter;
            break;
        case SceneNetEvent::Events::OBJECT_CREATED:
            if (auto const data = this->getObject(event._sid))
            {
                pck << static_cast<fge::SceneNetEvent::Events_t>(fge::SceneNetEvent::Events::OBJECT_CREATED);
                //SID
                pck << data->g_sid;
                //CLASS
                pck << fge::reg::GetClassId(data->g_object->getClassName());
                //PLAN
                pck << data->g_plan;
                //TYPE
                pck << static_cast<std::underlying_type_t<fge::ObjectTypes>>(data->g_type);

                data->g_object->pack(pck);
                ++counter;
            }
            break;
        case SceneNetEvent::Events::OBJECT_SIGNALED:
            pck << static_cast<fge::SceneNetEvent::Events_t>(fge::SceneNetEvent::Events::OBJECT_SIGNALED);
            //SID
            pck << event._sid;
            pck << event._signal;
            ++counter;
            break;
        default:
            throw fge::Exception("Unknown watchedEvent");
        }

        events.pop();
    }

    pck.pack(rewritePos, &counter, sizeof(counter));
}
std::optional<fge::net::Error> Scene::unpackWatchedEvent(fge::net::Packet const& pck)
{
    constexpr char const* const func = __func__;
    fge::ObjectSid buffSid;
    int8_t buffSignal;
    fge::reg::ClassId buffClass;
    fge::ObjectPlan buffPlan;
    fge::ObjectTypes buffType{fge::ObjectTypes::INVALID};

    using namespace fge::net::rules;

    return RValid<fge::net::SizeType>(pck)
            .and_for_each([&]([[maybe_unused]] auto& chain, [[maybe_unused]] auto& i) {
        //Event type
        return RStrictLess<fge::SceneNetEvent::Events_t>(
                       static_cast<fge::SceneNetEvent::Events_t>(fge::SceneNetEvent::Events::LAST_ENUM_), pck)
                .and_then([&](auto& chain) {
            switch (static_cast<fge::SceneNetEvent::Events>(chain.value()))
            {
            case SceneNetEvent::Events::OBJECT_DELETED:
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
                break;
            case SceneNetEvent::Events::OBJECT_CREATED:
            {
                //SID
                pck >> buffSid;
                //CLASS
                pck >> buffClass;
                //PLAN
                pck >> buffPlan;
                //TYPE
                fge::net::rules::RStrictLess<fge::ObjectTypes>(fge::ObjectTypes::_MAX_, pck).apply(buffType);
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
                this->newObject(FGE_NEWOBJECT_PTR(newObj), buffPlan, buffSid, buffType, false, OBJ_CONTEXT_NETWORK)
                        ->g_object->unpack(pck);
            }
            break;
            case SceneNetEvent::Events::OBJECT_SIGNALED:
                //SID
                pck >> buffSid;
                pck >> buffSignal;

                if (auto const object = this->getObject(buffSid))
                {
                    object->getObject()->netSignaled(buffSignal);
                }
                break;
            default:
                return chain.invalidate(
                        net::Error{net::Error::Types::ERR_EXTRACT, pck.getReadPos(), "unattended event", func});
            }

            return chain;
        }).end();
    }).end();
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
    if (auto ownView = this->getOwnView())
    {
        return ownView.get();
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

void Scene::save(nlohmann::json& jsonObject)
{
    jsonObject["SceneInfo"]["name"] = this->getName();

    jsonObject["SceneData"] = nlohmann::json::object();
    this->saveCustomData(jsonObject["SceneData"]);

    jsonObject["Objects"] = nlohmann::json::array();
    for (auto const& data: *this)
    {
        nlohmann::json objNewJson = nlohmann::json::object();
        nlohmann::json& objJson = objNewJson[data->getObject()->getClassName()];

        objJson = nlohmann::json::object();

        objJson["_sid"] = data->getSid();
        objJson["_plan"] = data->getPlan();
        objJson["_type"] = data->getType();

        data->getObject()->save(objJson);
        jsonObject["Objects"] += std::move(objNewJson);
    }
}
bool Scene::load(nlohmann::json& jsonObject, std::filesystem::path const& filePath, bool ignoreSid)
{
    this->clear();
    this->setName(jsonObject["SceneInfo"]["name"].get<std::string>());

    this->loadCustomData(jsonObject["SceneData"]);

    nlohmann::json& jsonObjArray = jsonObject["Objects"];
    for (auto& it: jsonObjArray)
    {
        fge::ObjectPtr buffObj{fge::reg::GetNewClassOf(it.begin().key())};

        if (buffObj)
        {
            nlohmann::json& objJson = it.begin().value();

            auto const sid = ignoreSid ? FGE_SCENE_BAD_SID : objJson["_sid"].get<fge::ObjectSid>();

            this->newObjectPlanBack(std::move(buffObj), objJson["_plan"].get<fge::ObjectPlan>(), sid,
                                    objJson["_type"].get<fge::ObjectTypes>())
                    ->g_object->load(objJson, filePath);
        }
        else
        {
            return false;
        }
    }
    return true;
}
bool Scene::saveInFile(std::filesystem::path const& path, int fieldWidth)
{
    nlohmann::json outputJson;
    this->save(outputJson);
    return SaveJsonToFile(path, outputJson, fieldWidth);
}
bool Scene::loadFromFile(std::filesystem::path const& path, bool ignoreSid)
{
    nlohmann::json inputJson;
    if (!LoadJsonFromFile(path, inputJson))
    {
        return false;
    }
    return this->load(inputJson, path, ignoreSid);
}

/** Iterator **/
Scene::const_iterator Scene::find(fge::ObjectSid sid) const
{
    auto it = this->g_objectsHashMap.find(sid);
    return it ? const_iterator{&(*it.value()), &this->g_drawTail} : this->end();
}
Scene::const_iterator Scene::find(fge::Object const* ptr) const
{
    for (auto it = this->begin(); it != this->end(); ++it)
    {
        if (it->get()->g_object.get() == ptr)
        {
            return it;
        }
    }
    return this->end();
}
Scene::const_iterator Scene::findPlan(fge::ObjectPlan plan) const
{
    auto it = this->g_planDataMap.find(plan);
    return (it != this->g_planDataMap.cend()) ? const_iterator{it->second, &this->g_drawTail} : this->end();
}

//Private
void Scene::hash_updatePlanDataMap(fge::ObjectPlan plan, fge::ObjectNode* who, bool isLeaving)
{
    /*
     * This function is called when an object (who) HAVE BEEN moved in this plan and must be checked
     * or an object IS ABOUT to leave the plan (still linked in the draw order).
     */

    if (isLeaving)
    {
        auto it = this->g_planDataMap.find(plan);
        if (it != this->g_planDataMap.end() && it->second == who)
        {
            auto* const next = who->g_next; //go to the next element and check is plan (or end)
            if (next != nullptr && next->_data->g_plan == plan)
            {
                it->second = next;
                return;
            }
            //erase the element
            this->g_planDataMap.erase(it);
        }
    }
    else
    {
        //we try to insert it
        auto pair = this->g_planDataMap.emplace(plan, who);
        if (!pair.second)
        { //we have to check the object on the left.
            if (pair.first->second->g_prev == who)
            { //the new object is on the left, so we update the stocked node
                pair.first->second = who;
            }
        }
    }
}
fge::ObjectNode* Scene::hash_getInsertionIteratorFromPlanDataMap(fge::ObjectPlan plan, bool planBack) const
{
    /*
     * Return the node before which a new object of this plan must be inserted (nullptr means the end):
     * - front of the plan: first object of the plan, or first object of the next higher plan
     * - back of the plan: first object of the next higher plan
     */
    auto it = planBack ? this->g_planDataMap.upper_bound(plan) : this->g_planDataMap.lower_bound(plan);
    return it != this->g_planDataMap.end() ? it->second : nullptr;
}

void Scene::drawOrder_insertBefore(fge::ObjectNode* node, fge::ObjectNode* before)
{
    node->g_next = before;

    if (before == nullptr)
    { //Push back
        node->g_prev = this->g_drawTail;
        if (this->g_drawTail != nullptr)
        {
            this->g_drawTail->g_next = node;
        }
        else
        {
            this->g_drawHead = node;
        }
        this->g_drawTail = node;
        return;
    }

    node->g_prev = before->g_prev;
    if (before->g_prev != nullptr)
    {
        before->g_prev->g_next = node;
    }
    else
    {
        this->g_drawHead = node;
    }
    before->g_prev = node;
}
void Scene::drawOrder_unlink(fge::ObjectNode* node)
{
    if (node->g_prev != nullptr)
    {
        node->g_prev->g_next = node->g_next;
    }
    else
    {
        this->g_drawHead = node->g_next;
    }

    if (node->g_next != nullptr)
    {
        node->g_next->g_prev = node->g_prev;
    }
    else
    {
        this->g_drawTail = node->g_prev;
    }

    node->g_prev = nullptr;
    node->g_next = nullptr;
}
void Scene::drawOrder_move(fge::ObjectNode* node, fge::ObjectNode* before)
{
    if (node == before)
    {
        return;
    }
    this->drawOrder_unlink(node);
    this->drawOrder_insertBefore(node, before);
}

} // namespace fge

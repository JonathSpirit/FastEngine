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

#include "FastEngine/object/C_childObjectsAccessor.hpp"
#include "FastEngine/C_scene.hpp"

namespace fge
{

void ChildObjectsAccessor::DataContext::NotHandledObjectDeleter::operator()(fge::ObjectData* data) const
{
    (void) data->releaseObject();
    delete data;
}

ChildObjectsAccessor::ChildObjectsAccessor(fge::Object* owner) :
        g_owner(owner)
{}

void ChildObjectsAccessor::clear()
{
    this->g_data.clear();
}

fge::ObjectDataShared ChildObjectsAccessor::addExistingObject(fge::Object* object, std::size_t insertionIndex)
{
    fge::Scene* linkedScene = nullptr;
    auto owner = this->g_owner->_myObjectData.lock();
    if (owner)
    {
        linkedScene = owner->getScene();
    }

    std::vector<DataContext>::iterator it;
    if (insertionIndex >= this->g_data.size())
    {
        it = this->g_data.insert(this->g_data.end(),
                                 {object, fge::ObjectDataShared{new fge::ObjectData{linkedScene, fge::ObjectPtr{object},
                                                                                    owner->getSid()},
                                                                DataContext::NotHandledObjectDeleter{}}});
    }
    else
    {
        it = this->g_data.insert(
                this->g_data.begin() + static_cast<std::vector<DataContext>::difference_type>(insertionIndex),
                {object,
                 fge::ObjectDataShared{new fge::ObjectData{linkedScene, fge::ObjectPtr{object}, owner->getSid()},
                                       DataContext::NotHandledObjectDeleter{}}});
    }

    it->_objData->setParent(owner);
    it->_objPtr->_myObjectData = it->_objData;

    if (linkedScene != nullptr)
    {
        it->_objPtr->first(*linkedScene);
    }

    return it->_objData;
}
fge::ObjectDataShared ChildObjectsAccessor::addNewObject(fge::ObjectPtr&& newObject, std::size_t insertionIndex)
{
    fge::Scene* linkedScene = nullptr;
    auto owner = this->g_owner->_myObjectData.lock();
    if (owner)
    {
        linkedScene = owner->getScene();
    }

    std::vector<DataContext>::iterator it;
    if (insertionIndex >= this->g_data.size())
    {
        it = this->g_data.insert(this->g_data.end(),
                                 {newObject.get(), std::make_shared<fge::ObjectData>(linkedScene, std::move(newObject),
                                                                                     owner->getSid())});
    }
    else
    {
        it = this->g_data.insert(
                this->g_data.begin() + static_cast<std::vector<DataContext>::difference_type>(insertionIndex),
                {newObject.get(),
                 std::make_shared<fge::ObjectData>(linkedScene, std::move(newObject), owner->getSid())});
    }

    it->_objData->setParent(owner);
    it->_objPtr->_myObjectData = it->_objData;

    if (linkedScene != nullptr)
    {
        it->_objPtr->first(*linkedScene);
    }

    return it->_objData;
}

std::size_t ChildObjectsAccessor::getSize() const
{
    return this->g_data.size();
}
fge::Object const* ChildObjectsAccessor::get(std::size_t index) const
{
    return this->g_data[index]._objPtr;
}
fge::Object* ChildObjectsAccessor::get(std::size_t index)
{
    return this->g_data[index]._objPtr;
}
fge::ObjectDataShared ChildObjectsAccessor::getSharedPtr(std::size_t index) const
{
    return this->g_data[index]._objData;
}

void ChildObjectsAccessor::remove(std::size_t index)
{
    if (index < this->g_data.size())
    {
        this->g_data.erase(this->g_data.begin() + static_cast<std::vector<DataContext>::difference_type>(index));
    }
}
void ChildObjectsAccessor::remove(std::size_t first, std::size_t last)
{
    if (first < this->g_data.size())
    {
        this->g_data.erase(this->g_data.begin() + static_cast<std::vector<DataContext>::difference_type>(first),
                           this->g_data.begin() + static_cast<std::vector<DataContext>::difference_type>(last));
    }
}

#ifdef FGE_DEF_SERVER
void ChildObjectsAccessor::update(fge::Event& event, std::chrono::microseconds const& deltaTime, fge::Scene& scene)
{
    for (this->g_actualIteratedIndex = 0; this->g_actualIteratedIndex < this->g_data.size();
         ++this->g_actualIteratedIndex)
    {
        this->g_data[this->g_actualIteratedIndex]._objPtr->update(event, deltaTime, scene);
    }
}
#else
void ChildObjectsAccessor::update(fge::RenderWindow& screen,
                                  fge::Event& event,
                                  std::chrono::microseconds const& deltaTime,
                                  fge::Scene& scene) const
{
    for (this->g_actualIteratedIndex = 0; this->g_actualIteratedIndex < this->g_data.size();
         ++this->g_actualIteratedIndex)
    {
        this->g_data[this->g_actualIteratedIndex]._objPtr->update(screen, event, deltaTime, scene);
    }
}
void ChildObjectsAccessor::draw(fge::RenderTarget& target, fge::RenderStates const& states) const
{
    for (this->g_actualIteratedIndex = 0; this->g_actualIteratedIndex < this->g_data.size();
         ++this->g_actualIteratedIndex)
    {
        this->g_data[this->g_actualIteratedIndex]._objData->setPlanDepth(this->g_actualIteratedIndex);
        this->g_data[this->g_actualIteratedIndex]._objPtr->draw(target, states);
    }
}
#endif //FGE_DEF_SERVER

void ChildObjectsAccessor::putInFront(std::size_t index)
{
    if (index < this->g_data.size() && index != 0)
    {
        auto data = this->g_data[index];
        this->g_data.erase(this->g_data.begin() + static_cast<std::vector<DataContext>::difference_type>(index));
        this->g_data.insert(this->g_data.begin(), std::move(data));
    }
}
void ChildObjectsAccessor::putInBack(std::size_t index)
{
    if (index < this->g_data.size() && index != this->g_data.size() - 1)
    {
        auto data = this->g_data[index];
        this->g_data.erase(this->g_data.begin() + static_cast<std::vector<DataContext>::difference_type>(index));
        this->g_data.push_back(std::move(data));
    }
}

std::size_t ChildObjectsAccessor::getActualIteratedIndex() const
{
    return this->g_actualIteratedIndex;
}

std::size_t ChildObjectsAccessor::getIndex(fge::Object* object) const
{
    for (std::size_t i = 0; i < this->g_data.size(); ++i)
    {
        if (this->g_data[i]._objPtr == object)
        {
            return i;
        }
    }
    return std::numeric_limits<std::size_t>::max();
}

} // namespace fge

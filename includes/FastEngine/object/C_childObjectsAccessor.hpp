/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_C_CHILDOBJECTSACCESSOR_HPP_INCLUDED
#define _FGE_C_CHILDOBJECTSACCESSOR_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/graphic/C_renderStates.hpp"
#include "FastEngine/graphic/C_renderTarget.hpp"
#include <chrono>
#include <limits>
#include <memory>
#include <vector>

namespace fge
{

using DeltaTime = std::chrono::microseconds;

class Event;
class Scene;
class Object;
class ObjectData;
using ObjectPtr = std::unique_ptr<fge::Object>;
using ObjectDataWeak = std::weak_ptr<fge::ObjectData>;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;
using ObjectPlan = uint16_t;

class FGE_API ChildObjectsAccessor
{
public:
    explicit ChildObjectsAccessor(fge::Object* owner);
    ChildObjectsAccessor([[maybe_unused]] ChildObjectsAccessor const& r) {}
    ChildObjectsAccessor([[maybe_unused]] ChildObjectsAccessor&& r) noexcept {}
    ~ChildObjectsAccessor();

    ChildObjectsAccessor& operator=([[maybe_unused]] ChildObjectsAccessor const& r) { return *this; }
    ChildObjectsAccessor& operator=([[maybe_unused]] ChildObjectsAccessor&& r) noexcept { return *this; }

    void clear();
    void clearDetachedObjects();

    fge::ObjectDataShared addExistingObject(fge::Object* object,
                                            std::size_t insertionIndex = std::numeric_limits<std::size_t>::max());
    fge::ObjectDataShared addNewObject(fge::ObjectPtr&& newObject,
                                       std::size_t insertionIndex = std::numeric_limits<std::size_t>::max());

    fge::ObjectDataShared addExistingDetachedObject(fge::Object* object, fge::ObjectPlan newPlan) const;
    fge::ObjectDataShared addNewDetachedObject(fge::ObjectPtr&& newObject, fge::ObjectPlan newPlan) const;

    bool detachObject(std::size_t index, fge::ObjectPlan newPlan);

    [[nodiscard]] std::size_t getSize() const;
    [[nodiscard]] fge::Object const* get(std::size_t index) const;
    [[nodiscard]] fge::Object* get(std::size_t index);
    [[nodiscard]] fge::ObjectDataShared getSharedPtr(std::size_t index) const;

    void remove(std::size_t index);
    void remove(std::size_t first, std::size_t last);

#ifdef FGE_DEF_SERVER
    void update(fge::Event& event, fge::DeltaTime const& deltaTime, fge::Scene& scene);
#else
    void update(fge::RenderTarget& target, fge::Event& event, fge::DeltaTime const& deltaTime, fge::Scene& scene) const;
    void draw(fge::RenderTarget& target, fge::RenderStates const& states) const;
#endif //FGE_DEF_SERVER

    void putInFront(std::size_t index);
    void putInBack(std::size_t index);

    [[nodiscard]] std::size_t getActualIteratedIndex() const;

    [[nodiscard]] std::size_t getIndex(fge::Object* object) const;

private:
    struct DataContext
    {
        struct NotHandledObjectDeleter
        {
            void operator()(fge::ObjectData* data) const;
        };

        fge::Object* _objPtr;
        fge::ObjectDataShared _objData;
    };

    void cleanupDetachedObjects() const;

    mutable std::vector<fge::ObjectDataWeak> g_detachedObjects;
    std::vector<DataContext> g_data;
    mutable std::size_t g_actualIteratedIndex{std::numeric_limits<std::size_t>::max()};
    fge::Object* g_owner{nullptr};
};

template<class TObject>
class DeclareChild
{
public:
    template<class TOwner, class... TArgs>
    constexpr DeclareChild(TOwner* owner, TArgs&&... args) :
            g_object(std::forward<TArgs>(args)...)
    {
        owner->_children.addExistingObject(&this->g_object);
    }

    [[nodiscard]] constexpr TObject* operator->() { return &this->g_object; }
    [[nodiscard]] constexpr TObject const* operator->() const { return &this->g_object; }

    [[nodiscard]] constexpr TObject& get() { return this->g_object; }
    [[nodiscard]] constexpr TObject const& get() const { return this->g_object; }

    template<class TOwner>
    inline bool detach(TOwner* owner, fge::ObjectPlan newPlan)
    {
        return owner->_children.detachObject(owner->_children.getIndex(&this->g_object), newPlan);
    }

private:
    TObject g_object;
};

} // namespace fge

#endif // _FGE_C_CHILDOBJECTSACCESSOR_HPP_INCLUDED
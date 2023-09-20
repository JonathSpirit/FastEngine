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

#ifndef _FGE_C_CHILDOBJECTSACCESSOR_HPP_INCLUDED
#define _FGE_C_CHILDOBJECTSACCESSOR_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/graphic/C_renderStates.hpp"
#include "FastEngine/graphic/C_renderWindow.hpp"
#include <chrono>
#include <limits>
#include <memory>
#include <vector>

namespace fge
{

class Event;
class Scene;
class Object;
class ObjectData;
using ObjectPtr = std::unique_ptr<fge::Object>;
using ObjectDataWeak = std::weak_ptr<fge::ObjectData>;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;

class FGE_API ChildObjectsAccessor
{
public:
    ChildObjectsAccessor() = default;
    ChildObjectsAccessor([[maybe_unused]] ChildObjectsAccessor const& r){};
    ChildObjectsAccessor([[maybe_unused]] ChildObjectsAccessor&& r) noexcept {};

    ChildObjectsAccessor& operator=([[maybe_unused]] ChildObjectsAccessor const& r) { return *this; };
    ChildObjectsAccessor& operator=([[maybe_unused]] ChildObjectsAccessor&& r) noexcept { return *this; };

    void clear();

    void addExistingObject(fge::ObjectDataWeak const& parent,
                           fge::Object* object,
                           fge::Scene* linkedScene,
                           std::size_t insertionIndex = std::numeric_limits<std::size_t>::max());
    void addNewObject(fge::ObjectDataWeak const& parent,
                      fge::ObjectPtr&& newObject,
                      fge::Scene* linkedScene,
                      std::size_t insertionIndex = std::numeric_limits<std::size_t>::max());

    [[nodiscard]] std::size_t getSize() const;
    [[nodiscard]] fge::Object const* get(std::size_t index) const;
    [[nodiscard]] fge::Object* get(std::size_t index);
    [[nodiscard]] fge::ObjectDataShared getSharedPtr(std::size_t index) const;

    void remove(std::size_t index);
    void remove(std::size_t first, std::size_t last);

#ifdef FGE_DEF_SERVER
    void update(fge::Event& event, std::chrono::milliseconds const& deltaTime, fge::Scene* scene);
#else
    void
    update(fge::RenderWindow& screen, fge::Event& event, std::chrono::milliseconds const& deltaTime, fge::Scene* scene);
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
            void operator()(fge::ObjectData* data);
        };

        fge::Object* _objPtr;
        fge::ObjectDataShared _objData;
    };

    std::vector<DataContext> g_data;
    mutable std::size_t g_actualIteratedIndex{std::numeric_limits<std::size_t>::max()};
};

} // namespace fge

#endif // _FGE_C_CHILDOBJECTSACCESSOR_HPP_INCLUDED
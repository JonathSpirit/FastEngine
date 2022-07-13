/*
 * Copyright 2022 Guillaume Guillet
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

#include <FastEngine/fastengine_extern.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <vector>
#include <chrono>
#include <limits>
#include <memory>

namespace fge
{

class Event;
class Scene;
class Object;
class ObjectData;
using ObjectDataWeak = std::weak_ptr<fge::ObjectData>;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;

class FGE_API ChildObjectsAccessor : public sf::Drawable
{
public:
    ChildObjectsAccessor() = default;
    ChildObjectsAccessor([[maybe_unused]]const ChildObjectsAccessor& r){};
    ChildObjectsAccessor([[maybe_unused]]ChildObjectsAccessor&& r) noexcept {};
    ~ChildObjectsAccessor() override = default;

    ChildObjectsAccessor& operator=([[maybe_unused]]const ChildObjectsAccessor& r){return *this;};
    ChildObjectsAccessor& operator=([[maybe_unused]]ChildObjectsAccessor&& r) noexcept {return *this;};

    void clear();

    void addExistingObject(const fge::ObjectDataWeak& parent, fge::Object* object, fge::Scene* linkedScene, std::size_t insertionIndex=std::numeric_limits<std::size_t>::max());
    void addNewObject(const fge::ObjectDataWeak& parent, fge::Object* newObject, fge::Scene* linkedScene, std::size_t insertionIndex=std::numeric_limits<std::size_t>::max());

    [[nodiscard]] std::size_t getSize() const;
    [[nodiscard]] const fge::Object* get(std::size_t index) const;
    [[nodiscard]] fge::Object* get(std::size_t index);
    [[nodiscard]] fge::ObjectDataShared getSharedPtr(std::size_t index) const;

    void remove(std::size_t index);
    void remove(std::size_t first, std::size_t last);

    void update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

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

}//end fge

#endif // _FGE_C_CHILDOBJECTSACCESSOR_HPP_INCLUDED
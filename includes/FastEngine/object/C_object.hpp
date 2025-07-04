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

#ifndef _FGE_C_OBJECT_HPP_INCLUDED
#define _FGE_C_OBJECT_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_childObjectsAccessor.hpp"
#include "FastEngine/C_event.hpp"
#include "FastEngine/C_propertyList.hpp"
#include "FastEngine/C_quad.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_tagList.hpp"
#include "FastEngine/graphic/C_drawable.hpp"
#include "FastEngine/graphic/C_transformable.hpp"
#include "FastEngine/network/C_networkType.hpp"
#include "FastEngine/object/C_objectAnchor.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "json.hpp"

#include <chrono>
#include <string>

#define FGE_OBJ_BADCLASSNAME "NULL"
#define FGE_OBJ_NOSCENE nullptr
#define FGE_OBJ_DEFAULT_COPYMETHOD(objClass)                                                                           \
    fge::Object* copy() override                                                                                       \
    {                                                                                                                  \
        return new objClass(*this);                                                                                    \
    }

#ifdef FGE_DEF_SERVER
    #define FGE_OBJ_UPDATE_DECLARE                                                                                     \
        void update(fge::Event& event, fge::DeltaTime const& deltaTime, fge::Scene& scene) override;
#else
    #define FGE_OBJ_UPDATE_DECLARE                                                                                     \
        void update(fge::RenderTarget& target, fge::Event& event, fge::DeltaTime const& deltaTime, fge::Scene& scene)  \
                override;
#endif //FGE_DEF_SERVER

#ifdef FGE_DEF_SERVER
    #define FGE_OBJ_UPDATE_BODY(class_)                                                                                \
        void class_::update([[maybe_unused]] fge::Event& event, [[maybe_unused]] fge::DeltaTime const& deltaTime,      \
                            [[maybe_unused]] fge::Scene& scene)

    #define FGE_OBJ_UPDATE_CALL(object_) object_.update(event, deltaTime, scene)
    #define FGE_OBJ_UPDATE_PTRCALL(object_) object_->update(event, deltaTime, scene)
#else
    #define FGE_OBJ_UPDATE_BODY(class_)                                                                                \
        void class_::update([[maybe_unused]] fge::RenderTarget& target, [[maybe_unused]] fge::Event& event,            \
                            [[maybe_unused]] fge::DeltaTime const& deltaTime, [[maybe_unused]] fge::Scene& scene)

    #define FGE_OBJ_UPDATE_CALL(object_) object_.update(target, event, deltaTime, scene)
    #define FGE_OBJ_UPDATE_PTRCALL(object_) object_->update(target, event, deltaTime, scene)
#endif //FGE_DEF_SERVER

#ifdef FGE_DEF_SERVER
    #define FGE_OBJ_DRAW_DECLARE
#else
    #define FGE_OBJ_DRAW_DECLARE void draw(fge::RenderTarget& target, const fge::RenderStates& states) const override;
#endif //FGE_DEF_SERVER

#define FGE_OBJ_DRAW_BODY(class_) void class_::draw(fge::RenderTarget& target, const fge::RenderStates& states) const

namespace fge
{

using DeltaTime = std::chrono::microseconds;

class GuiElementHandler;
class GuiElement;

class Scene;

class ObjectData;
using ObjectDataWeak = std::weak_ptr<fge::ObjectData>;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;

/**
 * \class Object
 * \ingroup objectControl
 * \brief The Object class is the base class for all objects in the engine.
 */
#ifdef FGE_DEF_SERVER
class FGE_API Object : public fge::Transformable, public fge::Anchor
#else
class FGE_API Object : public fge::Drawable, public fge::Transformable, public fge::Anchor
#endif //FGE_DEF_SERVER
{
public:
    Object();
    Object(Object const& r);
    Object(Object&& r) noexcept;
    ~Object() override = default;

    /**
     * \brief Duplicate the object
     *
     * By default, if the copy method is not overridden, the object is duplicated with
     * the help of the register manager (and the object class have to be registered).
     *
     * \return An allocated pointer to the duplicated object
     */
    virtual fge::Object* copy();

    /**
     * \brief Method called when the object is added to a scene for initialization purposes.
     *
     * \param scene The scene where the object is added
     */
    virtual void first(fge::Scene& scene);
    /**
     * \brief Method called when the object is transferred from a scene to another.
     *
     * This method is called after the object is removed from the old scene and added to the new scene.
     * The _myObjectData is updated to the new scene when this method is called.
     *
     * \param oldScene The old scene where the object was
     * \param newScene The new scene where the object is
     */
    virtual void transfered(fge::Scene& oldScene, fge::Scene& newScene);
    /**
     * \brief Ask the object to register all callbacks it needs to receive events.
     *
     * \param event The event system
     * \param guiElementHandlerPtr The GUI element handler
     */
    virtual void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr);
    /**
     * \brief Main method called every frame.
     *
     * \param target The target where the object is drawn
     * \param event The event system
     * \param deltaTime The time since the last frame
     * \param scene The scene where the object is updated
     */
#ifdef FGE_DEF_SERVER
    virtual void update(fge::Event& event, fge::DeltaTime const& deltaTime, fge::Scene& scene);
    void update(fge::Event& event, fge::DeltaTime const& deltaTime);
#else
    virtual void
    update(fge::RenderTarget& target, fge::Event& event, fge::DeltaTime const& deltaTime, fge::Scene& scene);
    void update(fge::RenderTarget& target, fge::Event& event, fge::DeltaTime const& deltaTime);
#endif //FGE_DEF_SERVER
    /**
     * \brief Method called every frame to draw the object
     *
     * \param target The target where the object is drawn
     * \param states The render states
     */
#ifndef FGE_DEF_SERVER
    virtual void draw(fge::RenderTarget& target, fge::RenderStates const& states) const override;
#endif //FGE_DEF_SERVER
    /**
     * \brief Register all network types needed by the object
     */
    virtual void networkRegister();
    /**
     * \brief Method called when the object is signaled by the network
     *
     * \param signal The signal received
     */
    virtual void netSignaled(int8_t signal);
    /**
     * \brief Method called when the object is removed from a scene
     *
     * \param scene The scene where the object is removed
     */
    virtual void removed(fge::Scene& scene);

    /**
     * \brief Save the object to a json object
     *
     * \param jsonObject The json object where the object is saved
     */
    virtual void save(nlohmann::json& jsonObject);
    /**
     * \brief Load the object from a json object
     *
     * \param jsonObject The json object where the object is loaded
     * \param filePath The path of the main file where the object is loaded from
     */
    virtual void load(nlohmann::json& jsonObject, std::filesystem::path const& filePath);
    /**
     * \brief Pack the object into a packet
     *
     * \param pck The packet where the object is packed
     */
    virtual void pack(fge::net::Packet& pck);
    /**
     * \brief Unpack the object from a packet
     *
     * \param pck The packet where the object is unpacked
     */
    virtual void unpack(fge::net::Packet const& pck);
    //TODO: Apply network rules on every extraction method on every objects.

    /**
     * \brief Get the unique class name of the object
     *
     * \return The unique class name of the object
     */
    virtual char const* getClassName() const;
    /**
     * \brief Get a readable version of the class name
     *
     * \return A readable version of the class name
     */
    virtual char const* getReadableClassName() const;

    /**
     * \brief Get the global bounds of the object
     *
     * \return The global bounds of the object
     */
    [[nodiscard]] virtual fge::RectFloat getGlobalBounds() const;
    [[nodiscard]] virtual fge::Quad getGlobalQuad() const;
    /**
     * \brief Get the local bounds of the object (without any transformations)
     *
     * \return The local bounds of the object
     */
    [[nodiscard]] virtual fge::RectFloat getLocalBounds() const;
    [[nodiscard]] virtual fge::Quad getLocalQuad() const;

    /**
     * \brief Save the object in a file
     *
     * \param path The path of the file
     * \param fieldWidth The width of the fields in the file
     * \param saveClassName If the class name must be saved
     * \return \b true if the object was saved, \b false otherwise
     */
    bool saveInFile(std::filesystem::path const& path, int fieldWidth = 2, bool saveClassName = true);
    /**
     * \brief Load the object from a file
     *
     * \param path The path of the file
     * \param loadClassName Load and verify the class name
     * \return \b true if the object was loaded, \b false otherwise
     */
    bool loadFromFile(std::filesystem::path const& path, bool loadClassName = true);
    /**
     * \brief Static form of the loadFromFile method
     *
     * \param path The path of the file
     * \return The allocated pointer of the loaded object or nullptr if the object was not loaded
     */
    [[nodiscard]] static std::unique_ptr<fge::Object> LoadFromFile(std::filesystem::path const& path);

    /**
     * \brief Get the GuiElement attached to this object if there is one
     *
     * \return The GuiElement pointer or \b nullptr
     */
    virtual fge::GuiElement* getGuiElement();

    /**
     * \brief Retrieve recursively all parents transform by combining them
     *
     * \return Parents transform
     */
    glm::mat4 getParentsTransform() const;
    /**
     * \brief Retrieve recursively all parents scale by combining them
     *
     * \return Parents scale
     */
    fge::Vector2f getParentsScale() const;
    /**
     * \brief Center the origin of the object from its local bounds
     */
    void centerOriginFromLocalBounds();

    //Data

    fge::TagList _tags;            ///< The tags of the object
    fge::PropertyList _properties; ///< The properties of the object

    //Network

    fge::net::NetworkTypeHandler _netList; ///< The network types container of the object

    //Scene control

    fge::ObjectDataWeak _myObjectData; ///< The object data of the object (valid only if the object is in a scene)

    enum class DrawModes : uint8_t
    {
        DRAW_IF_ON_TARGET,
        DRAW_ALWAYS_HIDDEN,
        DRAW_ALWAYS_DRAWN,

        DRAW_DEFAULT = DRAW_IF_ON_TARGET
    };
    DrawModes _drawMode{DrawModes::DRAW_DEFAULT}; ///< Tell a scene when this object should be drawn

    enum class CallbackContextModes : uint8_t
    {
        CONTEXT_MANUAL,
        CONTEXT_AUTO,

        CONTEXT_DEFAULT = CONTEXT_AUTO
    };
    CallbackContextModes _callbackContextMode{
            CallbackContextModes::CONTEXT_DEFAULT}; ///< Tell a scene how the callbackRegister must be called

    enum class NetSyncModes : uint8_t
    {
        NO_SYNC,
        FULL_SYNC,
        DELTA_SYNC,

#ifdef FGE_DEF_SERVER
        NETSYNC_DEFAULT = FULL_SYNC
#else
        NETSYNC_DEFAULT = NO_SYNC
#endif //FGE_DEF_SERVER
    };
    NetSyncModes _netSyncMode{NetSyncModes::NETSYNC_DEFAULT}; ///< Tell a scene how the object must be synchronised
    net::Identity _netOwner{};                                ///< The owner of the object

    //Child objects

    enum ChildrenControlFlags : uint8_t
    {
        CHILDREN_AUTO_CLEAR_ON_REMOVE = 1 << 0,
        CHILDREN_AUTO_UPDATE = 1 << 1,
        CHILDREN_AUTO_DRAW = 1 << 2,

        CHILDREN_DEFAULT = CHILDREN_AUTO_CLEAR_ON_REMOVE
    };
    using ChildrenControlFlags_t = std::underlying_type_t<ChildrenControlFlags>;

    ChildrenControlFlags_t _childrenControlFlags{CHILDREN_DEFAULT}; ///< The control flags of the child objects
    fge::ChildObjectsAccessor _children;                            ///< An access to child objects of this object
};

} // namespace fge

#endif // _FGE_C_OBJECT_HPP_INCLUDED

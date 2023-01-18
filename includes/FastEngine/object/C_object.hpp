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

#ifndef _FGE_C_OBJECT_HPP_INCLUDED
#define _FGE_C_OBJECT_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "C_childObjectsAccessor.hpp"
#include "FastEngine/C_event.hpp"
#include "FastEngine/C_networkType.hpp"
#include "FastEngine/C_packet.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_tagList.hpp"
#include "FastEngine/graphic/C_drawable.hpp"
#include "FastEngine/graphic/C_transformable.hpp"
#include "FastEngine/object/C_objectAnchor.hpp"
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
        void update(fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene) override;
#else
    #define FGE_OBJ_UPDATE_DECLARE                                                                                     \
        void update(fge::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime,           \
                    fge::Scene* scene) override;
#endif //FGE_DEF_SERVER

#ifdef FGE_DEF_SERVER
    #define FGE_OBJ_UPDATE_BODY(class_)                                                                                \
        void class_::update([[maybe_unused]] fge::Event& event,                                                        \
                            [[maybe_unused]] const std::chrono::milliseconds& deltaTime,                               \
                            [[maybe_unused]] fge::Scene* scene)

    #define FGE_OBJ_UPDATE_CALL(object_) object_.update(event, deltaTime, scene)
    #define FGE_OBJ_UPDATE_PTRCALL(object_) object_->update(event, deltaTime, scene)
#else
    #define FGE_OBJ_UPDATE_BODY(class_)                                                                                \
        void class_::update([[maybe_unused]] fge::RenderWindow& screen, [[maybe_unused]] fge::Event& event,             \
                            [[maybe_unused]] const std::chrono::milliseconds& deltaTime,                               \
                            [[maybe_unused]] fge::Scene* scene)

    #define FGE_OBJ_UPDATE_CALL(object_) object_.update(screen, event, deltaTime, scene)
    #define FGE_OBJ_UPDATE_PTRCALL(object_) object_->update(screen, event, deltaTime, scene)
#endif //FGE_DEF_SERVER

#ifdef FGE_DEF_SERVER
    #define FGE_OBJ_DRAW_DECLARE
#else
    #define FGE_OBJ_DRAW_DECLARE void draw(fge::RenderTarget& target, const fge::RenderStates& states) const override;
#endif //FGE_DEF_SERVER

#define FGE_OBJ_DRAW_BODY(class_) void class_::draw(fge::RenderTarget& target, const fge::RenderStates& states) const

namespace fge
{

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
    Object(const Object& r);
    Object(Object&& r);
    ~Object() override = default;

    /**
     * \brief Duplicate the object
     *
     * By default, if the copy method is not overridden, the object is duplicated with
     * the help of the register manager (and the object class have to be registered).
     *
     * \return A allocated pointer to the duplicated object
     */
    virtual fge::Object* copy();

    /**
     * \brief Method called when the object is added to a scene for initialization purposes.
     *
     * \param scene The scene where the object is added (can be nullptr)
     */
    virtual void first(fge::Scene* scene);
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
     * \param screen The screen where the object is drawn
     * \param event The event system
     * \param deltaTime The time since the last frame
     * \param scene The scene where the object is updated (can be nullptr)
     */
#ifdef FGE_DEF_SERVER
    virtual void update(fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene);
#else
    virtual void
    update(fge::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene);
#endif //FGE_DEF_SERVER
    /**
     * \brief Method called every frame to draw the object
     *
     * \param target The target where the object is drawn
     * \param states The SFML render states
     */
#ifndef FGE_DEF_SERVER
    virtual void draw(fge::RenderTarget& target, const fge::RenderStates& states) const override;
#endif //FGE_DEF_SERVER
    /**
     * \brief Register all network types needed by the object
     */
    virtual void networkRegister();
    /**
     * \brief Method called when the object is removed from a scene
     *
     * \param scene The scene where the object is removed (can be nullptr)
     */
    virtual void removed(fge::Scene* scene);

    /**
     * \brief Save the object to a json object
     *
     * \param jsonObject The json object where the object is saved
     * \param scene The scene where the object is saved (can be nullptr)
     */
    virtual void save(nlohmann::json& jsonObject, fge::Scene* scene);
    /**
     * \brief Load the object from a json object
     *
     * \param jsonObject The json object where the object is loaded
     * \param scene The scene where the object is loaded (can be nullptr)
     */
    virtual void load(nlohmann::json& jsonObject, fge::Scene* scene);
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
    virtual void unpack(fge::net::Packet& pck);
    //TODO: Apply network rules on every extraction method on every objects.

    /**
     * \brief Get the unique class name of the object
     *
     * \return The unique class name of the object
     */
    virtual const char* getClassName() const;
    /**
     * \brief Get a readable version of the class name
     *
     * \return A readable version of the class name
     */
    virtual const char* getReadableClassName() const;

    /**
     * \brief Get the global bounds of the object
     *
     * \return The global bounds of the object
     */
    virtual fge::RectFloat getGlobalBounds() const;
    /**
     * \brief Get the local bounds of the object (without any transformations)
     *
     * \return The local bounds of the object
     */
    virtual fge::RectFloat getLocalBounds() const;

    /**
     * \brief Save the object in a file
     *
     * \param path The path of the file
     * \return \b true if the object was saved, \b false otherwise
     */
    bool saveInFile(const std::string& path);
    /**
     * \brief Load the object from a file
     *
     * \param path The path of the file
     * \return \b true if the object was loaded, \b false otherwise
     */
    bool loadFromFile(const std::string& path);
    /**
     * \brief Static form of the loadFromFile method
     *
     * \param path The path of the file
     * \return The allocated pointer of the loaded object or nullptr if the object was not loaded
     */
    static fge::Object* LoadFromFile(const std::string& path);

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

    //Data

    fge::TagList _tags; ///< The tags of the object

    //Network

    fge::net::NetworkTypeContainer _netList; ///< The network types container of the object

    //Scene control

    fge::ObjectDataWeak _myObjectData{}; ///< The object data of the object (valid only if the object is in a scene)

    enum class DrawModes : uint8_t
    {
        DRAW_IF_ON_TARGET,
        DRAW_ALWAYS_HIDDEN,
        DRAW_ALWAYS_DRAWN,

        DRAW_DEFAULT = DRAW_IF_ON_TARGET
    };
    fge::Object::DrawModes _drawMode{
            fge::Object::DrawModes::DRAW_DEFAULT}; ///< Tell a scene when this object should be drawn

    enum class CallbackContextModes : uint8_t
    {
        CONTEXT_MANUAL,
        CONTEXT_AUTO,

        CONTEXT_DEFAULT = CONTEXT_AUTO
    };
    fge::Object::CallbackContextModes _callbackContextMode{
            fge::Object::CallbackContextModes::
                    CONTEXT_DEFAULT}; ///< Tell a scene how the callbackRegister must be called

    //Child objects

    fge::ChildObjectsAccessor _children; ///< An access to child objects of this object
};

} // namespace fge

#endif // _FGE_C_OBJECT_HPP_INCLUDED

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

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_tagList.hpp>
#include <FastEngine/C_networkType.hpp>
#include <FastEngine/C_event.hpp>
#include <FastEngine/C_packet.hpp>
#include <FastEngine/C_guiElement.hpp>
#include <SFML/Graphics.hpp>
#include <json.hpp>

#include <string>
#include <chrono>

#define FGE_OBJ_BADCLASSNAME "NULL"
#define FGE_OBJ_NOSCENE nullptr
#define FGE_OBJ_DEFAULT_COPYMETHOD(objClass) fge::Object* copy() override { return new objClass(*this); }

namespace fge
{

class Scene;

class ObjectData;
using ObjectDataWeak = std::weak_ptr<fge::ObjectData>;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;

/**
 * \class Object
 * \ingroup objectControl
 * \brief The Object class is the base class for all objects in the engine.
 */
class FGE_API Object : public sf::Drawable, public sf::Transformable
{
public:
    Object() = default;
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
     * \param scene_ptr The scene where the object is added (can be nullptr)
     */
    virtual void first(fge::Scene* scene_ptr);
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
     * \param scene_ptr The scene where the object is updated (can be nullptr)
     */
    virtual void update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr);
    /**
     * \brief Method called every frame to draw the object
     *
     * \param target The target where the object is drawn
     * \param states The SFML render states
     */
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    /**
     * \brief Register all network types needed by the object
     */
    virtual void networkRegister();
    /**
     * \brief Method called when the object is removed from a scene
     *
     * \param scene_ptr The scene where the object is removed (can be nullptr)
     */
    virtual void removed(fge::Scene* scene_ptr);

    /**
     * \brief Save the object to a json object
     *
     * \param jsonObject The json object where the object is saved
     * \param scene_ptr The scene where the object is saved (can be nullptr)
     */
    virtual void save(nlohmann::json& jsonObject, fge::Scene* scene_ptr);
    /**
     * \brief Load the object from a json object
     *
     * \param jsonObject The json object where the object is loaded
     * \param scene_ptr The scene where the object is loaded (can be nullptr)
     */
    virtual void load(nlohmann::json& jsonObject, fge::Scene* scene_ptr);
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

    /**
     * \brief Get the unique class name of the object
     *
     * \return The unique class name of the object
     */
    virtual std::string getClassName() const;
    /**
     * \brief Get a readable version of the class name
     *
     * \return A readable version of the class name
     */
    virtual std::string getReadableClassName() const;

    /**
     * \brief Get the global bounds of the object
     *
     * \return The global bounds of the object
     */
    virtual sf::FloatRect getGlobalBounds() const;
    /**
     * \brief Get the local bounds of the object (without any transformations)
     *
     * \return The local bounds of the object
     */
    virtual sf::FloatRect getLocalBounds() const;

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

    //Data

    fge::TagList _tags; ///< The tags of the object

 	//Network

    fge::net::NetworkTypeContainer _netList; ///< The network types container of the object

    //Scene control

    fge::ObjectDataWeak _myObjectData{}; ///< The object data of the object (valid only if the object is in a scene)
    bool _alwaysDrawed{false}; ///< \b true if the object is always drawn, \b false otherwise
};

}//end fge

#endif // _FGE_C_OBJECT_HPP_INCLUDED

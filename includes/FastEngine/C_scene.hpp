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

#ifndef _FGE_C_SCENE_HPP_INCLUDED
#define _FGE_C_SCENE_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <SFML/Graphics.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_propertyList.hpp>
#include <FastEngine/C_commandHandler.hpp>
#include <FastEngine/C_callback.hpp>
#include <FastEngine/C_identity.hpp>
#include <string>
#include <queue>
#include <unordered_map>
#include <memory>

#define FGE_SCENE_PLAN_HIDE_BACK (FGE_SCENE_PLAN_MIDDLE-4)
#define FGE_SCENE_PLAN_BACK (FGE_SCENE_PLAN_MIDDLE-2)
#define FGE_SCENE_PLAN_MIDDLE fge::ObjectPlan{100}
#define FGE_SCENE_PLAN_TOP (FGE_SCENE_PLAN_MIDDLE+2)
#define FGE_SCENE_PLAN_GUI (FGE_SCENE_PLAN_MIDDLE+4)
#define FGE_SCENE_PLAN_HIGH_TOP (FGE_SCENE_PLAN_MIDDLE+6)
#define FGE_SCENE_PLAN_DEFAULT FGE_SCENE_PLAN_MIDDLE

#define FGE_SCENE_BAD_SID std::numeric_limits<fge::ObjectSid>::max()
#define FGE_SCENE_BAD_PLANDEPTH std::numeric_limits<fge::ObjectPlanDepth>::max()
#define FGE_SCENE_BAD_PLAN std::numeric_limits<fge::ObjectPlan>::max()

#define FGE_SCENE_LIMIT_NAMESIZE 200

#define FGE_NEWOBJECT(objectType_, objectArgs_ ...) std::unique_ptr<fge::Object>{new objectType_{objectArgs_}}
#define FGE_NEWOBJECT_PTR(objectPtr_) std::unique_ptr<fge::Object>{objectPtr_}

namespace fge
{

namespace net
{

class ClientList;

}//end net

class Scene;

using ObjectPlanDepth = uint32_t;
using ObjectPlan = uint16_t;
using ObjectSid = uint32_t;
using ObjectPtr = std::unique_ptr<fge::Object>;

struct CallbackContext
{
    fge::Event* _event;
    fge::GuiElementHandler* _guiElementHandler;
};

/**
 * \struct SceneNetEvent
 * \ingroup network
 * \brief Structure that represent an network event related to Scene.
 *
 * This structure is necessary to synchronize events like Object deletion and creation.
 * This is handled via the Scene and you should not really use it.
 */
struct SceneNetEvent
{
    enum Events : uint8_t
    {
        SEVT_DELOBJECT = 0,
        SEVT_NEWOBJECT,

        SEVT_UNKNOWN,

        SEVT_MAX_
    };

    fge::SceneNetEvent::Events _event;
    fge::ObjectSid _sid;
};

/**
 * \enum ObjectType
 * \ingroup objectControl
 * \brief Represent different Object type
 */
enum ObjectType : uint8_t
{
    TYPE_NULL = 0,

    TYPE_OBJECT,
    TYPE_DECAY,
    TYPE_GUI,

    TYPE_MAX_
};

/**
 * \class ObjectData
 * \ingroup objectControl
 * \brief Data wrapper representing an Object in a Scene
 *
 * This is the class used to store data relating to an Object in a Scene.
 * There is :
 * - the linked scene of the object.
 * - the object.
 * - the SID of the object
 * - the plan of the object
 * - the type of the object
 * - the plan depth of the object
 */
class FGE_API ObjectData
{
public:
    ObjectData() :
        g_linkedScene(nullptr),

        g_object(nullptr),
        g_sid(FGE_SCENE_BAD_SID),
        g_plan(FGE_SCENE_PLAN_DEFAULT),
        g_type(fge::ObjectType::TYPE_NULL),

        g_planDepth(FGE_SCENE_BAD_PLANDEPTH)
    {}
    ObjectData(fge::Scene* linkedScene,
               std::unique_ptr<fge::Object>&& newObj,
               fge::ObjectSid newSid=FGE_SCENE_BAD_SID,
               fge::ObjectPlan newPlan=FGE_SCENE_PLAN_DEFAULT,
               fge::ObjectType newType=fge::ObjectType::TYPE_OBJECT) :
        g_linkedScene(linkedScene),

        g_object(std::move(newObj)),
        g_sid(newSid),
        g_plan(newPlan),
        g_type(newType),

        g_planDepth(FGE_SCENE_BAD_PLANDEPTH)
    {}

    /**
     * \brief Release the Object handled by the smart pointer.
     *
     * This method should only be useful if you use a custom Object handler to
     * handle your objects.
     *
     * \warning This is your responsibility to destroy the Object after a call to this method.
     *
     * \return The pointer of the Object
     */
    inline fge::Object* releaseObject()
    {
        return this->g_object.release();
    }

    /**
     * \brief Get the linked Scene.
     *
     * \return The linked Scene pointer or \b nullptr if there is no Scene
     */
    [[nodiscard]] inline fge::Scene* getLinkedScene() const
    {
        return this->g_linkedScene;
    }
    /**
     * \brief Get the Object pointer.
     *
     * \return The Object pointer
     */
    [[nodiscard]] inline fge::Object* getObject() const
    {
        return this->g_object.get();
    }
    /**
     * \brief Get the SID of the Object.
     *
     * An SID (for static identifier) is an unique id for every object in your scene.
     *
     * \return The SID of the object
     */
    [[nodiscard]] inline fge::ObjectSid getSid() const
    {
        return this->g_sid;
    }
    /**
     * \brief Get the plan of the Object.
     *
     * The plan determine the draw order of the Object.
     * If the plan is \b 0, the Object will be drawn first.
     *
     * \return The plan of the Object.
     */
    [[nodiscard]] inline fge::ObjectPlan getPlan() const
    {
        return this->g_plan;
    }
    /**
     * \brief Get the type of the Object.
     *
     * The type determine how the Object is synchronised on the network.
     *
     * - TYPE_OBJECT: a normal object that will be synchronised between client and server
     * - TYPE_DECAY: an object that will be sent from the server but not synchronised
     * - TYPE_GUI: an object that is client only and will be not destroyed by a full synchronisation
     *
     * \return The type of the Object.
     */
    [[nodiscard]] inline fge::ObjectType getType() const
    {
        return this->g_type;
    }

    /**
     * \brief Set the plan depth of the Object.
     *
     * \param depth The new depth
     * \see getPlanDepth
     */
    inline void setPlanDepth(fge::ObjectPlanDepth depth) const
    {
        this->g_planDepth = depth;
    }
    /**
     * \brief Get the plan depth of the Object.
     *
     * A plan depth is an index representing the position of the object
     * in a plan. If the depth is \b 0, the object is drawn before everyone else.
     *
     * The depth plan is generated by the Scene everytime it is drawn.
     *
     * This data is considered as dynamic, this means that it is local only and
     * generated by the Scene and should not be saved.
     *
     * \return The plan depth of the object.
     */
    [[nodiscard]] inline fge::ObjectPlanDepth getPlanDepth() const
    {
        return this->g_planDepth;
    }

    /**
     * \brief Set an parent object
     *
     * \param object The parent object
     */
    inline void setParent(fge::ObjectDataShared& object) const
    {
        if (object)
        {
            this->g_parent = object;
        }
    }
    /**
     * \brief Clear the parent object
     */
    inline void clearParent() const
    {
        this->g_parent.reset();
    }
    /**
     * \brief Get the parent object
     *
     * \return A weak pointer to the parent object
     */
    [[nodiscard]] inline fge::ObjectDataWeak getParent() const
    {
        return this->g_parent;
    }

    /**
     * \brief Check if the Object have an linked Scene.
     *
     * \return True if have a linked Scene, False otherwise
     */
    [[nodiscard]] inline bool isLinked() const
    {
        return this->g_linkedScene != nullptr;
    }

    /**
     * \brief Comparison with another SID
     *
     * \return True if same SID, False otherwise
     */
    inline bool operator ==(const fge::ObjectSid& sid) const
    {
        return this->g_sid == sid;
    }
    /**
     * \brief Comparison with another object pointer
     *
     * \return True if same address, False otherwise
     */
    inline bool operator ==(const fge::Object* ptr) const
    {
        return this->g_object.get() == ptr;
    }

    /**
     * \brief Get the Object pointer.
     *
     * \return The Object pointer
     */
    inline operator fge::Object*() const
    {
        return this->g_object.get();
    }
    /**
     * \brief Get the memory managed pointer of the Object.
     *
     * \return The memory managed pointer of the Object
     */
    inline operator const fge::ObjectPtr&() const
    {
        return this->g_object;
    }
    /**
     * \brief Get the SID of the Object.
     *
     * \return The SID of the object
     * \see getSid
     */
    inline operator const fge::ObjectSid&() const
    {
        return this->g_sid;
    }

    /**
     * \brief check if the provided shared pointer Object is valid.
     *
     * For an shared pointer Object to be considered valid he need
     * to be not \b nullptr and have a linked Scene.
     *
     * \param dataShared The shared pointer Object
     */
    static inline bool isValid(const std::shared_ptr<fge::ObjectData>& dataShared)
    {
        return dataShared && dataShared->isLinked();
    }

private:
    fge::Scene* g_linkedScene;

    fge::ObjectPtr g_object;
    fge::ObjectSid g_sid;
    fge::ObjectPlan g_plan;
    fge::ObjectType g_type;

    //Dynamic data (not saved, local only)
    mutable fge::ObjectPlanDepth g_planDepth;
    mutable fge::ObjectDataWeak g_parent;

    friend fge::Scene;
};

using ObjectDataWeak = std::weak_ptr<fge::ObjectData>;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;
using ObjectContainer = std::list<fge::ObjectDataShared>;
using ObjectDataMap = std::unordered_map<fge::ObjectSid, fge::ObjectContainer::iterator>;
using ObjectPlanDataMap = std::map<fge::ObjectPlan, fge::ObjectContainer::iterator>;

/**
 * \class Scene
 * \ingroup objectControl
 * \brief A scene contain a collection of object and handle them
 *
 * The job of a Scene is to hande a collection of Object and implement some
 * utility method for the user to control them.
 *
 * \see ObjectData
 */
class FGE_API Scene : public fge::CommandHandler
{
public:
    using NetworkEventQueuePerClient = std::unordered_map<fge::net::Identity, std::queue<fge::SceneNetEvent>, fge::net::IdentityHash>;

    Scene();
    explicit Scene(std::string sceneName);
    virtual ~Scene() = default;

    // Scene
    /**
     * \brief Get the name of the Scene.
     *
     * \return the name of the Scene
     */
    inline const std::string& getName() const
    {
        return this->g_name;
    }
    /**
     * \brief Set the name of the Scene.
     *
     * \param name The new name of the scene
     */
    inline void setName(std::string name)
    {
        if (name.size() <= FGE_SCENE_LIMIT_NAMESIZE)
        {
            this->g_name = std::move(name);
        }
    }

    /**
     * \brief Update of the Scene.
     *
     * This method call the Object::update method of every Object in the scene.
     *
     * \warning If the updated Object want to delete itself during an update, it have to use
     * the delUpdatedObject and not any others delete methode that will cause
     * undefined behaviour.
     *
     * \param screen A SFML RenderWindow
     * \param event The FastEngine Event class
     * \param deltaTime The time in milliseconds between two updates
     */
#ifdef FGE_DEF_SERVER
    void update(fge::Event& event, const std::chrono::milliseconds& deltaTime);
#else
    void update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime);
#endif
    /**
     * \brief Draw the Scene.
     *
     * This method call the Object::draw method of every Object in the scene.
     *
     * Before drawing an Object, the Scene check if the global bounds of the Object
     * is in the bounds of the screen and draw it if it's there.
     *
     * This behaviour can be surpassed if the Object::_alwaysDrawed is \b true.
     * \see Object::getGlobalBounds
     *
     * During the draw, the depth plan is re-assigned depending of the Object plan and position in the list.
     * \see ObjectData::getPlanDepth
     *
     * \param target A SFML RenderTarget
     * \param clear_target Set to \b true to let the Scene clear the target
     * \param clear_color If clear_target is set to \b true, this parameter is used to set the clear color
     * \param states The default SFML RenderStates to be used for every drawn Object
     */
#ifndef FGE_DEF_SERVER
    void draw(sf::RenderTarget& target, bool clear_target = true, const sf::Color& clear_color = sf::Color::White, sf::RenderStates states=sf::RenderStates::Default) const;
#endif //FGE_DEF_SERVER

    /**
     * \brief Clear the Scene.
     *
     * This method call delAllObject including GUI Object type and PropertyList::delAllProperties.
     */
    void clear();

    // Object
    /**
     * \brief Add a new Object in the Scene.
     *
     * This method add a created Object in the Scene.
     * The provided SID is passed to the virtual generateSid() method.
     *
     * \warning The provided pointer of the Object, have to be allocated in the \b heap and must not be
     * handled by the user.
     *
     * \warning If there is an error during the addition of the new Object, the returned shared pointer
     * is not valid and the provided Object is dismissed.
     *
     * If the object is created inside the update() method from a second object, the second object become parent.
     *
     * \param newObject The object pointer allocated by the user
     * \param plan The plan of the new object
     * \param sid The wanted SID
     * \param type The type of the new Object
     * \return An shared pointer of the ObjectData
     */
    fge::ObjectDataShared newObject(std::unique_ptr<fge::Object>&& newObject, fge::ObjectPlan plan = FGE_SCENE_PLAN_DEFAULT, fge::ObjectSid sid = FGE_SCENE_BAD_SID, fge::ObjectType type = fge::ObjectType::TYPE_OBJECT);
    /**
     * \brief Add a new Object in the Scene.
     *
     * This method add already handled shared ObjectData in the Scene.
     * The linked Scene is automatically set to the target Scene.
     *
     * \warning The provided ObjectData must have a valid Object pointer.
     *
     * If the object is created inside the update() method from a second object, the second object become parent.
     *
     * \param objectData The shared ObjectData
     * \return An shared pointer of the ObjectData
     */
    fge::ObjectDataShared newObject(const fge::ObjectDataShared& objectData);

    /**
     * \brief Duplicate the provided Object SID.
     *
     * This method duplicate the Object corresponding to the provided SID by giving an new SID
     * to the freshly duplicated Object.
     *
     * This method call the Object::copy method.
     *
     * \param sid The SID of the Object to be duplicated
     * \param newSid The new SID of the duplicated Object
     * \return An shared pointer of the new ObjectData
     */
    fge::ObjectDataShared duplicateObject(fge::ObjectSid sid, fge::ObjectSid newSid = FGE_SCENE_BAD_SID);

    /**
     * \brief Transfer the specified Object to another Scene.
     *
     * The provided Scene must not have another Object with the same SID,
     * causing the method to failed and return an invalid shared pointer.
     *
     * \warning This method is not meant to be used on the same Scene, however
     * it will work as expected.
     *
     * \param sid The SID of the Object to be transferred
     * \param newScene The Scene that will get the Object
     * \return An shared pointer of the transferred Object from the new scene
     */
    fge::ObjectDataShared transferObject(fge::ObjectSid sid, fge::Scene& newScene);

    /**
     * \brief Delete the actual updated Object.
     *
     * This method mark the actual Object to be deleted internally. When the actual Object has
     * finished is update, the update() method correctly delete it.
     *
     * \warning This method is not meant to be used outside of an update and will cause
     * weird behaviour if not respected. (like deleting the first updated Object)
     *
     * \see update
     */
    void delUpdatedObject();
    /**
     * \brief Delete the Object provided with his SID.
     *
     * \warning This method should not be called in the updated Object for deleting itself
     *
     * \see delUpdatedObject
     *
     * \param sid The SID of the Object to be deleted
     * \return \b true if the Object is correctly deleted or \b false if the Object is not found
     */
    bool delObject(fge::ObjectSid sid);
    /**
     * \brief Delete every Object in the Scene.
     *
     * \param ignoreGuiObject If \b true, every GUI type Object is not deleted by this method
     * \return The number of deleted Object
     */
    std::size_t delAllObject(bool ignoreGuiObject);

    /**
     * \brief Set the Object with a new SID.
     *
     * If the new SID is FGE_SCENE_BAD_SID, the method return \b false.
     *
     * \param sid Actual SID of the Object
     * \param newSid The new SID of the Object
     * \return \b true if the change is successful
     */
    bool setObjectSid(fge::ObjectSid sid, fge::ObjectSid newSid);
    /**
     * \brief Set a new Object pointer in place of the provided one.
     *
     * \warning The provided pointer must respect the same rule as a
     * newly created object with the newObject() method
     *
     * \param sid Actual SID of the Object
     * \param newObject The new Object pointer
     * \return \b true if the change is successful
     */
    bool setObject(fge::ObjectSid sid, std::unique_ptr<fge::Object>&& newObject);
    /**
     * \brief Set a new Object plan.
     *
     * \param sid Actual SID of the Object
     * \param newPlan The new plan
     * \return \b true if the change is successful
     */
    bool setObjectPlan(fge::ObjectSid sid, fge::ObjectPlan newPlan);
    /**
     * \brief Set an Object on top of his plan.
     *
     * This make the object be drawn first.
     *
     * \param sid Actual SID of the Object
     * \return \b true if the change is successful
     */
    bool setObjectPlanTop(fge::ObjectSid sid);
    /**
     * \brief Set an Object in the bottom of his plan.
     *
     * This make the object be drawn last.
     *
     * \param sid Actual SID of the Object
     * \return \b true if the change is successful
     */
    bool setObjectPlanBot(fge::ObjectSid sid);

    /**
     * \brief Get an Object with his SID.
     *
     * \param sid Actual SID of the wanted Object
     * \return The shared ObjectData pointer
     */
    fge::ObjectDataShared getObject(fge::ObjectSid sid) const;
    /**
     * \brief Get an Object with his pointer.
     *
     * \param ptr Actual pointer of the wanted Object
     * \return The shared ObjectData pointer
     */
    fge::ObjectDataShared getObject(const fge::Object* ptr) const;
    /**
     * \brief Get an Object pointer with his SID.
     *
     * \param sid Actual SID of the wanted Object
     * \return The Object pointer
     */
    fge::Object* getObjectPtr(fge::ObjectSid sid) const;
    /**
     * \brief Get the actual updated Object.
     *
     * \return The shared ObjectData pointer
     */
    fge::ObjectDataShared getUpdatedObject() const;

    /**
     * \brief Get total Object stored in this Scene.
     *
     * \return The size of the Scene.
     */
    inline std::size_t getObjectSize() const
    {
        return this->g_data.size();
    }

    // Search function
    /**
     * \brief Get all Object with a position.
     *
     * This function check if the global bounds of every Object
     * contain the provided position.
     *
     * \warning This function do not clear data in the ObjectContainer.
     *
     * \param pos The position
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_ByPosition(const sf::Vector2f& pos, fge::ObjectContainer& buff) const;
    /**
     * \brief Get all Object within a zone (or rectangle).
     *
     * This function check if the global bounds of every Object
     * intersect with the provided zone (or rectangle).
     *
     * \warning This function do not clear data in the ObjectContainer.
     *
     * \param zone The zone
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_ByZone(const sf::Rect<float>& zone, fge::ObjectContainer& buff) const;

#ifndef FGE_DEF_SERVER
    /**
     * \brief Get all Object with a local position.
     *
     * This function first convert the local position to global coordinate with
     * the \b custom \b view of the Scene if there is one.
     * Then it check if the global bounds of every Object
     * contain the provided position.
     *
     * \see setCustomView
     *
     * \warning This function do not clear data in the ObjectContainer.
     *
     * \param pos The local position
     * \param target An SFML RenderTarget
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_ByLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target, fge::ObjectContainer& buff) const;
    /**
     * \brief Get all Object within a local zone (or rectangle).
     *
     * This function first convert the local zone to global coordinate with
     * the \b custom \b view of the Scene if there is one.
     * Then it check if the global bounds of every Object
     * intersect with the provided zone (or rectangle).
     *
     * \warning This function do not clear data in the ObjectContainer.
     *
     * \param zone The local zone
     * \param target An SFML RenderTarget
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_ByLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target, fge::ObjectContainer& buff) const;
    /**
     * \brief Get all Object with a local position.
     *
     * Instead of converting the provided local position to global coordinate,
     * this function convert the global bounds of the Object to local coordinate
     * and check if converted local bounds contain the provided position.
     *
     * This function will use the \b custom \b view of the Scene if there is one.
     *
     * \see setCustomView getAllObj_ByLocalPosition
     *
     * \warning This function do not clear data in the ObjectContainer.
     *
     * \param pos The local position
     * \param target An SFML RenderTarget
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_FromLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target, fge::ObjectContainer& buff) const;
    /**
     * \brief Get all Object within a local zone (or rectangle).
     *
     * Instead of converting the provided local zone to global coordinate,
     * this function convert the global bounds of the Object to local coordinate
     * and check if the converted local bounds intersect with the provided zone.
     *
     * This function will use the \b custom \b view of the Scene if there is one.
     *
     * \see setCustomView getAllObj_ByLocalZone
     *
     * \warning This function do not clear data in the ObjectContainer.
     *
     * \param zone The local zone
     * \param target An SFML RenderTarget
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_FromLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target, fge::ObjectContainer& buff) const;
#endif //FGE_DEF_SERVER

    /**
     * \brief Get all Object with the same class name.
     *
     * \see Object::getClassName
     *
     * \warning This function do not clear data in the ObjectContainer.
     *
     * \param class_name The wanted class name
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_ByClass(std::string_view class_name, fge::ObjectContainer& buff) const;
    /**
     * \brief Get all Object that contain the provided tag.
     *
     * \see TagList
     *
     * \warning This function do not clear data in the ObjectContainer.
     *
     * \param tag_name The wanted tag
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_ByTag(std::string_view tag_name, fge::ObjectContainer& buff) const;

    /**
     * \brief Get the first Object with a position.
     *
     * \see getAllObj_ByPosition
     *
     * \param pos The position
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_ByPosition(const sf::Vector2f& pos) const;
    /**
     * \brief Get the first Object within a zone.
     *
     * \see getAllObj_ByZone
     *
     * \param zone The zone
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_ByZone(const sf::Rect<float>& zone) const;

#ifndef FGE_DEF_SERVER
    /**
     * \brief Get the first Object with a local position.
     *
     * \see getAllObj_ByLocalPosition
     *
     * \param pos The local position
     * \param target The SFML RenderTarget
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_ByLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target) const;
    /**
     * \brief Get the first Object within a local zone.
     *
     * \see getAllObj_ByLocalZone
     *
     * \param zone The local zone
     * \param target The SFML RenderTarget
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_ByLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target) const;
    /**
     * \brief Get the first Object with a local position position.
     *
     * \see getAllObj_FromLocalPosition
     *
     * \param pos The position
     * \param target The SFML RenderTarget
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_FromLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target) const;
    /**
     * \brief Get the first Object within a local zone.
     *
     * \see getAllObj_FromLocalZone
     *
     * \param zone The local zone
     * \param target The SFML RenderTarget
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_FromLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target) const;
    /**
     * \brief Get the first Object that match a provided class name.
     *
     * \see getAllObj_ByClass
     *
     * \param class_name The class name
     * \return The first Object that match the argument
     */
#endif //FGE_DEF_SERVER

    fge::ObjectDataShared getFirstObj_ByClass(std::string_view class_name) const;
    /**
     * \brief Get the first Object that match a provided tag.
     *
     * \see getAllObj_ByTag
     *
     * \param tag_name The tag
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_ByTag(std::string_view tag_name) const;

    // Static id
    /**
     * \brief Get the SID of the provided Object pointer.
     *
     * \param ptr The Object pointer
     * \return FGE_SCENE_BAD_SID if there is no match, the Object SID otherwise
     */
    fge::ObjectSid getSid(const fge::Object* ptr) const;

    /**
     * \brief Check if the SID correspond to an Object in this Scene.
     *
     * \param sid The Object SID
     * \return \b true if the SID correspond
     */
    inline bool isValid(fge::ObjectSid sid) const
    {
        return this->find(sid) != this->g_data.cend();
    }

    /**
     * \brief Generate an SID based on the provided wanted SID.
     *
     * By default, if the wanted SID is FGE_SCENE_BAD_SID, this function
     * try to generate one with random value using fge::_random.
     *
     * If the wanted SID is already taken, this will cause an random SID generation.
     *
     * \param wanted_sid The wanted SID
     * \return The SID generated
     */
    virtual fge::ObjectSid generateSid(fge::ObjectSid wanted_sid = FGE_SCENE_BAD_SID) const;

    // Network
    /**
     * \brief Pack all the Scene data in a Packet.
     *
     * This function is useful to do a full Scene synchronisation in a network from a server.
     *
     * \warning The maximum Object that can be packed is fge::net::SizeType.
     *
     * \param pck The network packet
     */
    void pack(fge::net::Packet& pck);
    /**
     * \brief Unpack all the received data of a serverside Scene.
     *
     * This function delete every actual Object in the Scene except GUI Object.
     *
     * \see pack
     *
     * \param pck The network packet
     */
    void unpack(fge::net::Packet& pck);
    /**
     * \brief Pack all modification in a net::Packet for a net::Client with clients checkup.
     *
     * This function do a net::NetworkTypeBase::clientsCheckup and try to see if there
     * is a modification in every network variable from the Object::_netList. This is done
     * per net::Client with the provided net::Identity.
     *
     * If there is a detected modification, the new value is packed in the network net::Packet, with some
     * basic Object information like the SID.
     *
     * The _netList of this Scene is verified too.
     *
     * This allow a partial synchronisation between multiple clients and a server. A partial sync is here
     * to avoid re-sending over and over the same or a bit modified full Scene data. If you have a lots of Object,
     * this can be helpful for Packet size and bandwidth.
     *
     * \see clientsCheckup
     *
     * \warning The maximum Object that can be packed is fge::net::SizeType.
     *
     * \param pck The network packet
     * \param clients The ClientList used for clients checkup
     * \param id The Identity of the client
     */
    void packModification(fge::net::Packet& pck, fge::net::ClientList& clients, const fge::net::Identity& id);
    /**
     * \brief Pack all modification in a net::Packet for a net::Client.
     *
     * This function do the same as packModification but without net::NetworkTypeBase::clientsCheckup
     *
     * \see clientsCheckup
     *
     * \warning The maximum Object that can be packed is fge::net::SizeType.
     *
     * \param pck The network packet
     * \param id The Identity of the client
     */
    void packModification(fge::net::Packet& pck, const fge::net::Identity& id);
    /**
     * \brief Unpack all modification of received data packet from a server.
     *
     * This function only extract Scene partial data, for full Scene sync please see pack and unpack
     *
     * \see packModification
     *
     * \param pck The network packet
     */
    void unpackModification(fge::net::Packet& pck);

    /**
     * \brief Pack object that need an explicit update from the server.
     *
     * This should be called by a client to ask the server for specific network value update.
     *
     * \see NetworkTypeContainer::packNeededUpdate
     *
     * \param pck The network packet
     */
    void packNeededUpdate(fge::net::Packet& pck);
    /**
     * \brief Unpack client object that require an explicit update.
     *
     * This should be called by a server.
     *
     * \see NetworkTypeContainer::packNeededUpdate
     *
     * \param pck The network packet
     * \param id The Identity of the client
     */
    void unpackNeededUpdate(fge::net::Packet& pck, const fge::net::Identity& id);

    /**
     * \brief Do a clients checkup for the Scene::_netList and Object::_netList.
     *
     * A clients checkup is necessary to keep an eye of new/removal client and keep a modification flag
     * for every one of them.
     *
     * Every clients latency will vary a lot, so to keep an eye of what partial Scene modification have to be
     * sent, a clients checkup is required.
     *
     * \see net::NetworkTypeBase::clientsCheckup
     *
     * \param clients The net::ClientList attributed to this Scene
     */
    void clientsCheckup(const fge::net::ClientList& clients);

    /**
     * \brief Force every network type modification flag to be \b true for a specified client net::Identity.
     *
     * When a modification flag is \b true for a net::NetworkType, every data affected will be
     * packed in the next call of packModification
     *
     * \param id The client net::Identity
     */
    void forceCheckClient(const fge::net::Identity& id);
    /**
     * \brief Force every network type modification flag to be \b false for a specified client net::Identity.
     *
     * \see forceCheckClient
     *
     * \param id The client net::Identity
     */
    void forceUncheckClient(const fge::net::Identity& id);

    // SceneNetEvent
    /**
     * \brief Do a clients checkup for Scene related events.
     *
     * \see clientsCheckup
     *
     * \param clients The net::ClientList attributed to this Scene
     */
    void clientsCheckupEvent(const fge::net::ClientList& clients);
    /**
     * \brief Manually push a Scene related event for every clients.
     *
     * \param netEvent The SceneNetEvent
     */
    void pushEvent(const fge::SceneNetEvent& netEvent);
    /**
     * \brief Manually push a Scene related event for a specified client.
     *
     * \param netEvent The SceneNetEvent
     * \param id A client net::Identity
     */
    bool pushEvent(const fge::SceneNetEvent& netEvent, const fge::net::Identity& id);
    /**
     * \brief Start to watch Scene related event or not.
     *
     * When \b true, the Scene will start to push SceneNetEvent for clients. This is
     * \b false by default to avoid events overflow.
     *
     * \warning SceneNetEvent have to be cleared manually by the user with deleteEvents.
     *
     * \param on Start or stop watching Scene related events
     */
    void watchEvent(bool on);
    /**
     * \brief Check if the Scene is currently watching events.
     "
     * \see watchEvent
     *
     * \return Watch Scene events stats
     */
    bool isWatchingEvent() const;

    /**
     * \brief Clear Scene events queue for the specified client.
     *
     * \param id A client net::Identity
     */
    void deleteEvents(const fge::net::Identity& id);
    /**
     * \brief Clear Scene events queue for all clients.
     */
    void deleteEvents();
    /**
     * \brief Remove all clients related data.
     *
     * This function clear all the clients data and queue. After a call to this function,
     * you have to recall clientsCheckupEvent to re-register clients.
     *
     * \warning not to be confounded with deleteEvents that just clear up events queue.
     */
    void clearEvents();

    /**
     * \brief Pack all Scene related events for a specified client.
     *
     * After a call to this function, the event queue is empty.
     *
     * \param pck The network packet
     * \param id The client net::Identity
     */
    void packWatchedEvent(fge::net::Packet& pck, const fge::net::Identity& id);
    /**
     * \brief Unpack all Scene related events from a server.
     *
     * \param pck The network packet
     */
    void unpackWatchedEvent(fge::net::Packet& pck);

    // Operator
    inline fge::ObjectDataShared operator[] (fge::ObjectSid sid) const
    {
        return this->getObject(sid);
    }

    // Custom view
    /**
     * \brief Set a custom shared view.
     *
     * This is useful if you need to draw a Scene in another place in the screen.
     *
     * \param customView The shared pointer of a SFML view
     */
    void setCustomView(std::shared_ptr<sf::View> customView);
    /**
     * \brief Get the custom shared view if there is one.
     *
     * \see setCustomView
     *
     * \return The shared point of the SFML view
     */
    const std::shared_ptr<sf::View>& getCustomView() const;
    /**
     * \brief Remove the actual custom view.
     *
     * \see setCustomView
     */
    void delCustomView();

    // Linked renderTarget
    /**
     * \brief Link a SFML RenderTarget to the Scene.
     *
     * This is useful for any Object that required an RenderTarget in its method.
     *
     * \param target The SFML RenderTarget (can be \b nullptr)
     */
    void setLinkedRenderTarget(sf::RenderTarget* target);
    /**
     * \brief Get the RenderTarget linked to this Scene.
     *
     * \see setLinkedRenderTarget
     *
     * \return The linked RenderTarget or \b nullptr if there is none
     */
    const sf::RenderTarget* getLinkedRenderTarget() const;
    /**
     * \brief Get the RenderTarget linked to this Scene (non-const).
     *
     * \see setLinkedRenderTarget
     *
     * \return The linked RenderTarget or \b nullptr if there is none
     */
    sf::RenderTarget* getLinkedRenderTarget();

    void setCallbackContext(fge::CallbackContext context);
    fge::CallbackContext getCallbackContext() const;

    // Save/Load in file
    /**
     * \brief Save some user defined custom data.
     *
     * This function doesn't do anything by default but can be override to save some
     * data during a saveInFile call.
     *
     * \see saveInFile
     *
     * \param jsonObject The json object
     */
    virtual void saveCustomData(nlohmann::json& jsonObject){};
    /**
     * \brief Load some user defined custom data.
     *
     * This function doesn't do anything by default but can be override to load some
     * data during a loadFromFile call.
     *
     * \see loadFromFile
     *
     * \param jsonObject The json object
     */
    virtual void loadCustomData(nlohmann::json& jsonObject){};

    /**
     * \brief Save all the Scene with its Object in a file.
     *
     * This function save all the data in a json format.
     *
     * \see loadFromFile
     *
     * \param path The path of the file
     * \return \b true if successful, \b false otherwise
     */
    bool saveInFile(const std::string& path);
    /**
     * \brief Load all the Scene data from a json file.
     *
     * This function load all the data from a json format.
     *
     * \warning This function clear everything in the Scene before
     * the file loading.
     *
     * \see saveInFile
     *
     * \param path The path of the file
     * \return \b true if successful, \b false otherwise
     */
    bool loadFromFile(const std::string& path);

    // Iterator
    inline fge::ObjectContainer::const_iterator begin() const
    {
        return this->g_data.begin();
    }
    inline fge::ObjectContainer::const_iterator end() const
    {
        return this->g_data.end();
    }
    inline fge::ObjectContainer::const_reverse_iterator rbegin() const
    {
        return this->g_data.rbegin();
    }
    inline fge::ObjectContainer::const_reverse_iterator rend() const
    {
        return this->g_data.rend();
    }

    /**
     * \brief Find an Object with the specified SID.
     *
     * \param sid The Object SID
     * \return An constant iterator representing the ObjectData
     */
    fge::ObjectContainer::const_iterator find(fge::ObjectSid sid) const;
    /**
     * \brief Find an Object with the specified Object pointer.
     *
     * \param ptr The Object pointer
     * \return An constant iterator representing the ObjectData
     */
    fge::ObjectContainer::const_iterator find(const fge::Object* ptr) const;
    /**
     * \brief Find an Object with the specified plan.
     *
     * \param plan The Object plan
     * \return An constant iterator representing the ObjectData
     */
    fge::ObjectContainer::const_iterator findPlan(fge::ObjectPlan plan) const;

    // Network type
    /**
     * The network list must be used to synchronised data between client
     * and server.
     */
    fge::net::NetworkTypeContainer _netList;

    // Properties
    /**
     * The properties list is a multi-type container for multi-purpose use.
     */
    fge::PropertyList _properties;

    // Event
    mutable fge::CallbackHandler<const fge::Scene*, sf::RenderTarget&, const sf::Color&> _onRenderTargetClear;

    mutable fge::CallbackHandler<fge::Scene*, fge::ObjectDataShared> _onNewObject;
    mutable fge::CallbackHandler<fge::Scene*, fge::ObjectDataShared> _onRemoveObject;

    mutable fge::CallbackHandler<fge::Scene*, fge::ObjectPlan> _onPlanUpdate;

private:
    void refreshPlanDataMap(fge::ObjectPlan plan, fge::ObjectContainer::iterator hintIt, bool isLeaving);
    fge::ObjectContainer::iterator getInsertBeginPositionWithPlan(fge::ObjectPlan plan);

    std::string g_name;

    fge::Scene::NetworkEventQueuePerClient g_networkEvents;
    bool g_enableNetworkEventsFlag;

    std::shared_ptr<sf::View> g_customView;
    sf::RenderTarget* g_linkedRenderTarget;

    bool g_deleteMe; //Delete an object while updating flag
    fge::ObjectContainer::iterator g_updatedObjectIterator; //The iterator of the updated object

    fge::ObjectContainer g_data;
    fge::ObjectDataMap g_dataMap;
    fge::ObjectPlanDataMap g_planDataMap;

    fge::CallbackContext g_callbackContext{nullptr, nullptr};
};

}//end fge

#endif // _FGE_C_SCENE_HPP_INCLUDED
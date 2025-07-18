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

#ifndef _FGE_C_SCENE_HPP_INCLUDED
#define _FGE_C_SCENE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"

#include "FastEngine/C_callback.hpp"
#include "FastEngine/C_commandHandler.hpp"
#include "FastEngine/C_propertyList.hpp"
#include "FastEngine/graphic/C_renderTarget.hpp"
#include "FastEngine/network/C_identity.hpp"
#include "FastEngine/object/C_object.hpp"
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

#define FGE_SCENE_PLAN_HIDE_BACK (FGE_SCENE_PLAN_MIDDLE - 4)
#define FGE_SCENE_PLAN_BACK (FGE_SCENE_PLAN_MIDDLE - 2)
#define FGE_SCENE_PLAN_MIDDLE                                                                                          \
    fge::ObjectPlan                                                                                                    \
    {                                                                                                                  \
        100                                                                                                            \
    }
#define FGE_SCENE_PLAN_TOP (FGE_SCENE_PLAN_MIDDLE + 2)
#define FGE_SCENE_PLAN_GUI (FGE_SCENE_PLAN_MIDDLE + 4)
#define FGE_SCENE_PLAN_HIGH_TOP (FGE_SCENE_PLAN_MIDDLE + 6)
#define FGE_SCENE_PLAN_DEFAULT FGE_SCENE_PLAN_MIDDLE

#define FGE_SCENE_BAD_SID std::numeric_limits<fge::ObjectSid>::max()
#define FGE_SCENE_BAD_PLANDEPTH std::numeric_limits<fge::ObjectPlanDepth>::max()
#define FGE_SCENE_BAD_PLAN std::numeric_limits<fge::ObjectPlan>::max()

#define FGE_SCENE_LIMIT_NAMESIZE 200

#define FGE_NEWOBJECT(objectType_, ...)                                                                                \
    fge::ObjectPtr                                                                                                     \
    {                                                                                                                  \
        new objectType_                                                                                                \
        {                                                                                                              \
            __VA_ARGS__                                                                                                \
        }                                                                                                              \
    }
#define FGE_NEWOBJECT_PTR(objectPtr_)                                                                                  \
    fge::ObjectPtr                                                                                                     \
    {                                                                                                                  \
        objectPtr_                                                                                                     \
    }

namespace fge
{

namespace net
{

class ClientList;

} // namespace net

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
    enum class Events : uint8_t
    {
        OBJECT_DELETED = 0,
        OBJECT_CREATED,
        OBJECT_SIGNALED,

        LAST_ENUM_
    };
    using Events_t = std::underlying_type_t<Events>;

    Events _event;
    ObjectSid _sid;
    int8_t _signal{0};
};

/**
 * \enum ObjectTypes
 * \ingroup objectControl
 * \brief Represent different Object type
 */
enum class ObjectTypes : uint8_t
{
    INVALID = 0,

    OBJECT,
    DECAY,
    GUI,

    _MAX_
};
using ObjectTypes_t = std::underlying_type_t<ObjectTypes>;

static_assert(sizeof(fge::ObjectSid) == sizeof(uint32_t), "fge::ObjectSid must be the same size as uint32_t");
static_assert(static_cast<ObjectTypes_t>(ObjectTypes::_MAX_) <= 4,
              "Too many ObjectTypes, DefaultSIDRanges must change");
enum class DefaultSIDRanges : ObjectSid
{
    MASK_SIZE = 2,
    MASK_POS = 30,
    MASK = ObjectSid{0x3} << MASK_POS,

    POS_NULL = ObjectSid{static_cast<ObjectTypes_t>(ObjectTypes::INVALID)} << MASK_POS,
    POS_OBJECT = ObjectSid{static_cast<ObjectTypes_t>(ObjectTypes::OBJECT)} << MASK_POS,
    POS_DECAY = ObjectSid{static_cast<ObjectTypes_t>(ObjectTypes::DECAY)} << MASK_POS,
    POS_GUI = ObjectSid{static_cast<ObjectTypes_t>(ObjectTypes::GUI)} << MASK_POS
};
using DefaultSIDRanges_t = std::underlying_type_t<DefaultSIDRanges>;

enum ObjectContextFlags : uint32_t
{
    OBJ_CONTEXT_NETWORK = 1 << 0, ///< The object is coming from the network

    OBJ_CONTEXT_NONE = 0,
    OBJ_CONTEXT_DEFAULT = OBJ_CONTEXT_NONE
};

/**
 * \class ObjectData
 * \ingroup objectControl
 * \brief Data wrapper representing an Object in a Scene
 *
 * This is the class used to store data relating to an Object in a Scene.
 * There is :
 * - the bound scene of the object.
 * - the object.
 * - the SID of the object
 * - the plan of the object
 * - the type of the object
 * - the plan depth of the object
 */
class ObjectData
{
public:
    inline ObjectData() :
            g_boundScene(nullptr),

            g_object(nullptr),
            g_sid(FGE_SCENE_BAD_SID),
            g_plan(FGE_SCENE_PLAN_DEFAULT),
            g_type(fge::ObjectTypes::INVALID),

            g_planDepth(FGE_SCENE_BAD_PLANDEPTH),
            g_requireForceClientsCheckup(true)
    {}
    inline ObjectData(fge::Scene* boundScene,
                      fge::ObjectPtr&& newObj,
                      fge::ObjectSid newSid = FGE_SCENE_BAD_SID,
                      fge::ObjectPlan newPlan = FGE_SCENE_PLAN_DEFAULT,
                      fge::ObjectTypes newType = fge::ObjectTypes::OBJECT) :
            g_boundScene(boundScene),

            g_object(std::move(newObj)),
            g_sid(newSid),
            g_plan(newPlan),
            g_type(newType),

            g_planDepth(FGE_SCENE_BAD_PLANDEPTH),
            g_requireForceClientsCheckup(true)
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
    [[nodiscard]] inline fge::Object* releaseObject() { return this->g_object.release(); }

    /**
     * \brief Get the current bound Scene.
     *
     * \return The bound Scene pointer or \b nullptr if there is no Scene
     */
    [[nodiscard]] inline fge::Scene* getScene() const { return this->g_boundScene; }
    /**
     * \brief Get the Object pointer.
     *
     * \return The Object pointer
     */
    [[nodiscard]] inline fge::Object* getObject() const { return this->g_object.get(); }
    /**
     * \brief Get the Object pointer and cast it.
     *
     * \tparam TObject The wanted Object type
     * \return The Object pointer
     */
    template<class TObject>
    [[nodiscard]] inline TObject* getObject() const
    {
        return reinterpret_cast<TObject*>(this->g_object.get());
    }
    /**
     * \brief Get the SID of the Object.
     *
     * An SID (for static identifier) is an unique id for every object in your scene.
     *
     * \return The SID of the object
     */
    [[nodiscard]] inline fge::ObjectSid getSid() const { return this->g_sid; }
    /**
     * \brief Get the plan of the Object.
     *
     * The plan determine the draw order of the Object.
     * If the plan is \b 0, the Object will be drawn first.
     *
     * \return The plan of the Object.
     */
    [[nodiscard]] inline fge::ObjectPlan getPlan() const { return this->g_plan; }
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
    [[nodiscard]] inline fge::ObjectTypes getType() const { return this->g_type; }

    /**
     * \brief Set the plan depth of the Object.
     *
     * \param depth The new depth
     * \see getPlanDepth
     */
    inline void setPlanDepth(fge::ObjectPlanDepth depth) const { this->g_planDepth = depth; }
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
    [[nodiscard]] inline fge::ObjectPlanDepth getPlanDepth() const { return this->g_planDepth; }

    /**
     * \brief Set an parent object
     *
     * \param object The parent object
     */
    inline void setParent(fge::ObjectDataShared const& object) const
    {
        if (object)
        {
            this->g_parent = object;
        }
    }
    /**
     * \brief Clear the parent object
     */
    inline void clearParent() const { this->g_parent.reset(); }
    /**
     * \brief Get the parent object
     *
     * \return A weak pointer to the parent object
     */
    [[nodiscard]] inline fge::ObjectDataWeak getParent() const { return this->g_parent; }

    /**
     * \brief Check if the Object is bound to a Scene.
     *
     * \return \b true if the Object is bound, \b false otherwise
     */
    [[nodiscard]] inline bool isBound() const { return this->g_boundScene != nullptr; }

    /**
     * \brief Check if the Object is a specific class.
     *
     * @param className The class name
     * @return True if the Object is the specified class, False otherwise
     */
    [[nodiscard]] inline bool isClass(std::string_view const className) const
    {
        return this->g_object->getClassName() == className;
    }

    [[nodiscard]] inline fge::EnumFlags<ObjectContextFlags>& getContextFlags() const { return this->g_contextFlags; }

    /**
     * \brief Comparison with another SID
     *
     * \return True if same SID, False otherwise
     */
    [[nodiscard]] inline bool operator==(fge::ObjectSid const& sid) const { return this->g_sid == sid; }
    /**
     * \brief Comparison with another object pointer
     *
     * \return True if same address, False otherwise
     */
    [[nodiscard]] inline bool operator==(fge::Object const* ptr) const { return this->g_object.get() == ptr; }

    /**
     * \brief check if the provided shared pointer Object is valid.
     *
     * For an shared pointer Object to be considered valid he need
     * to not be \b nullptr and have a bound Scene.
     *
     * \param data The shared pointer Object
     * \return \b true if the Object is valid or \b false otherwise
     */
    [[nodiscard]] static inline bool isValid(fge::ObjectDataShared const& data) { return data && data->isBound(); }

    /**
     * \brief check if the provided weak pointer Object is valid.
     *
     * \param data The weak pointer Object
     * \return The shared pointer Object if valid or \b nullptr otherwise
     */
    [[nodiscard]] static inline fge::ObjectDataShared isValid(fge::ObjectDataWeak const& data)
    {
        if (auto dataShared = data.lock())
        {
            return dataShared->isBound() ? dataShared : nullptr;
        }
        return nullptr;
    }

private:
    fge::Scene* g_boundScene;

    fge::ObjectPtr g_object;
    fge::ObjectSid g_sid;
    fge::ObjectPlan g_plan;
    fge::ObjectTypes g_type;

    //Dynamic data (not saved, local only)
    mutable fge::ObjectPlanDepth g_planDepth;
    mutable fge::ObjectDataWeak g_parent;
    mutable fge::EnumFlags<ObjectContextFlags> g_contextFlags; //TODO: make it uneditable ?

    bool g_requireForceClientsCheckup;

    friend class fge::Scene;
};

using ObjectDataWeak = std::weak_ptr<fge::ObjectData>;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;
using ObjectContainer = std::list<fge::ObjectDataShared>;
using ObjectPlanDataMap = std::map<fge::ObjectPlan, fge::ObjectContainer::iterator>;

/**
 * \class ObjectContainerHashMap
 * \ingroup objectControl
 * \brief A hash map to have direct access to an Object in a Scene
 */
class FGE_API ObjectContainerHashMap
{
public:
    using Map = std::unordered_map<ObjectSid, ObjectContainer::iterator>;

    ObjectContainerHashMap() = default;
    explicit ObjectContainerHashMap(ObjectContainer& objects);
    ObjectContainerHashMap(ObjectContainerHashMap const& r) = delete;
    ObjectContainerHashMap(ObjectContainerHashMap&& r) noexcept = default;
    ~ObjectContainerHashMap() = default;

    ObjectContainerHashMap& operator=(ObjectContainerHashMap const& r) = delete;
    ObjectContainerHashMap& operator=(ObjectContainerHashMap&& r) noexcept = default;

    void clear();
    void reMap(ObjectContainer& objects);

    /**
     * \brief Announce a new SID for an Object.
     *
     * If return \b false, the Scene will call the reMap method as
     * something went wrong.
     *
     * \param oldSid The old SID of the Object
     * \param newSid The new SID of the Object
     * \return \b true if the SID is changed, \b false otherwise
     */
    [[nodiscard]] bool newSid(ObjectSid oldSid, ObjectSid newSid);
    /**
     * \brief Announce a new Object in the hash map.
     *
     * If return \b false, the Scene will call the reMap method as
     * something went wrong.
     *
     * \param sid The SID of the Object
     * \param it A valid iterator of the Object from the ObjectContainer
     * \return \b true if the Object is added, \b false otherwise
     */
    [[nodiscard]] bool newObject(ObjectSid sid, ObjectContainer::iterator it);
    /**
     * \brief Announce the deletion of an Object.
     *
     * \param sid The SID of the Object
     */
    void delObject(ObjectSid sid);

    [[nodiscard]] std::optional<ObjectContainer::iterator> find(ObjectSid sid);
    [[nodiscard]] std::optional<ObjectContainer::const_iterator> find(ObjectSid sid) const;
    [[nodiscard]] fge::ObjectContainer::value_type retrieve(ObjectSid sid) const;
    [[nodiscard]] bool contains(ObjectSid sid) const;

    [[nodiscard]] std::size_t size() const;

private:
    Map g_objectMap;
};

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
    using NetworkEventQueue = std::queue<fge::SceneNetEvent>;
    using UpdateCount = uint16_t;
    struct UpdateCountRange
    {
        UpdateCount _last;
        UpdateCount _now;
    };

    enum UpdateFlags : uint32_t
    {
        NONE = 0,
        INCREMENT_UPDATE_COUNT = 1 << 0
    };

    Scene();
    explicit Scene(std::string sceneName);
    Scene(Scene const& r);
    Scene(Scene&& r) noexcept = delete; //TODO: implement move constructor
    virtual ~Scene() = default;

    Scene& operator=(Scene const& r);
    Scene& operator=(Scene&& r) noexcept = delete; //TODO: implement move operator

    // Scene
    /**
     * \brief Get the name of the Scene.
     *
     * \return the name of the Scene
     */
    inline std::string const& getName() const { return this->g_name; }
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
     * \param target A RenderTarget
     * \param event The FastEngine Event class
     * \param deltaTime The time in microseconds between two updates
     * \param flags Some flags to control the update like the update count
     */
#ifdef FGE_DEF_SERVER
    void update(fge::Event& event,
                fge::DeltaTime const& deltaTime,
                std::underlying_type_t<UpdateFlags> flags = UpdateFlags::INCREMENT_UPDATE_COUNT);
#else
    void update(fge::RenderTarget& target,
                fge::Event& event,
                fge::DeltaTime const& deltaTime,
                std::underlying_type_t<UpdateFlags> flags = UpdateFlags::NONE);
#endif
    /**
     * \brief Return the number of update.
     *
     * In order to increment the update count, the UpdateFlags::INCREMENT_UPDATE_COUNT flag must be used
     * while calling the update method.
     *
     * This flag is generally set by the server.
     * 
     * \return The number of update
     */
    [[nodiscard]] uint16_t getUpdateCount() const;

    /**
     * \brief Draw the Scene.
     *
     * This method call the Object::draw method of every Object in the scene.
     *
     * The scene will draw an object according to it's Object::_drawMode :
     * By default, the Scene check if the global bounds of the Object
     * is in the bounds of the screen and draw it if it's there.
     *
     * This behaviour can be surpassed by setting Object::_drawMode.
     * \see Object::getGlobalBounds
     *
     * During the draw, the depth plan is re-assigned depending on the Object plan and position in the list.
     * \see ObjectData::getPlanDepth
     *
     * \param target A RenderTarget
     * \param states The default RenderStates to be used for every drawn Object
     */
#ifndef FGE_DEF_SERVER
    void draw(fge::RenderTarget& target, fge::RenderStates const& states = fge::RenderStates{}) const;
#endif //FGE_DEF_SERVER

    /**
     * \brief update the ObjectPlanDepth for one object
     *
     * \param sid The object sid that will be updated
     * \return The new ObjectPlanDepth
     */
    fge::ObjectPlanDepth updatePlanDepth(fge::ObjectSid sid);
    /**
     * \brief update the ObjectPlanDepth for every objects inside the same plan
     *
     * \param plan The plan
     */
    void updateAllPlanDepth(fge::ObjectPlan plan);
    /**
     * \brief update the ObjectPlanDepth for every objects
     *
     * This method should not really be used as the depth is automatically
     * calculated inside the draw method.
     */
    void updateAllPlanDepth();

    /**
     * \brief Clear the Scene.
     *
     * This method call :
     * - delAllObject including GUI Object type.
     * - PropertyList::delAllProperties.
     * - fge::net::NetworkTypeContainer::clear.
     */
    void clear();

    // Object

    /**
     * \struct NewObjectParameters
     * \brief Parameters for the newObject method
     *
     * - _plan: The plan of the new object
     * - _sid: The wanted SID of the new object
     * - _type: The type of the new Object
     * - _silent: If \b true, the Scene will not call the Object::first or Object::callbackRegister methods
     */
    struct NewObjectParameters
    {
        fge::ObjectPlan _plan{FGE_SCENE_PLAN_DEFAULT};
        fge::ObjectSid _sid{FGE_SCENE_BAD_SID};
        fge::ObjectTypes _type{fge::ObjectTypes::OBJECT};
        bool _silent{false};
        fge::EnumFlags<ObjectContextFlags> _contextFlags{OBJ_CONTEXT_DEFAULT};
    };

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
     * \param silent If \b true, the Scene will not call the Object::first or Object::callbackRegister methods
     * \param contextFlags The context flags of the new Object
     * \return A shared pointer of the ObjectData
     */
    fge::ObjectDataShared newObject(fge::ObjectPtr&& newObject,
                                    fge::ObjectPlan plan = FGE_SCENE_PLAN_DEFAULT,
                                    fge::ObjectSid sid = FGE_SCENE_BAD_SID,
                                    fge::ObjectTypes type = fge::ObjectTypes::OBJECT,
                                    bool silent = false,
                                    fge::EnumFlags<ObjectContextFlags> contextFlags = OBJ_CONTEXT_DEFAULT);

    template<class TObject, class... TArgs>
    inline TObject* newObject(NewObjectParameters const& parameters, TArgs&&... args)
    {
        static_assert(std::is_base_of_v<fge::Object, TObject>, "TObject must be a child of fge::Object");
        auto object = this->newObject(std::make_unique<TObject>(std::forward<TArgs>(args)...), parameters._plan,
                                      parameters._sid, parameters._type, parameters._silent, parameters._contextFlags);
        return object ? object->template getObject<TObject>() : nullptr;
    }
    template<class TObject, class... TArgs>
    inline TObject* newObject(TArgs&&... args)
    {
        static_assert(std::is_base_of_v<fge::Object, TObject>, "TObject must be a child of fge::Object");
        auto object = this->newObject(std::make_unique<TObject>(std::forward<TArgs>(args)...));
        return object ? object->template getObject<TObject>() : nullptr;
    }
    template<class TObject>
    inline TObject* newObject()
    {
        static_assert(std::is_base_of_v<fge::Object, TObject>, "TObject must be a child of fge::Object");
        auto object = this->newObject(std::make_unique<TObject>());
        return object ? object->template getObject<TObject>() : nullptr;
    }
    /**
     * \brief Add a new Object in the Scene.
     *
     * This method add already handled shared ObjectData in the Scene.
     * The bound Scene is automatically set to the target Scene.
     *
     * \warning The provided ObjectData must have a valid Object pointer.
     *
     * If the object is created inside the update() method from a second object, the second object become parent.
     *
     * \param objectData The shared ObjectData
     * \param silent If \b true, the Scene will not call the Object::first or Object::callbackRegister methods
     * \return A shared pointer of the ObjectData
     */
    fge::ObjectDataShared newObject(fge::ObjectDataShared const& objectData, bool silent = false);

    /**
     * \brief Duplicate the provided Object SID.
     *
     * This method duplicate the Object corresponding to the provided SID by giving a new SID
     * to the freshly duplicated Object.
     *
     * This method call the Object::copy method.
     *
     * \param sid The SID of the Object to be duplicated
     * \param newSid The new SID of the duplicated Object
     * \return A shared pointer of the new ObjectData
     */
    fge::ObjectDataShared duplicateObject(fge::ObjectSid sid, fge::ObjectSid newSid = FGE_SCENE_BAD_SID);

    /**
     * \brief Transfer the specified Object to another Scene.
     *
     * The provided Scene must not have another Object with the same SID,
     * causing the method to failed and return an invalid shared pointer.
     *
     * Events/methods are called in this order :
     *    [Object destroyed from old Scene]
     *    _onObjectRemoved called on old Scene
     *    _onPlanUpdate called on old Scene
     *    [Object is added in new Scene]
     *    _onObjectAdded called on new Scene
     *    _onPlanUpdate called on new Scene
     *    Object::transfered() called
     *
     * \warning This method is not meant to be used on the same Scene, however
     * it will work as expected.
     *
     * \param sid The SID of the Object to be transferred
     * \param newScene The Scene that will get the Object
     * \return A shared pointer of the transferred Object from the new scene
     */
    fge::ObjectDataShared transferObject(fge::ObjectSid sid, fge::Scene& newScene);

    /**
     * \brief Delete the actual updated Object.
     *
     * This method mark the actual Object to be deleted internally. When the actual Object has
     * finished is update, the update() method correctly delete it.
     *
     * \warning This method is not meant to be used outside an update and will cause
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
     * If the new SID is FGE_SCENE_BAD_SID, the new SID
     * is automatically generated by the Scene.
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
    bool setObject(fge::ObjectSid sid, fge::ObjectPtr&& newObject);
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
    fge::ObjectDataShared getObject(fge::Object const* ptr) const;
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
    inline std::size_t getObjectSize() const { return this->g_objects.size(); }

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
    std::size_t getAllObj_ByPosition(fge::Vector2f const& pos, fge::ObjectContainer& buff) const;
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
    std::size_t getAllObj_ByZone(fge::RectFloat const& zone, fge::ObjectContainer& buff) const;

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
     * \param target An RenderTarget
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_ByLocalPosition(fge::Vector2i const& pos,
                                          fge::RenderTarget const& target,
                                          fge::ObjectContainer& buff) const;
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
     * \param target An RenderTarget
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t
    getAllObj_ByLocalZone(fge::RectInt const& zone, fge::RenderTarget const& target, fge::ObjectContainer& buff) const;
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
     * \param target An RenderTarget
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_FromLocalPosition(fge::Vector2i const& pos,
                                            fge::RenderTarget const& target,
                                            fge::ObjectContainer& buff) const;
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
     * \param target An RenderTarget
     * \param buff An ObjectContainer that will receive results
     * \return The number of Objects added in the container
     */
    std::size_t getAllObj_FromLocalZone(fge::RectInt const& zone,
                                        fge::RenderTarget const& target,
                                        fge::ObjectContainer& buff) const;
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
    fge::ObjectDataShared getFirstObj_ByPosition(fge::Vector2f const& pos) const;
    /**
     * \brief Get the first Object within a zone.
     *
     * \see getAllObj_ByZone
     *
     * \param zone The zone
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_ByZone(fge::RectFloat const& zone) const;

#ifndef FGE_DEF_SERVER
    /**
     * \brief Get the first Object with a local position.
     *
     * \see getAllObj_ByLocalPosition
     *
     * \param pos The local position
     * \param target The RenderTarget
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_ByLocalPosition(fge::Vector2i const& pos, fge::RenderTarget const& target) const;
    /**
     * \brief Get the first Object within a local zone.
     *
     * \see getAllObj_ByLocalZone
     *
     * \param zone The local zone
     * \param target The RenderTarget
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_ByLocalZone(fge::RectInt const& zone, fge::RenderTarget const& target) const;
    /**
     * \brief Get the first Object with a local position.
     *
     * \see getAllObj_FromLocalPosition
     *
     * \param pos The position
     * \param target The RenderTarget
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_FromLocalPosition(fge::Vector2i const& pos,
                                                        fge::RenderTarget const& target) const;
    /**
     * \brief Get the first Object within a local zone.
     *
     * \see getAllObj_FromLocalZone
     *
     * \param zone The local zone
     * \param target The RenderTarget
     * \return The first Object that match the argument
     */
    fge::ObjectDataShared getFirstObj_FromLocalZone(fge::RectInt const& zone, fge::RenderTarget const& target) const;
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
    fge::ObjectSid getSid(fge::Object const* ptr) const;

    /**
     * \brief Check if the SID correspond to an Object in this Scene.
     *
     * \param sid The Object SID
     * \return \b true if the SID correspond
     */
    inline bool isValid(fge::ObjectSid sid) const { return this->find(sid) != this->g_objects.cend(); }

    /**
     * \brief Generate an SID based on the provided wanted SID.
     *
     * By default, if the wanted SID is FGE_SCENE_BAD_SID, this function
     * try to generate one with random value using fge::_random.
     * The last 2bits in the SID is also here to separate from each of ObjectType.
     *
     * If the wanted SID is already taken, this will cause a random SID generation.
     *
     * \param wanted_sid The wanted SID
     * \param type The type of the Object
     * \return The SID generated
     */
    virtual fge::ObjectSid generateSid(fge::ObjectSid wanted_sid, fge::ObjectTypes type) const;

    // Network
    /**
     * \brief Signal an Object over the network.
     *
     * \param sid The SID of the Object
     * \param signal The signal to send
     */
    void signalObject(fge::ObjectSid sid, int8_t signal);

    /**
     * \brief Pack all the Scene data in a Packet for a net::Client.
     *
     * This function is useful to do a full Scene synchronisation in a network from a server.
     * By providing a net::Identity, the function will initialise/reset some internal sync stats
     * like update count.
     *
     * \b id can be invalid, in this case the Scene will just pack the Scene data without worrying about the client.
     *
     * \param pck The network packet
     * \param id The Identity of the client
     */
    void pack(fge::net::Packet& pck, fge::net::Identity const& id);
    /**
     * \brief Unpack all the received data of a serverside Scene.
     *
     * \see pack unpackModification
     *
     * \param pck The network packet
     * \param clearObjects If \b true, the Scene will clear every Object (except GUI ones) before unpacking
     */
    std::optional<fge::net::Error> unpack(fge::net::Packet const& pck, bool clearObjects = true);
    /**
     * \brief Pack all modification in a net::Packet for a net::Client.
     *
     * The scene check for variable modification, the new value is then packed in the network net::Packet, with some
     * basic Object information like the SID.
     *
     * This allows a partial synchronisation between multiple clients and a server. A partial sync is here
     * to avoid re-sending over and over the same or a bit modified full Scene data. If you have a lots of Object,
     * this can be helpful for Packet size and bandwidth.
     *
     * \see clientsCheckup
     *
     * \warning The maximum Object that can be packed is fge::net::SizeType.
     *
     * \param pck The network packet
     * \param id The Identity of the client
     */
    void packModification(fge::net::Packet& pck, fge::net::Identity const& id);
    /**
     * \brief Unpack all modification of received data packet from a server.
     *
     * This function only extract Scene partial data, for full Scene sync please see pack and unpack.
     *
     * When unpacking, if the updated object is already in the Scene without the fge::OBJ_CONTEXT_NETWORK context flag,
     * it will change it's SID before creating a new object.
     *
     * If the object class id do not match, the Object is destroyed and a new one is created.
     *
     * \see packModification
     *
     * \param pck The network packet
     * \param range The UpdateCountRange from the server
     * \param ignoreUpdateCount If \b true, the Scene will not check the server update count range
     */
    std::optional<fge::net::Error>
    unpackModification(fge::net::Packet const& pck, UpdateCountRange& range, bool ignoreUpdateCount = false);

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
    std::optional<fge::net::Error> unpackNeededUpdate(fge::net::Packet const& pck, fge::net::Identity const& id);

    /**
     * \brief Force every network type modification flag to be \b true for a specified client net::Identity.
     *
     * When a modification flag is \b true for a net::NetworkType, every data affected will be
     * packed in the next call of packModification
     *
     * \param id The client net::Identity
     */
    void forceCheckClient(fge::net::Identity const& id);
    /**
     * \brief Force every network type modification flag to be \b false for a specified client net::Identity.
     *
     * \see forceCheckClient
     *
     * \param id The client net::Identity
     */
    void forceUncheckClient(fge::net::Identity const& id);

    /**
     * \brief Do a clients checkup.
     *
     * A clients checkup is necessary to keep an eye of new/removal client and keep a modification flag
     * for every one of them.
     *
     * Clients latency will vary a lot, so to keep an eye of what partial Scene modification have to be
     * sent, a clients checkup is required.
     *
     * \see net::NetworkTypeBase::clientsCheckup
     *
     * \param clients The net::ClientList attributed to this Scene
     * \param force If \b true, all per clients data will clear and redo a full checkup
     */
    void clientsCheckup(fge::net::ClientList const& clients, bool force = false);

    // SceneNetEvent
    /**
     * \brief Manually push a Scene related event for every clients.
     *
     * \param netEvent The SceneNetEvent
     */
    void pushEvent(fge::SceneNetEvent const& netEvent);
    /**
     * \brief Manually push a Scene related event for a specified client.
     *
     * \param netEvent The SceneNetEvent
     * \param id A client net::Identity
     */
    bool pushEvent(fge::SceneNetEvent const& netEvent, fge::net::Identity const& id);
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
     * \brief Clear Scene network events queue for the specified client.
     *
     * \param id A client net::Identity
     */
    void clearNetEventsQueue(fge::net::Identity const& id);
    /**
     * \brief Clear Scene network events queue for all clients.
     */
    void clearNetEventsQueue();
    /**
     * \brief Remove all network clients related data.
     *
     * After a call to this function, you have to recall
     * clientsCheckupEvent to re-register clients.
     */
    void clearPerClientSyncData();

    /**
     * \brief Pack all Scene related events for a specified client.
     *
     * After a call to this function, the event queue is empty.
     *
     * \param pck The network packet
     * \param id The client net::Identity
     */
    void packWatchedEvent(fge::net::Packet& pck, fge::net::Identity const& id);
    /**
     * \brief Unpack all Scene related events from a server.
     *
     * \param pck The network packet
     */
    std::optional<fge::net::Error> unpackWatchedEvent(fge::net::Packet const& pck);

    // Operator
    inline fge::ObjectDataShared operator[](fge::ObjectSid sid) const { return this->getObject(sid); }

    // Custom view
    /**
     * \brief Set a custom shared view.
     *
     * This is useful if you need to draw a Scene in another place in the screen.
     *
     * \param customView The shared pointer of a view
     */
    void setCustomView(std::shared_ptr<fge::View> customView);
    /**
     * \brief Get the custom shared view if there is one.
     *
     * \see setCustomView
     *
     * \return The shared point of the view
     */
    std::shared_ptr<fge::View> const& getCustomView() const;
    /**
     * \brief Remove the actual custom view.
     *
     * \see setCustomView
     */
    void delCustomView();

    // Linked renderTarget
    /**
     * \brief Link a RenderTarget to the Scene.
     *
     * This is useful for any Object that required an RenderTarget in its method.
     *
     * \param target The RenderTarget (can be \b nullptr)
     */
    void setLinkedRenderTarget(fge::RenderTarget* target);
    /**
     * \brief Get the RenderTarget linked to this Scene.
     *
     * \see setLinkedRenderTarget
     *
     * \return The linked RenderTarget or \b nullptr if there is none
     */
    fge::RenderTarget const* getLinkedRenderTarget() const;
    /**
     * \brief Get the RenderTarget linked to this Scene (non-const).
     *
     * \see setLinkedRenderTarget
     *
     * \return The linked RenderTarget or \b nullptr if there is none
     */
    fge::RenderTarget* getLinkedRenderTarget();

    /**
     * \brief Get the related view of the scene.
     *
     * If the scene have a custom view set, the custom view is returned.
     * Else if the scene have a linked render target, the view of this
     * render target is returned.
     *
     * Else return \b nullptr.
     *
     * \return The related view pointer or \b nullptr if there is none
     */
    [[nodiscard]] fge::View const* getRelatedView() const;

    /**
     * \brief Set the callback context of this Scene.
     *
     * By setting a callback context, the scene will be able to
     * automatically call Object::callbackRegister during object
     * creation.
     *
     * \param context The callback context
     */
    void setCallbackContext(fge::CallbackContext context);
    /**
     * \brief Get the callback context of this Scene.
     *
     * \see setCallbackContext
     *
     * \return The callback context
     */
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
    virtual void saveCustomData([[maybe_unused]] nlohmann::json& jsonObject) {};
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
    virtual void loadCustomData([[maybe_unused]] nlohmann::json& jsonObject) {};

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
    bool saveInFile(std::filesystem::path const& path);
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
    bool loadFromFile(std::filesystem::path const& path);

    // Iterator
    inline fge::ObjectContainer::const_iterator begin() const { return this->g_objects.begin(); }
    inline fge::ObjectContainer::const_iterator end() const { return this->g_objects.end(); }
    inline fge::ObjectContainer::const_reverse_iterator rbegin() const { return this->g_objects.rbegin(); }
    inline fge::ObjectContainer::const_reverse_iterator rend() const { return this->g_objects.rend(); }

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
    fge::ObjectContainer::const_iterator find(fge::Object const* ptr) const;
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
    fge::net::NetworkTypeHandler _netList;

    // Properties
    /**
     * The properties list is a multi-type container for multi-purpose use.
     */
    fge::PropertyList _properties;

    // Event
    /// Event called when the Scene is about to be drawn
    mutable fge::CallbackHandler<fge::Scene const&, fge::RenderTarget&> _onDraw;

    /// Event called when a new Object has been added
    mutable fge::CallbackHandler<fge::Scene&, fge::ObjectDataShared const&> _onObjectAdded;
    /// Event called when an Object has been removed
    mutable fge::CallbackHandler<fge::Scene&, fge::ObjectDataShared const&> _onObjectRemoved;

    /**
     * \brief Event called when a change in the plan is detected
     *
     * When an Object change his plan, is created, is deleted ... this event is called.
     * The ObjectPlan argument can be FGE_SCENE_BAD_PLAN, this mean that all plans have been impacted.
     */
    mutable fge::CallbackHandler<fge::Scene&, fge::ObjectPlan> _onPlanUpdate;

    /**
     * \brief Event called only once after a Scene update
     *
     * You or an Object can register a one time callback that will be called after updating the Scene.
     * Once called, callbacks is cleared.
     */
    mutable fge::CallbackHandler<fge::Scene&> _onDelayedUpdate;

private:
    struct PerClientSync
    {
        inline explicit PerClientSync(UpdateCount updateCount) :
                _lastUpdateCount(updateCount)
        {}

        UpdateCount _lastUpdateCount;
        NetworkEventQueue _networkEvents;
    };
    using PerClientSyncMap = std::unordered_map<fge::net::Identity, PerClientSync, fge::net::IdentityHash>;

    void hash_updatePlanDataMap(fge::ObjectPlan plan, fge::ObjectContainer::iterator whoIterator, bool isLeaving);
    fge::ObjectContainer::iterator hash_getInsertionIteratorFromPlanDataMap(fge::ObjectPlan plan);

    std::string g_name;

    NetworkEventQueue g_sceneNetworkEvents;
    PerClientSyncMap g_perClientSyncs;
    bool g_enableNetworkEventsFlag;

    std::shared_ptr<fge::View> g_customView;
    fge::RenderTarget* g_linkedRenderTarget;

    uint16_t g_updateCount;
    bool g_deleteMe;                                        //Delete an object while updating flag
    fge::ObjectContainer::iterator g_updatedObjectIterator; //The iterator of the updated object

    fge::ObjectContainer g_objects;
    fge::ObjectContainerHashMap g_objectsHashMap;
    fge::ObjectPlanDataMap g_planDataMap;

    fge::CallbackContext g_callbackContext;
};

} // namespace fge

#endif // _FGE_C_SCENE_HPP_INCLUDED
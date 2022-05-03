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

        SEVT_UNKNOWN
    };

    fge::SceneNetEvent::Events _event;
    fge::ObjectSid _sid;
};

enum ObjectType : uint8_t
{
    TYPE_NULL = 0,

    TYPE_OBJECT,
    TYPE_DECAY,
    TYPE_GUI
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
               fge::Object* newObj,
               fge::ObjectSid newSid=FGE_SCENE_BAD_SID,
               fge::ObjectPlan newPlan=FGE_SCENE_PLAN_DEFAULT,
               fge::ObjectType newType=fge::ObjectType::TYPE_OBJECT) :
        g_linkedScene(linkedScene),

        g_object(newObj),
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
     * An SID (for secret identifier) is an unique id for every object in your scene.
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
     * \see getSid()
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

    friend fge::Scene;
};

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

    //Scene

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
        this->g_name = std::move(name);
    }

    /**
     * \brief Update of the Scene.
     *
     * This method call the Object::update() method of every Object in the scene.
     *
     * \warning If the updated Object want to delete itself during an update, it have to use
     * the delUpdatedObject() and not any others delete methode that will cause
     * undefined behaviour.
     *
     * \param screen A SFML RenderWindow
     * \param event The FastEngine Event class
     * \param deltaTime The time in milliseconds between two updates
     */
    void update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime);
    /**
     * \brief Draw the Scene.
     *
     * This method call the Object::draw() method of every Object in the scene.
     *
     * Before drawing an Object, the Scene check if the global bounds of the Object
     * is in the bounds of the screen and draw it if it's there.
     *
     * This behaviour can be surpassed if the Object::_alwaysDrawed is \b true.
     * \see Object::getGlobalBounds()
     *
     * During the draw, the depth plan is re-assigned depending of the Object plan and position in the list.
     * \see ObjectData::getPlanDepth()
     *
     * \param target A SFML RenderTarget
     * \param clear_target Set to \b true to let the Scene clear the target
     * \param clear_color If clear_target is set to \b true, this parameter is used to set the clear color
     * \param states The default SFML RenderStates to be used for every drawn Object
     */
    void draw(sf::RenderTarget& target, bool clear_target = true, const sf::Color& clear_color = sf::Color::White, sf::RenderStates states=sf::RenderStates::Default) const;

    /**
     * \brief Clear the Scene.
     *
     * This method call delAllObject() including GUI Object type and PropertyList::delAllProperties().
     */
    void clear();

    //Object

    /**
     * \brief Add a new Object in the Scene.
     *
     * This method add a created Object in the Scene.
     * The provided SID is passed to the virtual generateSid() method.
     *
     * \warning The provided pointer of the Object, have to be allocated in the \b heap and must not be
     * handled by the user. (The Scene will automatically put the pointer in a smart managed pointer)
     *
     * \warning If there is an error during the addition of the new Object, the returned shared pointer
     * is not valid and the provided Object pointer is dismissed and **not freed by the Scene**.
     *
     * \param newObject The object pointer allocated by the user
     * \param plan The plan of the new object
     * \param sid The wanted SID
     * \param type The type of the new Object
     * \return An shared pointer of the ObjectData
     */
    fge::ObjectDataShared newObject(fge::Object* newObject, fge::ObjectPlan plan = FGE_SCENE_PLAN_DEFAULT, fge::ObjectSid sid = FGE_SCENE_BAD_SID, fge::ObjectType type = fge::ObjectType::TYPE_OBJECT);
    /**
     * \brief Add a new Object in the Scene.
     *
     * This method add already handled shared ObjectData in the Scene.
     * The linked Scene is automatically set to the target Scene.
     *
     * \warning The provided ObjectData must have a valid Object pointer.
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
     * This method call the Object::copy() method.
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
     * \see update()
     */
    void delUpdatedObject();
    /**
     * \brief Delete the Object provided with his SID.
     *
     * \warning This method should not be called in the updated Object for deleting itself
     *
     * \see delUpdatedObject()
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
    bool setObject(fge::ObjectSid sid, fge::Object* newObject);
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

    fge::ObjectDataShared getObject(fge::ObjectSid sid) const;
    fge::ObjectDataShared getObject(const fge::Object* ptr) const;
    fge::Object* getObjectPtr(fge::ObjectSid sid) const;
    fge::ObjectDataShared getUpdatedObject() const;

    inline std::size_t getObjectSize() const
    {
        return this->g_data.size();
    }

    /** Search function **/
    std::size_t getAllObj_ByPosition(const sf::Vector2f& pos, fge::ObjectContainer& buff) const;
    std::size_t getAllObj_ByZone(const sf::Rect<float>& zone, fge::ObjectContainer& buff) const;
    std::size_t getAllObj_ByLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target, fge::ObjectContainer& buff) const;
    std::size_t getAllObj_ByLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target, fge::ObjectContainer& buff) const;
    std::size_t getAllObj_FromLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target, fge::ObjectContainer& buff) const;
    std::size_t getAllObj_FromLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target, fge::ObjectContainer& buff) const;
    std::size_t getAllObj_ByClass(const std::string& class_name, fge::ObjectContainer& buff) const;
    std::size_t getAllObj_ByTag(const std::string& tag_name, fge::ObjectContainer& buff) const;

    fge::ObjectDataShared getFirstObj_ByPosition(const sf::Vector2f& pos) const;
    fge::ObjectDataShared getFirstObj_ByZone(const sf::Rect<float>& zone) const;
    fge::ObjectDataShared getFirstObj_ByLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target) const;
    fge::ObjectDataShared getFirstObj_ByLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target) const;
    fge::ObjectDataShared getFirstObj_FromLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target) const;
    fge::ObjectDataShared getFirstObj_FromLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target) const;
    fge::ObjectDataShared getFirstObj_ByClass(const std::string& class_name) const;
    fge::ObjectDataShared getFirstObj_ByTag(const std::string& tag_name) const;

    /** Static id **/
    fge::ObjectSid getSid(const fge::Object* ptr) const;

    inline bool isValid(fge::ObjectSid sid) const
    {
        return this->find(sid) != this->g_data.cend();
    }

    virtual fge::ObjectSid generateSid(fge::ObjectSid wanted_sid = FGE_SCENE_BAD_SID) const;

    /** Network **/
    void pack(fge::net::Packet& pck);
    void unpack(fge::net::Packet& pck);
    void packModification(fge::net::Packet& pck, fge::net::ClientList& clients, const fge::net::Identity& id);
    void packModification(fge::net::Packet& pck, const fge::net::Identity& id);
    void unpackModification(fge::net::Packet& pck);

    void clientsCheckup(const fge::net::ClientList& clients);

    void forceCheckClient(const fge::net::Identity& id);
    void forceUncheckClient(const fge::net::Identity& id);

    /** SceneNetEvent **/
    void clientsCheckupEvent(const fge::net::ClientList& clients);
    void pushEvent(const fge::SceneNetEvent& netEvent);
    bool pushEvent(const fge::SceneNetEvent& netEvent, const fge::net::Identity& id);
    void watchEvent(bool on);
    bool isWatchingEvent() const;

    void deleteEvents(const fge::net::Identity& id);
    void deleteEvents();
    void clearEvents();

    void packWatchedEvent(fge::net::Packet& pck, const fge::net::Identity& id);
    void unpackWatchedEvent(fge::net::Packet& pck);

    /** Operator **/
    inline fge::ObjectDataShared operator[] (fge::ObjectSid sid) const
    {
        return this->getObject(sid);
    }

    /** Custom view **/
    void setCustomView(std::shared_ptr<sf::View>& customView);
    const std::shared_ptr<sf::View>& getCustomView() const;
    void delCustomView();

    /** Linked renderTarget **/
    void setLinkedRenderTarget(sf::RenderTarget* target);
    const sf::RenderTarget* getLinkedRenderTarget() const;
    sf::RenderTarget* getLinkedRenderTarget();

    /** Save/Load in file **/
    virtual void saveCustomData(nlohmann::json& jsonObject){};
    virtual void loadCustomData(nlohmann::json& jsonObject){};

    bool saveInFile(const std::string& path);
    bool loadFromFile(const std::string& path);

    /** Iterator **/
    inline fge::ObjectContainer::const_iterator begin() const
    {
        return this->g_data.begin();
    }
    inline fge::ObjectContainer::const_iterator end() const
    {
        return this->g_data.end();
    }

    fge::ObjectContainer::const_iterator find(fge::ObjectSid sid) const;
    fge::ObjectContainer::const_iterator find(const fge::Object* ptr) const;
    fge::ObjectContainer::const_iterator findPlan(fge::ObjectPlan plan) const;

    /** Network type **/
    fge::net::NetworkTypeContainer _netList;

    /** Properties **/
    fge::PropertyList _properties;

    /** Event **/
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
};

}//end fge

#endif // _FGE_C_SCENE_HPP_INCLUDED
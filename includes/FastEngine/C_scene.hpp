#ifndef _FGE_C_SCENE_HPP_INCLUDED
#define _FGE_C_SCENE_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <SFML/Graphics.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_valueList.hpp>
#include <FastEngine/C_commandHandler.hpp>
#include <FastEngine/C_callback.hpp>
#include <string>
#include <queue>
#include <unordered_map>
#include <memory>
#include <FastEngine/C_identity.hpp>

#define FGE_SCENE_PLAN_HIDE_BACK fge::ObjectPlan(0)
#define FGE_SCENE_PLAN_BACK fge::ObjectPlan(1)
#define FGE_SCENE_PLAN_MIDDLE fge::ObjectPlan(2)
#define FGE_SCENE_PLAN_TOP fge::ObjectPlan(3)
#define FGE_SCENE_PLAN_HIGH_TOP fge::ObjectPlan(4)
#define FGE_SCENE_PLAN_DEFAULT FGE_SCENE_PLAN_MIDDLE

#define FGE_SCENE_BAD_SID std::numeric_limits<fge::ObjectSid>::max()

namespace fge
{

namespace net
{

class ClientList;

}//end net

class Scene;

using ObjectPlan = uint16_t;
using ObjectSid = uint32_t;
using ObjectPtr = std::unique_ptr<fge::Object>;

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

class FGE_API ObjectData
{
public:
    ObjectData() :
        g_linkedScene(nullptr),

        g_object(nullptr),
        g_sid(FGE_SCENE_BAD_SID),
        g_plan(FGE_SCENE_PLAN_DEFAULT),
        g_type(fge::ObjectType::TYPE_NULL)
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
        g_type(newType)
    {}

    inline fge::Object* releaseObject()
    {
        return this->g_object.release();
    }

    inline fge::Scene* getLinkedScene() const
    {
        return this->g_linkedScene;
    }
    inline fge::Object* getObject() const
    {
        return this->g_object.get();
    }
    inline fge::ObjectSid getSid() const
    {
        return this->g_sid;
    }
    inline fge::ObjectPlan getPlan() const
    {
        return this->g_plan;
    }
    inline fge::ObjectType getType() const
    {
        return this->g_type;
    }

    inline bool isLinked() const
    {
        return this->g_linkedScene != nullptr;
    }

    inline bool operator ==(const fge::ObjectSid& sid) const
    {
        return this->g_sid == sid;
    }
    inline bool operator ==(const fge::Object* ptr) const
    {
        return this->g_object.get() == ptr;
    }

    inline operator fge::Object*() const
    {
        return this->g_object.get();
    }
    inline operator const fge::ObjectPtr&() const
    {
        return this->g_object;
    }
    inline operator const fge::ObjectSid&() const
    {
        return this->g_sid;
    }

    static inline bool isValid(const std::shared_ptr<fge::ObjectData>& dataShared)
    {
        return dataShared ? dataShared->isLinked() : false;
    }

private:
    fge::Scene* g_linkedScene;

    fge::ObjectPtr g_object;
    fge::ObjectSid g_sid;
    fge::ObjectPlan g_plan;
    fge::ObjectType g_type;

    friend fge::Scene;
};

using ObjectDataShared = std::shared_ptr<fge::ObjectData>;
using ObjectContainer = std::list<fge::ObjectDataShared>;
using ObjectDataMap = std::unordered_map<fge::ObjectSid, fge::ObjectContainer::iterator>;

class FGE_API Scene : public fge::CommandHandler
{
public:
    using NetworkEventQueuePerClient = std::unordered_map<fge::net::Identity, std::queue<fge::SceneNetEvent>, fge::net::IdentityHash>;

    Scene();
    Scene(const std::string& scene_name);
    virtual ~Scene() = default;

    /** Scene **/
    inline const std::string& getName() const
    {
        return this->g_name;
    }
    inline void setName(const std::string& name)
    {
        this->g_name = name;
    }

    void update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime);
    void draw(sf::RenderTarget& target, bool clear_target = true, const sf::Color& clear_color = sf::Color::White) const;

    void clear();

    /** Object **/
    fge::ObjectDataShared newObject(fge::Object* newObject, const fge::ObjectPlan& plan = FGE_SCENE_PLAN_DEFAULT, const fge::ObjectSid& sid = FGE_SCENE_BAD_SID, const fge::ObjectType& type = fge::ObjectType::TYPE_OBJECT);
    fge::ObjectDataShared newObject(const fge::ObjectDataShared& objectData);

    fge::ObjectDataShared duplicateObject(const fge::ObjectSid& sid, const fge::ObjectSid& newSid = FGE_SCENE_BAD_SID);

    fge::ObjectDataShared transferObject(const fge::ObjectSid& sid, fge::Scene& newScene);

    void delUpdatedObject();
    bool delObject(const fge::ObjectSid& sid);
    std::size_t delAllObject(bool ignoreGuiObject);

    bool setObjectSid(const fge::ObjectSid& sid, const fge::ObjectSid& newSid);
    bool setObject(const fge::ObjectSid& sid, fge::Object* newObject);
    bool setObjectPlan(const fge::ObjectSid& sid, const fge::ObjectPlan& newPlan);

    fge::ObjectDataShared getObject(const fge::ObjectSid& sid) const;
    fge::ObjectDataShared getObject(const fge::Object* ptr) const;
    fge::Object* getObjectPtr(const fge::ObjectSid& sid) const;
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

    inline bool isValid(const fge::ObjectSid& sid) const
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
    inline fge::ObjectDataShared operator[] (const fge::ObjectSid& sid) const
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
    inline fge::ObjectContainer::iterator begin()
    {
        return this->g_data.begin();
    }
    inline fge::ObjectContainer::iterator end()
    {
        return this->g_data.end();
    }
    inline fge::ObjectContainer::const_iterator cbegin() const
    {
        return this->g_data.cbegin();
    }
    inline fge::ObjectContainer::const_iterator cend() const
    {
        return this->g_data.cend();
    }

    fge::ObjectContainer::const_iterator find(const fge::ObjectSid& sid) const;
    fge::ObjectContainer::iterator find(const fge::ObjectSid& sid);
    fge::ObjectContainer::const_iterator find(const fge::Object* ptr) const;
    fge::ObjectContainer::iterator find(const fge::Object* ptr);

    /** Network type **/
    fge::net::NetworkTypeContainer _netList;

    /** Global Data **/
    fge::ValueList _globalData;

    /** Event **/
    mutable fge::CallbackHandler<const fge::Scene*, sf::RenderTarget&, const sf::Color&> _onRenderTargetClear;

    mutable fge::CallbackHandler<fge::Scene*, fge::ObjectDataShared> _onNewObject;
    mutable fge::CallbackHandler<fge::Scene*, fge::ObjectDataShared> _onRemoveObject;

private:
    std::string g_name;

    fge::Scene::NetworkEventQueuePerClient g_networkEvents;
    bool g_enableNetworkEventsFlag;

    std::shared_ptr<sf::View> g_customView;
    sf::RenderTarget* g_linkedRenderTarget;

    bool g_deleteMe; //Delete an object while updating flag
    fge::ObjectContainer::iterator g_updatedObjectIterator; //The iterator of the updated object

    fge::ObjectContainer g_data;
    fge::ObjectDataMap g_dataMap;
};

}//end fge

#endif // _FGE_C_SCENE_HPP_INCLUDED

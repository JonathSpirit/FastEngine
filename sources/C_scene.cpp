#include "FastEngine/C_scene.hpp"
#include "FastEngine/C_random.hpp"
#include "FastEngine/reg_manager.hpp"
#include "FastEngine/extra_function.hpp"
#include "FastEngine/C_clientList.hpp"

#include <fstream>
#include <iomanip>
#include <memory>

namespace fge
{

///Class Scene
FGE_API Scene::Scene() :
    g_name(),

    g_networkEvents(),
    g_enableNetworkEventsFlag(false),

    g_customView(),
    g_linkedRenderTarget(nullptr),

    g_deleteMe(false),
    g_updatedObjectIterator(),

    g_data(),
    g_dataMap()
{
}
FGE_API Scene::Scene(const std::string& scene_name) :
    g_name(scene_name),

    g_networkEvents(),
    g_enableNetworkEventsFlag(false),

    g_customView(),
    g_linkedRenderTarget(nullptr),

    g_deleteMe(false),
    g_updatedObjectIterator(),

    g_data(),
    g_dataMap()
{
}

/** Scene **/
void FGE_API Scene::update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime)
{
    for ( this->g_updatedObjectIterator=this->g_data.begin(); this->g_updatedObjectIterator!=this->g_data.end(); ++this->g_updatedObjectIterator )
    {
        (*this->g_updatedObjectIterator)->g_object->update(screen, event, deltaTime, this);
        if ( this->g_deleteMe )
        {
            this->g_deleteMe = false;
            if (this->g_enableNetworkEventsFlag)
            {
                this->pushEvent({fge::SceneNetEvent::SEVT_DELOBJECT, (*this->g_updatedObjectIterator)->g_sid});
            }

            (*this->g_updatedObjectIterator)->g_object->removed(this);
            this->_onRemoveObject.call(this, *this->g_updatedObjectIterator);
            (*this->g_updatedObjectIterator)->g_linkedScene = nullptr;
            (*this->g_updatedObjectIterator)->g_object->_myObjectData = nullptr;
            this->g_updatedObjectIterator = --this->g_data.erase(this->g_updatedObjectIterator);
        }
    }
}
void FGE_API Scene::draw(sf::RenderTarget& target, bool clear_target, const sf::Color& clear_color) const
{
    if ( clear_target )
    {
        target.clear( clear_color );
        this->_onRenderTargetClear.call(this, target, clear_color);
    }

    sf::FloatRect screenBounds = fge::GetScreenRect(target);

    sf::View backupView = target.getView();
    if ( this->g_customView )
    {
        target.setView( *this->g_customView );
    }

    for (const auto & data : this->g_data)
    {
        if (!data->g_object->_alwaysDrawed)
        {
            sf::FloatRect objectBounds = data->g_object->getGlobalBounds();
            if (objectBounds.width == 0.0f) ++objectBounds.width;
            if (objectBounds.height == 0.0f) ++objectBounds.height;

            if ( !objectBounds.intersects(screenBounds) )
            {
                continue;
            }
        }

        target.draw( *data->g_object );
    }

    target.setView( backupView );
}

void FGE_API Scene::clear()
{
    this->delAllObject(false);
    this->_globalData.delAllValues();
}

/** Object **/
fge::ObjectDataShared FGE_API Scene::newObject(fge::Object* newObject, const fge::ObjectPlan& plan, const fge::ObjectSid& sid, const fge::ObjectType& type)
{
    fge::ObjectSid thesid = this->generateSid(sid);
    if (thesid == FGE_SCENE_BAD_SID)
    {
        return nullptr;
    }
    if (this->g_enableNetworkEventsFlag)
    {
        this->pushEvent({fge::SceneNetEvent::SEVT_NEWOBJECT, thesid});
    }

    for ( auto it=this->g_data.begin(); it!=this->g_data.end(); ++it )
    {
        if ( plan <= (*it)->g_plan )
        {
            it = this->g_data.insert( it, std::make_shared<fge::ObjectData>(this, newObject, thesid, plan, type) );
            this->g_dataMap[thesid] = it;
            newObject->_myObjectData = *it;
            newObject->first(this);
            this->_onNewObject.call(this, *it);
            return *it;
        }
    }
    auto it = this->g_data.insert( this->g_data.end(), std::make_shared<fge::ObjectData>(this, newObject, thesid, plan, type) );
    this->g_dataMap[thesid] = it;
    newObject->_myObjectData = *it;
    newObject->first(this);
    this->_onNewObject.call(this, *it);
    return *it;
}
fge::ObjectDataShared FGE_API Scene::newObject(const fge::ObjectDataShared& objectData)
{
    fge::ObjectSid thesid = this->generateSid( objectData->g_sid );
    if (thesid == FGE_SCENE_BAD_SID)
    {
        return nullptr;
    }
    if (this->g_enableNetworkEventsFlag)
    {
        this->pushEvent({fge::SceneNetEvent::SEVT_NEWOBJECT, thesid});
    }

    for ( auto it=this->g_data.begin(); it!=this->g_data.end(); ++it )
    {
        if ( objectData->g_plan <= objectData->g_plan )
        {
            it = this->g_data.insert( it, objectData );
            this->g_dataMap[thesid] = it;
            objectData->g_linkedScene = this;
            objectData->g_object->_myObjectData = objectData;
            objectData->g_object->first(this);
            this->_onNewObject.call(this, objectData);
            return objectData;
        }
    }
    auto it = this->g_data.insert(this->g_data.end(), objectData);
    this->g_dataMap[thesid] = it;
    objectData->g_linkedScene = this;
    objectData->g_object->_myObjectData = objectData;
    objectData->g_object->first(this);
    this->_onNewObject.call(this, objectData);
    return objectData;
}

fge::ObjectDataShared FGE_API Scene::duplicateObject(const fge::ObjectSid& sid, const fge::ObjectSid& newSid)
{
    auto it = this->g_dataMap.find(sid);

    if ( it != this->g_dataMap.end() )
    {
        return this->newObject( (*it->second)->g_object->copy(), (*it->second)->g_plan, newSid, (*it->second)->g_type );
    }

    return nullptr;
}

fge::ObjectDataShared FGE_API Scene::transferObject(const fge::ObjectSid& sid, fge::Scene& newScene)
{
    auto it = this->g_dataMap.find(sid);

    if ( it != this->g_dataMap.end() )
    {
        if ( !newScene.isValid(sid) )
        {
            fge::ObjectDataShared buff = std::move(*it->second);
            buff->g_object->removed(this);
            this->_onRemoveObject.call(this, buff);
            this->g_data.erase(it->second);
            this->g_dataMap.erase(it);

            if (this->g_enableNetworkEventsFlag)
            {
                this->pushEvent({fge::SceneNetEvent::SEVT_DELOBJECT, sid});
            }

            return newScene.newObject(buff);
        }
    }
    return nullptr;
}

void FGE_API Scene::delUpdatedObject()
{
    this->g_deleteMe=true;
}
bool FGE_API Scene::delObject(const fge::ObjectSid& sid)
{
    auto it = this->g_dataMap.find(sid);

    if ( it != this->g_dataMap.end() )
    {
        if (this->g_enableNetworkEventsFlag)
        {
            this->pushEvent({fge::SceneNetEvent::SEVT_DELOBJECT, (*it->second)->g_sid});
        }

        (*it->second)->g_object->removed(this);
        this->_onRemoveObject.call(this, *it->second);
        (*it->second)->g_linkedScene = nullptr;
        (*it->second)->g_object->_myObjectData = nullptr;
        this->g_data.erase(it->second);
        this->g_dataMap.erase(it);

        return true;
    }
    return false;
}
std::size_t FGE_API Scene::delAllObject(bool ignoreGuiObject)
{
    if (this->g_enableNetworkEventsFlag)
    {
        this->deleteEvents();
        this->pushEvent({fge::SceneNetEvent::SEVT_DELOBJECT, FGE_SCENE_BAD_SID});
    }

    std::size_t buffSize = this->g_data.size();
    for (auto it=this->g_data.begin(); it != this->g_data.end(); ++it )
    {
        if (ignoreGuiObject)
        {
            if ((*it)->g_type == fge::ObjectType::TYPE_GUI)
            {
                --buffSize;
                continue;
            }
        }
        (*it)->g_object->removed(this);
        this->_onRemoveObject.call(this, *it);
        (*it)->g_linkedScene = nullptr;
        (*it)->g_object->_myObjectData = nullptr;
    }

    this->g_data.clear();
    this->g_dataMap.clear();
    return buffSize;
}

bool FGE_API Scene::setObjectSid(const fge::ObjectSid& sid, const fge::ObjectSid& newSid)
{
    if (newSid == FGE_SCENE_BAD_SID)
    {
        return false;
    }

    auto it = this->g_dataMap.find(newSid);

    if ( it == this->g_dataMap.end() )
    {
        it = this->g_dataMap.find(sid);

        if ( it != this->g_dataMap.end() )
        {
            if (this->g_enableNetworkEventsFlag)
            {
                this->pushEvent({fge::SceneNetEvent::SEVT_DELOBJECT, (*it->second)->g_sid});
                this->pushEvent({fge::SceneNetEvent::SEVT_NEWOBJECT, newSid});
            }

            (*it->second)->g_sid = newSid;
            this->g_dataMap[newSid] = std::move(it->second);
            this->g_dataMap.erase(it);
            return true;
        }
    }
    return false;
}
bool FGE_API Scene::setObject(const fge::ObjectSid& sid, fge::Object* newObject)
{
    auto it = this->g_dataMap.find(sid);

    if ( it != this->g_dataMap.end() )
    {
        if (this->g_enableNetworkEventsFlag)
        {
            this->pushEvent({fge::SceneNetEvent::SEVT_NEWOBJECT, (*it->second)->g_sid});
        }

        (*it->second)->g_object->removed(this);
        (*it->second)->g_linkedScene = nullptr;
        (*it->second)->g_object->_myObjectData = nullptr;

        (*it->second) = std::make_shared<fge::ObjectData>( this, newObject, (*it->second)->g_sid, (*it->second)->g_plan, (*it->second)->g_type );
        (*it->second)->g_object->_myObjectData = *it->second;
        (*it->second)->g_object->first(this);
        return true;
    }
    return false;
}
bool FGE_API Scene::setObjectPlan(const fge::ObjectSid& sid, const fge::ObjectPlan& newPlan)
{
    auto it = this->g_dataMap.find(sid);

    if ( it != this->g_dataMap.end() )
    {
        (*it->second)->g_plan = newPlan;

        for ( auto newPosIt=this->g_data.begin(); newPosIt!=this->g_data.end(); ++newPosIt )
        {
            if ( newPlan <= (*newPosIt)->g_plan )
            {
                this->g_data.splice(newPosIt, this->g_data, it->second);
                return true;
            }
        }

        this->g_data.splice(this->g_data.end(), this->g_data, it->second);
        return true;
    }
    return false;
}
fge::ObjectDataShared FGE_API Scene::getObject(const fge::ObjectSid& sid) const
{
    auto it = this->g_dataMap.find(sid);
    if ( it != this->g_dataMap.cend() )
    {
        return *it->second;
    }
    return nullptr;
}
fge::ObjectDataShared FGE_API Scene::getObject(const fge::Object* ptr) const
{
    auto it = this->find(ptr);
    if ( it != this->g_data.cend() )
    {
        return (*it);
    }
    return nullptr;
}
fge::Object* FGE_API Scene::getObjectPtr(const fge::ObjectSid& sid) const
{
    auto it = this->g_dataMap.find(sid);
    if ( it != this->g_dataMap.cend() )
    {
        return (*it->second)->g_object.get();
    }
    return nullptr;
}
fge::ObjectDataShared FGE_API Scene::getUpdatedObject() const
{
    return *this->g_updatedObjectIterator;
}

/** Global Data **/

/** Search function **/
std::size_t FGE_API Scene::getAllObj_ByPosition(const sf::Vector2f& pos, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (const auto & data : this->g_data)
    {
        sf::FloatRect objBounds = data->g_object->getGlobalBounds();
        if ( objBounds.contains(pos) )
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}
std::size_t FGE_API Scene::getAllObj_ByZone(const sf::Rect<float>& zone, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (const auto & data : this->g_data)
    {
        sf::FloatRect objBounds = data->g_object->getGlobalBounds();
        if ( objBounds.intersects(zone) )
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}
std::size_t FGE_API Scene::getAllObj_ByLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target, fge::ObjectContainer& buff) const
{
    return this->getAllObj_ByPosition( target.mapPixelToCoords(pos, this->g_customView ? *this->g_customView : target.getView()), buff );
}
std::size_t FGE_API Scene::getAllObj_ByLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target, fge::ObjectContainer& buff) const
{
    return this->getAllObj_ByZone( fge::PixelToCoordRect(zone, target, this->g_customView ? *this->g_customView : target.getView()), buff );
}
std::size_t FGE_API Scene::getAllObj_FromLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (const auto & data : this->g_data)
    {
        sf::IntRect objBounds = fge::CoordToPixelRect( data->g_object->getGlobalBounds(), target, this->g_customView ? *this->g_customView : target.getView() );
        if ( objBounds.contains(pos) )
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}
std::size_t FGE_API Scene::getAllObj_FromLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (const auto & data : this->g_data)
    {
        sf::IntRect objBounds = fge::CoordToPixelRect( data->g_object->getGlobalBounds(), target, this->g_customView ? *this->g_customView : target.getView() );
        if ( objBounds.intersects(zone) )
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}
std::size_t FGE_API Scene::getAllObj_ByClass(const std::string& class_name, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (const auto & data : this->g_data)
    {
        if ( data->g_object->getClassName() == class_name )
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}
std::size_t FGE_API Scene::getAllObj_ByTag(const std::string& tag_name, fge::ObjectContainer& buff) const
{
    std::size_t objCount = 0;
    for (const auto & data : this->g_data)
    {
        if ( data->g_object->_tags.check(tag_name) )
        {
            ++objCount;
            buff.push_back(data);
        }
    }
    return objCount;
}

fge::ObjectDataShared FGE_API Scene::getFirstObj_ByPosition(const sf::Vector2f& pos) const
{
    for (const auto & data : this->g_data)
    {
        const fge::ObjectPtr& buffObj = data->g_object;
        sf::FloatRect objBounds = buffObj->getGlobalBounds();
        if ( objBounds.contains(pos) )
        {
            return data;
        }
    }
    return nullptr;
}
fge::ObjectDataShared FGE_API Scene::getFirstObj_ByZone(const sf::Rect<float>& zone) const
{
    for (const auto & data : this->g_data)
    {
        const fge::ObjectPtr& buffObj = data->g_object;
        sf::FloatRect objBounds = buffObj->getGlobalBounds();
        if ( objBounds.intersects(zone) )
        {
            return data;
        }
    }
    return nullptr;
}
fge::ObjectDataShared FGE_API Scene::getFirstObj_ByLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target) const
{
    return this->getFirstObj_ByPosition( target.mapPixelToCoords(pos, this->g_customView ? *this->g_customView : target.getView()) );
}
fge::ObjectDataShared FGE_API Scene::getFirstObj_ByLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target) const
{
    return this->getFirstObj_ByZone( fge::PixelToCoordRect(zone, target, this->g_customView ? *this->g_customView : target.getView()) );
}
fge::ObjectDataShared FGE_API Scene::getFirstObj_FromLocalPosition(const sf::Vector2i& pos, const sf::RenderTarget& target) const
{
    for (const auto & data : this->g_data)
    {
        const fge::ObjectPtr& buffObj = data->g_object;
        sf::IntRect objBounds = fge::CoordToPixelRect( buffObj->getGlobalBounds(), target, this->g_customView ? *this->g_customView : target.getView() );
        if ( objBounds.contains(pos) )
        {
            return data;
        }
    }
    return nullptr;
}
fge::ObjectDataShared FGE_API Scene::getFirstObj_FromLocalZone(const sf::Rect<int>& zone, const sf::RenderTarget& target) const
{
    for (const auto & data : this->g_data)
    {
        const fge::ObjectPtr& buffObj = data->g_object;
        sf::IntRect objBounds = fge::CoordToPixelRect( buffObj->getGlobalBounds(), target, this->g_customView ? *this->g_customView : target.getView() );
        if ( objBounds.intersects(zone) )
        {
            return data;
        }
    }
    return nullptr;
}
fge::ObjectDataShared FGE_API Scene::getFirstObj_ByClass(const std::string& class_name) const
{
    for (const auto & data : this->g_data)
    {
        if ( data->g_object->getClassName() == class_name )
        {
            return data;
        }
    }
    return nullptr;
}
fge::ObjectDataShared FGE_API Scene::getFirstObj_ByTag(const std::string& tag_name) const
{
    for (const auto & data : this->g_data)
    {
        if ( data->g_object->_tags.check(tag_name) )
        {
            return data;
        }
    }
    return nullptr;
}

/** Static id **/
fge::ObjectSid FGE_API Scene::getSid(const fge::Object* ptr) const
{
    for (const auto & data : this->g_data)
    {
        if ( *data == ptr )
        {
            return data->g_sid;
        }
    }
    return FGE_SCENE_BAD_SID;
}

fge::ObjectSid FGE_API Scene::generateSid(fge::ObjectSid wanted_sid) const
{
    if ( wanted_sid != FGE_SCENE_BAD_SID )
    {
        if ( this->g_dataMap.find(wanted_sid) == this->g_dataMap.cend() )
        {
            return wanted_sid;
        }
        else
        {
            return FGE_SCENE_BAD_SID;
        }
    }

    fge::ObjectSid new_sid;
    while ( true )
    {
        new_sid = fge::__random.range<fge::ObjectSid>(0, FGE_SCENE_BAD_SID);

        if ( this->g_dataMap.find(wanted_sid) == this->g_dataMap.cend() )
        {
            return new_sid;
        }
    }
}

/** Network **/
void FGE_API Scene::pack(fge::net::Packet& pck)
{
    //SCENE NAME
    pck << this->g_name;

    //SCENE DATA
    for ( std::size_t a=0; a<this->_netList.size(); ++a )
    {
        this->_netList[a]->packData(pck);
    }

    //OBJECT SIZE
    pck << static_cast<uint32_t>(this->g_data.size());
    for (const auto & data : this->g_data)
    {
        if ( data->g_type == fge::ObjectType::TYPE_GUI )
        {//Ignore GUI
            pck << FGE_SCENE_BAD_SID;
            continue;
        }

        //SID
        pck << data->g_sid;
        //CLASS
        pck << fge::reg::GetClassId( data->g_object.get()->getClassName() );
        //PLAN
        pck << data->g_plan;
        //TYPE
        pck << data->g_type;

        data->g_object.get()->pack(pck);
    }
}
void FGE_API Scene::unpack(fge::net::Packet& pck)
{
    fge::reg::ClassId buff_class;
    fge::ObjectPlan buff_plan{FGE_SCENE_PLAN_DEFAULT};
    fge::ObjectSid buff_sid;
    fge::ObjectType buff_type;

    //SCENE NAME
    pck >> this->g_name;

    //SCENE DATA
    for ( std::size_t a=0; a<this->_netList.size(); ++a )
    {
        this->_netList[a]->applyData(pck);
    }

    //OBJECT SIZE
    this->delAllObject(true);

    uint32_t buff_size(0);
    pck >> buff_size;
    for ( uint32_t i=0; i<buff_size; ++i )
    {
        //SID
        pck >> buff_sid;
        if (buff_sid == FGE_SCENE_BAD_SID)
        {
            continue;
        }
        //CLASS
        pck >> buff_class;
        //PLAN
        pck >> buff_plan;
        //TYPE
        pck >> reinterpret_cast<uint8_t&>(buff_type);

        fge::Object* buff_obj = fge::reg::GetNewClassOf(buff_class);

        this->newObject( buff_obj, buff_plan, buff_sid, buff_type );

        buff_obj->unpack(pck);
    }
}
void FGE_API Scene::packModification(fge::net::Packet& pck, fge::net::ClientList& clients, const fge::net::Identity& id)
{
    //SCENE NAME
    pck << this->g_name;

    //SCENE DATA
    uint32_t countSceneDataModification = 0;
    pck.pack(&countSceneDataModification, sizeof(uint32_t)); //Will be rewrited

    for ( std::size_t i=0; i<this->_netList.size(); ++i )
    {
        fge::net::NetworkTypeBase* netType = this->_netList[i];

        netType->clientsCheckup(clients);
        if ( netType->checkClient(id) )
        {
            ++countSceneDataModification;
            pck << static_cast<uint32_t>(i);
            netType->packData(pck, id);
        }
    }
    pck.pack(0, &countSceneDataModification, sizeof(uint32_t)); //Rewriting size

    //OBJECT SIZE
    uint32_t countObject = 0;

    std::size_t countObjectPos = pck.getDataSize() - 1;
    pck.pack(&countObject, sizeof(uint32_t)); //Will be rewrited

    for (const auto & data : *this)
    {
        fge::net::Packet pckModifiedData;

        //MODIF COUNT/OBJECT DATA
        uint32_t countModification = 0;
        for ( std::size_t i=0; i<data->getObject()->_netList.size(); ++i )
        {
            fge::net::NetworkTypeBase* netType = data->getObject()->_netList[i];

            netType->clientsCheckup(clients);

            if ( netType->checkClient(id) )
            {
                ++countModification;
                pckModifiedData << static_cast<uint32_t>(i);
                netType->packData(pckModifiedData, id);
            }
        }
        if (countModification)
        {
            ++countObject;
            //SID
            pck << data->getSid();
            //CLASS
            pck << fge::reg::GetClassId( data->getObject()->getClassName() );
            //PLAN
            pck << data->getPlan();

            pck << countModification;
            pck.append(pckModifiedData.getData(), pckModifiedData.getDataSize());
        }
    }

    pck.pack(countObjectPos, &countObject, sizeof(uint32_t)); //Rewriting size
    clients.clearClientEvent();
}
void FGE_API Scene::packModification(fge::net::Packet& pck, const fge::net::Identity& id)
{
    //SCENE NAME
    pck << this->g_name;

    //SCENE DATA
    uint32_t countSceneDataModification = 0;
    pck.pack(&countSceneDataModification, sizeof(uint32_t)); //Will be rewrited

    for ( std::size_t i=0; i<this->_netList.size(); ++i )
    {
        fge::net::NetworkTypeBase* netType = this->_netList[i];

        if ( netType->checkClient(id) )
        {
            ++countSceneDataModification;
            pck << static_cast<uint32_t>(i);
            netType->packData(pck, id);
        }
    }
    pck.pack(0, &countSceneDataModification, sizeof(uint32_t)); //Rewriting size

    //OBJECT SIZE
    uint32_t countObject = 0;

    std::size_t countObjectPos = pck.getDataSize() - 1;
    pck.pack(&countObject, sizeof(uint32_t)); //Will be rewrited

    for (const auto & data : *this)
    {
        fge::net::Packet pckModifiedData;

        //MODIF COUNT/OBJECT DATA
        uint32_t countModification = 0;
        for ( std::size_t i=0; i<data->getObject()->_netList.size(); ++i )
        {
            fge::net::NetworkTypeBase* netType = data->getObject()->_netList[i];

            if ( netType->checkClient(id) )
            {
                ++countModification;
                pckModifiedData << static_cast<uint32_t>(i);
                netType->packData(pckModifiedData, id);
            }
        }
        if (countModification)
        {
            ++countObject;
            //SID
            pck << data->getSid();
            //CLASS
            pck << fge::reg::GetClassId( data->getObject()->getClassName() );
            //PLAN
            pck << data->getPlan();

            pck << countModification;
            pck.append(pckModifiedData.getData(), pckModifiedData.getDataSize());
        }
    }

    pck.pack(countObjectPos, &countObject, sizeof(uint32_t)); //Rewriting size
}
void FGE_API Scene::unpackModification(fge::net::Packet& pck)
{
    uint32_t buff_size = 0;

    //SCENE NAME
    pck >> this->g_name;

    //SCENE DATA
    pck >> buff_size;
    for ( uint32_t i=0; i<buff_size; ++i )
    {
        uint32_t buff_index;
        pck >> buff_index;

        this->_netList.at(buff_index)->applyData(pck);
    }

    //OBJECT SIZE
    buff_size = 0;
    pck >> buff_size;
    for ( uint32_t i=0; i<buff_size; ++i )
    {
        fge::reg::ClassId buff_class;
        fge::ObjectPlan buff_plan{FGE_SCENE_PLAN_DEFAULT};
        fge::ObjectSid buff_sid;

        //SID
        pck >> buff_sid;
        //CLASS
        pck >> buff_class;
        //PLAN
        pck >> buff_plan;

        fge::Object* buff_obj = this->getObjectPtr(buff_sid);
        if ( !buff_obj )
        {
            buff_obj = fge::reg::GetNewClassOf( buff_class );
            this->newObject( buff_obj, buff_plan, buff_sid );
        }

        //MODIF COUNT/OBJECT DATA
        uint32_t counterModif=0;
        pck >> counterModif;

        for ( uint32_t a=0; a<counterModif; ++a )
        {
            uint32_t buff_index;
            pck >> buff_index;

            buff_obj->_netList.at(buff_index)->applyData(pck);
        }
    }
}

void FGE_API Scene::clientsCheckup(const fge::net::ClientList& clients)
{
    this->_netList.clientsCheckup(clients);

    for (auto & data : *this)
    {
        data->getObject()->_netList.clientsCheckup(clients);
    }
}

void FGE_API Scene::forceCheckClient(const fge::net::Identity& id)
{
    this->_netList.forceCheckClient(id);

    for (auto & data : *this)
    {
        data->getObject()->_netList.forceCheckClient(id);
    }
}
void FGE_API Scene::forceUncheckClient(const fge::net::Identity& id)
{
    this->_netList.forceUncheckClient(id);

    for (auto & data : *this)
    {
        data->getObject()->_netList.forceUncheckClient(id);
    }
}

/** SceneNetEvent **/
void FGE_API Scene::clientsCheckupEvent(const fge::net::ClientList& clients)
{
    //Remove/Add client
    for (std::size_t i=0; i<clients.getClientEventSize(); ++i)
    {
        const fge::net::ClientListEvent& evt = clients.getClientEvent(i);
        if (evt._event == fge::net::ClientListEvent::CLEVT_DELCLIENT)
        {
            this->g_networkEvents.erase( evt._id );
        }
        else
        {
            this->g_networkEvents[evt._id];
        }
    }
}
void FGE_API Scene::pushEvent(const fge::SceneNetEvent& netEvent)
{
    for (auto & networkEvent : this->g_networkEvents)
    {
        networkEvent.second.push(netEvent);
    }
}
bool FGE_API Scene::pushEvent(const fge::SceneNetEvent& netEvent, const fge::net::Identity& id)
{
    auto it = this->g_networkEvents.find(id);
    if (it != this->g_networkEvents.end())
    {
        it->second.push(netEvent);
        return true;
    }
    return false;
}
void FGE_API Scene::watchEvent(bool on)
{
    if (!on)
    {
        this->clearEvents();
    }
    this->g_enableNetworkEventsFlag = on;
}
bool FGE_API Scene::isWatchingEvent() const
{
    return this->g_enableNetworkEventsFlag;
}

void FGE_API Scene::deleteEvents(const fge::net::Identity& id)
{
    auto it = this->g_networkEvents.find(id);
    if (it != this->g_networkEvents.end())
    {
        std::queue<fge::SceneNetEvent>().swap(it->second);
    }
}
void FGE_API Scene::deleteEvents()
{
    for (auto & networkEvent : this->g_networkEvents)
    {
        std::queue<fge::SceneNetEvent>().swap(networkEvent.second);
    }
}
void FGE_API Scene::clearEvents()
{
    this->g_networkEvents.clear();
}

void FGE_API Scene::packWatchedEvent(fge::net::Packet& pck, const fge::net::Identity& id)
{
    uint32_t counter=0;
    pck.pack(&counter, sizeof(uint32_t));

    auto it = this->g_networkEvents.find(id);
    if (it == this->g_networkEvents.end())
    {
        return;
    }

    std::queue<fge::SceneNetEvent>& events = it->second;

    while ( !events.empty() )
    {
        if ( events.front()._event == fge::SceneNetEvent::SEVT_NEWOBJECT )
        {//New object
            fge::ObjectDataShared data = this->getObject( events.front()._sid );
            if ( data )
            {
                pck << static_cast<uint8_t>(fge::SceneNetEvent::SEVT_NEWOBJECT);
                //SID
                pck << data->g_sid;
                //CLASS
                pck << data->g_object->getClassName();
                //PLAN
                pck << data->g_plan;

                data->g_object->pack(pck);
                ++counter;
            }
        }
        else
        {//Remove object
            pck << static_cast<uint8_t>(fge::SceneNetEvent::SEVT_DELOBJECT);
            //SID
            pck << events.front()._sid;
            ++counter;
        }
        events.pop();
    }

    pck.pack(0, &counter, sizeof(uint32_t));
}
void FGE_API Scene::unpackWatchedEvent(fge::net::Packet& pck)
{
    fge::ObjectSid buff_sid;
    std::string buff_class;
    fge::ObjectPlan buff_plan;

    uint8_t event = fge::SceneNetEvent::SEVT_UNKNOWN;

    uint32_t dataSize = 0;
    pck >> dataSize;
    for (uint32_t i=0; i<dataSize; ++i)
    {
        pck >> event; //Event type

        if (event == fge::SceneNetEvent::SEVT_NEWOBJECT)
        {//New object
            //SID
            pck >> buff_sid;
            //CLASS
            pck >> buff_class;
            //PLAN
            pck >> buff_plan;

            this->delObject(buff_sid);
            fge::Object* theNewObject = fge::reg::GetNewClassOf(buff_class);
            this->newObject(theNewObject, buff_plan, buff_sid);
            theNewObject->unpack(pck);
        }
        else if (event == fge::SceneNetEvent::SEVT_DELOBJECT)
        {//Remove object
            //SID
            pck >> buff_sid;
            if (buff_sid == FGE_SCENE_BAD_SID)
            {
                this->delAllObject(true);
            }
            else
            {
                this->delObject(buff_sid);
            }
        }
    }
}

/** Operator **/

/** Custom view **/
void FGE_API Scene::setCustomView(std::shared_ptr<sf::View>& customView)
{
    this->g_customView = customView;
}
const std::shared_ptr<sf::View>& FGE_API Scene::getCustomView() const
{
    return this->g_customView;
}
void FGE_API Scene::delCustomView()
{
    this->g_customView.reset();
}

/** Linked renderTarget **/
void FGE_API Scene::setLinkedRenderTarget(sf::RenderTarget* target)
{
    this->g_linkedRenderTarget = target;
}
const sf::RenderTarget* FGE_API Scene::getLinkedRenderTarget() const
{
    return this->g_linkedRenderTarget;
}
sf::RenderTarget* FGE_API Scene::getLinkedRenderTarget()
{
    return this->g_linkedRenderTarget;
}

/** Save/Load in file **/

bool FGE_API Scene::saveInFile(const std::string& path)
{
    nlohmann::json outputJson;

    outputJson["SceneInfo"]["name"] = this->getName();

    outputJson["SceneData"] = nlohmann::json::object();
    this->saveCustomData( outputJson["SceneData"] );

    outputJson["Objects"] = nlohmann::json::array();
    for (const auto & data : this->g_data)
    {
        nlohmann::json objNewJson = nlohmann::json::object();
        nlohmann::json& objJson = objNewJson[data->getObject()->getClassName()];

        objJson = nlohmann::json::object();

        objJson["_sid"] = data->getSid();
        objJson["_plan"] = data->getPlan();
        objJson["_type"] = data->getType();

        data->getObject()->save( objJson, this );
        outputJson["Objects"] += objNewJson;
    }

    std::ofstream outFile(path);
    if ( outFile )
    {
        outFile << std::setw(2) << outputJson << std::endl;
        outFile.close();
        return true;
    }
    outFile.close();
    return false;
}
bool FGE_API Scene::loadFromFile(const std::string& path)
{
    std::ifstream inFile(path);
    if ( !inFile )
    {
        inFile.close();
        return false;
    }

    nlohmann::json inputJson;
    inFile >> inputJson;
    inFile.close();

    this->clear();
    this->setName( inputJson["SceneInfo"]["name"].get<std::string>() );

    this->loadCustomData( inputJson["SceneData"] );

    nlohmann::json& jsonObjArray = inputJson["Objects"];
    for (auto & it : jsonObjArray)
    {
        fge::Object* buffObj = fge::reg::GetNewClassOf( it.begin().key() );

        if ( buffObj )
        {
            nlohmann::json& objJson = it.begin().value();

            this->newObject(buffObj, objJson["_plan"].get<fge::ObjectPlan>(),
                                     objJson["_sid"].get<fge::ObjectSid>(),
                                     objJson["_type"].get<fge::ObjectType>() );
            buffObj->load( objJson, this );
        }
        else
        {
            return false;
        }
    }
    return true;
}

/** Iterator **/
fge::ObjectContainer::const_iterator FGE_API Scene::find(const fge::ObjectSid& sid) const
{
    auto it = this->g_dataMap.find(sid);
    return (it != this->g_dataMap.cend()) ? it->second : this->g_data.cend();
}
fge::ObjectContainer::iterator FGE_API Scene::find(const fge::ObjectSid& sid)
{
    auto it = this->g_dataMap.find(sid);
    return (it != this->g_dataMap.end()) ? it->second : this->g_data.end();
}
fge::ObjectContainer::const_iterator FGE_API Scene::find(const fge::Object* ptr) const
{
    for ( auto it=this->g_data.cbegin(); it!=this->g_data.cend(); ++it )
    {
        if ( *(*it) == ptr )
        {
            return it;
        }
    }
    return this->g_data.cend();
}
fge::ObjectContainer::iterator FGE_API Scene::find(const fge::Object* ptr)
{
    for ( auto it=this->g_data.begin(); it!=this->g_data.end(); ++it )
    {
        if ( *(*it) == ptr )
        {
            return it;
        }
    }
    return this->g_data.end();
}

}//end fge

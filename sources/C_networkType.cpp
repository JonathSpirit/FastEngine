#include "FastEngine/C_networkType.hpp"
#include "FastEngine/network_manager.hpp"
#include "FastEngine/C_clientList.hpp"
#include "FastEngine/C_tagList.hpp"

namespace fge
{
namespace net
{

///NetworkTypeBase

bool NetworkTypeBase::clientsCheckup(const fge::net::ClientList& clients)
{
    //Remove/add extra clients
    for (std::size_t i=0; i<clients.getClientEventSize(); ++i)
    {
        const fge::net::ClientListEvent& evt = clients.getClientEvent(i);
        if (evt._event == fge::net::ClientListEvent::CLEVT_DELCLIENT)
        {
            this->_g_tableId.erase( evt._id );
        }
        else
        {
            this->_g_tableId.emplace( evt._id, false );
        }
    }

    //Apply modification flag to clients
    bool buff = this->check();
    if ( buff )
    {
        for (auto & it : this->_g_tableId)
        {
            it.second = true;
        }
        this->forceUncheck();
    }
    return buff;
}
bool NetworkTypeBase::checkClient(const fge::net::Identity& id) const
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.cend())
    {
        return it->second;
    }
    return false;
}
void NetworkTypeBase::forceCheckClient(const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second = true;
    }
}
void NetworkTypeBase::forceUncheckClient(const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second = false;
    }
}

///NetworkTypeScene

NetworkTypeScene::NetworkTypeScene(fge::Scene* source) :
    g_typeSource(source)
{

}

void* NetworkTypeScene::getSource() const
{
    return this->g_typeSource;
}

bool NetworkTypeScene::applyData(fge::net::Packet& pck)
{
    this->g_typeSource->unpackModification(pck);
    this->g_typeSource->unpackWatchedEvent(pck);
    this->_onApplied.call();
    return true;
}
void NetworkTypeScene::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    this->g_typeSource->packModification(pck, id);
    this->g_typeSource->packWatchedEvent(pck, id);
}
void NetworkTypeScene::packData(fge::net::Packet& pck)
{
    this->g_typeSource->pack(pck);
}

bool NetworkTypeScene::clientsCheckup(const fge::net::ClientList& clients)
{
    this->g_typeSource->clientsCheckupEvent(clients);
    this->g_typeSource->clientsCheckup(clients);
    return true;
}

bool NetworkTypeScene::checkClient([[maybe_unused]] const fge::net::Identity& id) const
{
    return true;
}
void NetworkTypeScene::forceCheckClient(const fge::net::Identity& id)
{
    this->g_typeSource->forceCheckClient(id);
}
void NetworkTypeScene::forceUncheckClient(const fge::net::Identity& id)
{
    this->g_typeSource->forceUncheckClient(id);
}

bool NetworkTypeScene::check() const
{
    return true;
}
void NetworkTypeScene::forceCheck()
{
}
void NetworkTypeScene::forceUncheck()
{
}

///NetworkTypeSmoothVec2Float

NetworkTypeSmoothVec2Float::NetworkTypeSmoothVec2Float(fge::net::SmoothVec2Float* source) :
    g_typeSource(source),
    g_typeCopy(source->_real),
    g_force(false)
{
}

void* NetworkTypeSmoothVec2Float::getSource() const
{
    return this->g_typeSource;
}

bool NetworkTypeSmoothVec2Float::applyData(fge::net::Packet& pck)
{
    if ( pck >> this->g_typeCopy )
    {
        this->g_typeSource->_cache = this->g_typeCopy;

        float error = std::abs(this->g_typeSource->_cache.x - this->g_typeSource->_real.x) + std::abs(this->g_typeSource->_cache.y - this->g_typeSource->_real.y);
        if ( error >= this->g_typeSource->_errorRange )
        {//Too much error
            this->g_typeSource->_real = this->g_typeSource->_cache;
            this->_onApplied.call();
            return true;
        }

        //Acceptable error, continuing
        return true;
    }
    return false;
}
void NetworkTypeSmoothVec2Float::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << this->g_typeSource->_real;
        it->second = false;
    }
}
void NetworkTypeSmoothVec2Float::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource->_real;
}

bool NetworkTypeSmoothVec2Float::check() const
{
    return (this->g_typeSource->_real != this->g_typeCopy) || this->g_force;
}
void NetworkTypeSmoothVec2Float::forceCheck()
{
    this->g_force = true;
}
void NetworkTypeSmoothVec2Float::forceUncheck()
{
    this->g_force = false;
    this->g_typeCopy = this->g_typeSource->_real;
}

///NetworkTypeSmoothFloat

NetworkTypeSmoothFloat::NetworkTypeSmoothFloat(fge::net::SmoothFloat* source) :
    g_typeSource(source),
    g_typeCopy(source->_real),
    g_force(false)
{
}

void* NetworkTypeSmoothFloat::getSource() const
{
    return this->g_typeSource;
}

bool NetworkTypeSmoothFloat::applyData(fge::net::Packet& pck)
{
    if ( pck >> this->g_typeCopy )
    {
        this->g_typeSource->_cache = this->g_typeCopy;

        float error = std::abs(this->g_typeSource->_cache - this->g_typeSource->_real);
        if ( error >= this->g_typeSource->_errorRange )
        {//Too much error
            this->g_typeSource->_real = this->g_typeSource->_cache;
            this->_onApplied.call();
            return true;
        }

        //Acceptable error, continuing
        return true;
    }
    return false;
}
void NetworkTypeSmoothFloat::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << this->g_typeSource->_real;
        it->second = false;
    }
}
void NetworkTypeSmoothFloat::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource->_real;
}

bool NetworkTypeSmoothFloat::check() const
{
    return (this->g_typeSource->_real != this->g_typeCopy) || this->g_force;
}
void NetworkTypeSmoothFloat::forceCheck()
{
    this->g_force = true;
}
void NetworkTypeSmoothFloat::forceUncheck()
{
    this->g_force = false;
    this->g_typeCopy = this->g_typeSource->_real;
}

///NetworkTypeTag

NetworkTypeTag::NetworkTypeTag(fge::TagList* source, const std::string& tag) :
    g_typeSource(source),
    g_tag(tag)
{
}

void* NetworkTypeTag::getSource() const
{
    return this->g_typeSource;
}

bool NetworkTypeTag::applyData(fge::net::Packet& pck)
{
    bool flag;
    pck >> flag;

    if (flag)
    {
        this->g_typeSource->add(this->g_tag);
    }
    else
    {
        this->g_typeSource->del(this->g_tag);
    }

    this->_onApplied.call();
    return true;
}
void NetworkTypeTag::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    pck << this->g_typeSource->check(this->g_tag);
}
void NetworkTypeTag::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource->check(this->g_tag);
}

bool NetworkTypeTag::check() const
{
    return true;
}
void NetworkTypeTag::forceCheck()
{
}
void NetworkTypeTag::forceUncheck()
{
}

///NetworkTypeContainer

void NetworkTypeContainer::clear()
{
    this->g_data.clear();
}

void NetworkTypeContainer::push(fge::net::NetworkTypeBase* newNet)
{
    this->g_data.push_back(std::unique_ptr<fge::net::NetworkTypeBase>(newNet));
}

void NetworkTypeContainer::reserve(size_t n)
{
    this->g_data.reserve(n);
}

void NetworkTypeContainer::clientsCheckup(const fge::net::ClientList& clients)
{
    for ( std::size_t i=0; i<this->g_data.size(); ++i )
    {
        this->g_data[i]->clientsCheckup(clients);
    }
}
void NetworkTypeContainer::forceCheckClient(const fge::net::Identity& id)
{
    for ( std::size_t i=0; i<this->g_data.size(); ++i )
    {
        this->g_data[i]->forceCheckClient(id);
    }
}
void NetworkTypeContainer::forceUncheckClient(const fge::net::Identity& id)
{
    for ( std::size_t i=0; i<this->g_data.size(); ++i )
    {
        this->g_data[i]->forceUncheckClient(id);
    }
}

}//end net
}//end fge

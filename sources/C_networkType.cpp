#include "FastEngine/C_networkType.hpp"
#include "FastEngine/network_manager.hpp"
#include "FastEngine/C_clientList.hpp"
#include "FastEngine/C_tagList.hpp"

namespace fge
{
namespace net
{

///NetworkTypeBase

bool FGE_API NetworkTypeBase::clientsCheckup(const fge::net::ClientList& clients)
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
bool FGE_API NetworkTypeBase::checkClient(const fge::net::Identity& id) const
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.cend())
    {
        return it->second;
    }
    return false;
}
void FGE_API NetworkTypeBase::forceCheckClient(const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second = true;
    }
}
void FGE_API NetworkTypeBase::forceUncheckClient(const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second = false;
    }
}

///NetworkTypeScene

FGE_API NetworkTypeScene::NetworkTypeScene(fge::Scene* source) :
    g_typeSource(source)
{

}

void* FGE_API NetworkTypeScene::getSource() const
{
    return this->g_typeSource;
}

bool FGE_API NetworkTypeScene::applyData(fge::net::Packet& pck)
{
    this->g_typeSource->unpackModification(pck);
    this->g_typeSource->unpackWatchedEvent(pck);
    this->_onApplied.call();
    return true;
}
void FGE_API NetworkTypeScene::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    this->g_typeSource->packModification(pck, id);
    this->g_typeSource->packWatchedEvent(pck, id);
}
void FGE_API NetworkTypeScene::packData(fge::net::Packet& pck)
{
    this->g_typeSource->pack(pck);
}

bool FGE_API NetworkTypeScene::clientsCheckup(const fge::net::ClientList& clients)
{
    this->g_typeSource->clientsCheckupEvent(clients);
    this->g_typeSource->clientsCheckup(clients);
    return true;
}

bool FGE_API NetworkTypeScene::checkClient([[maybe_unused]] const fge::net::Identity& id) const
{
    return true;
}
void FGE_API NetworkTypeScene::forceCheckClient(const fge::net::Identity& id)
{
    this->g_typeSource->forceCheckClient(id);
}
void FGE_API NetworkTypeScene::forceUncheckClient(const fge::net::Identity& id)
{
    this->g_typeSource->forceUncheckClient(id);
}

bool FGE_API NetworkTypeScene::check() const
{
    return true;
}
void FGE_API NetworkTypeScene::forceCheck()
{
}
void FGE_API NetworkTypeScene::forceUncheck()
{
}

///NetworkTypeSmoothVec2Float

FGE_API NetworkTypeSmoothVec2Float::NetworkTypeSmoothVec2Float(fge::net::SmoothVec2Float* source) :
    g_typeSource(source),
    g_typeCopy(source->_real),
    g_force(false)
{
}

void* FGE_API NetworkTypeSmoothVec2Float::getSource() const
{
    return this->g_typeSource;
}

bool FGE_API NetworkTypeSmoothVec2Float::applyData(fge::net::Packet& pck)
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
void FGE_API NetworkTypeSmoothVec2Float::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << this->g_typeSource->_real;
        it->second = false;
    }
}
void FGE_API NetworkTypeSmoothVec2Float::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource->_real;
}

bool FGE_API NetworkTypeSmoothVec2Float::check() const
{
    return (this->g_typeSource->_real != this->g_typeCopy) || this->g_force;
}
void FGE_API NetworkTypeSmoothVec2Float::forceCheck()
{
    this->g_force = true;
}
void FGE_API NetworkTypeSmoothVec2Float::forceUncheck()
{
    this->g_force = false;
    this->g_typeCopy = this->g_typeSource->_real;
}

///NetworkTypeSmoothFloat

FGE_API NetworkTypeSmoothFloat::NetworkTypeSmoothFloat(fge::net::SmoothFloat* source) :
    g_typeSource(source),
    g_typeCopy(source->_real),
    g_force(false)
{
}

void* FGE_API NetworkTypeSmoothFloat::getSource() const
{
    return this->g_typeSource;
}

bool FGE_API NetworkTypeSmoothFloat::applyData(fge::net::Packet& pck)
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
void FGE_API NetworkTypeSmoothFloat::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << this->g_typeSource->_real;
        it->second = false;
    }
}
void FGE_API NetworkTypeSmoothFloat::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource->_real;
}

bool FGE_API NetworkTypeSmoothFloat::check() const
{
    return (this->g_typeSource->_real != this->g_typeCopy) || this->g_force;
}
void FGE_API NetworkTypeSmoothFloat::forceCheck()
{
    this->g_force = true;
}
void FGE_API NetworkTypeSmoothFloat::forceUncheck()
{
    this->g_force = false;
    this->g_typeCopy = this->g_typeSource->_real;
}

///NetworkTypeTag

FGE_API NetworkTypeTag::NetworkTypeTag(fge::TagList* source, const std::string& tag) :
    g_typeSource(source),
    g_tag(tag)
{
}

void* FGE_API NetworkTypeTag::getSource() const
{
    return this->g_typeSource;
}

bool FGE_API NetworkTypeTag::applyData(fge::net::Packet& pck)
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
void FGE_API NetworkTypeTag::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    pck << this->g_typeSource->check(this->g_tag);
}
void FGE_API NetworkTypeTag::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource->check(this->g_tag);
}

bool FGE_API NetworkTypeTag::check() const
{
    return true;
}
void FGE_API NetworkTypeTag::forceCheck()
{
}
void FGE_API NetworkTypeTag::forceUncheck()
{
}

///NetworkTypeContainer

void FGE_API NetworkTypeContainer::clear()
{
    this->g_data.clear();
}

void FGE_API NetworkTypeContainer::push(fge::net::NetworkTypeBase* newNet)
{
    this->g_data.push_back(std::unique_ptr<fge::net::NetworkTypeBase>(newNet));
}

void FGE_API NetworkTypeContainer::reserve(size_t n)
{
    this->g_data.reserve(n);
}

void FGE_API NetworkTypeContainer::clientsCheckup(const fge::net::ClientList& clients)
{
    for ( std::size_t i=0; i<this->g_data.size(); ++i )
    {
        this->g_data[i]->clientsCheckup(clients);
    }
}
void FGE_API NetworkTypeContainer::forceCheckClient(const fge::net::Identity& id)
{
    for ( std::size_t i=0; i<this->g_data.size(); ++i )
    {
        this->g_data[i]->forceCheckClient(id);
    }
}
void FGE_API NetworkTypeContainer::forceUncheckClient(const fge::net::Identity& id)
{
    for ( std::size_t i=0; i<this->g_data.size(); ++i )
    {
        this->g_data[i]->forceUncheckClient(id);
    }
}

}//end net
}//end fge

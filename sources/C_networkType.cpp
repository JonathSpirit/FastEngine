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

#include "FastEngine/C_networkType.hpp"
#include "FastEngine/network_manager.hpp"
#include "FastEngine/C_clientList.hpp"
#include "FastEngine/C_tagList.hpp"

namespace fge::net
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
            this->_g_tableId.emplace( evt._id, 0 );
        }
    }

    //Apply modification flag to clients
    bool buff = this->check();
    if ( buff )
    {
        for (auto & it : this->_g_tableId)
        {
            it.second |= fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
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
        return (it->second & fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK) > 0;
    }
    return false;
}
void NetworkTypeBase::forceCheckClient(const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second |= fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
    }
}
void NetworkTypeBase::forceUncheckClient(const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second |= fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
    }
}
void NetworkTypeBase::requireExplicitUpdateClient(const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second |= fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_EXPLICIT_UPDATE;
    }
}

bool NetworkTypeBase::isForced() const
{
    return this->_g_force;
}

void NetworkTypeBase::clearNeedUpdateFlag()
{
    this->_g_needUpdate = false;
}
void NetworkTypeBase::needUpdate()
{
    this->_g_needUpdate = true;
}
bool NetworkTypeBase::isNeedingUpdate() const
{
    return this->_g_needUpdate;
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

///NetworkTypeSmoothVec2FloatSetter

NetworkTypeSmoothVec2FloatSetter::NetworkTypeSmoothVec2FloatSetter(const sf::Vector2f* source, std::function<void(const sf::Vector2f&)> setter, float errorRange) :
        g_typeSource(source),
        g_typeCopy(*source),
        g_setter(std::move(setter)),
        g_errorRange(errorRange)
{
}

void* NetworkTypeSmoothVec2FloatSetter::getSource() const
{
    ///TODO: getSource() should return a const void*
    return nullptr;
}

bool NetworkTypeSmoothVec2FloatSetter::applyData(fge::net::Packet& pck)
{
    if ( pck >> this->g_typeCopy )
    {
        float error = std::abs(this->g_typeCopy.x - this->g_typeSource->x) + std::abs(this->g_typeCopy.y - this->g_typeSource->y);
        if ( error >= this->g_errorRange )
        {//Too much error
            this->g_setter(this->g_typeCopy);
            this->_onApplied.call();
            return true;
        }

        //Acceptable error, continuing
        return true;
    }
    return false;
}
void NetworkTypeSmoothVec2FloatSetter::packData(fge::net::Packet& pck, const fge::net::Identity& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << *this->g_typeSource;
        it->second &=~ fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
    }
}
void NetworkTypeSmoothVec2FloatSetter::packData(fge::net::Packet& pck)
{
    pck << *this->g_typeSource;
}

bool NetworkTypeSmoothVec2FloatSetter::check() const
{
    return (*this->g_typeSource != this->g_typeCopy) || this->_g_force;
}
void NetworkTypeSmoothVec2FloatSetter::forceCheck()
{
    this->_g_force = true;
}
void NetworkTypeSmoothVec2FloatSetter::forceUncheck()
{
    this->_g_force = false;
    this->g_typeCopy = *this->g_typeSource;
}

const sf::Vector2f& NetworkTypeSmoothVec2FloatSetter::getCache() const
{
    return this->g_typeCopy;
}
void NetworkTypeSmoothVec2FloatSetter::setErrorRange(float range)
{
    this->g_errorRange = range;
}
float NetworkTypeSmoothVec2FloatSetter::getErrorRange() const
{
    return this->g_errorRange;
}

///NetworkTypeSmoothVec2Float

NetworkTypeSmoothVec2Float::NetworkTypeSmoothVec2Float(fge::net::SmoothVec2Float* source) :
    g_typeSource(source),
    g_typeCopy(source->_real)
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
        it->second &=~ fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
    }
}
void NetworkTypeSmoothVec2Float::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource->_real;
}

bool NetworkTypeSmoothVec2Float::check() const
{
    return (this->g_typeSource->_real != this->g_typeCopy) || this->_g_force;
}
void NetworkTypeSmoothVec2Float::forceCheck()
{
    this->_g_force = true;
}
void NetworkTypeSmoothVec2Float::forceUncheck()
{
    this->_g_force = false;
    this->g_typeCopy = this->g_typeSource->_real;
}

///NetworkTypeSmoothFloat

NetworkTypeSmoothFloat::NetworkTypeSmoothFloat(fge::net::SmoothFloat* source) :
    g_typeSource(source),
    g_typeCopy(source->_real)
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
        it->second &=~ fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
    }
}
void NetworkTypeSmoothFloat::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource->_real;
}

bool NetworkTypeSmoothFloat::check() const
{
    return (this->g_typeSource->_real != this->g_typeCopy) || this->_g_force;
}
void NetworkTypeSmoothFloat::forceCheck()
{
    this->_g_force = true;
}
void NetworkTypeSmoothFloat::forceUncheck()
{
    this->_g_force = false;
    this->g_typeCopy = this->g_typeSource->_real;
}

///NetworkTypeTag

NetworkTypeTag::NetworkTypeTag(fge::TagList* source, std::string tag) :
    g_typeSource(source),
    g_tag(std::move(tag))
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

std::size_t NetworkTypeContainer::packNeededUpdate(fge::net::Packet& pck)
{
    std::size_t rewritePos = pck.getDataSize();
    pck.append( sizeof(fge::net::SizeType) ); //Will be rewrited

    fge::net::SizeType count{0};
    for ( std::size_t i=0; i<this->g_data.size(); ++i )
    {
        if ( this->g_data[i]->isNeedingUpdate() )
        {
            pck << static_cast<fge::net::SizeType>(i);
            ++count;
            this->g_data[i]->clearNeedUpdateFlag();
        }
    }

    pck.pack(rewritePos, &count, sizeof(fge::net::SizeType));

    return count;
}
void NetworkTypeContainer::unpackNeededUpdate(fge::net::Packet& pck, const fge::net::Identity& id)
{
    fge::net::SizeType count{0};
    pck >> count;

    for ( fge::net::SizeType i=0; i<count; ++i )
    {
        fge::net::SizeType dataIndex{0};

        auto* net = this->g_data.at(dataIndex).get();
        net->forceCheckClient(id);
        net->requireExplicitUpdateClient(id);
    }
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

}//end fge::net

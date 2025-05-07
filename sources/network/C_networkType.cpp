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

#include "FastEngine/network/C_networkType.hpp"
#include "FastEngine/C_tagList.hpp"
#include "FastEngine/manager/network_manager.hpp"
#include "FastEngine/network/C_clientList.hpp"

namespace fge::net
{

///NetworkTypeBase

bool NetworkTypeBase::clientsCheckup(fge::net::ClientList const& clients, bool force)
{
    if (force)
    { //Clear and redo the table by ClientList
        for (auto& it: this->_g_tableId)
        {
            this->destroyClientCustomData(it.second._customData);
        }
        this->_g_tableId.clear();
        this->_g_tableId.reserve(clients.getSize());

        auto lock = clients.acquireLock();
        for (auto it = clients.begin(lock); it != clients.end(lock); ++it)
        {
            this->createClientCustomData(
                    this->_g_tableId.emplace(it->first, PerClientConfig{}).first->second._customData);
        }
    }
    else
    { //Remove/add extra clients by ClientList events
        for (std::size_t i = 0; i < clients.getClientEventSize(); ++i)
        {
            auto const& evt = clients.getClientEvent(i);
            if (evt._event == fge::net::ClientListEvent::CLEVT_DELCLIENT)
            {
                auto it = this->_g_tableId.find(evt._id);
                if (it != this->_g_tableId.end())
                {
                    this->destroyClientCustomData(it->second._customData);
                    this->_g_tableId.erase(it);
                }
            }
            else
            {
                this->createClientCustomData(
                        this->_g_tableId.emplace(evt._id, PerClientConfig{}).first->second._customData);
            }
        }
    }

    //Apply modification flag to clients
    bool buff = this->check();
    if (buff)
    {
        for (auto& it: this->_g_tableId)
        {
            it.second._config |= fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
            this->applyClientCustomData(it.second._customData);
        }
        this->forceUncheck();
    }
    return buff;
}
bool NetworkTypeBase::checkClient(fge::net::Identity const& id) const
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.cend())
    {
        return (it->second._config & fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG) > 0;
    }
    return false;
}
void NetworkTypeBase::forceCheckClient(fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second._config |= fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
    }
}
void NetworkTypeBase::forceUncheckClient(fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second._config &= ~fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
    }
}
void NetworkTypeBase::requireExplicitUpdateClient(fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        it->second._config |= fge::net::PerClientConfigs::CLIENTCONFIG_REQUIRE_EXPLICIT_UPDATE_FLAG;
    }
}

bool NetworkTypeBase::isForced() const
{
    return this->_g_force;
}

void NetworkTypeBase::clearExplicitUpdateFlag()
{
    this->_g_needExplicitUpdate = false;
}
void NetworkTypeBase::needExplicitUpdate()
{
    this->_g_needExplicitUpdate = true;
}
bool NetworkTypeBase::isNeedingExplicitUpdate() const
{
    return this->_g_needExplicitUpdate;
}

void NetworkTypeBase::clearWaitingUpdateFlag()
{
    this->_g_waitingUpdate = false;
}
void NetworkTypeBase::waitingUpdate()
{
    if (!this->_g_waitingUpdate)
    {
        this->setLastUpdateTime();
        this->_g_waitingUpdate = true;
    }
}
bool NetworkTypeBase::isWaitingUpdate() const
{
    return this->_g_waitingUpdate;
}

std::chrono::microseconds NetworkTypeBase::getLastUpdateTime() const
{
    return this->_g_lastUpdateTime;
}
void NetworkTypeBase::setLastUpdateTime()
{
    this->_g_lastUpdateTime = std::chrono::duration_cast<decltype(this->_g_lastUpdateTime)>(
            std::chrono::steady_clock::now().time_since_epoch());
}

///NetworkTypeScene

NetworkTypeScene::NetworkTypeScene(fge::Scene* source) :
        g_typeSource(source)
{}

void const* NetworkTypeScene::getSource() const
{
    return this->g_typeSource;
}

bool NetworkTypeScene::applyData(fge::net::Packet const& pck)
{
    Scene::UpdateCountRange updateRange{};
    auto err = this->g_typeSource->unpackModification(pck, updateRange);
    if (err)
    {
        return false; //TODO : handle error
    }
    this->g_typeSource->unpackWatchedEvent(pck);
    this->setLastUpdateTime();
    this->clearWaitingUpdateFlag();
    this->_onApplied.call();
    return true;
}
void NetworkTypeScene::packData(fge::net::Packet& pck, fge::net::Identity const& id)
{
    this->g_typeSource->packModification(pck, id);
    this->g_typeSource->packWatchedEvent(pck, id);
}
void NetworkTypeScene::packData(fge::net::Packet& pck)
{
    this->g_typeSource->pack(pck);
}

bool NetworkTypeScene::clientsCheckup(fge::net::ClientList const& clients, bool force)
{
    this->g_typeSource->clientsCheckup(clients, force);
    return true;
}

bool NetworkTypeScene::checkClient([[maybe_unused]] fge::net::Identity const& id) const
{
    return true;
}
void NetworkTypeScene::forceCheckClient(fge::net::Identity const& id)
{
    this->g_typeSource->forceCheckClient(id);
}
void NetworkTypeScene::forceUncheckClient(fge::net::Identity const& id)
{
    this->g_typeSource->forceUncheckClient(id);
}

bool NetworkTypeScene::check() const
{
    return true;
}
void NetworkTypeScene::forceCheck() {}
void NetworkTypeScene::forceUncheck() {}

///NetworkTypeSmoothVec2Float

NetworkTypeSmoothVec2Float::NetworkTypeSmoothVec2Float(fge::DataAccessor<fge::Vector2f> source, float errorRange) :
        g_typeCopy(source._getter()),
        g_typeSource(std::move(source)),
        g_errorRange(errorRange)
{}

void const* NetworkTypeSmoothVec2Float::getSource() const
{
    return nullptr;
}

bool NetworkTypeSmoothVec2Float::applyData(fge::net::Packet const& pck)
{
    if (pck >> this->g_typeCopy)
    {
        fge::Vector2f source = this->g_typeSource._getter();
        float error = std::abs(this->g_typeCopy.x - source.x) + std::abs(this->g_typeCopy.y - source.y);
        if (error >= this->g_errorRange)
        { //Too much error
            this->g_typeSource._setter(this->g_typeCopy);
            this->setLastUpdateTime();
            this->clearWaitingUpdateFlag();
            this->_onApplied.call();
            return true;
        }

        //Acceptable error, continuing
        return true;
    }
    return false;
}
void NetworkTypeSmoothVec2Float::packData(fge::net::Packet& pck, fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << this->g_typeSource._getter();
        it->second._config &= ~fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
    }
}
void NetworkTypeSmoothVec2Float::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource._getter();
}

bool NetworkTypeSmoothVec2Float::check() const
{
    return (this->g_typeSource._getter() != this->g_typeCopy) || this->_g_force;
}
void NetworkTypeSmoothVec2Float::forceCheck()
{
    this->_g_force = true;
}
void NetworkTypeSmoothVec2Float::forceUncheck()
{
    this->_g_force = false;
    this->g_typeCopy = this->g_typeSource._getter();
}

fge::Vector2f const& NetworkTypeSmoothVec2Float::getCache() const
{
    return this->g_typeCopy;
}
void NetworkTypeSmoothVec2Float::setErrorRange(float range)
{
    this->g_errorRange = range;
}
float NetworkTypeSmoothVec2Float::getErrorRange() const
{
    return this->g_errorRange;
}

///NetworkTypeSmoothFloat

NetworkTypeSmoothFloat::NetworkTypeSmoothFloat(fge::DataAccessor<float> source, float errorRange) :
        g_typeCopy(source._getter()),
        g_typeSource(std::move(source)),
        g_errorRange(errorRange)
{}

void const* NetworkTypeSmoothFloat::getSource() const
{
    return nullptr;
}

bool NetworkTypeSmoothFloat::applyData(fge::net::Packet const& pck)
{
    if (pck >> this->g_typeCopy)
    {
        float error = std::abs(this->g_typeCopy - this->g_typeSource._getter());
        if (error >= this->g_errorRange)
        { //Too much error
            this->g_typeSource._setter(this->g_typeCopy);
            this->setLastUpdateTime();
            this->clearWaitingUpdateFlag();
            this->_onApplied.call();
            return true;
        }

        //Acceptable error, continuing
        return true;
    }
    return false;
}
void NetworkTypeSmoothFloat::packData(fge::net::Packet& pck, fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        pck << this->g_typeSource._getter();
        it->second._config &= ~fge::net::PerClientConfigs::CLIENTCONFIG_MODIFIED_FLAG;
    }
}
void NetworkTypeSmoothFloat::packData(fge::net::Packet& pck)
{
    pck << this->g_typeSource._getter();
}

bool NetworkTypeSmoothFloat::check() const
{
    return (this->g_typeSource._getter() != this->g_typeCopy) || this->_g_force;
}
void NetworkTypeSmoothFloat::forceCheck()
{
    this->_g_force = true;
}
void NetworkTypeSmoothFloat::forceUncheck()
{
    this->_g_force = false;
    this->g_typeCopy = this->g_typeSource._getter();
}

float NetworkTypeSmoothFloat::getCache() const
{
    return this->g_typeCopy;
}
void NetworkTypeSmoothFloat::setErrorRange(float range)
{
    this->g_errorRange = range;
}
float NetworkTypeSmoothFloat::getErrorRange() const
{
    return this->g_errorRange;
}

///NetworkTypeTag

NetworkTypeTag::NetworkTypeTag(fge::TagList* source, std::string tag) :
        g_typeSource(source),
        g_tag(std::move(tag))
{}

void const* NetworkTypeTag::getSource() const
{
    return this->g_typeSource;
}

bool NetworkTypeTag::applyData(fge::net::Packet const& pck)
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

    this->setLastUpdateTime();
    this->clearWaitingUpdateFlag();
    this->_onApplied.call();
    return true;
}
void NetworkTypeTag::packData(fge::net::Packet& pck, [[maybe_unused]] fge::net::Identity const& id)
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
void NetworkTypeTag::forceCheck() {}
void NetworkTypeTag::forceUncheck() {}

///NetworkTypeContainer

void NetworkTypeHandler::clear()
{
    this->g_data.clear();
}

fge::net::NetworkTypeBase* NetworkTypeHandler::push(std::unique_ptr<fge::net::NetworkTypeBase>&& newNet)
{
    return this->g_data.emplace_back(std::move(newNet)).get();
}

std::size_t NetworkTypeHandler::packNeededUpdate(fge::net::Packet& pck) const
{
    auto const rewritePos = pck.getDataSize();
    pck.append(sizeof(fge::net::SizeType)); //Will be rewrited

    fge::net::SizeType count{0};
    for (std::size_t i = 0; i < this->g_data.size(); ++i)
    {
        auto* net = this->g_data[i].get();

        if (net->isNeedingExplicitUpdate())
        {
            pck << static_cast<fge::net::SizeType>(i);
            ++count;
            net->clearExplicitUpdateFlag();
            net->clearWaitingUpdateFlag();
            continue;
        }

        auto const lastUpdateTime = std::chrono::steady_clock::now().time_since_epoch() - net->getLastUpdateTime();
        if (net->isWaitingUpdate() && lastUpdateTime >= FGE_NET_WAITING_UPDATE_DELAY)
        {
            pck << static_cast<fge::net::SizeType>(i);
            ++count;
            net->clearWaitingUpdateFlag();
        }
    }

    pck.pack(rewritePos, &count, sizeof(fge::net::SizeType));

    return count;
}
void NetworkTypeHandler::unpackNeededUpdate(fge::net::Packet const& pck, fge::net::Identity const& id) const
{
    ///TODO : need safe data extraction with net rules
    fge::net::SizeType count{0};
    pck >> count;

    for (fge::net::SizeType i = 0; i < count; ++i)
    {
        fge::net::SizeType dataIndex{std::numeric_limits<fge::net::SizeType>::max()};
        pck >> dataIndex;

        if (dataIndex < this->g_data.size())
        {
            auto* net = this->g_data[dataIndex].get();
            net->forceCheckClient(id);
            net->requireExplicitUpdateClient(id);
        }
        else
        {
            return;
        }
    }
}

void NetworkTypeHandler::clientsCheckup(fge::net::ClientList const& clients, bool force) const
{
    for (auto const& netType: this->g_data)
    {
        netType->clientsCheckup(clients, force);
    }
}
void NetworkTypeHandler::forceCheckClient(fge::net::Identity const& id) const
{
    for (auto const& netType: this->g_data)
    {
        netType->forceCheckClient(id);
    }
}
void NetworkTypeHandler::forceUncheckClient(fge::net::Identity const& id) const
{
    for (auto const& netType: this->g_data)
    {
        netType->forceUncheckClient(id);
    }
}

} // namespace fge::net

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

//PerClientSyncContext

void PerClientSyncContext::setModificationFlag()
{
    for (auto& pair: this->g_syncTable)
    {
        pair.second._config.set(CLIENTCONFIG_MODIFIED_FLAG);
        this->applyClientData(pair.second._data);
    }
}
bool PerClientSyncContext::setModificationFlag(Identity const& client)
{
    auto it = this->g_syncTable.find(client);
    if (it != this->g_syncTable.end())
    {
        it->second._config.set(CLIENTCONFIG_MODIFIED_FLAG);
        this->applyClientData(it->second._data);
        return true;
    }
    return false;
}
bool PerClientSyncContext::clearModificationFlag(Identity const& client)
{
    auto it = this->g_syncTable.find(client);
    if (it != this->g_syncTable.end())
    {
        it->second._config.unset(CLIENTCONFIG_MODIFIED_FLAG);
        return true;
    }
    return false;
}
bool PerClientSyncContext::isModified(Identity const& client) const
{
    auto const it = this->g_syncTable.find(client);
    if (it != this->g_syncTable.end())
    {
        return it->second._config.has(CLIENTCONFIG_MODIFIED_FLAG);
    }
    return false;
}

void PerClientSyncContext::setRequireExplicitUpdateFlag(Identity const& client)
{
    auto it = this->g_syncTable.find(client);
    if (it != this->g_syncTable.end())
    {
        it->second._config.set(CLIENTCONFIG_REQUIRE_EXPLICIT_UPDATE_FLAG);
    }
}
bool PerClientSyncContext::isRequiringExplicitUpdate(Identity const& client) const
{
    auto const it = this->g_syncTable.find(client);
    if (it != this->g_syncTable.end())
    {
        return it->second._config.has(CLIENTCONFIG_REQUIRE_EXPLICIT_UPDATE_FLAG);
    }
    return false;
}

PerClientData& PerClientSyncContext::newClient(Identity const& client, fge::EnumFlags_t<PerClientConfigs> config)
{
    auto [it, alreadyInserted] = this->g_syncTable.emplace(client, config);

    if (alreadyInserted)
    {
        return it->second; //Client already exists, return its data
    }

    this->createClientData(it->second._data);
    return it->second;
}
void PerClientSyncContext::delClient(Identity const& client)
{
    auto it = this->g_syncTable.find(client);
    if (it != this->g_syncTable.end())
    {
        this->g_syncTable.erase(it);
    }
}

bool PerClientSyncContext::hasClient(Identity const& client) const
{
    return this->g_syncTable.contains(client);
}

PerClientData const* PerClientSyncContext::getClientData(Identity const& client) const
{
    auto const it = this->g_syncTable.find(client);
    if (it != this->g_syncTable.end())
    {
        return &it->second;
    }
    return nullptr;
}
PerClientData* PerClientSyncContext::getClientData(Identity const& client)
{
    auto it = this->g_syncTable.find(client);
    if (it != this->g_syncTable.end())
    {
        return &it->second;
    }
    return nullptr;
}

PerClientSyncContext::SyncTable::const_iterator PerClientSyncContext::begin() const
{
    return this->g_syncTable.begin();
}
PerClientSyncContext::SyncTable::iterator PerClientSyncContext::begin()
{
    return this->g_syncTable.begin();
}
PerClientSyncContext::SyncTable::const_iterator PerClientSyncContext::end() const
{
    return this->g_syncTable.end();
}
PerClientSyncContext::SyncTable::iterator PerClientSyncContext::end()
{
    return this->g_syncTable.end();
}

void PerClientSyncContext::clear()
{
    this->g_syncTable.clear();
}
void PerClientSyncContext::clientsCheckup(ClientList const& clients,
                                          bool force,
                                          fge::EnumFlags_t<PerClientConfigs> config)
{
    if (force)
    { //Clear and redo the table by looking all the clients in the ClientList
        this->g_syncTable.clear();

        auto lock = clients.acquireLock();
        for (auto it = clients.begin(lock); it != clients.end(lock); ++it)
        {
            this->createClientData(this->newClient(it->first, config)._data);
        }
        return;
    }

    //Remove/add extra clients by ClientList events
    for (std::size_t i = 0; i < clients.getClientEventSize(); ++i)
    {
        auto const& evt = clients.getClientEvent(i);

        if (evt._event == ClientListEvent::CLEVT_DELCLIENT)
        {
            this->delClient(evt._id);
        }
        else
        {
            this->createClientData(this->newClient(evt._id, config)._data);
        }
    }
}

//NetworkTypeBase

bool NetworkTypeBase::clientsCheckup(ClientList const& clients, bool force)
{
    PerClientSyncContext::clientsCheckup(clients, force);

    //Apply modification flag to clients
    bool const checkResult = this->check();
    if (checkResult)
    {
        this->setModificationFlag();
        this->forceUncheck();
    }
    return checkResult;
}
bool NetworkTypeBase::checkClient(Identity const& id) const
{
    return this->isModified(id);
}
void NetworkTypeBase::forceCheckClient(Identity const& id)
{
    this->setModificationFlag(id);
}
void NetworkTypeBase::forceUncheckClient(Identity const& id)
{
    this->clearModificationFlag(id);
}
void NetworkTypeBase::requireExplicitUpdateClient(Identity const& id)
{
    this->setRequireExplicitUpdateFlag(id);
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

bool NetworkTypeScene::applyData(Packet const& pck)
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
void NetworkTypeScene::packData(Packet& pck, Identity const& id)
{
    this->g_typeSource->packModification(pck, id);
    this->g_typeSource->packWatchedEvent(pck, id);
}
void NetworkTypeScene::packData(Packet& pck)
{
    this->g_typeSource->pack(pck, {});
}

bool NetworkTypeScene::clientsCheckup(ClientList const& clients, bool force)
{
    this->g_typeSource->clientsCheckup(clients, force);
    return true;
}

bool NetworkTypeScene::checkClient([[maybe_unused]] Identity const& id) const
{
    return true;
}
void NetworkTypeScene::forceCheckClient(Identity const& id)
{
    this->g_typeSource->forceCheckClient(id);
}
void NetworkTypeScene::forceUncheckClient(Identity const& id)
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

bool NetworkTypeSmoothVec2Float::applyData(Packet const& pck)
{
    if (pck >> this->g_typeCopy)
    {
        fge::Vector2f const source = this->g_typeSource._getter();
        float const error = std::abs(this->g_typeCopy.x - source.x) + std::abs(this->g_typeCopy.y - source.y);
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
void NetworkTypeSmoothVec2Float::packData(Packet& pck, Identity const& id)
{
    if (this->clearModificationFlag(id))
    {
        pck << this->g_typeSource._getter();
    }
}
void NetworkTypeSmoothVec2Float::packData(Packet& pck)
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

bool NetworkTypeSmoothFloat::applyData(Packet const& pck)
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
void NetworkTypeSmoothFloat::packData(Packet& pck, Identity const& id)
{
    if (this->clearModificationFlag(id))
    {
        pck << this->g_typeSource._getter();
    }
}
void NetworkTypeSmoothFloat::packData(Packet& pck)
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

bool NetworkTypeTag::applyData(Packet const& pck)
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
void NetworkTypeTag::packData(Packet& pck, [[maybe_unused]] Identity const& id)
{
    pck << this->g_typeSource->check(this->g_tag);
}
void NetworkTypeTag::packData(Packet& pck)
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

NetworkTypeBase* NetworkTypeHandler::push(std::unique_ptr<NetworkTypeBase>&& newNet)
{
    return this->g_data.emplace_back(std::move(newNet)).get();
}

std::size_t NetworkTypeHandler::packNeededUpdate(Packet& pck) const
{
    auto const rewritePos = pck.getDataSize();
    pck.append(sizeof(SizeType)); //Will be rewritten

    SizeType count{0};
    for (SizeType i = 0; i < this->g_data.size(); ++i)
    {
        auto* net = this->g_data[i].get();

        if (net->isNeedingExplicitUpdate())
        {
            pck << i;
            ++count;
            net->clearExplicitUpdateFlag();
            net->clearWaitingUpdateFlag();
            continue;
        }

        auto const lastUpdateTime = std::chrono::steady_clock::now().time_since_epoch() - net->getLastUpdateTime();
        if (net->isWaitingUpdate() && lastUpdateTime >= FGE_NET_WAITING_UPDATE_DELAY)
        {
            pck << i;
            ++count;
            net->clearWaitingUpdateFlag();
        }
    }

    pck.pack(rewritePos, &count, sizeof(SizeType));

    return count;
}
void NetworkTypeHandler::unpackNeededUpdate(Packet const& pck, Identity const& id) const
{
    ///TODO : need safe data extraction with net rules
    SizeType count{0};
    pck >> count;

    for (SizeType i = 0; i < count; ++i)
    {
        SizeType dataIndex{std::numeric_limits<SizeType>::max()};
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

void NetworkTypeHandler::clientsCheckup(ClientList const& clients, bool force) const
{
    for (auto const& netType: this->g_data)
    {
        netType->clientsCheckup(clients, force);
    }
}
void NetworkTypeHandler::forceCheckClient(Identity const& id) const
{
    for (auto const& netType: this->g_data)
    {
        netType->forceCheckClient(id);
    }
}
void NetworkTypeHandler::forceUncheckClient(Identity const& id) const
{
    for (auto const& netType: this->g_data)
    {
        netType->forceUncheckClient(id);
    }
}

void NetworkTypeHandler::ignoreClient(Identity const& id)
{
    this->g_ignoredClients.insert(id);
}
void NetworkTypeHandler::unignoreClient(Identity const& id)
{
    this->g_ignoredClients.erase(id);
}
bool NetworkTypeHandler::isIgnored(Identity const& id) const
{
    return this->g_ignoredClients.contains(id);
}
void NetworkTypeHandler::clearIgnoredClients()
{
    this->g_ignoredClients.clear();
}

} // namespace fge::net

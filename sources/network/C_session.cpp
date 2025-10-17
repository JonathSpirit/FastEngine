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

#include "FastEngine/network/C_session.hpp"

namespace fge::net
{

//Session
Session::Session(Id session) :
        g_id(session)
{}
Session::Session(Session&& r) noexcept :
        g_lastReceivedPacketTimePoint(r.g_lastReceivedPacketTimePoint),
        g_lastRealmTimePoint(r.g_lastRealmTimePoint),

        g_serverRealm(r.g_serverRealm),
        g_serverPacketCounter(r.g_serverPacketCounter),
        g_clientPacketCounter(r.g_clientPacketCounter),

        g_packetCache(std::move(r.g_packetCache)),
        g_acknowledgedPackets(std::move(r.g_acknowledgedPackets)),
        g_packetReorderer(std::move(r.g_packetReorderer)),
        g_packetDefragmentation(std::move(r.g_packetDefragmentation)),

        g_state(r.g_state),
        g_id(r.g_id),

        g_mtu(r.g_mtu),
        g_forceMTU(r.g_forceMTU),
        g_enableCache(r.g_enableCache),
        g_enableReorderer(r.g_enableReorderer),
        g_enableDefragmentation(r.g_enableDefragmentation)
{}

void Session::updateSession(SessionManager& manager, std::chrono::milliseconds deltaTime)
{
    std::scoped_lock const lck(this->g_mutex);

    switch (this->g_state)
    {
    case States::UNINITIALIZED:
        switch (this->g_comState)
        {
        case ComStates::NONE:
            break;
        case ComStates::NEED_ACK:
            break;
        }
        break;
    case States::INITIALIZED:
        break;
    case States::RECONFIGURING:
        break;
    case States::DELETING:
        break;
    }
}

ProtocolPacket::RealmType Session::advanceRealm()
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_serverPacketCounter = 0;
    this->g_serverRealm++;
    this->g_lastRealmTimePoint = std::chrono::steady_clock::now();
    if (this->g_state == States::INITIALIZED)
    {
        this->g_state = States::RECONFIGURING;
    }
    return this->g_serverRealm;
}

void Session::resetPacketCounter()
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_serverPacketCounter = 0;
    if (this->g_state == States::INITIALIZED)
    {
        this->g_state = States::RECONFIGURING;
    }
}
void Session::resetRealm()
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_serverRealm = FGE_NET_DEFAULT_REALM;
    this->g_serverPacketCounter = 0;
    this->g_lastRealmTimePoint = std::chrono::steady_clock::now();
    if (this->g_state == States::INITIALIZED)
    {
        this->g_state = States::RECONFIGURING;
    }
}
void Session::setMTU(MTU mtu)
{
    std::scoped_lock const lck(this->g_mutex);
    if (this->g_forceMTU && mtu != this->g_mtu)
    {
        if (this->g_state == States::INITIALIZED)
        {
            this->g_state = States::RECONFIGURING;
        }
    }
    this->g_mtu = mtu;
}
void Session::forceMTU(bool force)
{
    std::scoped_lock const lck(this->g_mutex);
    if (force == this->g_forceMTU)
    {
        return;
    }
    this->g_forceMTU = force;
    if (this->g_state == States::INITIALIZED)
    {
        this->g_state = States::RECONFIGURING;
    }
}

MTU Session::getMTU() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_mtu;
}
bool Session::isMTUForced() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_forceMTU;
}

void Session::pushPacket(ReceivedPacketPtr&& pck)
{

}
ReceivedPacketPtr Session::popPacket()
{

}

Session::States Session::getCurrentState() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_state;
}

void Session::forceState(States state)
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_state = state;
}

//SessionManager

SessionManager::SessionManager()
{
    auto& defaultSession = this->g_sessions.emplace_back(FGE_NET_DEFAULT_SESSION);
    defaultSession.forceMTU(true);
    defaultSession.forceState(Session::States::INITIALIZED);
}

void SessionManager::updateSessions(std::chrono::milliseconds deltaTime)
{

}
Session& SessionManager::createSession()
{

}
void SessionManager::deleteSession(Session::Id id)
{

}
Session* SessionManager::getSession(Session::Id id)
{

}
void SessionManager::clearSessions()
{

}
std::size_t SessionManager::getSessionCount() const
{

}

void SessionManager::handlePacketReception(ReceivedPacketPtr&& packet) {}

void SessionManager::clearPackets() {}

void SessionManager::pushPacket(TransmitPacketPtr pck) {}

void SessionManager::pushForcedFrontPacket(TransmitPacketPtr pck) {}

TransmitPacketPtr SessionManager::popPacket() {}

bool SessionManager::isPendingPacketsEmpty() const {}


} // namespace fge::net

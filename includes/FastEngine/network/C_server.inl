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

namespace fge::net
{

//ClientSideNetUdp

template<class TPacket>
void ClientSideNetUdp::sendTo(TransmitPacketPtr& pck, Identity const& id)
{ ///TODO: have a transmission queue ?
    if (!pck->packet() || !pck->haveCorrectHeaderSize())
    { //Last verification of the packet
        return;
    }

    pck->doNotReorder();

    std::scoped_lock const lock(this->_g_mutexFlux);
    TPacket packet = pck->packet();
    this->g_socket.sendTo(packet, id._ip, id._port);
}

} // namespace fge::net

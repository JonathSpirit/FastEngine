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

namespace fge
{
namespace net
{

fge::net::Packet& SetHeader(fge::net::Packet& pck, fge::net::PacketHeader header)
{
    pck.clear();
    pck << header;
    return pck;
}
fge::net::PacketHeader GetHeader(fge::net::Packet& pck)
{
    fge::net::PacketHeader header = FGE_NET_BAD_HEADER;
    pck >> header;
    return header;
}

bool CheckSkey(fge::net::Packet& pck, fge::net::Skey skey)
{
    fge::net::Skey buff;
    if ( pck >> buff)
    {
        return buff == skey;
    }
    return false;
}
fge::net::Skey GetSkey(fge::net::Packet& pck)
{
    fge::net::Skey buff;
    if ( pck >> buff)
    {
        return buff;
    }
    return FGE_NET_BAD_SKEY;
}

}//end net
}//end fge

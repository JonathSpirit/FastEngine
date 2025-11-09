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

#ifndef _FGE_FGE_CRYPT_HPP_INCLUDED
#define _FGE_FGE_CRYPT_HPP_INCLUDED

#include "FastEngine/network/C_client.hpp"

namespace fge::net
{

class Packet;

} // namespace fge::net

namespace fge::priv
{

[[nodiscard]] bool CryptClientInit(void*& ctx);
[[nodiscard]] bool CryptServerInit(void*& ctx);
void CryptUninit(void*& ctx);

[[nodiscard]] bool CryptClientCreate(void* ctx, net::CryptInfo& client);
[[nodiscard]] bool CryptServerCreate(void* ctx, net::CryptInfo& client);
void CryptClientDestroy(net::CryptInfo& client);

[[nodiscard]] bool CryptEncrypt(net::Client& client, net::Packet& packet);
[[nodiscard]] bool CryptDecrypt(net::Client& client, net::Packet& packet);

} // namespace fge::priv

#endif // _FGE_FGE_CRYPT_HPP_INCLUDED

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

#ifndef _FGE_NETWORK_MANAGER_HPP_INCLUDED
#define _FGE_NETWORK_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_scene.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_packet.hpp>
#include <FastEngine/C_client.hpp>
#include <optional>

#define FGE_NET_BAD_HEADER 0
#define FGE_NET_RULES_START {auto chainedArgs_=
#define FGE_NET_RULES_TRY if (!chainedArgs_._pck->isValid()) {return;}
#define FGE_NET_RULES_AFFECT(var_) FGE_NET_RULES_TRY if constexpr (std::is_move_constructible<decltype(var_)>::value){(var_) = std::move(chainedArgs_._value.value());}else{(var_) = chainedArgs_._value.value();}
#define FGE_NET_RULES_AFFECT_SETTER(setter_) FGE_NET_RULES_TRY setter_(std::move(chainedArgs_._value.value()));
#define FGE_NET_RULES_RESULT std::move(chainedArgs_._value.value())
#define FGE_NET_RULES_RESULT_N chainedArgs_._value.value()
#define FGE_NET_RULES_END }
#define FGE_NET_RULES_AFFECT_END(var_) FGE_NET_RULES_AFFECT(var_) FGE_NET_RULES_END
#define FGE_NET_RULES_AFFECT_SETTER_END(setter_) FGE_NET_RULES_AFFECT_SETTER(setter_) FGE_NET_RULES_END

namespace fge::net
{

using PacketHeader = uint16_t;

FGE_API uint32_t GetSceneChecksum(fge::Scene& scene);

FGE_API bool WritePacketDataToFile(fge::net::Packet& pck, const std::string& file);


inline fge::net::Packet& SetHeader(fge::net::Packet& pck, fge::net::PacketHeader header);
inline fge::net::PacketHeader GetHeader(fge::net::Packet& pck);

inline bool CheckSkey(fge::net::Packet& pck, fge::net::Skey skey);
inline fge::net::Skey GetSkey(fge::net::Packet& pck);

namespace rules
{

template<class TValue>
struct ChainedArguments
{
    fge::net::Packet* _pck;
    std::optional<TValue> _value{std::nullopt};
};

template<class TValue>
TValue& Extract(fge::net::rules::ChainedArguments<TValue>& args);

template<class TValue, bool TInvertResult=false>
fge::net::rules::ChainedArguments<TValue> RRange(const TValue& min, const TValue& max, fge::net::rules::ChainedArguments<TValue> args);

template<class TValue, bool TInvertResult=false>
fge::net::rules::ChainedArguments<TValue> RMustEqual(const TValue& a, fge::net::rules::ChainedArguments<TValue> args);

}//end rules

}//end fge::net

#include <FastEngine/network_manager.inl>

#endif // _FGE_NETWORK_MANAGER_HPP_INCLUDED

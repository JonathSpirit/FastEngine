#ifndef _FGE_NETWORK_MANAGER_HPP_INCLUDED
#define _FGE_NETWORK_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_scene.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_packet.hpp>
#include <FastEngine/C_client.hpp>

#define FGE_NET_BAD_HEADER 0

namespace fge
{
namespace net
{

using PacketHeader = uint16_t;

FGE_API uint32_t GetSceneChecksum(fge::Scene& scene);

FGE_API bool WritePacketDataToFile(fge::net::Packet& pck, const std::string& file);


inline fge::net::Packet& SetHeader(fge::net::Packet& pck, fge::net::PacketHeader header);
inline fge::net::PacketHeader GetHeader(fge::net::Packet& pck);

inline bool CheckSkey(fge::net::Packet& pck, fge::net::Skey skey);
inline fge::net::Skey GetSkey(fge::net::Packet& pck);

}//end net
}//end fge

#include <FastEngine/network_manager.inl>

#endif // _FGE_NETWORK_MANAGER_HPP_INCLUDED

#include "FastEngine/network_manager.hpp"
#include "FastEngine/reg_manager.hpp"
#include <string>
#include <vector>
#include <fstream>

namespace fge
{
namespace net
{

uint32_t GetSceneChecksum(fge::Scene& scene)
{
    uint32_t result = 0;
    for (const auto& object : scene)
    {
        result += object->getSid();
    }
    return result;
}

bool WritePacketDataToFile(fge::net::Packet& pck, const std::string& file)
{
    std::ofstream theFile(file, std::ios::binary);
    if (!theFile)
    {
        return false;
    }

    theFile.write(reinterpret_cast<char*>(pck.getData()), pck.getDataSize());
    theFile.close();
    return true;
}

bool WriteOnSendPacketDataToFile(fge::net::Packet& pck, const std::string& file)
{
    std::ofstream theFile(file, std::ios::binary);
    if (!theFile)
    {
        return false;
    }

    std::vector<uint8_t> buffer;
    pck.onSend(buffer, 0);

    theFile.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
    theFile.close();
    return true;
}

}//end net
}//end fge


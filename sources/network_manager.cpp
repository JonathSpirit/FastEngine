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

#include "FastEngine/network_manager.hpp"
#include "FastEngine/reg_manager.hpp"
#include <string>
#include <vector>
#include <fstream>

namespace fge::net
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

}//end fge::net


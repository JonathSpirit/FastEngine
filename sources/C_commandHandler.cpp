/*
 * Copyright 2024 Guillaume Guillet
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

#include "FastEngine/C_commandHandler.hpp"
#include <limits>

#define _FGE_CMD_RESERVESIZE 30

namespace fge
{

CommandHandler::CommandHandler()
{
    this->g_cmdData.reserve(_FGE_CMD_RESERVESIZE);
}

bool CommandHandler::addCmd(std::string_view name, fge::CommandHandler* handle, fge::CommandFunction cmdfunc)
{
    auto it = this->g_cmdDataMap.find(name);

    if (it == this->g_cmdDataMap.end())
    {
        auto& cmdData =
                this->g_cmdData.emplace_back(fge::CommandHandler::CommandData({handle, cmdfunc, std::string(name)}));
        this->g_cmdDataMap[cmdData._name] = this->g_cmdData.size() - 1;
        return true;
    }
    return false;
}

void CommandHandler::delCmd(std::string_view name)
{
    auto it = this->g_cmdDataMap.find(name);

    if (it != this->g_cmdDataMap.end())
    {
        this->g_cmdData.erase(this->g_cmdData.begin() +
                              static_cast<fge::CommandHandler::CommandDataType::difference_type>(it->second));
        this->g_cmdDataMap.erase(it);

        for (std::size_t i = 0; i < this->g_cmdData.size(); ++i)
        {
            this->g_cmdDataMap[this->g_cmdData[i]._name] = i;
        }
    }
}

bool CommandHandler::replaceCmd(std::string_view name, fge::CommandHandler* handle, fge::CommandFunction cmdfunc)
{
    auto it = this->g_cmdDataMap.find(name);

    if (it != this->g_cmdDataMap.end())
    {
        this->g_cmdData[it->second]._handle = handle;
        this->g_cmdData[it->second]._func = cmdfunc;
        return true;
    }
    return false;
}

void CommandHandler::clearCmd()
{
    this->g_cmdData.clear();
    this->g_cmdDataMap.clear();
}

fge::Property
CommandHandler::callCmd(std::string_view name, fge::Object* caller, fge::Property const& arg, fge::Scene* caller_scene)
{
    auto it = this->g_cmdDataMap.find(name);

    if (it != this->g_cmdDataMap.end())
    {
        return (this->g_cmdData[it->second]._handle->*this->g_cmdData[it->second]._func)(caller, arg, caller_scene);
    }
    return {};
}
fge::Property
CommandHandler::callCmd(std::size_t index, fge::Object* caller, fge::Property const& arg, fge::Scene* caller_scene)
{
    if (index < this->g_cmdData.size())
    {
        return (this->g_cmdData[index]._handle->*this->g_cmdData[index]._func)(caller, arg, caller_scene);
    }
    return {};
}

std::size_t CommandHandler::getCmdIndex(std::string_view name) const
{
    auto it = this->g_cmdDataMap.find(name);

    if (it != this->g_cmdDataMap.end())
    {
        return it->second;
    }
    return std::numeric_limits<std::size_t>::max();
}
std::string_view CommandHandler::getCmdName(std::size_t index) const
{
    if (index < this->g_cmdData.size())
    {
        return this->g_cmdData[index]._name;
    }
    return {};
}

fge::CommandHandler::CommandData const* CommandHandler::getCmd(std::string_view name) const
{
    auto it = this->g_cmdDataMap.find(name);

    if (it != this->g_cmdDataMap.end())
    {
        return &this->g_cmdData[it->second];
    }
    return nullptr;
}

std::size_t CommandHandler::getCmdSize() const
{
    return this->g_cmdData.size();
}

fge::CommandHandler::CommandDataType const& CommandHandler::getCmdList() const
{
    return this->g_cmdData;
}

} // namespace fge

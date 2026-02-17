/*
 * Copyright 2026 Guillaume Guillet
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

namespace fge
{

CommandHandler::CommandHandler()
{
    this->g_cmdData.reserve(FGE_COMMAND_DEFAULT_RESERVE_SIZE);
}

bool CommandHandler::addCmd(std::string_view name, fge::CommandFunction cmdfunc)
{
    auto it = this->g_cmdDataMap.find(name);

    if (it == this->g_cmdDataMap.end())
    {
        auto& cmdData = this->g_cmdData.emplace_back(std::move(cmdfunc), name);
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
        this->g_cmdData.erase(this->g_cmdData.begin() + static_cast<CommandDataType::difference_type>(it->second));
        this->g_cmdDataMap.erase(it);

        for (std::size_t i = 0; i < this->g_cmdData.size(); ++i)
        {
            this->g_cmdDataMap[this->g_cmdData[i]._name] = i;
        }
    }
}

bool CommandHandler::replaceCmd(std::string_view name, fge::CommandFunction cmdfunc)
{
    auto it = this->g_cmdDataMap.find(name);

    if (it != this->g_cmdDataMap.end())
    {
        this->g_cmdData[it->second]._func = std::move(cmdfunc);
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
CommandHandler::callCmd(std::string_view name, fge::Object* caller, fge::Property const& arg, fge::Scene* callerScene)
{
    auto it = this->g_cmdDataMap.find(name);

    if (it != this->g_cmdDataMap.end())
    {
        return this->g_cmdData[it->second]._func->call(caller, arg, callerScene);
    }
    return {};
}
fge::Property
CommandHandler::callCmd(std::size_t index, fge::Object* caller, fge::Property const& arg, fge::Scene* callerScene)
{
    if (index < this->g_cmdData.size())
    {
        return this->g_cmdData[index]._func->call(caller, arg, callerScene);
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

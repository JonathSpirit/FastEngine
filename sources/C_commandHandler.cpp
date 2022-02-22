#include "FastEngine/C_commandHandler.hpp"
#include <limits>

#define _FGE_CMD_RESERVESIZE 30

namespace fge
{

CommandHandler::CommandHandler()
{
    this->g_cmdData.reserve(_FGE_CMD_RESERVESIZE);
}

bool CommandHandler::addCmd(const std::string &name, fge::CommandHandler *handle, fge::CommandFunction cmdfunc)
{
    auto it = this->g_cmdDataMap.find(name);

    if (it == this->g_cmdDataMap.end())
    {
        this->g_cmdData.emplace_back( fge::CommandHandler::CommandData({handle, cmdfunc, name}) );
        this->g_cmdDataMap[name] = this->g_cmdData.size()-1;
        return true;
    }
    return false;
}

void CommandHandler::delCmd(const std::string &name)
{
    auto it = this->g_cmdDataMap.find(name);

    if (it != this->g_cmdDataMap.end())
    {
        this->g_cmdData.erase(this->g_cmdData.begin()+it->second);
        this->g_cmdDataMap.erase(it);

        for (std::size_t i=0; i<this->g_cmdData.size(); ++i)
        {
            this->g_cmdDataMap[this->g_cmdData[i]._name] = i;
        }
    }
}

bool CommandHandler::replaceCmd(const std::string &name, fge::CommandHandler *handle, fge::CommandFunction cmdfunc)
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

fge::Property CommandHandler::callCmd(const std::string &name, fge::Object *caller, const fge::Property &arg, fge::Scene *caller_scene)
{
    auto it = this->g_cmdDataMap.find(name);

    if (it != this->g_cmdDataMap.end())
    {
        return (this->g_cmdData[it->second]._handle->*this->g_cmdData[it->second]._func)(caller, arg, caller_scene);
    }
    return {};
}
fge::Property CommandHandler::callCmd(std::size_t index, fge::Object *caller, const fge::Property &arg, fge::Scene *caller_scene)
{
    if (index < this->g_cmdData.size())
    {
        return (this->g_cmdData[index]._handle->*this->g_cmdData[index]._func)(caller, arg, caller_scene);
    }
    return {};
}

std::size_t CommandHandler::getCmdIndex(const std::string& name) const
{
    auto it = this->g_cmdDataMap.find(name);

    if (it != this->g_cmdDataMap.end())
    {
        return it->second;
    }
    return std::numeric_limits<std::size_t>::max();
}
std::string CommandHandler::getCmdName(std::size_t index) const
{
    if (index < this->g_cmdData.size())
    {
        return this->g_cmdData[index]._name;
    }
    return {};
}

const fge::CommandHandler::CommandData* CommandHandler::getCmd(const std::string& name) const
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

const fge::CommandHandler::CommandDataType& CommandHandler::getCmdList() const
{
    return this->g_cmdData;
}

}//end fge

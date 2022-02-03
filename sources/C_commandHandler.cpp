#include "FastEngine/C_commandHandler.hpp"

namespace fge
{

bool FGE_API CommandHandler::addCmd(const std::string& name, fge::CommandHandler* handle, fge::CommandFunction cmdfunc)
{
    return this->g_cmdData.emplace(name, fge::CommandHandler::CommandData({handle, cmdfunc})).second;
}
void FGE_API CommandHandler::delCmd(const std::string& name)
{
    this->g_cmdData.erase(name);
}
bool FGE_API CommandHandler::replaceCmd(const std::string& name, fge::CommandHandler* handle, fge::CommandFunction cmdfunc)
{
    fge::CommandHandler::CommandDataType::iterator it = this->g_cmdData.find(name);

    if (it != this->g_cmdData.end())
    {
        it->second._handle = handle;
        it->second._func = cmdfunc;
        return true;
    }
    return false;
}

void FGE_API CommandHandler::clearCmd()
{
    this->g_cmdData.clear();
}

fge::Value FGE_API CommandHandler::callCmd(const std::string& name, fge::Object* caller, const fge::Value& arg, fge::Scene* caller_scene)
{
    fge::CommandHandler::CommandDataType::iterator it = this->g_cmdData.find(name);

    if (it != this->g_cmdData.end())
    {
        return (it->second._handle->*it->second._func)(caller, arg, caller_scene);
    }
    return fge::Value();
}

const fge::CommandHandler::CommandData* FGE_API CommandHandler::getCmd(const std::string& name) const
{
    fge::CommandHandler::CommandDataType::const_iterator it = this->g_cmdData.find(name);

    if (it != this->g_cmdData.cend())
    {
        return &it->second;
    }
    return nullptr;
}

std::size_t FGE_API CommandHandler::getCmdSize() const
{
    return this->g_cmdData.size();
}

fge::CommandHandler::CommandDataType::const_iterator FGE_API CommandHandler::cbegin() const
{
    return this->g_cmdData.cbegin();
}
fge::CommandHandler::CommandDataType::const_iterator FGE_API CommandHandler::cend() const
{
    return this->g_cmdData.cend();
}

}//end fge

#ifndef _FGE_C_COMMANDHANDLER_HPP_INCLUDED
#define _FGE_C_COMMANDHANDLER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_value.hpp>
#include <unordered_map>

#define FGE_CMD_FUNC(x) static_cast<fge::CommandFunction>(x)

namespace fge
{

class CommandHandler;
class Scene;
class Object;

using CommandFunction = fge::Value (CommandHandler::*) (fge::Object* caller, const fge::Value& arg, fge::Scene* caller_scene);

class CommandHandler
{
public:
    struct CommandData
    {
        fge::CommandHandler* _handle;
        fge::CommandFunction _func;
    };

    using CommandDataType = std::unordered_map<std::string, fge::CommandHandler::CommandData>;

    CommandHandler() = default;

    bool addCmd(const std::string& name, fge::CommandHandler* handle, fge::CommandFunction cmdfunc);
    void delCmd(const std::string& name);
    bool replaceCmd(const std::string& name, fge::CommandHandler* handle, fge::CommandFunction cmdfunc);

    void clearCmd();

    fge::Value callCmd(const std::string& name, fge::Object* caller, const fge::Value& arg, fge::Scene* caller_scene);

    const fge::CommandHandler::CommandData* getCmd(const std::string& name) const;

    std::size_t getCmdSize() const;

    fge::CommandHandler::CommandDataType::const_iterator cbegin() const;
    fge::CommandHandler::CommandDataType::const_iterator cend() const;

private:
    fge::CommandHandler::CommandDataType g_cmdData;
};

}//end fge

#endif // _FGE_C_COMMANDHANDLER_HPP_INCLUDED

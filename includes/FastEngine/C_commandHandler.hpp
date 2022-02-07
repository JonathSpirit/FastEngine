#ifndef _FGE_C_COMMANDHANDLER_HPP_INCLUDED
#define _FGE_C_COMMANDHANDLER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_value.hpp>
#include <string>
#include <vector>
#include <unordered_map>

#define FGE_CMD_FUNC(x) static_cast<fge::CommandFunction>(x)

namespace fge
{

class CommandHandler;
class Scene;
class Object;

using CommandFunction = fge::Value (CommandHandler::*) (fge::Object* caller, const fge::Value& arg, fge::Scene* caller_scene);

class FGE_API CommandHandler
{
public:
    struct CommandData
    {
        fge::CommandHandler* _handle;
        fge::CommandFunction _func;
        std::string _name;
    };

    using CommandDataType = std::vector<fge::CommandHandler::CommandData>;

    CommandHandler();

    bool addCmd(const std::string& name, fge::CommandHandler* handle, fge::CommandFunction cmdfunc);
    void delCmd(const std::string& name);
    bool replaceCmd(const std::string& name, fge::CommandHandler* handle, fge::CommandFunction cmdfunc);

    void clearCmd();

    fge::Value callCmd(const std::string& name, fge::Object* caller, const fge::Value& arg, fge::Scene* caller_scene);
    fge::Value callCmd(std::size_t index, fge::Object* caller, const fge::Value& arg, fge::Scene* caller_scene);

    [[nodiscard]] std::size_t getCmdIndex(const std::string& name) const;
    [[nodiscard]] std::string getCmdName(std::size_t index) const;

    [[nodiscard]] const fge::CommandHandler::CommandData* getCmd(const std::string& name) const;

    [[nodiscard]] std::size_t getCmdSize() const;

    [[nodiscard]] fge::CommandHandler::CommandDataType::const_iterator cbegin() const;
    [[nodiscard]] fge::CommandHandler::CommandDataType::const_iterator cend() const;

private:
    fge::CommandHandler::CommandDataType g_cmdData;
    std::unordered_map<std::string, std::size_t> g_cmdDataMap;
};

}//end fge

#endif // _FGE_C_COMMANDHANDLER_HPP_INCLUDED

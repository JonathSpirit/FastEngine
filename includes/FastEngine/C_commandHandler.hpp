/*
 * Copyright 2023 Guillaume Guillet
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

#ifndef _FGE_C_COMMANDHANDLER_HPP_INCLUDED
#define _FGE_C_COMMANDHANDLER_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_property.hpp"
#include <string>
#include <unordered_map>
#include <vector>

/**
 * \def FGE_CMD_FUNC
 * \ingroup objectControl
 * \brief Helper to case a command function
 */
#define FGE_CMD_FUNC(x) static_cast<fge::CommandFunction>(x)

namespace fge
{

class CommandHandler;
class Scene;
class Object;

using CommandFunction = fge::Property (CommandHandler::*)(fge::Object* caller,
                                                          const fge::Property& arg,
                                                          fge::Scene* caller_scene);

/**
 * \class CommandHandler
 * \ingroup objectControl
 * \brief CommandHandler is a class that can be used to handle commands
 *
 * A command is a well-defined function signature that can be attributed to an name (string) and
 * be called by another object with ease.
 *
 * A command is also indexed to avoid sending the command name on a network communication.
 */
class FGE_API CommandHandler
{
public:
    /**
     * \struct CommandData
     * \brief This struct contain the data of a command, like the name and the function pointer
     */
    struct CommandData
    {
        fge::CommandHandler* _handle;
        fge::CommandFunction _func;
        std::string _name;
    };

    using CommandDataType = std::vector<fge::CommandHandler::CommandData>;

    CommandHandler();

    /**
     * \brief Add a new command to the handler
     *
     * An object should inherit from CommandHandler and add commands to it.
     *
     * \param name The name of the command
     * \param handle The object that will handle the command
     * \param cmdfunc The function pointer of the command
     * \return \b true if the command was added, \b false otherwise
     */
    bool addCmd(std::string_view name, fge::CommandHandler* handle, fge::CommandFunction cmdfunc);
    /**
     * \brief Delete a command from the handler
     *
     * \param name The name of the command
     */
    void delCmd(std::string_view name);
    /**
     * \brief Replace a command from the handler
     *
     * \param name The name of the command
     * \param handle The new object that will handle the command
     * \param cmdfunc The new function pointer of the command
     * \return \b true if the command was replaced, \b false otherwise
     */
    bool replaceCmd(std::string_view name, fge::CommandHandler* handle, fge::CommandFunction cmdfunc);

    /**
     * \brief Clear all commands from the handler
     */
    void clearCmd();

    /**
     * \brief Call a command by its name
     *
     * \param name The name of the command
     * \param caller The object that call the command
     * \param arg The arguments of the command
     * \param caller_scene The scene that contains the caller
     * \return A Property containing the result of the command
     */
    fge::Property
    callCmd(std::string_view name, fge::Object* caller, const fge::Property& arg, fge::Scene* caller_scene);
    /**
     * \brief Call a command by its index
     *
     * \param index The index of the command
     * \param caller The object that call the command
     * \param arg The arguments of the command
     * \param caller_scene The scene that contains the caller
     * \return A Property containing the result of the command
     */
    fge::Property callCmd(std::size_t index, fge::Object* caller, const fge::Property& arg, fge::Scene* caller_scene);

    /**
     * \brief Get the index of a command by its name
     *
     * \param name The name of the command
     * \return The index of the command or std::numeric_limits<std::size_t>::max() if the command doesn't exist
     */
    [[nodiscard]] std::size_t getCmdIndex(std::string_view name) const;
    /**
     * \brief Get the name of a command by its index
     *
     * \param index The index of the command
     * \return The name of the command or an empty string if the command doesn't exist
     */
    [[nodiscard]] std::string_view getCmdName(std::size_t index) const;

    /**
     * \brief Get a command by its name
     *
     * \param name The name of the command
     * \return The command or nullptr if the command doesn't exist
     */
    [[nodiscard]] const fge::CommandHandler::CommandData* getCmd(std::string_view name) const;

    /**
     * \brief Get the number of commands
     *
     * \return The number of commands
     */
    [[nodiscard]] std::size_t getCmdSize() const;

    /**
     * \brief Get the commands list
     *
     * \return The commands list
     */
    [[nodiscard]] const fge::CommandHandler::CommandDataType& getCmdList() const;

private:
    fge::CommandHandler::CommandDataType g_cmdData;
    std::unordered_map<std::string_view, std::size_t> g_cmdDataMap;
};

} // namespace fge

#endif // _FGE_C_COMMANDHANDLER_HPP_INCLUDED

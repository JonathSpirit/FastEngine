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

#ifndef _FGE_C_COMMANDHANDLER_HPP_INCLUDED
#define _FGE_C_COMMANDHANDLER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_callback.hpp"
#include "FastEngine/C_property.hpp"
#include <string>
#include <unordered_map>
#include <vector>

#define FGE_COMMAND_DEFAULT_RESERVE_SIZE 16

namespace fge
{

class CommandHandler;
class Scene;
class Object;

using CommandFunction = fge::CalleeUniquePtr<fge::Property, fge::Object*, fge::Property const&, fge::Scene*>;
using CommandStaticHelpers =
        fge::CallbackStaticHelpers<fge::Property, CommandFunction, fge::Object*, fge::Property const&, fge::Scene*>;

/**
 * \class CommandHandler
 * \ingroup objectControl
 * \brief CommandHandler can implement functions attached to an Object that can be called by others with a name
 *
 * A command is a well-defined function signature that can be attributed to a name (string) and
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
        inline CommandData(fge::CommandFunction cmdfunc, std::string_view name) :
                _func(std::move(cmdfunc)),
                _name(name)
        {}

        fge::CommandFunction _func;
        std::string _name;
    };

    using CommandDataType = std::vector<CommandData>;

    CommandHandler();
    CommandHandler(CommandHandler const& r) = delete;
    CommandHandler(CommandHandler&& r) noexcept = delete;

    CommandHandler& operator=(CommandHandler const& r) = delete;
    CommandHandler& operator=(CommandHandler&& r) noexcept = delete;

    /**
     * \brief Add a new command to the handler
     *
     * An object should inherit from CommandHandler and add commands to it.
     *
     * \param name The name of the command
     * \param cmdfunc The command
     * \return \b true if the command was added, \b false otherwise
     */
    bool addCmd(std::string_view name, fge::CommandFunction cmdfunc);
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
     * \param cmdfunc The command
     * \return \b true if the command was replaced, \b false otherwise
     */
    bool replaceCmd(std::string_view name, fge::CommandFunction cmdfunc);

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
     * \param callerScene The scene that contains the caller
     * \return A Property containing the result of the command
     */
    fge::Property
    callCmd(std::string_view name, fge::Object* caller, fge::Property const& arg, fge::Scene* callerScene);
    /**
     * \brief Call a command by its index
     *
     * \param index The index of the command
     * \param caller The object that call the command
     * \param arg The arguments of the command
     * \param callerScene The scene that contains the caller
     * \return A Property containing the result of the command
     */
    fge::Property callCmd(std::size_t index, fge::Object* caller, fge::Property const& arg, fge::Scene* callerScene);

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
    [[nodiscard]] CommandData const* getCmd(std::string_view name) const;

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
    [[nodiscard]] CommandDataType const& getCmdList() const;

private:
    CommandDataType g_cmdData;
    std::unordered_map<std::string_view, std::size_t> g_cmdDataMap;
};

} // namespace fge

#endif // _FGE_C_COMMANDHANDLER_HPP_INCLUDED

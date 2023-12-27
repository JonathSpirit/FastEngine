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

#include "FastEngine/C_task.hpp"
#include "FastEngine/manager/task_manager.hpp"

namespace fge
{

//NetworkTypeTasks
NetworkTypeTasks::NetworkTypeTasks(fge::TaskHandler* source) :
        g_tasksSource(source),
        g_checksumCopy(source->getChecksum())
{}

void const* NetworkTypeTasks::getSource() const
{
    return this->g_tasksSource;
}

bool NetworkTypeTasks::applyData(fge::net::Packet const& pck)
{
    std::underlying_type<NetworkTypeTasks::SyncType>::type syncType;

    if (pck >> syncType)
    {
        switch (static_cast<NetworkTypeTasks::SyncType>(syncType))
        {
        case NetworkTypeTasks::SYNC_CHECKSUM:
        {
            fge::TasksChecksum checksum{0};
            pck >> checksum;

            if (this->g_tasksSource->getChecksum() != checksum)
            {
                this->needUpdate();
            }
        }
        break;
        case NetworkTypeTasks::SYNC_FULL:
        {
            fge::net::SizeType size{0};
            pck >> size;

            this->g_tasksSource->clearTasks();
            for (fge::net::SizeType i = 0; i < size; ++i)
            {
                fge::TaskTypeIndex taskIndex;
                pck >> taskIndex;
                auto* task = fge::task::CreateNewTask(taskIndex);
                if (task == nullptr)
                {
                    break;
                }
                task->setParentObject(this->g_tasksSource->getParentObject());
                task->unpackAndInit(pck);
                this->g_tasksSource->addSubTask(std::unique_ptr<fge::Task>(task));
            }
            this->g_checksumCopy = this->g_tasksSource->getChecksum();
        }
        break;
        default:
            return false;
        }

        this->_onApplied.call();
        return true;
    }
    return false;
}
void NetworkTypeTasks::packData(fge::net::Packet& pck, fge::net::Identity const& id)
{
    auto it = this->_g_tableId.find(id);
    if (it != this->_g_tableId.end())
    {
        if (it->second & fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_EXPLICIT_UPDATE)
        { //The client need an explicit update
            auto const& tasks = this->g_tasksSource->getTasks();

            pck << static_cast<std::underlying_type<NetworkTypeTasks::SyncType>::type>(
                    fge::NetworkTypeTasks::SyncType::SYNC_FULL);
            pck << static_cast<fge::net::SizeType>(tasks.size());
            for (auto const& task: tasks)
            {
                pck << task->getTypeIndex();
                task->pack(pck);
            }

            it->second &= ~fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_EXPLICIT_UPDATE;
            it->second &= ~fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
        }
        else
        {
            pck << static_cast<std::underlying_type<NetworkTypeTasks::SyncType>::type>(
                    fge::NetworkTypeTasks::SyncType::SYNC_CHECKSUM);
            pck << this->g_tasksSource->getChecksum();
            it->second &= ~fge::net::NetworkPerClientConfigByteMasks::CONFIG_BYTE_MODIFIED_CHECK;
        }
    }
}
void NetworkTypeTasks::packData(fge::net::Packet& pck)
{
    auto const& tasks = this->g_tasksSource->getTasks();

    pck << static_cast<std::underlying_type<NetworkTypeTasks::SyncType>::type>(
            fge::NetworkTypeTasks::SyncType::SYNC_FULL);
    pck << static_cast<fge::net::SizeType>(tasks.size());
    for (auto const& task: tasks)
    {
        pck << task->getTypeIndex();
        task->pack(pck);
    }
}

bool NetworkTypeTasks::check() const
{
    return true;
}
void NetworkTypeTasks::forceCheck()
{
    this->_g_force = true;
}
void NetworkTypeTasks::forceUncheck()
{
    this->_g_force = false;
    this->g_checksumCopy = this->g_tasksSource->getChecksum();
}

//TaskHandler
TaskHandler::TaskHandler(TaskHandler&& r) noexcept :
        g_parentObject(r.g_parentObject),
        g_tasks(std::move(r.g_tasks)),
        g_lastTask(r.g_lastTask),
        g_tasksChecksum(r.g_tasksChecksum)
{}

void TaskHandler::setParentObject(fge::Object& parentObject)
{
    this->g_parentObject = &parentObject;
}
fge::Object* TaskHandler::getParentObject() const
{
    return this->g_parentObject;
}

std::size_t TaskHandler::getTaskSize() const
{
    return this->g_tasks.size();
}

fge::Task* TaskHandler::getMainTask() const
{
    return this->g_tasks.empty() ? nullptr : this->g_tasks.front().get();
}
fge::Task* TaskHandler::getActualTask() const
{
    return this->g_tasks.empty() ? nullptr : this->g_tasks.back().get();
}
std::optional<fge::TaskTypeIndex> TaskHandler::getActualTaskType() const
{
    return this->g_tasks.empty() ? std::nullopt
                                 : std::optional<fge::TaskTypeIndex>{this->g_tasks.back()->getTypeIndex()};
}
void TaskHandler::popTask()
{
    if (!this->g_tasks.empty())
    {
        this->g_tasks.pop_back();
    }
    else
    {
        this->clearLastTask();
    }
    this->computeChecksum();
}
void TaskHandler::clearTasks()
{
    this->g_tasks.clear();
    this->clearLastTask();
    this->computeChecksum();
}

fge::TaskList const& TaskHandler::getTasks() const
{
    return this->g_tasks;
}

std::optional<fge::TaskTypeIndex> TaskHandler::getLastTask() const
{
    return this->g_lastTask;
}
void TaskHandler::clearLastTask()
{
    this->g_lastTask = std::nullopt;
    this->computeChecksum();
}

void TaskHandler::networkRegister(fge::net::NetworkTypeContainer& netList)
{
    netList.push(new fge::NetworkTypeTasks(this));
}

fge::TasksChecksum TaskHandler::getChecksum() const
{
    return this->g_tasksChecksum;
}

void TaskHandler::computeChecksum()
{
    this->g_tasksChecksum = static_cast<fge::TasksChecksum>(this->g_tasks.size());
    for (auto& task: this->g_tasks)
    {
        this->g_tasksChecksum += static_cast<fge::TasksChecksum>(task->getTypeIndex());
    }
}

} // namespace fge

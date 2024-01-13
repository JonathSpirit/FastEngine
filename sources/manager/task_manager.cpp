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

#include "FastEngine/manager/task_manager.hpp"
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace fge::task
{

namespace
{

std::vector<std::unique_ptr<fge::TaskTypeBase>> _dataTasks;
std::unordered_map<std::type_index, fge::TaskTypeIndex> _dataTasksIndex;
fge::TaskTypeIndex _dataIndex{0};

} // namespace

std::optional<fge::TaskTypeIndex> RegisterNewTask(std::unique_ptr<fge::TaskTypeBase> taskType)
{
    _dataTasksIndex[taskType->getType()] = _dataIndex;
    _dataTasks.push_back(std::move(taskType));
    return _dataIndex++;
}
fge::Task* CreateNewTask(fge::TaskTypeIndex index)
{
    if (static_cast<std::size_t>(index) < _dataTasks.size())
    {
        return _dataTasks[static_cast<std::size_t>(index)]->createTask();
    }
    return nullptr;
}
std::optional<fge::TaskTypeIndex> GetTaskIndex(std::type_info const& type)
{
    auto it = _dataTasksIndex.find(type);
    if (it != _dataTasksIndex.end())
    {
        return it->second;
    }
    return std::nullopt;
}

} // namespace fge::task

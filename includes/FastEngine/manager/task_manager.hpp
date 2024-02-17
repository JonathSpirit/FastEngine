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

#ifndef _FGE_TASK_MANAGER_HPP_INCLUDED
#define _FGE_TASK_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include <memory>
#include <optional>
#include <typeinfo>

namespace fge
{

class Task;

using TaskTypeIndex = uint16_t;

class TaskTypeBase
{
public:
    TaskTypeBase() = default;
    TaskTypeBase(TaskTypeBase const& r) = delete;
    TaskTypeBase(TaskTypeBase&& r) noexcept = delete;
    virtual ~TaskTypeBase() = default;

    TaskTypeBase& operator=(TaskTypeBase const& r) = delete;
    TaskTypeBase& operator=(TaskTypeBase&& r) noexcept = delete;

    [[nodiscard]] virtual fge::Task* createTask() const = 0;
    [[nodiscard]] virtual std::type_info const& getType() const = 0;
};

template<class T>
class TaskType : public TaskTypeBase
{
public:
    TaskType() = default;
    ~TaskType() override = default;

    [[nodiscard]] inline fge::Task* createTask() const override { return new T(); }
    [[nodiscard]] inline std::type_info const& getType() const override { return typeid(T); }
};

namespace task
{

/**
 * \brief Register a new task
 *
 * This function should be called at beginning of the program and once per task.
 *
 * \param taskType The task to register using TaskType<T> helper
 * \return The index of the task
 */
FGE_API std::optional<fge::TaskTypeIndex> RegisterNewTask(std::unique_ptr<fge::TaskTypeBase> taskType);
template<class T>
inline std::optional<fge::TaskTypeIndex> RegisterNewTask()
{
    return RegisterNewTask(std::make_unique<fge::TaskType<T>>());
}
/**
 * \brief Create a new task by it's type index
 *
 * \param index The type index of the task
 * \return The new task or nullptr if the index is invalid
 */
[[nodiscard]] FGE_API fge::Task* CreateNewTask(fge::TaskTypeIndex index);
/**
 * \brief Get the task index by it's type
 *
 * \param type The type of the task
 * \return The index of the task or std::nullopt if the type is invalid
 */
[[nodiscard]] FGE_API std::optional<fge::TaskTypeIndex> GetTaskIndex(std::type_info const& type);

} // namespace task
} // namespace fge

#endif // _FGE_TASK_MANAGER_HPP_INCLUDED

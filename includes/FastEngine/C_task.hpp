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

#ifndef _FGE_C_TASK_HPP_INCLUDED
#define _FGE_C_TASK_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_event.hpp"
#include "FastEngine/C_networkType.hpp"
#include "FastEngine/manager/task_manager.hpp"
#include <optional>
#include <vector>

#define FGE_TASK_DEFAULT_GETTER(type_)                                                                                 \
    [[nodiscard]] fge::TaskTypeIndex getTypeIndex() const override                                                     \
    {                                                                                                                  \
        return fge::task::GetTaskIndex(typeid(type_)).value();                                                         \
    }                                                                                                                  \
    [[nodiscard]] static fge::TaskTypeIndex GetTypeIndex()                                                             \
    {                                                                                                                  \
        return fge::task::GetTaskIndex(typeid(type_)).value();                                                         \
    }

namespace fge
{

class Object;

using TasksChecksum = uint16_t;
using TaskTypeIndex = uint16_t;

enum class TaskResult
{
    TASK_RESULT_ERROR,

    TASK_RESULT_UNFINISHED,
    TASK_RESULT_FINISHED,
    TASK_RESULT_SUBTASK_REQUIRED
};

class TaskHandler;

/**
 * \class NetworkTypeTasks
 * \brief Network type for the TaskHandler
 * \ingroup network
 *
 * This class is used to synchronize the tasks of an object.
 * \see TaskHandler
 */
class FGE_API NetworkTypeTasks : public fge::net::NetworkTypeBase
{
public:
    enum SyncType : uint8_t
    {
        SYNC_CHECKSUM,
        SYNC_FULL
    };

    explicit NetworkTypeTasks(fge::TaskHandler* source);
    ~NetworkTypeTasks() override = default;

    void const* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, fge::net::Identity const& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    fge::TaskHandler* g_tasksSource;
    fge::TasksChecksum g_checksumCopy;
};

/**
 * \class Task
 * \brief Base class for all tasks
 * \ingroup objectControl
 *
 * A Task represent an action that can be done by an object. An action can be composed of multiple sub-tasks.
 *
 * This class can also be network aware.
 * \see NetworkTypeTasks
 */
class FGE_API Task
{
public:
    Task() = default;
    virtual ~Task() = default;

    /**
     * \brief Update the task
     *
     * In order to complete a task, it must be updated.
     *
     * This function can return multiple values:
     * - TASK_RESULT_ERROR: The task has encountered an error and must be stopped.
     * - TASK_RESULT_UNFINISHED: The task is not finished yet.
     * - TASK_RESULT_FINISHED: The task is finished.
     * - TASK_RESULT_SUBTASK_REQUIRED: The task is finished but a sub-task is required and has been created.
     *
     * When receiving ERROR, all tasks from the TaskHandler should be cleared by calling TaskHandler::clearTasks().
     * When receiving FINISHED, the top task should be removed by calling TaskHandler::popTask().
     * \see TaskHandler
     *
     * \param taskHandler The TaskHandler that handle this task
     * \param event The event object
     * \param deltaTime The time elapsed since the last update
     * \param scenePtr The scene pointer
     * \return The result of the update
     */
    virtual fge::TaskResult update(fge::TaskHandler& taskHandler,
                                   fge::Event& event,
                                   std::chrono::microseconds const& deltaTime,
                                   fge::Scene* scenePtr) = 0;

    /**
     * \brief Get the type index of the task
     *
     * Every task must have a unique type index that can be retrieved using the task_manager.hpp functions.
     * \see task::RegisterNewTask
     *
     * \return The type index of the task
     */
    [[nodiscard]] virtual fge::TaskTypeIndex getTypeIndex() const = 0;
    /**
     * \brief Get the custom status of the task as a string
     *
     * \return The custom status of the task
     */
    [[nodiscard]] virtual std::string_view getStringStatus() const = 0;

    /**
     * \brief Get the progression of the task as a percentage
     *
     * \return The progression of the task
     */
    [[nodiscard]] inline float getProgression() const { return this->_g_progress; }

    /**
     * \brief Pack the task data into a packet
     *
     * \param pck The packet
     */
    virtual void pack(fge::net::Packet& pck) = 0;
    /**
     * \brief Unpack the task data from a packet and initialize the task
     *
     * \param pck The packet
     */
    virtual void unpackAndInit(fge::net::Packet& pck) = 0;

    /**
     * \brief Get the parent object of the task
     *
     * This is always an object that should generally contain the TaskHandler.
     *
     * \return The parent object of the task
     */
    [[nodiscard]] inline fge::Object* getParentObject() const { return this->_g_parentObject; }

private:
    friend class TaskHandler;
    friend class NetworkTypeTasks;
    inline void setParentObject(fge::Object* parentObject) { this->_g_parentObject = parentObject; }

protected:
    float _g_progress{0.0f};
    fge::Object* _g_parentObject{nullptr};
};

using TaskList = std::vector<std::unique_ptr<fge::Task>>;

/**
 * \class TaskHandler
 * \brief Handle the tasks of an object
 * \ingroup objectControl
 *
 * A TaskHandler is used to handle the tasks of an object.
 * It can have multiple sub-tasks and one main task.
 */
class FGE_API TaskHandler
{
public:
    TaskHandler() = default;
    TaskHandler(TaskHandler const& r) = delete;
    TaskHandler(TaskHandler&& r) noexcept;
    ~TaskHandler() = default;

    TaskHandler& operator=(TaskHandler const& r) = delete;
    TaskHandler& operator=(TaskHandler&& r) noexcept = delete;

    /**
     * \brief Set the parent object of the TaskHandler
     *
     * All created tasks will have this object as parent.
     *
     * \param parentObject The parent object
     */
    void setParentObject(fge::Object& parentObject);
    [[nodiscard]] fge::Object* getParentObject() const;

    [[nodiscard]] std::size_t getTaskSize() const;

    /**
     * \brief Set the main task
     *
     * All tasks is cleared before setting the new main task.
     *
     * \tparam T The type of the task for convenience
     * \param newTask The new task
     * \return The pointer to the new task with type T
     */
    template<class T = fge::Task>
    T* setMainTask(std::unique_ptr<T>&& newTask);
    /**
     * \brief Add a sub-task
     *
     * This function should be called when a task is present and a sub-task is required.
     *
     * \tparam T The type of the task for convenience
     * \param newTask The new task
     * \return The pointer to the new task with type T
     */
    template<class T = fge::Task>
    T* addSubTask(std::unique_ptr<T>&& newTask);
    [[nodiscard]] fge::Task* getMainTask() const;
    [[nodiscard]] fge::Task* getActualTask() const;
    [[nodiscard]] std::optional<fge::TaskTypeIndex> getActualTaskType() const;
    void popTask();
    void clearTasks();

    [[nodiscard]] fge::TaskList const& getTasks() const;

    [[nodiscard]] std::optional<fge::TaskTypeIndex> getLastTask() const;
    void clearLastTask();

    /**
     * \brief Register the network types of the tasks
     *
     * This is a helper function that register the network type using the NetworkTypeTasks class.
     * It should be called inside the Object::networkRegister() function.
     *
     * \param netList The network type container
     */
    void networkRegister(fge::net::NetworkTypeContainer& netList);

    [[nodiscard]] fge::TasksChecksum getChecksum() const;

    fge::CallbackHandler<TaskHandler&> _onMainTaskChanged; ///< Called when the main task is changed

private:
    void computeChecksum();

    fge::Object* g_parentObject{nullptr};
    fge::TaskList g_tasks;
    std::optional<fge::TaskTypeIndex> g_lastTask;
    fge::TasksChecksum g_tasksChecksum{0};
};

} // namespace fge

#include "FastEngine/C_task.inl"

#endif // _FGE_C_TASK_HPP_INCLUDED

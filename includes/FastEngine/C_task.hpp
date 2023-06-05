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
#include <optional>
#include <vector>

#define FGE_TASK_DEFAULT_GETTYPE(type_)                                                                                \
    [[nodiscard]] fge::TaskTypeIndex getType() const override                                                          \
    {                                                                                                                  \
        return fge::task::GetTaskIndex(typeid(type_));                                                                 \
    }                                                                                                                  \
    [[nodiscard]] static fge::TaskTypeIndex GetType()                                                                  \
    {                                                                                                                  \
        return fge::task::GetTaskIndex(typeid(type_));                                                                 \
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

    const void* getSource() const override;

    bool applyData(fge::net::Packet& pck) override;
    void packData(fge::net::Packet& pck, const fge::net::Identity& id) override;
    void packData(fge::net::Packet& pck) override;

    bool check() const override;
    void forceCheck() override;
    void forceUncheck() override;

private:
    fge::TaskHandler* g_tasksSource;
    fge::TasksChecksum g_checksumCopy;
};

class FGE_API Task
{
public:
    Task() = default;
    virtual ~Task() = default;

    virtual fge::TaskResult update(fge::TaskHandler& taskHandler,
                                   fge::Event& event,
                                   std::chrono::microseconds const& deltaTime,
                                   fge::Scene* scenePtr) = 0;

    [[nodiscard]] virtual fge::TaskTypeIndex getTypeIndex() const = 0;
    [[nodiscard]] virtual std::string_view getStringStatus() const = 0;

    [[nodiscard]] inline float getProgression() const { return this->_g_progress; }

    virtual void pack(fge::net::Packet& pck) = 0;
    virtual void unpackAndInit(fge::net::Packet& pck) = 0;

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

class FGE_API TaskHandler
{
public:
    TaskHandler() = default;
    TaskHandler(TaskHandler const& r) = delete;
    TaskHandler(TaskHandler&& r) noexcept;
    ~TaskHandler() = default;

    TaskHandler& operator=(TaskHandler const& r) = delete;
    TaskHandler& operator=(TaskHandler&& r) noexcept = delete;

    void setParentObject(fge::Object& parentObject);
    [[nodiscard]] fge::Object* getParentObject() const;

    [[nodiscard]] std::size_t getTaskSize() const;

    template<class T = fge::Task>
    T* setMainTask(std::unique_ptr<T>&& newTask);
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

    void networkRegister(fge::net::NetworkTypeContainer& netList);

    [[nodiscard]] fge::TasksChecksum getChecksum() const;

    fge::CallbackHandler<TaskHandler&> _onMainTaskChanged;

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

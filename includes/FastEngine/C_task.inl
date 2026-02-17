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

namespace fge
{

template<class T>
T* TaskHandler::setMainTask(std::unique_ptr<T>&& newTask)
{
    this->clearTasks();
    this->g_tasks.push_back(std::move(newTask));
    this->g_tasks.back()->setParentObject(this->g_parentObject);
    this->computeChecksum();
    this->_onMainTaskChanged.call(*this);
    return static_cast<T*>(this->g_tasks.back().get());
}
template<class T>
T* TaskHandler::setMainTask()
{
    return this->setMainTask(std::make_unique<T>());
}
template<class T, class... TArgs>
T* TaskHandler::setMainTaskAndInit(TArgs&&... args)
{
    auto* task = this->setMainTask(std::make_unique<T>());
    task->init(std::forward<TArgs>(args)...);
    return task;
}

template<class T>
T* TaskHandler::addSubTask(std::unique_ptr<T>&& newTask)
{
    this->g_lastTask = newTask->getTypeIndex();
    this->g_tasks.push_back(std::move(newTask));
    this->g_tasks.back()->setParentObject(this->g_parentObject);
    this->computeChecksum();
    return static_cast<T*>(this->g_tasks.back().get());
}
template<class T>
T* TaskHandler::addSubTask()
{
    return this->addSubTask(std::make_unique<T>());
}
template<class T, class... TArgs>
T* TaskHandler::addSubTaskAndInit(TArgs&&... args)
{
    auto* task = this->addSubTask(std::make_unique<T>());
    task->init(std::forward<TArgs>(args)...);
    return task;
}

} // namespace fge

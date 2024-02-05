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

#ifndef _FGE_C_ACCESSLOCK_HPP_INCLUDED
#define _FGE_C_ACCESSLOCK_HPP_INCLUDED

#include "FastEngine/fge_except.hpp"
#include <mutex>

namespace fge
{

/**
 * \class AccessLock
 * \ingroup utility
 * \brief Class that lock a mutex and unlock it only when the object is destroyed
 *
 * This is a strict class that lock a mutex and unlock it only when the object is destroyed and
 * disallow copy, move operations and even the unary operator&.
 *
 * \tparam TMutex The mutex type
 */
template<class TMutex>
class AccessLock
{
public:
    explicit AccessLock(TMutex& mutex) :
            g_mutex(&mutex)
    {
        this->g_mutex->lock();
    }
    ~AccessLock() { this->g_mutex->unlock(); }

    AccessLock(AccessLock const&) = delete;
    AccessLock(AccessLock&&) noexcept = delete;

    AccessLock& operator=(AccessLock const&) = delete;
    AccessLock& operator=(AccessLock&&) noexcept = delete;

    [[nodiscard]] inline bool operator==(TMutex const& r) const { return this->g_mutex == &r; }
    [[nodiscard]] inline bool operator!=(TMutex const& r) const { return this->g_mutex != &r; }

    inline void throwIfDifferent(TMutex const& r) const
    {
        if (this->g_mutex != &r)
        {
            throw fge::Exception("AccessLock: provided mutex is different");
        }
    }

    [[nodiscard]] inline AccessLock<TMutex>* operator&() = delete;

private:
    TMutex* g_mutex;
};

} // namespace fge

#endif // _FGE_C_ACCESSLOCK_HPP_INCLUDED

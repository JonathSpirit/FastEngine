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

namespace fge::manager
{

template<class TData, class TDataBlock>
std::size_t BaseManager<TData, TDataBlock>::size() const
{
    return this->g_data.size();
}

template<class TData, class TDataBlock>
AccessLock<std::mutex> BaseManager<TData, TDataBlock>::acquireLock() const
{
    return AccessLock<std::mutex>(this->g_mutex);
}
template<class TData, class TDataBlock>
typename BaseManager<TData, TDataBlock>::Map::const_iterator
BaseManager<TData, TDataBlock>::begin(AccessLock<std::mutex> const& lock) const
{
    lock.throwIfDifferent(this->g_mutex);
    return this->g_data.begin();
}
template<class TData, class TDataBlock>
typename BaseManager<TData, TDataBlock>::Map::const_iterator
BaseManager<TData, TDataBlock>::end(AccessLock<std::mutex> const& lock) const
{
    lock.throwIfDifferent(this->g_mutex);
    return this->g_data.end();
}

template<class TData, class TDataBlock>
typename BaseManager<TData, TDataBlock>::DataBlockPointer const& BaseManager<TData, TDataBlock>::getBadElement() const
{
    return this->_g_badElement;
}
template<class TData, class TDataBlock>
typename BaseManager<TData, TDataBlock>::DataBlockPointer
BaseManager<TData, TDataBlock>::getElement(std::string_view name) const
{
    if (name.empty())
    {
        return this->_g_badElement;
    }

    std::scoped_lock const lck(this->g_mutex);
    auto it = this->g_data.find(name);

    if (it != this->g_data.end())
    {
        return it->second;
    }
    return this->_g_badElement;
}
template<class TData, class TDataBlock>
bool BaseManager<TData, TDataBlock>::contains(std::string_view name) const
{
    if (name.empty())
    {
        return false;
    }

    std::scoped_lock const lck(this->g_mutex);
    return this->g_data.find(name) != this->g_data.end();
}
template<class TData, class TDataBlock>
bool BaseManager<TData, TDataBlock>::unload(std::string_view name)
{
    if (name.empty())
    {
        return false;
    }

    std::scoped_lock const lck(this->g_mutex);
    auto it = this->g_data.find(name);

    if (it != this->g_data.end())
    {
        it->second->_valid = false;
        it->second->_ptr = this->_g_badElement->_ptr;
        it->second->unload();
        this->g_data.erase(it);
        return true;
    }
    return false;
}
template<class TData, class TDataBlock>
void BaseManager<TData, TDataBlock>::unloadAll()
{
    std::scoped_lock const lck(this->g_mutex);

    for (auto& data: this->g_data)
    {
        data.second->_valid = false;
        data.second->_ptr = this->_g_badElement->_ptr;
        data.second->unload();
    }
    this->g_data.clear();
}
template<class TData, class TDataBlock>
bool BaseManager<TData, TDataBlock>::push(std::string_view name, DataBlockPointer block)
{
    if (name.empty() || !block || !block->_ptr)
    {
        return false;
    }

    if (this->contains(name))
    {
        return false;
    }

    std::scoped_lock const lck(this->g_mutex);
    this->g_data.emplace(name, std::move(block));
    return true;
}

} // namespace fge::manager

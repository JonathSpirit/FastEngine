/*
 * Copyright 2025 Guillaume Guillet
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

//BaseManager

template<class TData, class TDataBlock>
bool BaseManager<TData, TDataBlock>::isInitialized()
{
    return this->_g_badElement != nullptr;
}
template<class TData, class TDataBlock>
void BaseManager<TData, TDataBlock>::uninitialize()
{
    if (!this->isInitialized())
    {
        return;
    }

    this->unloadAll();
    this->_g_badElement.reset();
}

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

//BaseDataAccessor

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::BaseDataAccessor() :
        g_data(TDataAccessorManagerInfo()().getBadElement()),
        g_name(FGE_MANAGER_BAD)
{}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::BaseDataAccessor(std::string_view name) :
        g_data(TDataAccessorManagerInfo()().getElement(name)),
        g_name(name)
{}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::BaseDataAccessor(char const name[]) :
        g_data(TDataAccessorManagerInfo()().getElement(name)),
        g_name(name)
{}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::BaseDataAccessor(std::string name) :
        g_data(TDataAccessorManagerInfo()().getElement(name)),
        g_name(std::move(name))
{}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::BaseDataAccessor(SharedDataType data) :
        g_data(std::move(data)),
        g_name(FGE_MANAGER_BAD)
{}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::BaseDataAccessor(SharedType data)
    requires(TOption == DataAccessorOptions::ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER)
        :
        g_data(std::move(data)),
        g_name(FGE_MANAGER_BAD)
{}

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
void BaseDataAccessor<TDataAccessorManagerInfo, TOption>::clear()
{
    this->g_data = TDataAccessorManagerInfo()().getBadElement();
    this->g_name = FGE_MANAGER_BAD;
}

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
void BaseDataAccessor<TDataAccessorManagerInfo, TOption>::reload()
{
    this->g_data = TDataAccessorManagerInfo()().getElement(this->g_name);
}

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
bool BaseDataAccessor<TDataAccessorManagerInfo, TOption>::valid() const
{
    if constexpr (TOption == DataAccessorOptions::BLOCKPOINTER_ONLY)
    {
        return this->g_data->_valid;
    }
    else
    {
        if (std::holds_alternative<SharedDataType>(this->g_data))
        {
            return std::get<SharedDataType>(this->g_data)->_valid;
        }
        return static_cast<bool>(std::get<SharedType>(this->g_data));
    }
}

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
typename BaseDataAccessor<TDataAccessorManagerInfo, TOption>::SharedDataType const&
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::getSharedBlock() const
{
    if constexpr (TOption == DataAccessorOptions::BLOCKPOINTER_ONLY)
    {
        return this->g_data;
    }
    else
    {
        if (std::holds_alternative<SharedDataType>(this->g_data))
        {
            return std::get<SharedDataType>(this->g_data);
        }
        return TDataAccessorManagerInfo()().getBadElement();
    }
}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
typename BaseDataAccessor<TDataAccessorManagerInfo, TOption>::SharedType const&
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::getSharedData() const
    requires(TOption == DataAccessorOptions::ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER)
{
    if (std::holds_alternative<SharedDataType>(this->g_data))
    {
        return std::get<SharedDataType>(this->g_data)->_ptr;
    }

    if (std::get<SharedType>(this->g_data))
    {
        return std::get<SharedType>(this->g_data);
    }
    return TDataAccessorManagerInfo()().getBadElement()->_ptr;
}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
std::string const& BaseDataAccessor<TDataAccessorManagerInfo, TOption>::getName() const
{
    return this->g_name;
}

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
auto& BaseDataAccessor<TDataAccessorManagerInfo, TOption>::operator=(std::string_view name)
{
    if (this->g_name == name)
    {
        return *this;
    }

    this->g_name = name;
    this->g_data = TDataAccessorManagerInfo()().getElement(name);
    return *this;
}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
auto& BaseDataAccessor<TDataAccessorManagerInfo, TOption>::operator=(char const name[])
{
    if (this->g_name == name)
    {
        return *this;
    }

    this->g_name = name;
    this->g_data = TDataAccessorManagerInfo()().getElement(name);
    return *this;
}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
auto& BaseDataAccessor<TDataAccessorManagerInfo, TOption>::operator=(std::string name)
{
    if (this->g_name == name)
    {
        return *this;
    }

    this->g_name = std::move(name);
    this->g_data = TDataAccessorManagerInfo()().getElement(this->g_name);
    return *this;
}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
auto& BaseDataAccessor<TDataAccessorManagerInfo, TOption>::operator=(SharedDataType data)
{
    this->g_name = FGE_MANAGER_BAD;
    this->g_data = std::move(data);
    return *this;
}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
auto& BaseDataAccessor<TDataAccessorManagerInfo, TOption>::operator=(SharedType data)
    requires(TOption == DataAccessorOptions::ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER)
{
    this->g_name = FGE_MANAGER_BAD;
    this->g_data = std::move(data);
    return *this;
}

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
typename TDataAccessorManagerInfo::Manager::DataType* BaseDataAccessor<TDataAccessorManagerInfo, TOption>::retrieve()
{
    if constexpr (TOption == DataAccessorOptions::BLOCKPOINTER_ONLY)
    {
        return this->g_data->_ptr.get();
    }
    else
    {
        if (std::holds_alternative<SharedDataType>(this->g_data))
        {
            return std::get<SharedDataType>(this->g_data)->_ptr.get();
        }

        if (std::get<SharedType>(this->g_data))
        {
            return std::get<SharedType>(this->g_data).get();
        }
        return TDataAccessorManagerInfo()().getBadElement()->_ptr.get();
    }
}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
typename TDataAccessorManagerInfo::Manager::DataType const*
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::retrieve() const
{
    if constexpr (TOption == DataAccessorOptions::BLOCKPOINTER_ONLY)
    {
        return this->g_data->_ptr.get();
    }
    else
    {
        if (std::holds_alternative<SharedDataType>(this->g_data))
        {
            return std::get<SharedDataType>(this->g_data)->_ptr.get();
        }

        if (std::get<SharedType>(this->g_data))
        {
            return std::get<SharedType>(this->g_data).get();
        }
        return TDataAccessorManagerInfo()().getBadElement()->_ptr.get();
    }
}

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
typename TDataAccessorManagerInfo::Manager::DataType*
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::retrieveValid()
{
    if constexpr (TOption == DataAccessorOptions::BLOCKPOINTER_ONLY)
    {
        return this->g_data->_valid ? this->g_data->_ptr.get() : nullptr;
    }
    else
    {
        if (std::holds_alternative<SharedDataType>(this->g_data))
        {
            return std::get<SharedDataType>(this->g_data)->_valid ? std::get<SharedDataType>(this->g_data)->_ptr.get()
                                                                  : nullptr;
        }

        if (std::get<SharedType>(this->g_data))
        {
            return std::get<SharedType>(this->g_data).get();
        }
        return nullptr;
    }
}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
typename TDataAccessorManagerInfo::Manager::DataType const*
BaseDataAccessor<TDataAccessorManagerInfo, TOption>::retrieveValid() const
{
    if constexpr (TOption == DataAccessorOptions::BLOCKPOINTER_ONLY)
    {
        return this->g_data->_valid ? this->g_data->_ptr.get() : nullptr;
    }
    else
    {
        if (std::holds_alternative<SharedDataType>(this->g_data))
        {
            return std::get<SharedDataType>(this->g_data)->_valid ? std::get<SharedDataType>(this->g_data)->_ptr.get()
                                                                  : nullptr;
        }

        if (std::get<SharedType>(this->g_data))
        {
            return std::get<SharedType>(this->g_data).get();
        }
        return nullptr;
    }
}

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
static net::Packet const& operator>>(net::Packet const& pck, BaseDataAccessor<TDataAccessorManagerInfo, TOption>& data)
{
    std::string name;
    pck >> name;
    data = std::move(name);
    return pck;
}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
static net::Packet& operator<<(net::Packet& pck, BaseDataAccessor<TDataAccessorManagerInfo, TOption> const& data)
{
    return pck << data.getName();
}

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
static void to_json(nlohmann::json& j, BaseDataAccessor<TDataAccessorManagerInfo, TOption> const& p)
{
    j = p.getName();
}
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
static void from_json(nlohmann::json const& j, BaseDataAccessor<TDataAccessorManagerInfo, TOption>& p)
{
    std::string name;
    j.get_to(name);
    p = std::move(name);
}

} // namespace fge::manager

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

#ifndef _FGE_C_BASEMANAGER_HPP_INCLUDED
#define _FGE_C_BASEMANAGER_HPP_INCLUDED

#include "FastEngine/fge_except.hpp"
#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#define FGE_MANAGER_BAD std::string_view()

namespace fge::manager
{

template<class TData>
struct BaseDataBlock
{
    virtual ~BaseDataBlock() = default;
    using DataPointer = std::shared_ptr<TData>;

    inline virtual void unload() {}

    DataPointer _ptr;
    bool _valid{false};
    std::filesystem::path _path;
};

template<class TData, class TDataBlock = BaseDataBlock<TData>>
class BaseManager
{
public:
    using DataType = TData;
    using DataBlockPointer = std::shared_ptr<TDataBlock>;
    using Map = std::map<std::string,
                         DataBlockPointer,
                         std::less<>>; //TODO: should be std::unordered_map once fge drop c++17 compatibility

    BaseManager() = default;
    BaseManager(BaseManager const& r) = delete;
    BaseManager(BaseManager&& r) noexcept = delete;
    virtual ~BaseManager() = default;

    BaseManager& operator=(BaseManager const& r) = delete;
    BaseManager& operator=(BaseManager&& r) noexcept = delete;

    virtual bool initialize() = 0;
    [[nodiscard]] virtual bool isInitialized() = 0;
    virtual void destroy() = 0;

    [[nodiscard]] std::size_t size() const;

    [[nodiscard]] std::unique_lock<std::mutex> acquireLock() const;
    [[nodiscard]] typename Map::const_iterator begin(std::unique_lock<std::mutex> const& lock) const;
    [[nodiscard]] typename Map::const_iterator end(std::unique_lock<std::mutex> const& lock) const;

    [[nodiscard]] DataBlockPointer const& getBadElement() const;

    [[nodiscard]] DataBlockPointer getElement(std::string_view name) const;
    [[nodiscard]] bool contains(std::string_view name) const;
    [[nodiscard]] bool unload(std::string_view name);
    void unloadAll();

    bool push(std::string_view name, DataBlockPointer block);

private:
    Map g_data;
    mutable std::mutex g_mutex;

protected:
    DataBlockPointer _g_badElement;
};

} // namespace fge::manager

#include "C_baseManager.inl"

#endif // _FGE_C_BASEMANAGER_HPP_INCLUDED

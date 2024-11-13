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
#include "FastEngine/string_hash.hpp"
#include <cstddef>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

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

/**
 * \class BaseManager
 * \ingroup managers
 * \brief Base class for all managers
 *
 * A manager (or registry) is a class that manages a set of resources with a string key.
 * This class is thread-safe.
 */
template<class TData, class TDataBlock = BaseDataBlock<TData>>
class BaseManager
{
public:
    using DataType = TData;
    using DataBlockPointer = std::shared_ptr<TDataBlock>;
    using Map = std::unordered_map<std::string, DataBlockPointer, StringHash, std::equal_to<>>;

    BaseManager() = default;
    BaseManager(BaseManager const& r) = delete;
    BaseManager(BaseManager&& r) noexcept = delete;
    virtual ~BaseManager() = default;

    BaseManager& operator=(BaseManager const& r) = delete;
    BaseManager& operator=(BaseManager&& r) noexcept = delete;

    /**
     * \brief Initialize the manager
     *
     * The philosophy with this class is to always return a "valid" element even if the element is not found.
     * When this method is called, a "bad" element is created and stored in the manager.
     *
     * \warning Note that there is no verification about the above statement, and you can always create your own manager without a "bad" element.
     *
     * return \b true if the manager is initialized, \b false otherwise
     */
    virtual bool initialize() = 0;
    [[nodiscard]] virtual bool isInitialized() = 0;
    virtual void uninitialize() = 0;

    /**
     * \brief Get the number of elements in the manager
     *
     * return The number of elements in the manager without the "bad" element
     */
    [[nodiscard]] std::size_t size() const;

    /**
     * \brief Acquire a unique lock, with the manager mutex
     *
     * In order to use iterators, you have to acquire a unique lock from this
     * function.
     * The lock is not differed and will lock the mutex.
     *
     * \return A unique lock bound to this mutex
     */
    [[nodiscard]] std::unique_lock<std::mutex> acquireLock() const;
    /**
     * \brief Get the "begin" iterator of the manager
     *
     * You have to provide a valid reference to a unique lock acquire with
     * the function AcquireLock().
     * This function will throw if one of this is not respected :
     * - The lock does not own the associated mutex.
     * - The mutex pointer of the lock does not correspond to this mutex.
     *
     * \param lock A unique lock bound to this mutex
     * \return The "begin" iterator of the texture manager
     */
    [[nodiscard]] typename Map::const_iterator begin(std::unique_lock<std::mutex> const& lock) const;
    /**
     * \brief Get the "end" iterator of the manager
     *
     * \see begin()
     *
     * \param lock A unique lock bound to this mutex
     * \return The "end" iterator of the manager
     */
    [[nodiscard]] typename Map::const_iterator end(std::unique_lock<std::mutex> const& lock) const;

    /**
     * \brief Get the "bad" element
     *
     * A bad element is a "valid" default resource that is returned when the requested resource is not found.
     * return The "bad" element
     */
    [[nodiscard]] DataBlockPointer const& getBadElement() const;

    /**
     * \brief Get the resource with the given name
     *
     * \param name The name of the resource to get
     * \return The resource with the given name or the bad resource if not found
     */
    [[nodiscard]] DataBlockPointer getElement(std::string_view name) const;
    [[nodiscard]] bool contains(std::string_view name) const;
    [[nodiscard]] bool unload(std::string_view name);
    void unloadAll();

    /**
     * \brief Add a user handled resource
     *
     * \param name The name of the texture to add
     * \param block The block data to add
     * \return \b true if the resource was added, \b false otherwise
     */
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

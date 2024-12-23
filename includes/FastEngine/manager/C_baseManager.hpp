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

#include "FastEngine/C_accessLock.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/network/C_packet.hpp"
#include "FastEngine/string_hash.hpp"
#include "json.hpp"
#include <cstddef>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>

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
    using DataBlockType = TDataBlock;
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
    virtual void uninitialize();

    /**
     * \brief Get the number of elements in the manager
     *
     * return The number of elements in the manager without the "bad" element
     */
    [[nodiscard]] std::size_t size() const;

    /**
     * \brief Acquire a AccessLock, with the manager mutex
     *
     * In order to use iterators, you have to acquire a unique lock from this
     * function.
     * The lock is not differed and will lock the mutex.
     *
     * \return A AccessLock bound to this mutex
     */
    [[nodiscard]] AccessLock<std::mutex> acquireLock() const;
    /**
     * \brief Get the "begin" iterator of the manager
     *
     * You have to provide a valid reference to a AccessLock acquired with
     * the function AcquireLock().
     * This function will throw if one of this is not respected :
     * - The mutex pointer of the lock does not correspond to this mutex.
     *
     * \param lock A AccessLock bound to this mutex
     * \return The "begin" iterator of the texture manager
     */
    [[nodiscard]] typename Map::const_iterator begin(AccessLock<std::mutex> const& lock) const;
    /**
     * \brief Get the "end" iterator of the manager
     *
     * \see begin()
     *
     * \param lock A AccessLock bound to this mutex
     * \return The "end" iterator of the manager
     */
    [[nodiscard]] typename Map::const_iterator end(AccessLock<std::mutex> const& lock) const;

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
    bool unload(std::string_view name);
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

enum class DataAccessorOptions
{
    BLOCKPOINTER_ONLY,
    ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER
};

template<class TManager, TManager* TGlobalManager>
struct GlobalDataAccessorManagerInfo
{
    using Manager = TManager;
    [[nodiscard]] inline constexpr TManager& operator()() const { return *TGlobalManager; }
};

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
class BaseDataAccessor
{
public:
    using SharedDataType = typename TDataAccessorManagerInfo::Manager::DataBlockPointer;
    using SharedType = typename TDataAccessorManagerInfo::Manager::DataBlockType::DataPointer;

    BaseDataAccessor();
    /**
     * \brief Get resource by its name
     *
     * \param name The name of the loaded resource
     */
    BaseDataAccessor(std::string_view name);
    BaseDataAccessor(char const name[]);
    BaseDataAccessor(std::string name);
    /**
     * \brief Get resource from a user provided data block.
     *
     * \param data The user provided data block
     */
    BaseDataAccessor(SharedDataType data);
    /**
     * \brief Get resource from a user provided data pointer.
     *
     * \param data The user provided data pointer
     */
    BaseDataAccessor(SharedType data)
        requires(TOption == DataAccessorOptions::ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER);
    virtual ~BaseDataAccessor() = default;

    /**
     * \brief Clear the resource
     *
     * This method clear the resource by setting it to the default/bad "valid" resource.
     */
    virtual void clear();

    /**
     * \brief Reload the cached resource from the same name
     */
    void reload();

    /**
     * \brief Check if the font is valid (not unloaded)
     *
     * \return \b True if the font is valid, \b false otherwise
     */
    [[nodiscard]] bool valid() const;

    /**
     * \brief Get the resource block data
     *
     * \warning If ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER is set, this method can a "bad" element.
     *
     * \return The resource block data
     */
    [[nodiscard]] SharedDataType const& getSharedBlock() const;
    /**
     * \brief Get the shared resource data
     *
     * \return The shared resource data
     */
    [[nodiscard]] SharedType const& getSharedData() const
        requires(TOption == DataAccessorOptions::ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER);
    /**
     * \brief Get the name of the resource
     *
     * \return The name of the resource, or an empty string if the resource is not valid
     */
    [[nodiscard]] std::string const& getName() const;

    auto& operator=(std::string_view name);
    auto& operator=(char const name[]);
    auto& operator=(std::string name);
    auto& operator=(SharedDataType data);
    auto& operator=(SharedType data)
        requires(TOption == DataAccessorOptions::ALLOW_VARIANT_OF_DATAPOINTER_AND_BLOCKPOINTER);

    /**
     * \brief Retrieve the raw shared pointer from the current resource
     *
     * \warning Will never be \b nullptr if the manager was correctly initialized.
     *
     * \return The raw resource pointer
     */
    [[nodiscard]] typename TDataAccessorManagerInfo::Manager::DataType* retrieve();
    [[nodiscard]] typename TDataAccessorManagerInfo::Manager::DataType const* retrieve() const;

    /**
     * \brief Retrieve the raw shared pointer from the current resource only if the resource is valid
     *
     * \return The raw resource pointer, or \b nullptr if the resource is not valid
     */
    [[nodiscard]] typename TDataAccessorManagerInfo::Manager::DataType* retrieveValid();
    [[nodiscard]] typename TDataAccessorManagerInfo::Manager::DataType const* retrieveValid() const;

private:
    using VariantType = std::variant<SharedDataType, SharedType>;
    using Type = std::conditional_t<TOption == DataAccessorOptions::BLOCKPOINTER_ONLY, SharedDataType, VariantType>;

    Type g_data;
    std::string g_name;
};

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
static net::Packet const& operator>>(net::Packet const& pck, BaseDataAccessor<TDataAccessorManagerInfo, TOption>& data);
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
static net::Packet& operator<<(net::Packet& pck, BaseDataAccessor<TDataAccessorManagerInfo, TOption> const& data);

template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
static void to_json(nlohmann::json& j, BaseDataAccessor<TDataAccessorManagerInfo, TOption> const& p);
template<class TDataAccessorManagerInfo, DataAccessorOptions TOption>
static void from_json(nlohmann::json const& j, BaseDataAccessor<TDataAccessorManagerInfo, TOption>& p);

} // namespace fge::manager

#include "C_baseManager.inl"

#endif // _FGE_C_BASEMANAGER_HPP_INCLUDED

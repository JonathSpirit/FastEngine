/*
 * Copyright 2022 Guillaume Guillet
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

#include "FastEngine/manager/reg_manager.hpp"
#include "private/string_hash.hpp"

#include "FastEngine/C_scene.hpp"
#include <unordered_map>
#include <vector>

namespace fge::reg
{

namespace
{

using ClassNameMapType = std::unordered_map<std::string, fge::reg::ClassId, fge::priv::string_hash, std::equal_to<>>;
using ClassIdMapType = std::vector<std::unique_ptr<fge::reg::BaseStamp>>;

ClassNameMapType _dataClassNameMap;
ClassIdMapType _dataClassIdMap;

} // namespace

void ClearAll()
{
    _dataClassIdMap.clear();
    _dataClassNameMap.clear();
}

bool RegisterNewClass(std::unique_ptr<fge::reg::BaseStamp>&& newStamp)
{
    if (fge::reg::Check(newStamp->getClassName()))
    {
        return false;
    }

    _dataClassNameMap[newStamp->getClassName()] = static_cast<fge::reg::ClassId>(_dataClassIdMap.size());
    _dataClassIdMap.push_back(std::move(newStamp));

    return true;
}

bool Check(std::string_view className)
{
    return _dataClassNameMap.find(className) != _dataClassNameMap.cend();
}
bool Check(fge::reg::ClassId classId)
{
    return classId < _dataClassIdMap.size();
}

fge::Object* Duplicate(const fge::Object* obj)
{
    auto it = _dataClassNameMap.find(obj->getClassName());

    if (it != _dataClassNameMap.cend())
    {
        return _dataClassIdMap[it->second]->duplicate(obj);
    }
    return nullptr;
}

bool Replace(std::string_view className, std::unique_ptr<fge::reg::BaseStamp>&& newStamp)
{
    auto it = _dataClassNameMap.find(className);

    if (it != _dataClassNameMap.cend())
    {
        _dataClassIdMap[it->second] = std::move(newStamp);
        return true;
    }
    return false;
}
bool Replace(fge::reg::ClassId classId, std::unique_ptr<fge::reg::BaseStamp>&& newStamp)
{
    if (classId < _dataClassIdMap.size())
    {
        _dataClassIdMap[classId] = std::move(newStamp);
        return true;
    }
    return false;
}

std::size_t GetRegisterSize()
{
    return _dataClassIdMap.size();
}

fge::Object* GetNewClassOf(std::string_view className)
{
    auto it = _dataClassNameMap.find(className);

    if (it != _dataClassNameMap.cend())
    {
        return _dataClassIdMap[it->second]->createNew();
    }
    return nullptr;
}
fge::Object* GetNewClassOf(fge::reg::ClassId classId)
{
    if (classId < _dataClassIdMap.size())
    {
        return _dataClassIdMap[classId]->createNew();
    }
    return nullptr;
}

fge::reg::ClassId GetClassId(std::string_view className)
{
    auto it = _dataClassNameMap.find(className);

    if (it != _dataClassNameMap.cend())
    {
        return it->second;
    }
    return FGE_REG_BADCLASSID;
}
std::string GetClassName(fge::reg::ClassId classId)
{
    if (classId < _dataClassIdMap.size())
    {
        return _dataClassIdMap[classId]->getClassName();
    }
    return FGE_OBJ_BADCLASSNAME;
}

fge::reg::BaseStamp* GetStampOf(std::string_view className)
{
    auto it = _dataClassNameMap.find(className);

    if (it != _dataClassNameMap.cend())
    {
        return _dataClassIdMap[it->second].get();
    }
    return nullptr;
}
fge::reg::BaseStamp* GetStampOf(fge::reg::ClassId classId)
{
    if (classId < _dataClassIdMap.size())
    {
        return _dataClassIdMap[classId].get();
    }
    return nullptr;
}

} // namespace fge::reg

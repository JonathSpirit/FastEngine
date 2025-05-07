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

#ifndef _FGE_REG_MANAGER_HPP_INCLUDED
#define _FGE_REG_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"

#include "FastEngine/object/C_object.hpp"
#include <memory>
#include <string>

#define FGE_REG_BADCLASSID std::numeric_limits<fge::reg::ClassId>::max()

namespace fge::reg
{

using ClassId = uint16_t;

class BaseStamp
{
public:
    BaseStamp() = default;
    virtual ~BaseStamp() = default;

    [[nodiscard]] virtual fge::Object* createNew() const = 0;
    [[nodiscard]] virtual fge::Object* duplicate(fge::Object const* obj) const = 0;

    [[nodiscard]] std::string const& getClassName() const { return this->g_className; }

protected:
    std::string g_className;
};
template<class T>
class Stamp : public BaseStamp
{
public:
    Stamp()
    {
        T obj;
        this->g_className = obj.getClassName();
    }

    [[nodiscard]] fge::Object* createNew() const final { return new T(); }
    [[nodiscard]] fge::Object* duplicate(fge::Object const* obj) const final
    {
        return new T(*reinterpret_cast<T const*>(obj));
    }
};

FGE_API void ClearAll();

FGE_API bool RegisterNewClass(std::unique_ptr<fge::reg::BaseStamp>&& newStamp);
template<class T>
inline bool RegisterNewClass()
{
    return fge::reg::RegisterNewClass(std::make_unique<fge::reg::Stamp<T>>());
}

FGE_API bool Check(std::string_view className);
FGE_API bool Check(fge::reg::ClassId classId);

FGE_API fge::Object* Duplicate(fge::Object const* obj);

FGE_API bool Replace(std::string_view className, std::unique_ptr<fge::reg::BaseStamp>&& newStamp);
FGE_API bool Replace(fge::reg::ClassId classId, std::unique_ptr<fge::reg::BaseStamp>&& newStamp);

FGE_API std::size_t GetRegisterSize();

FGE_API fge::Object* GetNewClassOf(std::string_view className);
FGE_API fge::Object* GetNewClassOf(fge::reg::ClassId classId);

FGE_API fge::reg::ClassId GetClassId(std::string_view className);
FGE_API std::string GetClassName(fge::reg::ClassId classId);

FGE_API fge::reg::BaseStamp* GetStampOf(std::string_view className);
FGE_API fge::reg::BaseStamp* GetStampOf(fge::reg::ClassId classId);

} // namespace fge::reg


#endif // _FGE_REG_MANAGER_HPP_INCLUDED

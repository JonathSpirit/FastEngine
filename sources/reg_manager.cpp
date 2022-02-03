#include "FastEngine/reg_manager.hpp"
#include "FastEngine/extra_string.hpp"

#include <unordered_map>
#include <vector>
#include "FastEngine/C_scene.hpp"

namespace fge
{
namespace reg
{

namespace
{

const std::string __badClassName = FGE_OBJ_BADCLASSNAME;

using ClassNameMapType = std::unordered_map<std::string, fge::reg::ClassId>;
using ClassIdMapType = std::vector<std::unique_ptr<fge::reg::BaseStamp> >;

ClassNameMapType __dataClassNameMap;
ClassIdMapType __dataClassIdMap;

}//end

void FGE_API ClearAll()
{
    __dataClassIdMap.clear();
    __dataClassNameMap.clear();
}

bool FGE_API RegisterNewClass(std::unique_ptr<fge::reg::BaseStamp>&& newStamp)
{
    if ( fge::reg::Check( newStamp->getClassName() ) )
    {
        return false;
    }

    __dataClassNameMap[newStamp->getClassName()] = static_cast<fge::reg::ClassId>(__dataClassIdMap.size());
    __dataClassIdMap.push_back(std::move(newStamp));

    return true;
}

bool FGE_API Check(const std::string& className)
{
    return __dataClassNameMap.find(className) != __dataClassNameMap.cend();
}
bool FGE_API Check(fge::reg::ClassId classId)
{
    return classId < __dataClassIdMap.size();
}

fge::Object* FGE_API Duplicate(const fge::Object* obj)
{
    fge::reg::ClassNameMapType::const_iterator it = __dataClassNameMap.find(obj->getClassName());

    if (it != __dataClassNameMap.cend())
    {
        return __dataClassIdMap[it->second]->duplicate(obj);
    }
    return new fge::Object();
}

bool FGE_API Replace(const std::string& className, std::unique_ptr<fge::reg::BaseStamp>&& newStamp)
{
    fge::reg::ClassNameMapType::const_iterator it = __dataClassNameMap.find(className);

    if (it != __dataClassNameMap.cend())
    {
        __dataClassIdMap[it->second] = std::move(newStamp);
        return true;
    }
    return false;
}
bool FGE_API Replace(fge::reg::ClassId classId, std::unique_ptr<fge::reg::BaseStamp>&& newStamp)
{
    if (classId < __dataClassIdMap.size())
    {
        __dataClassIdMap[classId] = std::move(newStamp);
        return true;
    }
    return false;
}

std::size_t FGE_API GetRegisterSize()
{
    return __dataClassIdMap.size();
}

fge::Object* FGE_API GetNewClassOf(const std::string& className)
{
    fge::reg::ClassNameMapType::const_iterator it = __dataClassNameMap.find(className);

    if (it != __dataClassNameMap.cend())
    {
        return __dataClassIdMap[it->second]->createNew();
    }
    return new fge::Object();
}
fge::Object* FGE_API GetNewClassOf(fge::reg::ClassId classId)
{
    if (classId < __dataClassIdMap.size())
    {
        return __dataClassIdMap[classId]->createNew();
    }
    return new fge::Object();
}

fge::reg::ClassId FGE_API GetClassId(const std::string& className)
{
    fge::reg::ClassNameMapType::const_iterator it = __dataClassNameMap.find(className);

    if (it != __dataClassNameMap.cend())
    {
        return it->second;
    }
    return FGE_REG_BADCLASSID;
}
const std::string& FGE_API GetClassName(fge::reg::ClassId classId)
{
    if (classId < __dataClassIdMap.size())
    {
        return __dataClassIdMap[classId]->getClassName();
    }
    return __badClassName;
}

fge::reg::BaseStamp* FGE_API GetStampOf(const std::string& className)
{
    fge::reg::ClassNameMapType::const_iterator it = __dataClassNameMap.find(className);

    if (it != __dataClassNameMap.cend())
    {
        return __dataClassIdMap[it->second].get();
    }
    return nullptr;
}
fge::reg::BaseStamp* FGE_API GetStampOf(fge::reg::ClassId classId)
{
    if (classId < __dataClassIdMap.size())
    {
        return __dataClassIdMap[classId].get();
    }
    return nullptr;
}

}//end reg
}//end fge

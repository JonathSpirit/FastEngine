#ifndef _FGE_REG_MANAGER_HPP_INCLUDED
#define _FGE_REG_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <memory>
#include <string>
#include <FastEngine/C_object.hpp>

#define FGE_REG_BADCLASSID std::numeric_limits<fge::reg::ClassId>::max()

namespace fge
{
namespace reg
{

using ClassId = uint16_t;

class BaseStamp
{
public:
    BaseStamp() = default;
    virtual ~BaseStamp() = default;

    virtual fge::Object* createNew() const = 0;
    virtual fge::Object* duplicate(const fge::Object* obj) const = 0;

    const std::string& getClassName() const
    {
        return this->g_className;
    }

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

    fge::Object* createNew() const final { return new T(); }
    fge::Object* duplicate(const fge::Object* obj) const final { return new T(*reinterpret_cast<const T*>(obj)); }
};

void FGE_API ClearAll();

bool FGE_API RegisterNewClass(std::unique_ptr<fge::reg::BaseStamp>&& newStamp);

bool FGE_API Check(const std::string& className);
bool FGE_API Check(fge::reg::ClassId classId);

fge::Object* FGE_API Duplicate(const fge::Object* obj);

bool FGE_API Replace(const std::string& className, std::unique_ptr<fge::reg::BaseStamp>&& newStamp);
bool FGE_API Replace(fge::reg::ClassId classId, std::unique_ptr<fge::reg::BaseStamp>&& newStamp);

std::size_t FGE_API GetRegisterSize();

fge::Object* FGE_API GetNewClassOf(const std::string& className);
fge::Object* FGE_API GetNewClassOf(fge::reg::ClassId classId);

fge::reg::ClassId FGE_API GetClassId(const std::string& className);
const std::string& FGE_API GetClassName(fge::reg::ClassId classId);

fge::reg::BaseStamp* FGE_API GetStampOf(const std::string& className);
fge::reg::BaseStamp* FGE_API GetStampOf(fge::reg::ClassId classId);

}//end reg
}//end fge


#endif // _FGE_REG_MANAGER_HPP_INCLUDED

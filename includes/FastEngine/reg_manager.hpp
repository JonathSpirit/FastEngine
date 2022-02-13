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

    [[nodiscard]] virtual fge::Object* createNew() const = 0;
    [[nodiscard]] virtual fge::Object* duplicate(const fge::Object* obj) const = 0;

    [[nodiscard]] const std::string& getClassName() const
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

    [[nodiscard]] fge::Object* createNew() const final { return new T(); }
    [[nodiscard]] fge::Object* duplicate(const fge::Object* obj) const final { return new T(*reinterpret_cast<const T*>(obj)); }
};

FGE_API void ClearAll();

FGE_API bool RegisterNewClass(std::unique_ptr<fge::reg::BaseStamp>&& newStamp);

FGE_API bool Check(const std::string& className);
FGE_API bool Check(fge::reg::ClassId classId);

FGE_API fge::Object* Duplicate(const fge::Object* obj);

FGE_API bool Replace(const std::string& className, std::unique_ptr<fge::reg::BaseStamp>&& newStamp);
FGE_API bool Replace(fge::reg::ClassId classId, std::unique_ptr<fge::reg::BaseStamp>&& newStamp);

FGE_API std::size_t GetRegisterSize();

FGE_API fge::Object* GetNewClassOf(const std::string& className);
FGE_API fge::Object* GetNewClassOf(fge::reg::ClassId classId);

FGE_API fge::reg::ClassId GetClassId(const std::string& className);
FGE_API std::string GetClassName(fge::reg::ClassId classId);

FGE_API fge::reg::BaseStamp* GetStampOf(const std::string& className);
FGE_API fge::reg::BaseStamp* GetStampOf(fge::reg::ClassId classId);

}//end reg
}//end fge


#endif // _FGE_REG_MANAGER_HPP_INCLUDED

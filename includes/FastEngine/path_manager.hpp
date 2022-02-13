#ifndef _FGE_PATH_MANAGER_HPP_INCLUDED
#define _FGE_PATH_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <string>

namespace fge
{
namespace path
{

FGE_API const std::string& Get(const std::string& name);

FGE_API std::size_t GetPathSize();

FGE_API void Remove(const std::string& name);

FGE_API bool Check(const std::string& name);

FGE_API bool New(const std::string& name, const std::string& path);

FGE_API bool Replace(const std::string& name, const std::string& path);

}//end path
}//end fge


#endif // _FGE_PATH_MANAGER_HPP_INCLUDED

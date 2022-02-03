#ifndef _FGE_PATH_MANAGER_HPP_INCLUDED
#define _FGE_PATH_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <string>

namespace fge
{
namespace path
{

const std::string& FGE_API Get(const std::string& name);

std::size_t FGE_API GetPathSize();

void FGE_API Remove(const std::string& name);

bool FGE_API Check(const std::string& name);

bool FGE_API New(const std::string& name, const std::string& path);

bool FGE_API Replace(const std::string& name, const std::string& path);

}//end path
}//end fge


#endif // _FGE_PATH_MANAGER_HPP_INCLUDED

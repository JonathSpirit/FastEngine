#ifndef _FGE_LOG_MANAGER_HPP_INCLUDED
#define _FGE_LOG_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <string>

namespace fge
{
namespace log
{

const std::string& FGE_API SetDefaultFolder (const std::string& default_folder);

bool FGE_API Remove (const std::string& name);
bool FGE_API Clean (const std::string& name);
bool FGE_API Rename (const std::string& name, const std::string& new_name);
bool FGE_API Write (const std::string& name, const std::string& text, const std::string& desc=std::string());

}//end log
}//end fge

#endif // _FGE_LOG_MANAGER_HPP_INCLUDED

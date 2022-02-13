#ifndef _FGE_LOG_MANAGER_HPP_INCLUDED
#define _FGE_LOG_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <string>

namespace fge
{
namespace log
{

FGE_API const std::string& SetDefaultFolder (const std::string& default_folder);

FGE_API bool Remove (const std::string& name);
FGE_API bool Clean (const std::string& name);
FGE_API bool Rename (const std::string& name, const std::string& new_name);
FGE_API bool Write (const std::string& name, const std::string& text, const std::string& desc=std::string());

}//end log
}//end fge

#endif // _FGE_LOG_MANAGER_HPP_INCLUDED

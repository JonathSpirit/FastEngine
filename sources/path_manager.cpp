#include "FastEngine/path_manager.hpp"
#include <unordered_map>

namespace fge
{
namespace path
{

namespace
{

std::unordered_map<std::string, std::string> __dataPath;
const std::string __dataPathBad;

}//end

const std::string& FGE_API Get(const std::string& name)
{
    auto it = __dataPath.find(name);
    return (it != __dataPath.cend()) ? it->second : __dataPathBad;
}

std::size_t FGE_API GetPathSize()
{
    return __dataPath.size();
}

void FGE_API Remove(const std::string& name)
{
    __dataPath.erase(name);
}

bool FGE_API Check(const std::string& name)
{
    return __dataPath.find(name) != __dataPath.cend();
}

bool FGE_API New(const std::string& name, const std::string& path)
{
    if ( fge::path::Check(name) )
    {
        return false;
    }

    __dataPath[name] = path;
    return true;
}

bool FGE_API Replace(const std::string& name, const std::string& path)
{
    auto it = __dataPath.find(name);

    if ( it != __dataPath.end() )
    {
        it->second = path;
        return true;
    }
    return false;
}

}//end path
}//end fge

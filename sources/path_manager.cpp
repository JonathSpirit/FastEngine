#include "FastEngine/path_manager.hpp"
#include <unordered_map>

namespace fge
{
namespace path
{

namespace
{

std::unordered_map<std::string, std::string> _dataPath;
const std::string _dataPathBad;

}//end

const std::string& Get(const std::string& name)
{
    auto it = _dataPath.find(name);
    return (it != _dataPath.cend()) ? it->second : _dataPathBad;
}

std::size_t GetPathSize()
{
    return _dataPath.size();
}

void Remove(const std::string& name)
{
    _dataPath.erase(name);
}

bool Check(const std::string& name)
{
    return _dataPath.find(name) != _dataPath.cend();
}

bool New(const std::string& name, const std::string& path)
{
    if ( fge::path::Check(name) )
    {
        return false;
    }

    _dataPath[name] = path;
    return true;
}

bool Replace(const std::string& name, const std::string& path)
{
    auto it = _dataPath.find(name);

    if ( it != _dataPath.end() )
    {
        it->second = path;
        return true;
    }
    return false;
}

}//end path
}//end fge

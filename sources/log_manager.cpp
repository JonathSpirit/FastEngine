#include "FastEngine/log_manager.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <ctime>

namespace
{
    std::string l_default_folder;
}

namespace fge
{
namespace log
{

const std::string& FGE_API SetDefaultFolder (const std::string& default_folder)
{
    l_default_folder = default_folder;

    if (l_default_folder.back() == '\\')
    {
        l_default_folder.pop_back();
    }
    if ((l_default_folder.length() != 0) && (l_default_folder.back() != '/'))
    {
        l_default_folder.push_back('/');
    }

    return l_default_folder;
}

bool FGE_API Remove (const std::string& name)
{
    int return_value;

    return_value = remove( (l_default_folder + name).c_str() );
    if ( !return_value )
    {
        return true;
    }
    return false;
}
bool FGE_API Clean (const std::string& name)
{
    std::ofstream file_log;
    file_log.open( (l_default_folder + name).c_str() );

    if ( file_log )
    {
        file_log.close();
        return true;
    }
    return false;
}
bool FGE_API Rename (const std::string& name, const std::string& new_name)
{
    int return_value;

    return_value = rename( (l_default_folder + name).c_str(), (l_default_folder + new_name).c_str() );
    if ( !return_value )
    {
        return true;
    }
    return false;
}
bool FGE_API Write (const std::string& name, const std::string& text, const std::string& desc)
{
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

    std::ofstream file_log;
    file_log.open( l_default_folder + name, std::ios::app );

    if (!file_log)
    {
        return false;
    }

    file_log << "[" << std::setw(2) << std::setfill('0') << now->tm_mday << "." << std::setw(2) << std::setfill('0') << now->tm_mon+1 << "." << 1900 + now->tm_year << "]";
    file_log << "(" << std::setw(2) << std::setfill('0') << now->tm_hour << ":" << std::setw(2) << std::setfill('0') << now->tm_min << ":" << std::setw(2) << std::setfill('0') << now->tm_sec << ") ";
    if ( !desc.empty() )
    {
        file_log << "[" << desc << "] ";
    }
    file_log << text << std::endl;

    return true;
}

}//end log
}//end fge

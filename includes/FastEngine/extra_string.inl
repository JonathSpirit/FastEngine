namespace fge
{
namespace string
{

template <class T>
std::string ToStr(const std::list<T>& val, char separator)
{
    std::string result;
    for (auto it=val.cbegin(); it!=val.cend(); ++it)
    {
        result += fge::string::ToStr(*it) + separator;
    }
    if ( !result.empty() )
    {
        result.pop_back();
    }
    return result;
}
template <class T>
std::string ToStr(const std::vector<T>& val, char separator)
{
    std::string result;
    for (auto it=val.cbegin(); it!=val.cend(); ++it)
    {
        result += fge::string::ToStr(*it) + separator;
    }
    if ( !result.empty() )
    {
        result.pop_back();
    }
    return result;
}



}//end string
}//end fge

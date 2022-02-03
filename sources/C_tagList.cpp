#include "FastEngine/C_tagList.hpp"

namespace fge
{

void FGE_API TagList::clear()
{
    this->g_tags.clear();
}

void FGE_API TagList::add(const std::string& tag)
{
    this->g_tags.insert(tag);
}
void FGE_API TagList::del(const std::string& tag)
{
    this->g_tags.erase(tag);
}

bool FGE_API TagList::check(const std::string& tag) const
{
    return this->g_tags.find(tag) != this->g_tags.cend();
}

std::size_t FGE_API TagList::getSize() const
{
    return this->g_tags.size();
}

fge::TagList::TagListType::const_iterator TagList::cbegin() const
{
    return this->g_tags.cbegin();
}
fge::TagList::TagListType::const_iterator TagList::cend() const
{
    return this->g_tags.cend();
}

}//end fge

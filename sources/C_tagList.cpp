#include "FastEngine/C_tagList.hpp"

namespace fge
{

void TagList::clear()
{
    this->g_tags.clear();
}

void TagList::add(const std::string& tag)
{
    this->g_tags.insert(tag);
}
void TagList::del(const std::string& tag)
{
    this->g_tags.erase(tag);
}

bool TagList::check(const std::string& tag) const
{
    return this->g_tags.find(tag) != this->g_tags.cend();
}

std::size_t TagList::getSize() const
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

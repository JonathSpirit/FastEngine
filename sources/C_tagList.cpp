#include "FastEngine/C_tagList.hpp"

namespace fge
{

void TagList::clear()
{
    this->g_tags.clear();
}

void TagList::add(std::string_view tag)
{
    this->g_tags.insert(std::move( std::string(tag) ));
}
void TagList::del(std::string_view tag)
{
    for (auto it = this->g_tags.cbegin(); it != this->g_tags.cend(); ++it)
    {
        if ((*it) == tag)
        {
            this->g_tags.erase(it);
            return;
        }
    }
}

bool TagList::check(std::string_view tag) const
{
    for (const auto& value : this->g_tags)
    {
        if (value == tag)
        {
            return true;
        }
    }
    return false;
}

std::size_t TagList::getSize() const
{
    return this->g_tags.size();
}

fge::TagList::TagListType::const_iterator TagList::begin() const
{
    return this->g_tags.begin();
}
fge::TagList::TagListType::const_iterator TagList::end() const
{
    return this->g_tags.end();
}

}//end fge

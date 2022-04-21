#ifndef _FGE_C_TAGLIST_HPP_INCLUDED
#define _FGE_C_TAGLIST_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <string>
#include <unordered_set>

namespace fge
{

class FGE_API TagList
{
public:
    using TagListType = std::unordered_set<std::string>;

    TagList() = default;
    ~TagList() = default;

    void clear();

    void add(std::string_view tag);
    void del(std::string_view tag);

    bool check(std::string_view tag) const;

    std::size_t getSize() const;

    fge::TagList::TagListType::const_iterator begin() const;
    fge::TagList::TagListType::const_iterator end() const;

private:
    fge::TagList::TagListType g_tags;
};

}//end fge

#endif // _FGE_C_TAGLIST_HPP_INCLUDED

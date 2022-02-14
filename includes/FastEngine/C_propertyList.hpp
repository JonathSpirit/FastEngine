#ifndef _FGE_C_PROPERTYLIST_HPP_INCLUDED
#define _FGE_C_PROPERTYLIST_HPP_INCLUDED

#include <FastEngine/C_property.hpp>
#include <string>
#include <unordered_map>

namespace fge
{

class PropertyList
{
public:
    using PropertyListType = std::unordered_map<std::string, fge::Property>;

    inline PropertyList() = default;
    inline PropertyList(const PropertyList& r);
    inline PropertyList(PropertyList&& r) noexcept;
    inline ~PropertyList() = default;

    inline void delAllProperties();
    inline void delProperty(const std::string& key);

    inline bool checkProperty(const std::string& key) const;

    inline void setProperty(const std::string& key, const fge::Property& value);
    inline void setProperty(const std::string& key, fge::Property&& value);

    template <typename T>
    inline T* getPropertyType(const std::string& key);
    template <typename T>
    inline const T* getPropertyType(const std::string& key) const;

    inline fge::Property& getProperty(const std::string& key);
    inline const fge::Property& getProperty(const std::string& key) const;

    inline fge::Property& operator[] (const std::string& key);
    inline const fge::Property& operator[] (const std::string& key) const;

    inline std::size_t getPropertiesSize() const;

    inline fge::PropertyList::PropertyListType::iterator begin();
    inline fge::PropertyList::PropertyListType::iterator end();
    inline fge::PropertyList::PropertyListType::const_iterator cbegin();
    inline fge::PropertyList::PropertyListType::const_iterator cend();

    inline fge::PropertyList::PropertyListType::const_iterator find(const std::string& key) const;
    inline fge::PropertyList::PropertyListType::iterator find(const std::string& key);

    inline void clearAllModificationFlags();
    inline std::size_t countAllModificationFlags() const;

private:
    fge::PropertyList::PropertyListType g_data;
};

}//end fge

#include <FastEngine/C_propertyList.inl>

#endif // _FGE_C_PROPERTYLIST_HPP_INCLUDED

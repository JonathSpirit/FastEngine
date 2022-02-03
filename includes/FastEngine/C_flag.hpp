#ifndef _FGE_C_FLAG_HPP_INCLUDED
#define _FGE_C_FLAG_HPP_INCLUDED

namespace fge
{

class Flag
{
public:
    inline Flag(bool defaultValue=false) :
        g_flag(defaultValue)
    {}

    inline bool check(bool input)
    {
        if (!this->g_flag)
        {
            this->g_flag = input;
            return input;
        }
        if (!input)
        {
            this->g_flag = false;
        }
        return false;
    }

    inline void set(bool value)
    {
        this->g_flag = value;
    }
    inline bool get() const
    {
        return this->g_flag;
    }

    inline bool operator=(bool value)
    {
        return this->g_flag = value;
    }

    inline operator bool() const
    {
        return this->g_flag;
    }

private:
    bool g_flag;
};

}//end fge

#endif // _FGE_C_FLAG_HPP_INCLUDED

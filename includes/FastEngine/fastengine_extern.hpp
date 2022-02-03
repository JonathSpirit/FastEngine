#ifndef _FGE_FASTENGINE_EXTERN_HPP_INCLUDED
#define _FGE_FASTENGINE_EXTERN_HPP_INCLUDED

#ifdef __linux__
    #define FGE_API
#else
    #ifdef _FGE_DEF_BUILDDLL
        #define FGE_API __declspec(dllexport)
    #else
        #define FGE_API __declspec(dllimport)
    #endif // _FGE_DEF_BUILDDLL
#endif //__linux__

#endif // _FGE_FASTENGINE_EXTERN_HPP_INCLUDED

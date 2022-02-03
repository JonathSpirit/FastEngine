#ifndef _FGE_FASTENGINE_EXTERN_HPP_INCLUDED
#define _FGE_FASTENGINE_EXTERN_HPP_INCLUDED

#ifdef _FGE_DEF_BUILDDLL
    #define FGE_API __declspec(dllexport)
#else
    #define FGE_API __declspec(dllimport)
#endif // FGE_API

#endif // _FGE_FASTENGINE_EXTERN_HPP_INCLUDED

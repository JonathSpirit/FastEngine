#ifndef _FGE_FASTENGINE_EXTERN_HPP_INCLUDED
#define _FGE_FASTENGINE_EXTERN_HPP_INCLUDED

#ifndef _WIN32
    #define FGE_API
#else
    #ifdef _FGE_DEF_BUILDDLL
        #define FGE_API __declspec(dllexport)
    #else
        #define FGE_API __declspec(dllimport)
    #endif // _FGE_DEF_BUILDDLL
#endif //_WIN32

#endif // _FGE_FASTENGINE_EXTERN_HPP_INCLUDED

/**
 *  \defgroup objectControl Object control
 *  \brief Everything related to objects
 *
 *  \defgroup network Network
 *  \brief Everything related to network
 *
 *  \defgroup utility Utility/Tools
 *  \brief Everything related to some utility/tools
 */

#ifndef _EXFGE_DEFINITION_HPP_INCLUDED
#define _EXFGE_DEFINITION_HPP_INCLUDED

#include <FastEngine/manager/network_manager.hpp>

#define LIFESIM_VERSION 1
#define LIFESIM_CLIENT_PORT 42047
#define LIFESIM_SERVER_PORT 42048
#define LIFESIM_CONNECTION_TEXT "_IWANTTOCONNECT_42//%"

#define LIFESIM_MAP_SIZE_MAXX 1550
#define LIFESIM_MAP_SIZE_MINX 50

#define LIFESIM_MAP_SIZE_MAXY 850
#define LIFESIM_MAP_SIZE_MINY 50

#define LIFESIM_CLIENTDATA_TIMEOUT "timeout"
#define LIFESIM_SERVER_TICK 20
#define LIFESIM_TIMEOUT 200

namespace ls
{

enum ProtocolHeaders : uint16_t
{
    LS_PROTOCOL_ALL_PING = 0,
    /*
    IN:
        -
    OUT:
        -
    */
    LS_PROTOCOL_ALL_GOODBYE,
    /*
    IN:
        string REASON
    OUT:
        -
    */
    LS_PROTOCOL_C_PLEASE_CONNECT_ME,
    /*
    IN:
        string "Hello"
        string CONNECTION_TEXT
    OUT:
        bool VALID
    */

    LS_PROTOCOL_S_UPDATE,
    /*
    IN:
        modifiedScene MODIFIED_SCENE_DATA
        eventScene EVENT_SCENE_DATA
    OUT:
        -
    */
    LS_PROTOCOL_S_UPDATE_ALL
    /*
    IN:
        scene SCENE_DATA
    OUT:
        -
    */
};

}//end ls

#endif // _EXFGE_DEFINITION_HPP_INCLUDED

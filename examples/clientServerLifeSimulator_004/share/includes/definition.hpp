#ifndef _EXFGE_DEFINITION_HPP_INCLUDED
#define _EXFGE_DEFINITION_HPP_INCLUDED

#include <FastEngine/manager/network_manager.hpp>

#define LIFESIM_VERSION 1
#define LIFESIM_CLIENT_PORT 42047
#define LIFESIM_SERVER_PORT 42048
#define LIFESIM_CONNECTION_TEXT1 "Hello"
#define LIFESIM_CONNECTION_TEXT2 "_IWANTTOCONNECT_42//%"

#define LIFESIM_MAP_SIZE_MAXX 1550
#define LIFESIM_MAP_SIZE_MINX 50

#define LIFESIM_MAP_SIZE_MAXY 850
#define LIFESIM_MAP_SIZE_MINY 50

#define LIFESIM_START_CREATURES_COUNT 20
#define LIFESIM_TIME_NEW_FOODS std::chrono::milliseconds{10000}
#define LIFESIM_NEW_DRINK_COUNT 15
#define LIFESIM_NEW_FOOD_COUNT 15
#define LIFESIM_TIME_WORLDTICK std::chrono::milliseconds{20000}

#define LIFESIM_CLIENTDATA_TIMEOUT "timeout"
#define LIFESIM_CLIENTDATA_LATENCY "latencyCTOS"
#define LIFESIM_SERVER_TICK 20
#define LIFESIM_TIME_TIMEOUT std::chrono::milliseconds{100}
#define LIFESIM_TIMEOUT_COUNT 30

#define LIFESIM_VIDEOMODE sf::VideoMode(1600, 900)
#define LIFESIM_FRAMERATE 60
#define LIFESIM_TIME_CONNECTION_TIMEOUT std::chrono::milliseconds{3000}
#define LIFESIM_TIME_CLIENT_UPDATE std::chrono::milliseconds{1000}

namespace ls
{

enum ProtocolHeaders : fge::net::PacketHeader
{
    LS_PROTOCOL_ALL_PING = 0,
    /* check if the receiver is alive
    IN:
        -
    OUT:
        -
    */
    LS_PROTOCOL_ALL_PONG,
    /* a response to a ping
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
    LS_PROTOCOL_C_UPDATE,
    /*
    IN:
        Timestamp TIMESTAMP_CLIENT
        Latency_ms LATENCY_STOC
    OUT:
        -
    */
    LS_PROTOCOL_C_PLEASE_CONNECT_ME,
    /*
    IN:
        string "Hello"
        string CONNECTION_TEXT
        Timestamp SYNC_TIMESTAMP_CLIENT_START
    OUT:
        bool VALID
        Timestamp SYNC_TIMESTAMP_CLIENT_START
        Timestamp SYNC_TIMESTAMP_SERVER_START
        Timestamp SYNC_TIMESTAMP_SERVER_STOP
    */

    LS_PROTOCOL_S_UPDATE,
    /*
    IN:
        Timestamp TIMESTAMP_SERVER
        Latency_ms LATENCY_CTOS
        sceneModification MODIFIED_SCENE_DATA
        sceneWatchedEvent EVENT_SCENE_DATA
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

/*
 * Copyright 2024 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _EXFGE_DEFINITION_HPP_INCLUDED
#define _EXFGE_DEFINITION_HPP_INCLUDED

#include "FastEngine/network/C_protocol.hpp"

#define LIFESIM_VERSION 1
#define LIFESIM_CLIENT_PORT 42047
#define LIFESIM_SERVER_PORT 42048
#define LIFESIM_CONNECTION_TEXT1 "Hello"
#define LIFESIM_CONNECTION_TEXT2 "_IWANTTOCONNECT_42//%"

#define LIFESIM_MAP_WIDTH 800
#define LIFESIM_MAP_HEIGHT 600

#define LIFESIM_MAP_SIZE_MAXX LIFESIM_MAP_WIDTH - 50
#define LIFESIM_MAP_SIZE_MINX 50

#define LIFESIM_MAP_SIZE_MAXY LIFESIM_MAP_HEIGHT - 50
#define LIFESIM_MAP_SIZE_MINY 50

#define LIFESIM_START_CREATURES_COUNT 20
#define LIFESIM_TIME_NEW_FOODS                                                                                         \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        10000                                                                                                          \
    }
#define LIFESIM_NEW_DRINK_COUNT 15
#define LIFESIM_NEW_FOOD_COUNT 15
#define LIFESIM_TIME_WORLDTICK                                                                                         \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        20000                                                                                                          \
    }

#define LIFESIM_CLIENTDATA_TIMEOUT "timeout"
#define LIFESIM_SERVER_TICK 20
#define LIFESIM_TIME_TIMEOUT                                                                                           \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        100                                                                                                            \
    }
#define LIFESIM_TIMEOUT_COUNT 30

#define LIFESIM_VIDEOMODE sf::VideoMode(1600, 900)
#define LIFESIM_FRAMERATE 60
#define LIFESIM_TIME_CONNECTION_TIMEOUT                                                                                \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        3000                                                                                                           \
    }
#define LIFESIM_TIME_CLIENT_UPDATE                                                                                     \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        1000                                                                                                           \
    }

namespace ls
{

enum ProtocolHeaders : fge::net::ProtocolPacket::Header
{
    LS_PROTOCOL_ALL_PING = FGE_NET_HEADERID_START,
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
        LatencyPlanner LATENCY_PLANNER_DATA
    OUT:
        -
    */
    LS_PROTOCOL_C_PLEASE_CONNECT_ME,
    /*
    IN:
        string "Hello"
        string CONNECTION_TEXT
        LatencyPlanner LATENCY_PLANNER_DATA
    OUT:
        bool VALID
        LatencyPlanner LATENCY_PLANNER_DATA
    */
    LS_PROTOCOL_C_ASK_FULL_UPDATE,
    /*
    IN:
        -
    OUT:
        -
    */

    LS_PROTOCOL_S_UPDATE,
    /*
    IN:
        LatencyPlanner LATENCY_PLANNER_DATA
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

} // namespace ls

#endif // _EXFGE_DEFINITION_HPP_INCLUDED

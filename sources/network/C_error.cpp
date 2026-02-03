/*
 * Copyright 2025 Guillaume Guillet
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

#include "FastEngine/network/C_error.hpp"

namespace fge::net
{

void Error::dump(std::ostream& os) const
{
    char const* typeStr = "UNKNOWN";

    switch (this->_type)
    {
    case Types::ERR_ALREADY_INVALID:
        typeStr = "ALREADY_INVALID";
        break;
    case Types::ERR_EXTRACT:
        typeStr = "EXTRACT";
        break;
    case Types::ERR_RULE:
        typeStr = "RULE";
        break;
    case Types::ERR_SCENE_OLD_PACKET:
        typeStr = "SCENE_OLD_PACKET";
        break;
    case Types::ERR_TRANSMIT:
        typeStr = "TRANSMIT";
        break;
    case Types::ERR_DATA:
        typeStr = "DATA";
        break;
    }

    os << "network error :\n"
       << "\ttype: " << typeStr << '\n'
       << "\terror: " << (this->_error == nullptr ? "UNKNOWN" : this->_error) << '\n'
       << "\tfunction: " << (this->_function == nullptr ? "UNKNOWN" : this->_function) << '\n'
       << "\treadPos: " << this->_readPos << std::endl;
}

} // namespace fge::net

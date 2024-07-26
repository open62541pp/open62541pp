#pragma once

#include <ostream>

#include "open62541pp/Bitmask.h"

namespace opcua {

template <typename T>
std::ostream& operator<<(std::ostream& os, Bitmask<T> mask) {
    os << mask.get();
    return os;
}

}  // namespace opcua

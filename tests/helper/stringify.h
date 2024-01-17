#pragma once

#include <ostream>

#include "open62541pp/BitMask.h"

namespace opcua {

template <typename T>
std::ostream& operator<<(std::ostream& os, BitMask<T> mask) {
    os << mask.get();
    return os;
}

}  // namespace opcua

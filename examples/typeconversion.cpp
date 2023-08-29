/**
 * Type conversions between arbitrary types and native `UA_*` types can be added.
 * Conversions are handled by the TypeConverter struct. A new template specialization must be added
 * to define a new conversion - this can happen outside of this library.
 *
 * In this case, we will enable conversion between the new C++17 type `std::byte` and `UA_Byte`.
 */

#include <array>
#include <cstddef>  // byte, to_integer
#include <iostream>

#include "open62541pp/open62541pp.h"

namespace opcua {

template <>
struct TypeConverter<std::byte> {
    using ValueType = std::byte;
    using NativeType = UA_Byte;
    using ValidTypes = TypeIndexList<UA_TYPES_BYTE>;  // type index of UA_TYPES array

    // use `const NativeType& src` for non-primitive types
    static void fromNative(NativeType src, ValueType& dst) {
        dst = std::byte(src);
    }

    // use `const ValueType& src` for non-primitive types
    static void toNative(ValueType src, NativeType& dst) {
        dst = std::to_integer<UA_Byte>(src);
    }
};

}  // namespace opcua

int main() {
    opcua::Variant variant;

    // Write std::byte to variant
    variant.setScalarCopy(std::byte{11});

    // Read std::byte from variant (conversion requires copy)
    const auto value = variant.getScalarCopy<std::byte>();
    std::cout << "Byte value: " << std::to_integer<int>(value) << std::endl;

    // Read UA_Byte from variant (zero copy possible)
    const auto& valueNative = variant.getScalar<UA_Byte>();
    std::cout << "Byte value: " << static_cast<int>(valueNative) << std::endl;

    // Write array of bytes to variant
    std::array<std::byte, 3> array{};
    variant.setArrayCopy(array);  // use array container
    variant.setArrayCopy(opcua::Span{array.data(), array.size()});  // use raw array and size
    variant.setArrayCopy(array.begin(), array.end());  // use iterator pair

    std::cout << "Array size: " << variant.getArrayLength() << std::endl;
}

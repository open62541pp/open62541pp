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

#include <open62541pp/typeconverter.hpp>
#include <open62541pp/types.hpp>

namespace opcua {

template <>
struct TypeConverter<std::byte> {
    using NativeType = UA_Byte;

    // use `const NativeType& src` for non-primitive types
    static void fromNative(UA_Byte src, std::byte& dst) {
        dst = std::byte(src);
    }

    // use `const ValueType& src` for non-primitive types
    static void toNative(std::byte src, UA_Byte& dst) {
        dst = std::to_integer<UA_Byte>(src);
    }
};

}  // namespace opcua

int main() {
    opcua::Variant variant;

    // Write std::byte to variant
    variant.assign(std::byte{11});
    // Use assignment operator
    variant = std::byte{11};

    // Read UA_Byte from variant (reference possible)
    const auto& valueNative = variant.scalar<UA_Byte>();
    std::cout << "Byte value: " << static_cast<int>(valueNative) << std::endl;

    // Read std::byte from variant (copy and conversion required)
    const auto value = variant.to<std::byte>();
    std::cout << "Byte value: " << std::to_integer<int>(value) << std::endl;

    // Write array of bytes to variant
    std::array<std::byte, 3> array{};
    variant.assign(array);  // use array container
    variant.assign(opcua::Span{array.data(), array.size()});  // use raw array and size
    variant.assign(array.begin(), array.end());  // use iterator pair

    std::cout << "Array size: " << variant.arrayLength() << std::endl;
}

#include <iostream>
#include <string>

#include "open62541pp/open62541pp.h"

#include "custom_datatypes.h"

int main() {
    opcua::Client client;

    // Get custom type definitions from common header
    const auto& dataTypePoint = getPointDataType();
    const auto& dataTypeMeasurements = getMeasurementsDataType();
    const auto& dataTypeOpt = getOptDataType();
    const auto& dataTypeUni = getUniDataType();

    // Provide custom data type definitions to client
    client.setCustomDataTypes({dataTypePoint, dataTypeMeasurements, dataTypeOpt, dataTypeUni});

    client.connect("opc.tcp://localhost:4840");

    // Read custom variables
    opcua::Variant variant;

    client.getNode({1, "Point"}).readValue(variant);
    if (variant.isType(dataTypePoint)) {
        const auto* p = static_cast<Point*>(variant.getScalar());
        std::cout << "Point:\n";
        std::cout << "- x = " << p->x << "\n";
        std::cout << "- y = " << p->y << "\n";
        std::cout << "- z = " << p->z << "\n";
    }

    client.getNode({1, "PointVec"}).readValue(variant);
    // Variants store non-builtin data types as ExtensionObjects. If the data type is known to the
    // client/server, open62541 unwraps scalar objects transparently in the encoding layer:
    // https://www.open62541.org/doc/master/types.html#variant
    // Arrays can not be unwrapped easily, because the array is an array of ExtensionObjects.
    // The array of unwrapped objects isn't available contiguously in memory and open62541 won't
    // transparently unwrap the array. So we have to do the unwrapping ourselves:
    if (variant.isType(opcua::Type::ExtensionObject)) {
        auto* arrExt = variant.getArray<opcua::ExtensionObject>();
        for (size_t i = 0; i < variant.getArrayLength(); ++i) {
            const auto* p = static_cast<Point*>(arrExt[i].getDecodedData());  // NOLINT
            std::cout << "PointVec[" << i << "]:\n";
            std::cout << "- x = " << p->x << "\n";
            std::cout << "- y = " << p->y << "\n";
            std::cout << "- z = " << p->z << "\n";
        }
    }

    client.getNode({1, "Measurements"}).readValue(variant);
    if (variant.isType(dataTypeMeasurements)) {
        const auto* m = static_cast<Measurements*>(variant.getScalar());
        std::cout << "Measurements:\n";
        std::cout << "- description = " << m->description << "\n";
        for (size_t i = 0; i < m->measurementsSize; ++i) {
            std::cout << "- measurements[" << i << "] = " << m->measurements[i] << "\n";  // NOLINT
        }
    }

    client.getNode({1, "Opt"}).readValue(variant);
    auto formatOptional = [](const auto* ptr) {
        return ptr == nullptr ? "NULL" : std::to_string(*ptr);
    };
    if (variant.isType(dataTypeOpt)) {
        const auto* opt = static_cast<Opt*>(variant.getScalar());
        std::cout << "Opt:\n";
        std::cout << "- a = " << opt->a << "\n";
        std::cout << "- b = " << formatOptional(opt->b) << "\n";
        std::cout << "- c = " << formatOptional(opt->c) << "\n";
    }

    client.getNode({1, "Uni"}).readValue(variant);
    if (variant.isType(dataTypeUni)) {
        const auto* uni = static_cast<Uni*>(variant.getScalar());
        std::cout << "Uni:\n";
        std::cout << "- switchField = " << static_cast<int>(uni->switchField) << "\n";
        if (uni->switchField == UniSwitch::OptionA) {
            std::cout << "- optionA = " << uni->fields.optionA << "\n";  // NOLINT
        }
        if (uni->switchField == UniSwitch::OptionB) {
            std::cout << "- optionB = " << opcua::String(uni->fields.optionB) << "\n";  // NOLINT
        }
    }
}

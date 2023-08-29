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

    variant = client.getNode({1, "Point"}).readValue();
    if (variant.isType(dataTypePoint)) {
        const auto* p = static_cast<Point*>(variant.data());
        std::cout << "Point:\n";
        std::cout << "- x = " << p->x << "\n";
        std::cout << "- y = " << p->y << "\n";
        std::cout << "- z = " << p->z << "\n";
    }

    variant = client.getNode({1, "PointVec"}).readValue();
    // Variants store non-builtin data types as ExtensionObjects. If the data type is known to the
    // client/server, open62541 unwraps scalar objects transparently in the encoding layer:
    // https://www.open62541.org/doc/master/types.html#variant
    // Arrays can not be unwrapped easily, because the array is an array of ExtensionObjects.
    // The array of unwrapped objects isn't available contiguously in memory and open62541 won't
    // transparently unwrap the array. So we have to do the unwrapping ourselves:
    if (variant.isArray() && variant.isType(opcua::Type::ExtensionObject)) {
        size_t i = 0;
        for (auto&& extObj : variant.getArray<opcua::ExtensionObject>()) {
            const auto* p = static_cast<Point*>(extObj.getDecodedData());
            std::cout << "PointVec[" << i++ << "]:\n";
            std::cout << "- x = " << p->x << "\n";
            std::cout << "- y = " << p->y << "\n";
            std::cout << "- z = " << p->z << "\n";
        }
    }

    variant = client.getNode({1, "Measurements"}).readValue();
    if (variant.isType(dataTypeMeasurements)) {
        const auto* m = static_cast<Measurements*>(variant.data());
        std::cout << "Measurements:\n";
        std::cout << "- description = " << m->description << "\n";
        size_t i = 0;
        for (auto&& value : opcua::Span(m->measurements, m->measurementsSize)) {
            std::cout << "- measurements[" << i++ << "] = " << value << "\n";
        }
    }

    variant = client.getNode({1, "Opt"}).readValue();
    auto formatOptional = [](const auto* ptr) {
        return ptr == nullptr ? "NULL" : std::to_string(*ptr);
    };
    if (variant.isScalar() && variant.isType(dataTypeOpt)) {
        const auto* opt = static_cast<Opt*>(variant.data());
        std::cout << "Opt:\n";
        std::cout << "- a = " << opt->a << "\n";
        std::cout << "- b = " << formatOptional(opt->b) << "\n";
        std::cout << "- c = " << formatOptional(opt->c) << "\n";
    }

    variant = client.getNode({1, "Uni"}).readValue();
    if (variant.isType(dataTypeUni)) {
        const auto* uni = static_cast<Uni*>(variant.data());
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

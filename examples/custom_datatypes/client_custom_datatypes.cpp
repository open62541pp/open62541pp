#include <iostream>
#include <string>

#include <open62541pp/client.hpp>
#include <open62541pp/node.hpp>

#include "custom_datatypes.hpp"

int main() {
    // Get custom type definitions from common header
    const auto& dataTypePoint = getPointDataType();
    const auto& dataTypeMeasurements = getMeasurementsDataType();
    const auto& dataTypeOpt = getOptDataType();
    const auto& dataTypeUni = getUniDataType();
    const auto& dataTypeColor = getColorDataType();

    opcua::ClientConfig config;
    // Provide custom data type definitions to client
    config.setCustomDataTypes({
        dataTypePoint,
        dataTypeMeasurements,
        dataTypeOpt,
        dataTypeUni,
        dataTypeColor,
    });
    opcua::Client client(std::move(config));

    client.connect("opc.tcp://localhost:4840");

    // Read custom variables
    opcua::Variant variant;

    variant = opcua::Node(client, {1, "Point"}).readValue();
    if (variant.isType(dataTypePoint)) {
        const auto* p = static_cast<Point*>(variant.data());
        std::cout << "Point:\n";
        std::cout << "- x = " << p->x << "\n";
        std::cout << "- y = " << p->y << "\n";
        std::cout << "- z = " << p->z << "\n";
    }

    variant = opcua::Node(client, {1, "PointVec"}).readValue();
    // Variants store non-builtin data types as ExtensionObjects. If the data type is known to the
    // client/server, open62541 unwraps scalar objects transparently in the encoding layer:
    // https://www.open62541.org/doc/master/types.html#variant
    // Arrays can not be unwrapped easily, because the array is an array of ExtensionObjects.
    // The array of unwrapped objects isn't available contiguously in memory and open62541 won't
    // transparently unwrap the array. So we have to do the unwrapping ourselves:
    if (variant.isArray() && variant.isType<opcua::ExtensionObject>()) {
        size_t i = 0;
        for (auto&& extObj : variant.getArray<opcua::ExtensionObject>()) {
            const auto* p = static_cast<Point*>(extObj.getDecodedData());
            std::cout << "PointVec[" << i++ << "]:\n";
            std::cout << "- x = " << p->x << "\n";
            std::cout << "- y = " << p->y << "\n";
            std::cout << "- z = " << p->z << "\n";
        }
    }

    variant = opcua::Node(client, {1, "Measurements"}).readValue();
    if (variant.isType(dataTypeMeasurements)) {
        const auto* m = static_cast<Measurements*>(variant.data());
        std::cout << "Measurements:\n";
        std::cout << "- description = " << m->description << "\n";
        size_t i = 0;
        for (auto&& value : opcua::Span(m->measurements, m->measurementsSize)) {
            std::cout << "- measurements[" << i++ << "] = " << value << "\n";
        }
    }

    variant = opcua::Node(client, {1, "Opt"}).readValue();
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

    variant = opcua::Node(client, {1, "Uni"}).readValue();
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

    variant = opcua::Node(client, {1, "Color"}).readValue();
    if (variant.isType<int32_t>()) {
        std::cout << "Color: " << variant.getScalar<int32_t>() << "\n";
    }
}

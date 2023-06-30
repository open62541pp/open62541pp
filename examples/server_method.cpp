#include <algorithm>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    auto objectNode = server.getObjectsNode();

    // Add a method to return "Hello " + provided name.
    objectNode.addMethod(
        {1, 1000},
        "Greet",
        [](const std::vector<opcua::Variant>& input, std::vector<opcua::Variant>& output) {
            const auto& name = input.at(0).getScalar<opcua::String>();
            const auto greeting = std::string("Hello ").append(name.get());
            output.at(0).setScalarCopy(opcua::String(greeting));
        },
        {{"name", {"en-US", "your name"}, opcua::DataTypeId::String, opcua::ValueRank::Scalar}},
        {{"greeting", {"en-US", "greeting"}, opcua::DataTypeId::String, opcua::ValueRank::Scalar}}
    );

    // Add a method that takes an array of 5 integers and a scalar as input.
    // It returns a copy of the array with every entry increased by the scalar.
    objectNode.addMethod(
        {1, 1001},
        "IncInt32ArrayValues",
        [](const std::vector<opcua::Variant>& input, std::vector<opcua::Variant>& output) {
            auto array = input.at(0).getArrayCopy<int32_t>();
            const auto delta = input.at(1).getScalarCopy<int32_t>();
            std::for_each(array.begin(), array.end(), [&](auto& v) { v += delta; });
            output.at(0).setArrayCopy(array);
        },
        {
            opcua::Argument(
                "int32 array",
                {"en-US", "int32[5] array"},
                opcua::DataTypeId::Int32,
                opcua::ValueRank::OneDimension,
                {5}
            ),
            opcua::Argument(
                "int32 delta",
                {"en-US", "int32 delta"},
                opcua::DataTypeId::Int32,
                opcua::ValueRank::Scalar
            ),
        },
        {
            opcua::Argument(
                "each entry is incremented by the delta",
                {"en-US", "int32[5] array"},
                opcua::DataTypeId::Int32,
                opcua::ValueRank::OneDimension,
                {5}
            ),
        }
    );

    server.run();
}

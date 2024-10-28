#include <open62541pp/node.hpp>
#include <open62541pp/server.hpp>

#include "custom_datatypes.hpp"

int main() {
    opcua::Server server;

    // Get custom type definitions from common header
    const auto& dataTypePoint = getPointDataType();
    const auto& dataTypeMeasurements = getMeasurementsDataType();
    const auto& dataTypeOpt = getOptDataType();
    const auto& dataTypeUni = getUniDataType();
    const auto& dataTypeColor = getColorDataType();

    // Provide custom data type definitions to server
    server.setCustomDataTypes({
        dataTypePoint,
        dataTypeMeasurements,
        dataTypeOpt,
        dataTypeUni,
        dataTypeColor,
    });

    // Add data type nodes
    opcua::Node structureDataTypeNode(server, opcua::DataTypeId::Structure);
    structureDataTypeNode.addDataType(dataTypePoint.getTypeId(), "PointDataType");
    structureDataTypeNode.addDataType(dataTypeMeasurements.getTypeId(), "MeasurementsDataType");
    structureDataTypeNode.addDataType(dataTypeOpt.getTypeId(), "OptDataType");
    structureDataTypeNode.addDataType(dataTypeUni.getTypeId(), "UniDataType");
    opcua::Node enumerationDataTypeNode(server, opcua::DataTypeId::Enumeration);
    enumerationDataTypeNode.addDataType(dataTypeColor.getTypeId(), "Color")
        .addProperty(
            {0, 0},  // auto-generate node id
            "EnumValues",
            opcua::VariableAttributes{}
                .setDataType<opcua::EnumValueType>()
                .setValueRank(opcua::ValueRank::OneDimension)
                .setArrayDimensions({0})
                .setValueArray(opcua::Span<const opcua::EnumValueType>{
                    {0, {"", "Red"}, {}},
                    {1, {"", "Green"}, {}},
                    {2, {"", "Yellow"}, {}},
                })
        )
        .addModellingRule(opcua::ModellingRule::Mandatory);

    // Add variable type nodes (optional)
    opcua::Node baseDataVariableTypeNode(server, opcua::VariableTypeId::BaseDataVariableType);
    auto variableTypePointNode = baseDataVariableTypeNode.addVariableType(
        {1, 4243},
        "PointType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypePoint.getTypeId())
            .setValueRank(opcua::ValueRank::ScalarOrOneDimension)
            .setValueScalar(Point{1, 2, 3}, dataTypePoint)
    );
    auto variableTypeMeasurementNode = baseDataVariableTypeNode.addVariableType(
        {1, 4444},
        "MeasurementsType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypeMeasurements.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(Measurements{}, dataTypeMeasurements)
    );
    auto variableTypeOptNode = baseDataVariableTypeNode.addVariableType(
        {1, 4645},
        "OptType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypeOpt.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(Opt{}, dataTypeOpt)
    );
    auto variableTypeUniNode = baseDataVariableTypeNode.addVariableType(
        {1, 4846},
        "UniType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypeUni.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(Uni{}, dataTypeUni)
    );

    // Add variable nodes with some values
    opcua::Node objectsNode(server, opcua::ObjectId::ObjectsFolder);

    const Point point{3.0, 4.0, 5.0};
    objectsNode.addVariable(
        {1, "Point"},
        "Point",
        opcua::VariableAttributes{}
            .setDataType(dataTypePoint.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(point, dataTypePoint),
        variableTypePointNode.id()
    );

    const std::vector<Point> pointVec{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    objectsNode.addVariable(
        {1, "PointVec"},
        "PointVec",
        opcua::VariableAttributes{}
            .setDataType(dataTypePoint.getTypeId())
            .setArrayDimensions({0})  // single dimension but unknown in size
            .setValueRank(opcua::ValueRank::OneDimension)
            .setValueArray(pointVec, dataTypePoint),
        variableTypePointNode.id()
    );

    std::vector<float> measurementsValues{19.1F, 20.2F, 19.7F};
    const Measurements measurements{
        opcua::String("Test description"),
        measurementsValues.size(),
        measurementsValues.data(),
    };
    objectsNode.addVariable(
        {1, "Measurements"},
        "Measurements",
        opcua::VariableAttributes{}
            .setDataType(dataTypeMeasurements.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(measurements, dataTypeMeasurements),
        variableTypeMeasurementNode.id()
    );

    float optC = 10.10F;
    const Opt opt{3, nullptr, &optC};
    objectsNode.addVariable(
        {1, "Opt"},
        "Opt",
        opcua::VariableAttributes{}
            .setDataType(dataTypeOpt.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(opt, dataTypeOpt),
        variableTypeOptNode.id()
    );

    Uni uni{};
    uni.switchField = UniSwitch::OptionB;
    uni.fields.optionB = UA_STRING_STATIC("test string");  // NOLINT
    objectsNode.addVariable(
        {1, "Uni"},
        "Uni",
        opcua::VariableAttributes{}
            .setDataType(dataTypeUni.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(uni, dataTypeUni),
        variableTypeUniNode.id()
    );

    objectsNode.addVariable(
        {1, "Color"},
        "Color",
        opcua::VariableAttributes{}
            .setDataType(dataTypeColor.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(Color::Green, dataTypeColor)
    );

    server.run();
}

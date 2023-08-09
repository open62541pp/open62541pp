#include "open62541pp/open62541pp.h"

#include "custom_datatypes.h"

int main() {
    opcua::Server server;

    // Get custom type definitions from common header
    const auto& dataTypePoint = getPointDataType();
    const auto& dataTypeMeasurements = getMeasurementsDataType();
    const auto& dataTypeOpt = getOptDataType();
    const auto& dataTypeUni = getUniDataType();

    // Provide custom data type definitions to server
    server.setCustomDataTypes({dataTypePoint, dataTypeMeasurements, dataTypeOpt, dataTypeUni});

    // Add data type nodes
    auto nodeStructureDataType = server.getNode(opcua::DataTypeId::Structure);
    nodeStructureDataType.addDataType(dataTypePoint.getTypeId(), "Point");
    nodeStructureDataType.addDataType(dataTypeMeasurements.getTypeId(), "Measurements");
    nodeStructureDataType.addDataType(dataTypeOpt.getTypeId(), "Opt");
    nodeStructureDataType.addDataType(dataTypeUni.getTypeId(), "Uni");

    // Add variable type nodes
    auto nodeBaseDataVariableType = server.getNode(opcua::VariableTypeId::BaseDataVariableType);
    auto nodeVariableTypePoint = nodeBaseDataVariableType.addVariableType(
        {1, 4243},
        "PointType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypePoint.getTypeId())
            .setValueRank(opcua::ValueRank::ScalarOrOneDimension)
            .setValueScalar(Point{1, 2, 3}, dataTypePoint)
    );
    auto nodeVariableTypeMeasurement = nodeBaseDataVariableType.addVariableType(
        {1, 4444},
        "MeasurementsType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypeMeasurements.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(Measurements{}, dataTypeMeasurements)
    );
    auto nodeVariableTypeOpt = nodeBaseDataVariableType.addVariableType(
        {1, 4645},
        "OptType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypeOpt.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(Opt{}, dataTypeOpt)
    );
    auto nodeVariableTypeUni = nodeBaseDataVariableType.addVariableType(
        {1, 4846},
        "UniType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypeUni.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(Uni{}, dataTypeUni)
    );

    // Add variable nodes with some values
    auto nodeObjects = server.getObjectsNode();

    const Point point{3.0, 4.0, 5.0};
    nodeObjects.addVariable(
        {1, "Point"},
        "Point",
        opcua::VariableAttributes{}
            .setDataType(dataTypePoint.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(point, dataTypePoint),
        nodeVariableTypePoint.getNodeId()
    );

    const std::vector<Point> pointVec{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    nodeObjects.addVariable(
        {1, "PointVec"},
        "PointVec",
        opcua::VariableAttributes{}
            .setDataType(dataTypePoint.getTypeId())
            .setArrayDimensions({0})  // single dimension but unknown in size
            .setValueRank(opcua::ValueRank::OneDimension)
            .setValueArray(pointVec, dataTypePoint),
        nodeVariableTypePoint.getNodeId()
    );

    std::vector<float> measurementsValues{19.1F, 20.2F, 19.7F};
    const Measurements measurements{
        opcua::String("Test description"),
        measurementsValues.size(),
        measurementsValues.data(),
    };
    nodeObjects.addVariable(
        {1, "Measurements"},
        "Measurements",
        opcua::VariableAttributes{}
            .setDataType(dataTypeMeasurements.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(measurements, dataTypeMeasurements),
        nodeVariableTypeMeasurement.getNodeId()
    );

    float optC = 10.10F;
    const Opt opt{3, nullptr, &optC};
    nodeObjects.addVariable(
        {1, "Opt"},
        "Opt",
        opcua::VariableAttributes{}
            .setDataType(dataTypeOpt.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(opt, dataTypeOpt),
        nodeVariableTypeOpt.getNodeId()
    );

    Uni uni{};
    uni.switchField = UniSwitch::OptionB;
    uni.fields.optionB = UA_STRING_STATIC("test string");  // NOLINT
    nodeObjects.addVariable(
        {1, "Uni"},
        "Uni",
        opcua::VariableAttributes{}
            .setDataType(dataTypeUni.getTypeId())
            .setValueRank(opcua::ValueRank::Scalar)
            .setValueScalar(uni, dataTypeUni),
        nodeVariableTypeUni.getNodeId()
    );

    server.run();
}

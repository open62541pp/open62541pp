#pragma once

#include <cstdint>

#include "open62541pp/open62541pp.h"

// Example struct
struct Point {
    float x;
    float y;
    float z;
};

const opcua::DataType& getPointDataType() {
    static const opcua::DataType dt =
        opcua::DataTypeBuilder<Point>::createStructure("Point", {1, 4242}, {1, 1})
            .addField<&Point::x>("x")
            .addField<&Point::y>("y")
            .addField<&Point::z>("z")
            .build();
    return dt;
}

// Example struct with an array
struct Measurements {
    opcua::String description;
    size_t measurementsSize;
    float* measurements;
};

const opcua::DataType& getMeasurementsDataType() {
    static const opcua::DataType dt =
        opcua::DataTypeBuilder<Measurements>::createStructure("Measurements", {1, 4443}, {1, 2})
            .addField<&Measurements::description>("description")
            .addField<&Measurements::measurementsSize, &Measurements::measurements>("measurements")
            .build();
    return dt;
}

// Example struct with optional fields
struct Opt {
    int16_t a;
    float* b;
    float* c;
};

const opcua::DataType& getOptDataType() {
    static const opcua::DataType dt =
        opcua::DataTypeBuilder<Opt>::createStructure("Opt", {1, 4644}, {1, 3})
            .addField<&Opt::a>("a")
            .addField<&Opt::b>("b")
            .addField<&Opt::c>("c")
            .build();
    return dt;
}

// Example union
enum class UniSwitch : uint32_t { None = 0, OptionA = 1, OptionB = 2 };

struct Uni {
    UniSwitch switchField;

    union {
        double optionA;
        UA_String optionB;
    } fields;
};

const opcua::DataType& getUniDataType() {
    static const opcua::DataType dt =
        opcua::DataTypeBuilder<Uni>::createUnion("Uni", {1, 4845}, {1, 4})
            .addUnionField<&Uni::fields, double>("optionA")
            .addUnionField<&Uni::fields, UA_String>("optionB", UA_TYPES[UA_TYPES_STRING])
            .build();
    return dt;
}

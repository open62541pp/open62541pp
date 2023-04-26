#include <array>
#include <utility>  // move
#include <vector>

#include <doctest/doctest.h>

#include "open62541pp/Common.h"
#include "open62541pp/detail/helper.h"  // detail::toString
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

using namespace opcua;

TEST_CASE_TEMPLATE("StringLike", T, String, ByteString, XmlElement) {
    SUBCASE("Construct with const char*") {
        T wrapper("test");
        CHECK(wrapper.handle()->length == 4);
        CHECK(std::string(wrapper.get()) == "test");
    }

    SUBCASE("Construct from non-null-terminated view") {
        std::string str("test123");
        std::string_view sv(str.c_str(), 4);
        T wrapper(sv);
        CHECK(std::string(wrapper.get()) == "test");
    }

    SUBCASE("Empty") {
        CHECK(T().empty());
        CHECK_FALSE(T("test").empty());
    }

    SUBCASE("Equality") {
        CHECK(T("test") == T("test"));
        CHECK(T("test") != T());
    }
}

TEST_CASE("Guid") {
    SUBCASE("Construct") {
        UA_UInt32 data1{11};
        UA_UInt16 data2{22};
        UA_UInt16 data3{33};
        std::array<UA_Byte, 8> data4{1, 2, 3, 4, 5, 6, 7, 8};

        const Guid wrapper(data1, data2, data3, data4);

        CHECK(wrapper.handle()->data1 == data1);
        CHECK(wrapper.handle()->data2 == data2);
        CHECK(wrapper.handle()->data3 == data3);
        for (int i = 0; i < 8; ++i) {
            CHECK(wrapper.handle()->data4[i] == data4[i]);  // NOLINT
        }
    }

    SUBCASE("Random") {
        CHECK(Guid::random() != Guid());
        CHECK(Guid::random() != Guid::random());
    }

    SUBCASE("toString") {
        {
            const Guid guid{};
            CHECK(guid.toString() == "00000000-0000-0000-0000-000000000000");
        }
        {
            const Guid guid{3298187146, 3582, 19343, {135, 10, 116, 82, 56, 198, 174, 174}};
            CHECK(guid.toString() == "C496578A-0DFE-4B8F-870A-745238C6AEAE");
        }
    }
}

TEST_CASE("DateTime") {
    SUBCASE("Empty") {
        const DateTime dt;
        CHECK(dt.get() == 0);
        CHECK(*dt.handle() == 0);
        // UA time starts before Unix time -> 0
        CHECK(dt.toTimePoint().time_since_epoch().count() == 0);
        CHECK(dt.toUnixTime() == 0);

        const auto dts = dt.toStruct();
        CHECK(dts.nanoSec == 0);
        CHECK(dts.microSec == 0);
        CHECK(dts.milliSec == 0);
        CHECK(dts.sec == 0);
        CHECK(dts.min == 0);
        CHECK(dts.hour == 0);
        CHECK(dts.day == 1);
        CHECK(dts.month == 1);
        CHECK(dts.year == 1601);
    }

    SUBCASE("Static methods") {
        CHECK_NOTHROW(DateTime::localTimeUtcOffset());
    }

    SUBCASE("From std::chrono::time_point") {
        using namespace std::chrono;

        const auto now = system_clock::now();
        const int64_t secSinceEpoch = duration_cast<seconds>(now.time_since_epoch()).count();
        const int64_t nsecSinceEpoch = duration_cast<nanoseconds>(now.time_since_epoch()).count();

        const DateTime dt(now);
        CHECK(dt.get() == (nsecSinceEpoch / 100) + UA_DATETIME_UNIX_EPOCH);
        CHECK(dt.toUnixTime() == secSinceEpoch);
    }

    SUBCASE("Format") {
        CHECK(DateTime().format("%Y-%m-%d %H:%M:%S") == "1970-01-01 00:00:00");
    }

    SUBCASE("Comparison") {
        const auto zero = DateTime(0);
        const auto now = DateTime::now();
        CHECK(zero != now);
        CHECK(zero < now);
    }
}

TEST_CASE("NodeId") {
    SUBCASE("Copy") {
        NodeId src(1, 0);
        NodeId dst(src);
        CHECK(dst == src);
    }

    SUBCASE("Assignment") {
        NodeId src(1, 0);
        NodeId dst(2, 1);
        dst = src;
        CHECK(dst == src);
    }

    SUBCASE("Namespace index") {
        CHECK(NodeId(2, 1).getNamespaceIndex() == 2);
        CHECK(NodeId(0, 1).getNamespaceIndex() == 0);
    }

    SUBCASE("Comparison") {
        CHECK(NodeId(0, 1) == NodeId(0, 1));
        CHECK(NodeId(0, 1) <= NodeId(0, 1));
        CHECK(NodeId(0, 1) >= NodeId(0, 1));
        CHECK(NodeId(0, 1) != NodeId(0, 2));
        CHECK(NodeId(0, 1) != NodeId(1, 1));
        CHECK(NodeId(0, "a") == NodeId(0, "a"));
        CHECK(NodeId(0, "a") != NodeId(0, "b"));
        CHECK(NodeId(0, "a") != NodeId(1, "a"));

        // namespace index is compared before identifier
        CHECK(NodeId(0, 1) < NodeId(1, 0));
        CHECK(NodeId(0, 1) < NodeId(0, 2));

        CHECK(NodeId(1, "a") < NodeId(1, "b"));
        CHECK(NodeId(1, "b") > NodeId(1, "a"));
    }

    SUBCASE("Hash") {
        CHECK(NodeId(0, 1).hash() == NodeId(0, 1).hash());
        CHECK(NodeId(0, 1).hash() != NodeId(0, 2).hash());
        CHECK(NodeId(0, 1).hash() != NodeId(1, 1).hash());
    }

    SUBCASE("Get properties (getIdentifierType, getNamespaceIndex, getIdentifier") {
        {
            NodeId id(UA_NODEID_NUMERIC(1, 111));
            CHECK(id.getIdentifierType() == NodeIdType::Numeric);
            CHECK(id.getNamespaceIndex() == 1);
            CHECK(id.getIdentifierAs<NodeIdType::Numeric>() == 111);
        }
        {
            NodeId id(UA_NODEID_STRING_ALLOC(2, "Test123"));
            CHECK(id.getIdentifierType() == NodeIdType::String);
            CHECK(id.getNamespaceIndex() == 2);
            CHECK(id.getIdentifierAs<String>() == String("Test123"));
        }
        {
            Guid guid(11, 22, 33, {1, 2, 3, 4, 5, 6, 7, 8});
            NodeId id(3, guid);
            CHECK(id.getIdentifierType() == NodeIdType::Guid);
            CHECK(id.getNamespaceIndex() == 3);
            CHECK(id.getIdentifierAs<Guid>() == guid);
        }
        {
            ByteString byteString("Test456");
            NodeId id(4, byteString);
            CHECK(id.getIdentifierType() == NodeIdType::ByteString);
            CHECK(id.getNamespaceIndex() == 4);
            CHECK(id.getIdentifierAs<ByteString>() == byteString);
        }
    }
}

TEST_CASE("ExpandedNodeId") {
    ExpandedNodeId idLocal({1, "local"}, {}, 0);
    CHECK(idLocal.isLocal());
    CHECK(idLocal.getNodeId() == NodeId{1, "local"});
    CHECK(idLocal.getNodeId().handle() == &idLocal.handle()->nodeId);  // return ref
    CHECK(idLocal.getNamespaceUri().empty());
    CHECK(idLocal.getServerIndex() == 0);

    ExpandedNodeId idFull({1, "full"}, "namespace", 1);
    CHECK(idFull.getNodeId() == NodeId{1, "full"});
    CHECK(std::string(idFull.getNamespaceUri()) == "namespace");
    CHECK(idFull.getServerIndex() == 1);

    CHECK(idLocal == idLocal);
    CHECK(idLocal != idFull);
}

TEST_CASE("Variant") {
    SUBCASE("Empty variant") {
        Variant varEmpty;
        CHECK(varEmpty.isEmpty());
        CHECK(!varEmpty.isScalar());
        CHECK(!varEmpty.isArray());
        CHECK(varEmpty.getVariantType() == std::nullopt);
        CHECK(varEmpty.getArrayLength() == 0);
        CHECK(varEmpty.getArrayDimensions().empty());

        SUBCASE("Type checks") {
            CHECK_FALSE(varEmpty.isType(Type::Boolean));
            CHECK_FALSE(varEmpty.isType(Type::Int16));
            CHECK_FALSE(varEmpty.isType(Type::UInt16));
            CHECK_FALSE(varEmpty.isType(Type::Int32));
            CHECK_FALSE(varEmpty.isType(Type::UInt32));
            CHECK_FALSE(varEmpty.isType(Type::Int64));
            CHECK_FALSE(varEmpty.isType(Type::UInt64));
            CHECK_FALSE(varEmpty.isType(Type::Float));
            CHECK_FALSE(varEmpty.isType(Type::Double));
            // ...
        }
    }

    SUBCASE("Create from scalar") {
        SUBCASE("Assign if possible") {
            double value = 11.11;
            const auto var = Variant::fromScalar(value);
            CHECK(var.isScalar());
            CHECK(var->data == &value);
        }
        SUBCASE("Copy if const") {
            const double value = 11.11;
            const auto var = Variant::fromScalar(value);
            CHECK(var.isScalar());
            CHECK(var->data != &value);
        }
        SUBCASE("Copy rvalue") {
            auto var = Variant::fromScalar(11.11);
            CHECK(var.isScalar());
            CHECK(var.getScalar<double>() == 11.11);
        }
        SUBCASE("Copy if not assignable (const or conversion CHECKd)") {
            std::string value{"test"};
            const auto var = Variant::fromScalar<std::string, Type::String>(value);
            CHECK(var.isScalar());
            CHECK(var->data != &value);
        }
    }

    SUBCASE("Create from array") {
        SUBCASE("Assign if possible") {
            std::vector<double> vec{1.1, 2.2, 3.3};
            const auto var = Variant::fromArray(vec.data(), vec.size());
            CHECK(var.isArray());
            CHECK(var->data == vec.data());
        }
        SUBCASE("Copy if const") {
            const std::vector<double> vec{1.1, 2.2, 3.3};
            const auto var = Variant::fromArray(vec);
            CHECK(var.isArray());
            CHECK(var->data != vec.data());
        }
        SUBCASE("Copy from iterator") {
            const std::vector<double> vec{1.1, 2.2, 3.3};
            const auto var = Variant::fromArray(vec.begin(), vec.end());
            CHECK(var.isArray());
            CHECK(var->data != vec.data());
        }
    }

    SUBCASE("Set/get scalar") {
        Variant var;
        int32_t value = 5;
        var.setScalar(value);

        CHECK(var.isScalar());
        CHECK(var.isType(&UA_TYPES[UA_TYPES_INT32]));
        CHECK(var.isType(Type::Int32));
        CHECK(var.isType(NodeId{0, UA_NS0ID_INT32}));
        CHECK(var.getVariantType().value() == Type::Int32);

        CHECK_THROWS(var.getScalar<bool>());
        CHECK_THROWS(var.getScalar<int16_t>());
        CHECK_THROWS(var.getArrayCopy<int32_t>());
        CHECK(var.getScalar<int32_t>() == value);
        CHECK(var.getScalarCopy<int32_t>() == value);
    }

    SUBCASE("Set/get scalar reference") {
        Variant var;
        int value = 3;
        var.setScalar(value);
        int& ref = var.getScalar<int>();
        CHECK(ref == value);
        CHECK(&ref == &value);

        value++;
        CHECK(ref == value);
        ref++;
        CHECK(ref == value);
    }

    SUBCASE("Set/get mixed scalar types") {
        Variant var;

        var.setScalarCopy(static_cast<int>(11));
        CHECK(var.getScalar<int>() == 11);
        CHECK(var.getScalarCopy<int>() == 11);

        var.setScalarCopy(static_cast<float>(11.11));
        CHECK(var.getScalar<float>() == 11.11f);
        CHECK(var.getScalarCopy<float>() == 11.11f);

        var.setScalarCopy(static_cast<short>(1));
        CHECK(var.getScalar<short>() == 1);
        CHECK(var.getScalarCopy<short>() == 1);
    }

    SUBCASE("Set/get wrapped scalar types") {
        Variant var;

        {
            TypeWrapper<int32_t, UA_TYPES_INT32> value(10);
            var.setScalar(value);
            CHECK(var.getScalar<int32_t>() == 10);
            CHECK(var.getScalarCopy<int32_t>() == 10);
        }
        {
            LocalizedText value("en-US", "text");
            var.setScalar(value);
            CHECK(var.getScalarCopy<LocalizedText>() == value);
        }
    }

    SUBCASE("Set/get array (copy)") {
        Variant var;
        std::vector<float> value{0, 1, 2, 3, 4, 5};
        var.setArrayCopy(value);

        CHECK(var.isArray());
        CHECK(var.isType(Type::Float));
        CHECK(var.isType(NodeId{0, UA_NS0ID_FLOAT}));
        CHECK(var.getVariantType().value() == Type::Float);
        CHECK(var.getArrayLength() == value.size());
        CHECK(var.handle()->data != value.data());

        CHECK_THROWS(var.getArrayCopy<int32_t>());
        CHECK_THROWS(var.getArrayCopy<bool>());
        CHECK(var.getArrayCopy<float>() == value);
    }

    SUBCASE("Set/get array reference") {
        Variant var;
        std::vector<float> value{0, 1, 2};
        var.setArray(value);
        CHECK(var.getArray<float>() == value.data());
        CHECK(var.getArrayCopy<float>() == value);

        std::vector<float> valueChanged({3, 4, 5});
        value.assign(valueChanged.begin(), valueChanged.end());
        CHECK(var.getArrayCopy<float>() == valueChanged);
    }

    SUBCASE("Set array of native strings") {
        Variant var;
        std::array array{
            detail::allocUaString("item1"),
            detail::allocUaString("item2"),
            detail::allocUaString("item3"),
        };

        var.setArray<UA_String, Type::String>(array.data(), array.size());
        CHECK(var.getArrayLength() == array.size());
        CHECK(var.handle()->data == array.data());

        UA_clear(&array[0], &UA_TYPES[UA_TYPES_STRING]);
        UA_clear(&array[1], &UA_TYPES[UA_TYPES_STRING]);
        UA_clear(&array[2], &UA_TYPES[UA_TYPES_STRING]);
    }

    SUBCASE("Set/get array of strings") {
        Variant var;
        std::vector<std::string> value{"a", "b", "c"};
        var.setArrayCopy<std::string, Type::String>(value);

        CHECK(var.isArray());
        CHECK(var.isType(Type::String));
        CHECK(var.isType(NodeId{0, UA_NS0ID_STRING}));
        CHECK(var.getVariantType().value() == Type::String);

        CHECK_THROWS(var.getScalarCopy<std::string>());
        CHECK_THROWS(var.getArrayCopy<int32_t>());
        CHECK_THROWS(var.getArrayCopy<bool>());
        CHECK(var.getArrayCopy<std::string>() == value);
    }
}

TEST_CASE("DataValue") {
    SUBCASE("Create from scalar") {
        CHECK(DataValue::fromScalar(5).getValue().value().getScalar<int>() == 5);
    }

    SUBCASE("Create from aray") {
        std::vector<int> vec{1, 2, 3};
        CHECK(DataValue::fromArray(vec).getValue().value().getArrayCopy<int>() == vec);
    }

    SUBCASE("Empty") {
        DataValue dv{};
        CHECK(dv.getValuePtr() == nullptr);
        CHECK_FALSE(dv.getValue().has_value());
        CHECK_FALSE(dv.getSourceTimestamp().has_value());
        CHECK_FALSE(dv.getServerTimestamp().has_value());
        CHECK_FALSE(dv.getSourcePicoseconds().has_value());
        CHECK_FALSE(dv.getServerPicoseconds().has_value());
        CHECK_FALSE(dv.getStatusCode().has_value());
    }

    SUBCASE("Constructor with all optional parameter empty") {
        DataValue dv({}, {}, {}, {}, {}, {});
        CHECK(dv.getValuePtr() != nullptr);
        CHECK(dv.getValuePtr()->handle() == &dv.handle()->value);
        CHECK(dv.getValue().has_value());
        CHECK_FALSE(dv.getSourceTimestamp().has_value());
        CHECK_FALSE(dv.getServerTimestamp().has_value());
        CHECK_FALSE(dv.getSourcePicoseconds().has_value());
        CHECK_FALSE(dv.getServerPicoseconds().has_value());
        CHECK_FALSE(dv.getStatusCode().has_value());
    }

    SUBCASE("Constructor with all optional parameter specified") {
        Variant var;
        DataValue dv(var, DateTime{1}, DateTime{2}, 3, 4, 5);
        CHECK(dv.getValuePtr() != nullptr);
        CHECK(dv.getValuePtr()->handle() == &dv.handle()->value);
        CHECK(dv.getValue().has_value());
        CHECK(dv.getSourceTimestamp().value() == DateTime{1});
        CHECK(dv.getServerTimestamp().value() == DateTime{2});
        CHECK(dv.getSourcePicoseconds().value() == 3);
        CHECK(dv.getServerPicoseconds().value() == 4);
        CHECK(dv.getStatusCode().value() == 5);
    }

    SUBCASE("Setter methods") {
        DataValue dv;
        SUBCASE("Value (move)") {
            float value = 11.11f;
            Variant var;
            var.setScalar(value);
            CHECK(var->data == &value);
            dv.setValue(std::move(var));
            CHECK(dv.getValue().value().getScalar<float>() == value);
            CHECK(dv->value.data == &value);
        }
        SUBCASE("Value (copy)") {
            float value = 11.11f;
            Variant var;
            var.setScalar(value);
            dv.setValue(var);
            CHECK(dv.getValue().value().getScalar<float>() == value);
        }
        SUBCASE("Source timestamp") {
            DateTime dt{123};
            dv.setSourceTimestamp(dt);
            CHECK(dv.getSourceTimestamp().value() == dt);
        }
        SUBCASE("Server timestamp") {
            DateTime dt{456};
            dv.setServerTimestamp(dt);
            CHECK(dv.getServerTimestamp().value() == dt);
        }
        SUBCASE("Source picoseconds") {
            const uint16_t ps = 123;
            dv.setSourcePicoseconds(ps);
            CHECK(dv.getSourcePicoseconds().value() == ps);
        }
        SUBCASE("Server picoseconds") {
            const uint16_t ps = 456;
            dv.setServerPicoseconds(ps);
            CHECK(dv.getServerPicoseconds().value() == ps);
        }
        SUBCASE("Status code") {
            const UA_StatusCode statusCode = UA_STATUSCODE_BADALREADYEXISTS;
            dv.setStatusCode(statusCode);
            CHECK(dv.getStatusCode().value() == statusCode);
        }
    }
}

TEST_CASE("BrowseDescription") {
    BrowseDescription bd(NodeId(1, 1000), BrowseDirection::Forward);
    CHECK(bd.getNodeId() == NodeId(1, 1000));
    CHECK(bd.getBrowseDirection() == BrowseDirection::Forward);
    CHECK(bd.getReferenceTypeId() == NodeId(0, UA_NS0ID_REFERENCES));
    CHECK(bd.getIncludeSubtypes() == true);
    CHECK(bd.getNodeClassMask() == UA_NODECLASS_UNSPECIFIED);
    CHECK(bd.getResultMask() == UA_BROWSERESULTMASK_ALL);
}

TEST_CASE("RelativePathElement") {
    const RelativePathElement rpe(ReferenceType::HasComponent, false, false, {0, "test"});
    CHECK(rpe.getReferenceTypeId() == NodeId{0, UA_NS0ID_HASCOMPONENT});
    CHECK(rpe.getIsInverse() == false);
    CHECK(rpe.getIncludeSubtypes() == false);
    CHECK(rpe.getTargetName() == QualifiedName(0, "test"));
}

TEST_CASE("RelativePath") {
    const RelativePath rp{
        {ReferenceType::HasComponent, false, false, {0, "child1"}},
        {ReferenceType::HasComponent, false, false, {0, "child2"}},
    };
    const auto elements = rp.getElements();
    CHECK(elements.size() == 2);
    CHECK(elements.at(0).getTargetName() == QualifiedName(0, "child1"));
    CHECK(elements.at(1).getTargetName() == QualifiedName(0, "child2"));
}

TEST_CASE("BrowsePath") {
    const BrowsePath bp(
        {0, UA_NS0ID_OBJECTSFOLDER}, {{ReferenceType::HasComponent, false, false, {0, "child"}}}
    );
    CHECK(bp.getStartingNode() == NodeId(0, UA_NS0ID_OBJECTSFOLDER));
    CHECK(bp.getRelativePath().getElements().size() == 1);
}

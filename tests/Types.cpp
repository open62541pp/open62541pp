#include <array>
#include <sstream>
#include <string>
#include <utility>  // move
#include <vector>

#include <doctest/doctest.h>

#include "open62541pp/Common.h"
#include "open62541pp/Config.h"
#include "open62541pp/NodeIds.h"
#include "open62541pp/detail/helper.h"  // detail::toString
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/ExtensionObject.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

using namespace opcua;

TEST_CASE("StatusCode") {
    SUBCASE("Good") {
        StatusCode code;
        CHECK(code == UA_STATUSCODE_GOOD);
        CHECK(code.get() == UA_STATUSCODE_GOOD);
        CHECK(code.name() == "Good");
        CHECK(code.isGood());
        CHECK(!code.isUncertain());
        CHECK(!code.isBad());
        CHECK_NOTHROW(code.throwIfBad());
    }

#ifdef UA_STATUSCODE_UNCERTAIN
    SUBCASE("Uncertain") {
        StatusCode code(UA_STATUSCODE_UNCERTAIN);
        CHECK(code == UA_STATUSCODE_UNCERTAIN);
        CHECK(code.get() == UA_STATUSCODE_UNCERTAIN);
        CHECK(code.name() == "Uncertain");
        CHECK(!code.isGood());
        CHECK(code.isUncertain());
        CHECK(!code.isBad());
        CHECK_NOTHROW(code.throwIfBad());
    }
#endif

    SUBCASE("Bad") {
        StatusCode code(UA_STATUSCODE_BADTIMEOUT);
        CHECK(code == UA_STATUSCODE_BADTIMEOUT);
        CHECK(code.get() == UA_STATUSCODE_BADTIMEOUT);
        CHECK(code.name() == "BadTimeout");
        CHECK(!code.isGood());
        CHECK(!code.isUncertain());
        CHECK(code.isBad());
        CHECK_THROWS_AS_MESSAGE(code.throwIfBad(), BadStatus, "BadTimeout");
    }
}

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

TEST_CASE_TEMPLATE("StringLike equality overloads", T, String, ByteString) {
    CHECK(T("test") == std::string("test"));
    CHECK(T("test") != std::string("abc"));
    CHECK(std::string("test") == T("test"));
    CHECK(std::string("test") != T("abc"));
}

TEST_CASE_TEMPLATE("StringLike ostream overloads", T, String, XmlElement) {
    std::ostringstream ss;
    ss << T("test123");
    CHECK(ss.str() == "test123");
}

TEST_CASE_TEMPLATE("StringLike implicit conversion to string_view", T, String, XmlElement) {
    T str("test123");
    std::string_view view = str;
    CHECK(view == str.get());
}

TEST_CASE("ByteString") {
    SUBCASE("Construct from string") {
        const ByteString bs("XYZ");
        CHECK(bs->length == 3);
        CHECK(bs->data[0] == 88);
        CHECK(bs->data[1] == 89);
        CHECK(bs->data[2] == 90);
    }

    SUBCASE("Construct from vector") {
        const ByteString bs({88, 89, 90});
        CHECK(bs->length == 3);
        CHECK(bs->data[0] == 88);
        CHECK(bs->data[1] == 89);
        CHECK(bs->data[2] == 90);
        CHECK(std::string(bs.get()) == "XYZ");
    }

    SUBCASE("toFile / fromFile") {
        const ByteString bs({88, 89, 90});
        CHECK_NOTHROW(bs.toFile("bytestring.bin"));
        CHECK(ByteString::fromFile("bytestring.bin") == bs);
    }

#if UAPP_OPEN62541_VER_GE(1, 1)
    SUBCASE("fromBase64 / to Base64") {
        CHECK(ByteString::fromBase64("dGVzdDEyMw==") == ByteString("test123"));
        CHECK(ByteString("test123").toBase64() == "dGVzdDEyMw==");
    }
#endif
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

    SUBCASE("ostream overload") {
        std::ostringstream ss;
        ss << Guid{};
        CHECK(ss.str() == "00000000-0000-0000-0000-000000000000");
    }
}

TEST_CASE("NumericRangeDimension") {
    CHECK(NumericRangeDimension{} == NumericRangeDimension{});
    CHECK(NumericRangeDimension{1, 2} == NumericRangeDimension{1, 2});
    CHECK(NumericRangeDimension{1, 2} != NumericRangeDimension{1, 3});
}

TEST_CASE("NumericRange") {
    SUBCASE("Construct from native") {
        UA_NumericRange native{};
        std::vector<UA_NumericRangeDimension> dimensions{{1, 2}, {3, 4}};
        native.dimensionsSize = dimensions.size();
        native.dimensions = dimensions.data();
        const NumericRange nr(native);
        CHECK(!nr.empty());
        CHECK(nr.get().size() == 2);
        CHECK(nr.get().at(0) == NumericRangeDimension{1, 2});
        CHECK(nr.get().at(1) == NumericRangeDimension{3, 4});
    }

    SUBCASE("Empty") {
        const NumericRange nr;
        CHECK(nr.empty());
        CHECK(nr.get().size() == 0);
    }

    SUBCASE("Parse invalid") {
        CHECK_THROWS(NumericRange("abc"));
    }

    SUBCASE("Parse") {
        const NumericRange nr("1:2,0:3,5");
        CHECK(nr.get().size() == 3);
        CHECK(nr.get().at(0) == NumericRangeDimension{1, 2});
        CHECK(nr.get().at(1) == NumericRangeDimension{0, 3});
        CHECK(nr.get().at(2) == NumericRangeDimension{5, 5});
    }

    SUBCASE("toString") {
        CHECK(NumericRange({{1, 1}}).toString() == "1");
        CHECK(NumericRange({{1, 2}}).toString() == "1:2");
        CHECK(NumericRange({{1, 2}, {0, 3}, {5, 5}}).toString() == "1:2,0:3,5");
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
    SUBCASE("Create from ids") {
        CHECK(NodeId(Type::Boolean) == NodeId(0, UA_NS0ID_BOOLEAN));
        CHECK(NodeId(DataTypeId::Boolean) == NodeId(0, UA_NS0ID_BOOLEAN));
        CHECK(NodeId(ReferenceTypeId::References) == NodeId(0, UA_NS0ID_REFERENCES));
        CHECK(NodeId(ObjectTypeId::BaseObjectType) == NodeId(0, UA_NS0ID_BASEOBJECTTYPE));
        CHECK(NodeId(VariableTypeId::BaseVariableType) == NodeId(0, UA_NS0ID_BASEVARIABLETYPE));
        CHECK(NodeId(ObjectId::RootFolder) == NodeId(0, UA_NS0ID_ROOTFOLDER));
        CHECK(NodeId(VariableId::LocalTime) == NodeId(0, UA_NS0ID_LOCALTIME));
        CHECK(NodeId(MethodId::AddCommentMethodType) == NodeId(0, UA_NS0ID_ADDCOMMENTMETHODTYPE));
    }

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

    SUBCASE("isNull") {
        CHECK(NodeId().isNull());
        CHECK_FALSE(NodeId(0, 1).isNull());
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

    SUBCASE("toString") {
        CHECK(NodeId(0, 13).toString() == "i=13");
        CHECK(NodeId(10, 1).toString() == "ns=10;i=1");
        CHECK(NodeId(10, "Hello:World").toString() == "ns=10;s=Hello:World");
        CHECK(NodeId(0, Guid()).toString() == "g=00000000-0000-0000-0000-000000000000");
#if UAPP_OPEN62541_VER_GE(1, 1)
        CHECK(NodeId(1, ByteString("test123")).toString() == "ns=1;b=dGVzdDEyMw==");
#endif
    }

    SUBCASE("std::hash specialization") {
        const NodeId id(1, "Test123");
        CHECK(std::hash<NodeId>{}(id) == id.hash());
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

    SUBCASE("Hash") {
        CHECK(ExpandedNodeId().hash() == ExpandedNodeId().hash());
        CHECK(ExpandedNodeId().hash() != idLocal.hash());
        CHECK(ExpandedNodeId().hash() != idFull.hash());
    }

    SUBCASE("toString") {
        CHECK_EQ(ExpandedNodeId({2, 10157}).toString(), "ns=2;i=10157");
        CHECK_EQ(
            ExpandedNodeId({2, 10157}, "http://test.org/UA/Data/", 0).toString(),
            "nsu=http://test.org/UA/Data/;ns=2;i=10157"
        );
        CHECK_EQ(
            ExpandedNodeId({2, 10157}, "http://test.org/UA/Data/", 1).toString(),
            "svr=1;nsu=http://test.org/UA/Data/;ns=2;i=10157"
        );
    }

    SUBCASE("std::hash specialization") {
        CHECK(std::hash<ExpandedNodeId>()(idLocal) == idLocal.hash());
    }
}

TEST_CASE("Variant") {
    SUBCASE("Empty variant") {
        Variant varEmpty;
        CHECK(varEmpty.isEmpty());
        CHECK(!varEmpty.isScalar());
        CHECK(!varEmpty.isArray());
        CHECK(varEmpty.getDataType() == nullptr);
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
        SUBCASE("Assign with custom datatype") {
            double value = 11.11;
            const auto var = Variant::fromScalar(value, UA_TYPES[UA_TYPES_DOUBLE]);
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
        SUBCASE("Copy if not assignable (const or conversion check failed)") {
            std::string value{"test"};
            const auto var = Variant::fromScalar<std::string>(value);
            CHECK(var.isScalar());
            CHECK(var->data != &value);
        }
        SUBCASE("Copy with custom data type") {
            const double value = 11.11;
            const auto var = Variant::fromScalar(value, UA_TYPES[UA_TYPES_DOUBLE]);
            CHECK(var.isScalar());
            CHECK(var->data != &value);
        }
    }

    SUBCASE("Create from array") {
        SUBCASE("Assign if possible") {
            std::vector<double> vec{1.1, 2.2, 3.3};
            const auto var = Variant::fromArray(vec);
            CHECK(var.isArray());
            CHECK(var->data == vec.data());
        }
        SUBCASE("Assign with custom data type") {
            std::vector<UA_WriteValue> vec{{}, {}};
            const auto var = Variant::fromArray(vec, UA_TYPES[UA_TYPES_WRITEVALUE]);
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
        SUBCASE("Copy with custom data type") {
            const std::vector<UA_WriteValue> vec{{}, {}};
            const auto var = Variant::fromArray(vec, UA_TYPES[UA_TYPES_WRITEVALUE]);
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
        CHECK(var.getDataType() == &UA_TYPES[UA_TYPES_INT32]);
        CHECK(var.getVariantType().value() == Type::Int32);
        CHECK(var.data() == &value);

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
        CHECK(&var.getScalar<int>() == &value);
        CHECK(&std::as_const(var).getScalar<int>() == &value);
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
            CHECK(var.getScalar<LocalizedText>() == value);
            CHECK(var.getScalarCopy<LocalizedText>() == value);
        }
    }

    SUBCASE("Set/get array (copy)") {
        Variant var;
        std::vector<float> array{0, 1, 2, 3, 4, 5};
        var.setArrayCopy(array);

        CHECK(var.isArray());
        CHECK(var.isType(Type::Float));
        CHECK(var.isType(NodeId{0, UA_NS0ID_FLOAT}));
        CHECK(var.getDataType() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(var.getVariantType().value() == Type::Float);
        CHECK(var.data() != array.data());
        CHECK(var.getArrayLength() == array.size());

        CHECK_THROWS(var.getArrayCopy<int32_t>());
        CHECK_THROWS(var.getArrayCopy<bool>());
        CHECK(var.getArrayCopy<float>() == array);
    }

    SUBCASE("Set/get array reference") {
        Variant var;
        std::vector<float> array{0, 1, 2};
        var.setArray(array);
        CHECK(var.getArray<float>().data() == array.data());
        CHECK(std::as_const(var).getArray<float>().data() == array.data());
        CHECK(var.getArrayCopy<float>() == array);

        std::vector<float> arrayChanged({3, 4, 5});
        array.assign(arrayChanged.begin(), arrayChanged.end());
        CHECK(var.getArrayCopy<float>() == arrayChanged);
    }

    SUBCASE("Set array of native strings") {
        Variant var;
        std::array array{
            detail::allocUaString("item1"),
            detail::allocUaString("item2"),
            detail::allocUaString("item3"),
        };

        var.setArray(Span{array.data(), array.size()}, UA_TYPES[UA_TYPES_STRING]);
        CHECK(var.data() == array.data());
        CHECK(var.getArrayLength() == array.size());

        UA_clear(&array[0], &UA_TYPES[UA_TYPES_STRING]);
        UA_clear(&array[1], &UA_TYPES[UA_TYPES_STRING]);
        UA_clear(&array[2], &UA_TYPES[UA_TYPES_STRING]);
    }

    SUBCASE("Set array of wrapper strings") {
        Variant var;
        std::vector<String> array{String{"item1"}, String{"item2"}, String{"item3"}};

        var.setArray(array);
        CHECK(var.data() == array.data());
        CHECK(var.getArrayLength() == array.size());
        CHECK(var.getArray<String>().data() == array.data());
    }

    SUBCASE("Set/get array of strings") {
        Variant var;
        std::vector<std::string> value{"a", "b", "c"};
        var.setArrayCopy(value);

        CHECK(var.isArray());
        CHECK(var.isType(Type::String));
        CHECK(var.isType(NodeId{0, UA_NS0ID_STRING}));
        CHECK(var.getDataType() == &UA_TYPES[UA_TYPES_STRING]);
        CHECK(var.getVariantType().value() == Type::String);

        CHECK_THROWS(var.getScalarCopy<std::string>());
        CHECK_THROWS(var.getArrayCopy<int32_t>());
        CHECK_THROWS(var.getArrayCopy<bool>());
        CHECK(var.getArrayCopy<std::string>() == value);
    }

    SUBCASE("Set array from initializer list") {
        Variant var;
        var.setArrayCopy<const int>({1, 2, 3});  // TODO: avoid manual template types
    }

    SUBCASE("Set/get non-builtin data types") {
        using CustomType = UA_WriteValue;
        const auto& dt = UA_TYPES[UA_TYPES_WRITEVALUE];

        Variant var;
        CustomType value{};
        value.attributeId = 1;

        SUBCASE("Scalar") {
            var.setScalar(value, dt);
            CHECK(var.isScalar());
            CHECK(var.getDataType() == &dt);
            CHECK(var.data() == &value);
            CHECK(var.getScalar<CustomType>().attributeId == 1);
        }

        SUBCASE("Scalar (copy)") {
            var.setScalarCopy(value, dt);
            CHECK(var.isScalar());
            CHECK(var.getDataType() == &dt);
            CHECK(var.data() != &value);
            CHECK(var.getScalar<CustomType>().attributeId == 1);
        }

        std::vector<CustomType> array(3);

        SUBCASE("Array") {
            var.setArray(array, dt);
            CHECK(var.isArray());
            CHECK(var.getDataType() == &dt);
            CHECK(var.data() == array.data());
            CHECK(var.getArrayLength() == 3);
            CHECK(var.getArray<CustomType>().data() == array.data());
        }

        SUBCASE("Array (copy)") {
            var.setArrayCopy(array, dt);
            CHECK(var.isArray());
            CHECK(var.getDataType() == &dt);
            CHECK(var.data() != array.data());
            CHECK(var.getArrayLength() == 3);
            CHECK(var.getArray<CustomType>().data() != array.data());
        }
    }
}

TEST_CASE("DataValue") {
    SUBCASE("Create from scalar") {
        CHECK(DataValue::fromScalar(5).getValue().getScalar<int>() == 5);
    }

    SUBCASE("Create from array") {
        std::vector<int> vec{1, 2, 3};
        CHECK(DataValue::fromArray(vec).getValue().getArrayCopy<int>() == vec);
    }

    SUBCASE("Empty") {
        DataValue dv{};
        CHECK_FALSE(dv.hasValue());
        CHECK_FALSE(dv.hasSourceTimestamp());
        CHECK_FALSE(dv.hasServerTimestamp());
        CHECK_FALSE(dv.hasSourcePicoseconds());
        CHECK_FALSE(dv.hasServerPicoseconds());
        CHECK_FALSE(dv.hasStatusCode());
    }

    SUBCASE("Constructor with all optional parameter empty") {
        DataValue dv({}, {}, {}, {}, {}, {});
        CHECK(dv.hasValue());
        CHECK_FALSE(dv.hasSourceTimestamp());
        CHECK_FALSE(dv.hasServerTimestamp());
        CHECK_FALSE(dv.hasSourcePicoseconds());
        CHECK_FALSE(dv.hasServerPicoseconds());
        CHECK_FALSE(dv.hasStatusCode());
    }

    SUBCASE("Constructor with all optional parameter specified") {
        DataValue dv(
            Variant::fromScalar(5),
            DateTime{1},
            DateTime{2},
            uint16_t{3},
            uint16_t{4},
            UA_STATUSCODE_BADINTERNALERROR
        );
        CHECK(dv.getValue().isScalar());
        CHECK(dv.getSourceTimestamp() == DateTime{1});
        CHECK(dv.getServerTimestamp() == DateTime{2});
        CHECK(dv.getSourcePicoseconds() == 3);
        CHECK(dv.getServerPicoseconds() == 4);
        CHECK(dv.getStatusCode() == UA_STATUSCODE_BADINTERNALERROR);
    }

    SUBCASE("Setter methods") {
        DataValue dv;
        SUBCASE("Value (move)") {
            float value = 11.11f;
            Variant var;
            var.setScalar(value);
            CHECK(var->data == &value);
            dv.setValue(std::move(var));
            CHECK(dv.getValue().getScalar<float>() == value);
            CHECK(dv->value.data == &value);
        }
        SUBCASE("Value (copy)") {
            float value = 11.11f;
            Variant var;
            var.setScalar(value);
            dv.setValue(var);
            CHECK(dv.getValue().getScalar<float>() == value);
        }
        SUBCASE("Source timestamp") {
            DateTime dt{123};
            dv.setSourceTimestamp(dt);
            CHECK(dv.getSourceTimestamp() == dt);
        }
        SUBCASE("Server timestamp") {
            DateTime dt{456};
            dv.setServerTimestamp(dt);
            CHECK(dv.getServerTimestamp() == dt);
        }
        SUBCASE("Source picoseconds") {
            const uint16_t ps = 123;
            dv.setSourcePicoseconds(ps);
            CHECK(dv.getSourcePicoseconds() == ps);
        }
        SUBCASE("Server picoseconds") {
            const uint16_t ps = 456;
            dv.setServerPicoseconds(ps);
            CHECK(dv.getServerPicoseconds() == ps);
        }
        SUBCASE("Status code") {
            const UA_StatusCode statusCode = UA_STATUSCODE_BADALREADYEXISTS;
            dv.setStatusCode(statusCode);
            CHECK(dv.getStatusCode() == statusCode);
        }
    }
}

TEST_CASE("ExtensionObject") {
    SUBCASE("Empty") {
        ExtensionObject obj;
        CHECK(obj.isEmpty());
        CHECK(obj.getEncoding() == ExtensionObjectEncoding::EncodedNoBody);
        CHECK(obj.getEncodedTypeId().value() == NodeId(0, 0));  // UA_NODEID_NULL
        CHECK(obj.getEncodedBody().value().empty());
        CHECK(obj.getDecodedDataType() == nullptr);
        CHECK(obj.getDecodedData() == nullptr);
    }

    SUBCASE("fromDecoded (type erased variant)") {
        int32_t value = 11;
        const auto obj = ExtensionObject::fromDecoded(&value, UA_TYPES[UA_TYPES_INT32]);
        CHECK(obj.getEncoding() == ExtensionObjectEncoding::DecodedNoDelete);
        CHECK(obj.isDecoded());
        CHECK(obj.getDecodedDataType() == &UA_TYPES[UA_TYPES_INT32]);
        CHECK(obj.getDecodedData() == &value);
    }

    SUBCASE("fromDecoded") {
        String value("test123");
        const auto obj = ExtensionObject::fromDecoded(value);
        CHECK(obj.getEncoding() == ExtensionObjectEncoding::DecodedNoDelete);
        CHECK(obj.isDecoded());
        CHECK(obj.getDecodedDataType() == &UA_TYPES[UA_TYPES_STRING]);
        CHECK(obj.getDecodedData() == value.handle());
    }

    SUBCASE("fromDecodedCopy") {
        auto obj = ExtensionObject::fromDecodedCopy(Variant::fromScalar(11.11));
        CHECK(obj.getEncoding() == ExtensionObjectEncoding::Decoded);
        CHECK(obj.isDecoded());
        CHECK(obj.getDecodedDataType() == &UA_TYPES[UA_TYPES_VARIANT]);
        auto* varPtr = static_cast<Variant*>(obj.getDecodedData());
        CHECK(varPtr != nullptr);
        CHECK(varPtr->getScalar<double>() == 11.11);
    }

    SUBCASE("getDecodedData") {
        double value = 11.11;
        auto obj = ExtensionObject::fromDecoded(value);
        CHECK(obj.getDecodedData() == &value);
        CHECK(obj.getDecodedData<int>() == nullptr);
        CHECK(obj.getDecodedData<double>() == &value);
    }
}

TEST_CASE("RequestHeader") {
    const auto now = DateTime::now();
    const RequestHeader header({1, 1000}, now, 1, 2, "auditEntryId", 10, {});
    CHECK(header.getAuthenticationToken() == NodeId(1, 1000));
    CHECK(header.getTimestamp() == now);
    CHECK(header.getRequestHandle() == 1);
    CHECK(header.getReturnDiagnostics() == 2);
    CHECK(header.getAuditEntryId() == String("auditEntryId"));
    CHECK(header.getAdditionalHeader().isEmpty());
}

TEST_CASE("UserTokenPolicy") {
    UserTokenPolicy userTokenPolicy(
        "policyId",
        UserTokenType::Username,
        "issuedTokenType",
        "issuerEndpointUrl",
        "securityPolicyUri"
    );
    CHECK(userTokenPolicy.getPolicyId() == String("policyId"));
    CHECK(userTokenPolicy.getTokenType() == UserTokenType::Username);
    CHECK(userTokenPolicy.getIssuedTokenType() == String("issuedTokenType"));
    CHECK(userTokenPolicy.getIssuerEndpointUrl() == String("issuerEndpointUrl"));
    CHECK(userTokenPolicy.getSecurityPolicyUri() == String("securityPolicyUri"));
}

TEST_CASE("NodeAttributes") {
    // getters/setters are generated by specialized macros
    // just test the macros with VariableAttributes here
    VariableAttributes attr;
    CHECK(attr.getSpecifiedAttributes() == 0U);

    SUBCASE("Primitive type") {
        attr.setWriteMask(UA_WRITEMASK_DATATYPE);
        CHECK(attr.getWriteMask() == UA_WRITEMASK_DATATYPE);
        CHECK(attr.getSpecifiedAttributes() == UA_NODEATTRIBUTESMASK_WRITEMASK);
    }

    SUBCASE("Cast type") {
        attr.setValueRank(ValueRank::TwoDimensions);
        CHECK(attr.getValueRank() == ValueRank::TwoDimensions);
        CHECK(attr.getSpecifiedAttributes() == UA_NODEATTRIBUTESMASK_VALUERANK);
    }

    SUBCASE("Wrapper type") {
        attr.setDisplayName({"", "Name"});
        CHECK(attr.getDisplayName() == LocalizedText{"", "Name"});
        CHECK(attr.getSpecifiedAttributes() == UA_NODEATTRIBUTESMASK_DISPLAYNAME);
    }

    SUBCASE("Array type") {
        CHECK(attr.getArrayDimensions().empty());
        attr.setArrayDimensions({1, 2});
        CHECK(attr.getArrayDimensions() == Span<const uint32_t>{1, 2});
        CHECK(attr.getSpecifiedAttributes() == UA_NODEATTRIBUTESMASK_ARRAYDIMENSIONS);
    }
}

TEST_CASE("NodeAttributes fluent interface") {
    const auto attr = NodeAttributes{}.setDisplayName({"", "displayName"}).setWriteMask(0xFFFFFFFF);
    CHECK(attr.getDisplayName() == LocalizedText("", "displayName"));
    CHECK(attr.getWriteMask() == 0xFFFFFFFF);
}

TEST_CASE_TEMPLATE("NodeAttributes setDataType", T, VariableAttributes, VariableTypeAttributes) {
    CHECK(T{}.setDataType(DataTypeId::Boolean).getDataType() == NodeId(DataTypeId::Boolean));
    CHECK(T{}.template setDataType<bool>().getDataType() == NodeId(DataTypeId::Boolean));
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
    const RelativePathElement rpe(ReferenceTypeId::HasComponent, false, false, {0, "test"});
    CHECK(rpe.getReferenceTypeId() == NodeId{0, UA_NS0ID_HASCOMPONENT});
    CHECK(rpe.getIsInverse() == false);
    CHECK(rpe.getIncludeSubtypes() == false);
    CHECK(rpe.getTargetName() == QualifiedName(0, "test"));
}

TEST_CASE("RelativePath") {
    const RelativePath rp{
        {ReferenceTypeId::HasComponent, false, false, {0, "child1"}},
        {ReferenceTypeId::HasComponent, false, false, {0, "child2"}},
    };
    const auto elements = rp.getElements();
    CHECK(elements.size() == 2);
    CHECK(elements[0].getTargetName() == QualifiedName(0, "child1"));
    CHECK(elements[1].getTargetName() == QualifiedName(0, "child2"));
}

TEST_CASE("BrowsePath") {
    const BrowsePath bp(
        {0, UA_NS0ID_OBJECTSFOLDER}, {{ReferenceTypeId::HasComponent, false, false, {0, "child"}}}
    );
    CHECK(bp.getStartingNode() == NodeId(0, UA_NS0ID_OBJECTSFOLDER));
    CHECK(bp.getRelativePath().getElements().size() == 1);
}

TEST_CASE("ReadValueId") {
    const ReadValueId rvid(NodeId(1, 1000), AttributeId::Value);
    CHECK(rvid.getNodeId() == NodeId(1, 1000));
    CHECK(rvid.getAttributeId() == AttributeId::Value);
    CHECK(rvid.getIndexRange().empty());
    CHECK(rvid.getDataEncoding() == QualifiedName());
}

TEST_CASE("ReadRequest") {
    const ReadRequest request(
        {},
        111.11,
        TimestampsToReturn::Both,
        {
            {{1, 1000}, AttributeId::Value},
        }
    );
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getMaxAge() == 111.11);
    CHECK(request.getTimestampsToReturn() == TimestampsToReturn::Both);
    CHECK(request.getNodesToRead().size() == 1);
    CHECK(request.getNodesToRead()[0].getNodeId() == NodeId(1, 1000));
    CHECK(request.getNodesToRead()[0].getAttributeId() == AttributeId::Value);
}

TEST_CASE("WriteValue") {
    const WriteValue wv({1, 1000}, AttributeId::Value, {}, DataValue::fromScalar(11.11));
    CHECK(wv.getNodeId() == NodeId(1, 1000));
    CHECK(wv.getAttributeId() == AttributeId::Value);
    CHECK(wv.getIndexRange().empty());
    CHECK(wv.getValue().getValue().getScalar<double>() == 11.11);
}

TEST_CASE("WriteRequest") {
    const WriteRequest request(
        {},
        {
            {{1, 1000}, AttributeId::Value, {}, DataValue::fromScalar(11.11)},
        }
    );
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getNodesToWrite().size() == 1);
    CHECK(request.getNodesToWrite()[0].getNodeId() == NodeId(1, 1000));
    CHECK(request.getNodesToWrite()[0].getAttributeId() == AttributeId::Value);
    CHECK(request.getNodesToWrite()[0].getValue().getValue().getScalar<double>() == 11.11);
}

TEST_CASE("WriteResponse") {
    const WriteResponse response;
    CHECK_NOTHROW(response.getResponseHeader());
    CHECK(response.getResults().empty());
    CHECK(response.getDiagnosticInfos().empty());
}

#ifdef UA_ENABLE_METHODCALLS

TEST_CASE("Argument") {
    const Argument argument(
        "name", {"", "description"}, DataTypeId::Int32, ValueRank::TwoDimensions, {2, 3}
    );
    CHECK(argument.getName() == String("name"));
    CHECK(argument.getDescription() == LocalizedText("", "description"));
    CHECK(argument.getDataType() == NodeId(DataTypeId::Int32));
    CHECK(argument.getValueRank() == ValueRank::TwoDimensions);
    CHECK(argument.getArrayDimensions().size() == 2);
    CHECK(argument.getArrayDimensions()[0] == 2);
    CHECK(argument.getArrayDimensions()[1] == 3);
}

#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS

TEST_CASE("ElementOperand") {
    CHECK(ElementOperand(11).getIndex() == 11);
}

TEST_CASE("LiteralOperand") {
    CHECK(LiteralOperand(Variant::fromScalar(11)).getValue().getScalar<int>() == 11);
    CHECK(LiteralOperand(11).getValue().getScalar<int>() == 11);
}

TEST_CASE("AttributeOperand") {
    const AttributeOperand operand(
        ObjectTypeId::BaseEventType,
        "alias",
        {
            {ReferenceTypeId::HasComponent, false, false, {0, "child1"}},
            {ReferenceTypeId::HasComponent, false, false, {0, "child2"}},
        },
        AttributeId::Value,
        {}
    );
    CHECK(operand.getNodeId() == NodeId(ObjectTypeId::BaseEventType));
    CHECK(operand.getAlias() == String("alias"));
    CHECK(operand.getBrowsePath().getElements().size() == 2);
    CHECK(operand.getAttributeId() == AttributeId::Value);
    CHECK(operand.getIndexRange().empty());
}

TEST_CASE("SimpleAttributeOperand") {
    const SimpleAttributeOperand operand(
        ObjectTypeId::BaseEventType, {{0, "child1"}, {0, "child2"}}, AttributeId::Value, {}
    );
    CHECK(operand.getTypeDefinitionId() == NodeId(ObjectTypeId::BaseEventType));
    CHECK(operand.getBrowsePath().size() == 2);
    CHECK(operand.getAttributeId() == AttributeId::Value);
    CHECK(operand.getIndexRange().empty());
}

TEST_CASE("ContentFilter(Element)") {
    const ContentFilter contentFilter{
        {FilterOperator::And, {ElementOperand(1), ElementOperand(2)}},
        {FilterOperator::OfType, {LiteralOperand(NodeId(ObjectTypeId::BaseEventType))}},
        {FilterOperator::Equals, {LiteralOperand(99), LiteralOperand(99)}},
    };

    auto elements = contentFilter.getElements();
    CHECK(elements.size() == 3);
    CHECK(elements[0].getFilterOperator() == FilterOperator::And);
    CHECK(elements[0].getFilterOperands().size() == 2);
    CHECK(elements[0].getFilterOperands()[0].getDecodedData<ElementOperand>()->getIndex() == 1);
    CHECK(elements[0].getFilterOperands()[1].getDecodedData<ElementOperand>()->getIndex() == 2);
    CHECK(elements[1].getFilterOperator() == FilterOperator::OfType);
    CHECK(elements[1].getFilterOperands().size() == 1);
    CHECK(elements[1].getFilterOperands()[0].getDecodedData<LiteralOperand>() != nullptr);
    CHECK(elements[2].getFilterOperator() == FilterOperator::Equals);
    CHECK(elements[2].getFilterOperands().size() == 2);
    CHECK(elements[2].getFilterOperands()[0].getDecodedData<LiteralOperand>() != nullptr);
    CHECK(elements[2].getFilterOperands()[1].getDecodedData<LiteralOperand>() != nullptr);
}

TEST_CASE("ContentFilter(Element) operators") {
    const ContentFilterElement filterElement(
        FilterOperator::GreaterThan,
        {
            SimpleAttributeOperand(
                ObjectTypeId::BaseEventType, {{0, "Severity"}}, AttributeId::Value
            ),
            LiteralOperand(200),
        }
    );

    const ContentFilter filter{
        {FilterOperator::And, {ElementOperand(1), ElementOperand(2)}},
        {FilterOperator::OfType, {LiteralOperand(NodeId(ObjectTypeId::BaseEventType))}},
        {FilterOperator::Equals, {LiteralOperand(99), LiteralOperand(99)}},
    };

    auto elementOperandIndex = [](const ExtensionObject& operand) {
        return operand.getDecodedData<ElementOperand>()->getIndex();
    };

    auto firstOperator = [](const ContentFilter& contentFilter) {
        return contentFilter.getElements()[0].getFilterOperator();
    };

    SUBCASE("Not") {
        const ContentFilter filterElementNot = !filterElement;
        CHECK(filterElementNot.getElements().size() == 2);
        CHECK(firstOperator(filterElementNot) == FilterOperator::Not);
        CHECK(elementOperandIndex(filterElementNot.getElements()[0].getFilterOperands()[0]) == 1);
        CHECK(filterElementNot.getElements()[1].getFilterOperator() == FilterOperator::GreaterThan);

        const ContentFilter filterNot = !filter;
        CHECK(filterNot.getElements().size() == 4);
        CHECK(firstOperator(filterNot) == FilterOperator::Not);
        CHECK(elementOperandIndex(filterNot.getElements()[0].getFilterOperands()[0]) == 1);
        CHECK(elementOperandIndex(filterNot.getElements()[1].getFilterOperands()[0]) == 2);
        CHECK(elementOperandIndex(filterNot.getElements()[1].getFilterOperands()[1]) == 3);
    }

    SUBCASE("And") {
        CHECK((filterElement && filterElement).getElements().size() == 3);
        CHECK((filterElement && filter).getElements().size() == 5);
        CHECK((filter && filterElement).getElements().size() == 5);
        CHECK((filter && filter).getElements().size() == 7);

        CHECK(firstOperator(filterElement && filterElement) == FilterOperator::And);
        CHECK(firstOperator(filterElement && filter) == FilterOperator::And);
        CHECK(firstOperator(filter && filterElement) == FilterOperator::And);
        CHECK(firstOperator(filter && filter) == FilterOperator::And);

        SUBCASE("Increment operand indexes") {
            const ContentFilter filterAdd = filter && filter;
            CHECK(filterAdd.getElements().size() == 7);
            // and
            CHECK(elementOperandIndex(filterAdd.getElements()[0].getFilterOperands()[0]) == 1);
            CHECK(elementOperandIndex(filterAdd.getElements()[0].getFilterOperands()[1]) == 4);
            // lhs and
            CHECK(elementOperandIndex(filterAdd.getElements()[1].getFilterOperands()[0]) == 2);
            CHECK(elementOperandIndex(filterAdd.getElements()[1].getFilterOperands()[1]) == 3);
            // rhs and
            CHECK(elementOperandIndex(filterAdd.getElements()[4].getFilterOperands()[0]) == 5);
            CHECK(elementOperandIndex(filterAdd.getElements()[4].getFilterOperands()[1]) == 6);
        }
    }

    SUBCASE("Or") {
        CHECK((filterElement || filterElement).getElements().size() == 3);
        CHECK((filterElement || filter).getElements().size() == 5);
        CHECK((filter || filterElement).getElements().size() == 5);
        CHECK((filter || filter).getElements().size() == 7);

        CHECK(firstOperator(filterElement || filterElement) == FilterOperator::Or);
        CHECK(firstOperator(filterElement || filter) == FilterOperator::Or);
        CHECK(firstOperator(filter || filterElement) == FilterOperator::Or);
        CHECK(firstOperator(filter || filter) == FilterOperator::Or);
    }
}

TEST_CASE("DataChangeFilter") {
    const DataChangeFilter dataChangeFilter(
        DataChangeTrigger::StatusValue, DeadbandType::Percent, 11.11
    );

    CHECK(dataChangeFilter.getTrigger() == DataChangeTrigger::StatusValue);
    CHECK(dataChangeFilter.getDeadbandType() == DeadbandType::Percent);
    CHECK(dataChangeFilter.getDeadbandValue() == 11.11);
}

TEST_CASE("EventFilter") {
    const EventFilter eventFilter(
        {
            {{}, {{0, "Time"}}, AttributeId::Value},
            {{}, {{0, "Severity"}}, AttributeId::Value},
            {{}, {{0, "Message"}}, AttributeId::Value},
        },
        {
            {FilterOperator::OfType, {LiteralOperand(NodeId(ObjectTypeId::BaseEventType))}},
        }
    );

    CHECK(eventFilter.getSelectClauses().size() == 3);
    CHECK(eventFilter.getWhereClause().getElements().size() == 1);
}

TEST_CASE("AggregateFilter") {
    const DateTime startTime = DateTime::now();
    AggregateConfiguration aggregateConfiguration{};
    aggregateConfiguration.useSlopedExtrapolation = true;

    const AggregateFilter aggregateFilter(
        startTime, ObjectId::AggregateFunction_Average, 11.11, aggregateConfiguration
    );

    CHECK(aggregateFilter.getStartTime() == startTime);
    CHECK(aggregateFilter.getAggregateType() == NodeId(ObjectId::AggregateFunction_Average));
    CHECK(aggregateFilter.getProcessingInterval() == 11.11);
    CHECK(aggregateFilter.getAggregateConfiguration().useSlopedExtrapolation == true);
}

#endif

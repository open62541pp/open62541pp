#include <sstream>
#include <string>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/string_utils.hpp"  // toNativeString
#include "open62541pp/types.hpp"
#include "open62541pp/ua/nodeids.hpp"
#include "open62541pp/ua/typeregistry.hpp"

using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::Message;
using namespace opcua;

TEST_CASE("StatusCode") {
    SECTION("Good") {
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
    SECTION("Uncertain") {
        StatusCode code{UA_STATUSCODE_UNCERTAIN};
        CHECK(code == UA_STATUSCODE_UNCERTAIN);
        CHECK(code.get() == UA_STATUSCODE_UNCERTAIN);
        CHECK(code.name() == "Uncertain");
        CHECK(!code.isGood());
        CHECK(code.isUncertain());
        CHECK(!code.isBad());
        CHECK_NOTHROW(code.throwIfBad());
    }
#endif

    SECTION("Bad") {
        StatusCode code{UA_STATUSCODE_BADTIMEOUT};
        CHECK(code == UA_STATUSCODE_BADTIMEOUT);
        CHECK(code.get() == UA_STATUSCODE_BADTIMEOUT);
        CHECK(code.name() == "BadTimeout");
        CHECK(!code.isGood());
        CHECK(!code.isUncertain());
        CHECK(code.isBad());
        CHECK_THROWS_MATCHES(code.throwIfBad(), BadStatus, Message("BadTimeout"));
    }
}

TEMPLATE_TEST_CASE("StringLikeMixin constructors", "", String, const String) {
    SECTION("Default") {
        TestType str;
        CHECK(str.size() == 0);
        CHECK(str.length() == 0);
        CHECK(str.empty());
        CHECK(str.data() == nullptr);
    }

    SECTION("From empty iterator pair") {
        std::string_view sv;
        TestType str{sv.begin(), sv.end()};
        CHECK(str.size() == 0);
        CHECK(str.length() == 0);
        CHECK(str.empty());
        CHECK(str.data() == nullptr);
    }

    SECTION("From iterator pair") {
        std::string_view sv{"abc"};
        TestType str{sv.begin(), sv.end()};
        CHECK(str.size() == 3);
        CHECK(str.length() == 3);
        CHECK_FALSE(str.empty());
        CHECK(str.data() != nullptr);
        CHECK(std::string_view(str.data(), str.size()) == sv);
    }

    SECTION("From iterator pair (input iterator, single-pass)") {
        std::istringstream ss("abc");  // allows only single-pass reading
        std::istream_iterator<char> first(ss), last;
        TestType str{first, last};
        CHECK(str.size() == 3);
        CHECK(str.length() == 3);
        CHECK_FALSE(str.empty());
        CHECK(str.data() != nullptr);
        CHECK(std::string_view(str.data(), str.size()) == "abc");
    }

    SECTION("From initializer list") {
        TestType str{'a', 'b', 'c'};
        CHECK(str.size() == 3);
        CHECK(str.length() == 3);
        CHECK_FALSE(str.empty());
        CHECK(str.data() != nullptr);
        CHECK(std::string_view(str.data(), str.size()) == "abc");
    }
}

TEMPLATE_TEST_CASE("StringLikeMixin element access", "", String, const String) {
    TestType str{'a', 'b', 'c'};

    SECTION("operator[]") {
        CHECK(str[0] == 'a');
        CHECK(str[1] == 'b');
        CHECK(str[2] == 'c');
    }

    SECTION("front() and back()") {
        CHECK(str.front() == 'a');
        CHECK(str.back() == 'c');
    }
}

TEMPLATE_TEST_CASE("StringLikeMixin iterators", "", String, const String) {
    TestType str{'a', 'b', 'c'};

    SECTION("begin(), end() iterators") {
        CHECK(*str.begin() == 'a');
        CHECK(*(str.begin() + 1) == 'b');
        CHECK(*(str.begin() + 2) == 'c');
        CHECK(str.end() - str.begin() == 3);

        std::string result;
        for (auto it = str.begin(); it != str.end(); ++it) {
            result += *it;
        }
        CHECK(result == "abc");
    }

    SECTION("rbegin(), rend() reverse iterators") {
        CHECK(*str.rbegin() == 'c');
        CHECK(*(str.rbegin() + 1) == 'b');
        CHECK(*(str.rbegin() + 2) == 'a');
        CHECK(str.rend() - str.rbegin() == 3);

        std::string result;
        for (auto it = str.rbegin(); it != str.rend(); ++it) {
            result += *it;
        }
        CHECK(result == "cba");
    }
}

TEMPLATE_TEST_CASE("StringLike constructors", "", String, XmlElement) {
    SECTION("From const char*") {
        TestType str{"hello"};
        CHECK(str.size() == 5);
        CHECK(str.length() == 5);
        CHECK_FALSE(str.empty());
        CHECK(str.data() != nullptr);
        CHECK(std::string_view(str.data(), str.size()) == "hello");
    }

    SECTION("From std::string_view") {
        std::string_view sv = "world";
        TestType str{sv};
        CHECK(str.size() == 5);
        CHECK(str.length() == 5);
        CHECK_FALSE(str.empty());
        CHECK(str.data() != nullptr);
        CHECK(std::string_view(str.data(), str.size()) == "world");
    }

    SECTION("From empty string") {
        TestType str{""};
        CHECK(str.size() == 0);
        CHECK(str.length() == 0);
        CHECK(str.empty());
        CHECK(str.data() != nullptr);
    }
}

TEMPLATE_TEST_CASE("StringLike assign const char*", "", String, XmlElement) {
    TestType str;
    str = "test123";
    CHECK(static_cast<std::string_view>(str) == "test123");
}

TEMPLATE_TEST_CASE("StringLike assign string_view", "", String, XmlElement) {
    TestType str;
    str = std::string_view{"test123"};
    CHECK(static_cast<std::string_view>(str) == "test123");
}

TEMPLATE_TEST_CASE("StringLike implicit conversion to string_view", "", String, XmlElement) {
    TestType str{"test123"};
    std::string_view view = str;
    CHECK(view == "test123");
}

TEMPLATE_TEST_CASE("StringLike explicit conversion to string_view", "", ByteString) {
    TestType str{"test123"};
    CHECK(static_cast<std::string_view>(str) == "test123");
}

TEMPLATE_TEST_CASE("StringLike equality overloads", "", String, ByteString, XmlElement) {
    CHECK(TestType{"test"} == TestType{"test"});
    CHECK(TestType{"test"} != TestType{});
}

TEMPLATE_TEST_CASE("StringLike equality overloads with std::string_view", "", String) {
    CHECK(TestType{"test"} == std::string_view{"test"});
    CHECK(TestType{"test"} != std::string_view{"abc"});
    CHECK(std::string_view{"test"} == TestType{"test"});
    CHECK(std::string_view{"test"} != TestType{"abc"});
}

TEMPLATE_TEST_CASE("StringLike ostream overloads", "", String) {
    std::ostringstream ss;
    ss << TestType{"test123"};
    CHECK(ss.str() == "test123");
}

TEST_CASE("ByteString") {
    SECTION("Construct from string") {
        const ByteString bs{"XYZ"};
        CHECK(bs->length == 3);
        CHECK(bs->data[0] == 88);
        CHECK(bs->data[1] == 89);
        CHECK(bs->data[2] == 90);
    }

    SECTION("Construct from vector") {
        const ByteString bs{{88, 89, 90}};
        CHECK(bs->length == 3);
        CHECK(bs->data[0] == 88);
        CHECK(bs->data[1] == 89);
        CHECK(bs->data[2] == 90);
    }

#if UAPP_OPEN62541_VER_GE(1, 1)
    SECTION("fromBase64 / to Base64") {
        CHECK(ByteString::fromBase64("dGVzdDEyMw==") == ByteString{"test123"});
        CHECK(ByteString{"test123"}.toBase64() == "dGVzdDEyMw==");
    }
#endif
}

TEST_CASE("Guid") {
    SECTION("Construct") {
        const Guid wrapper{11, 22, 33, {0, 1, 2, 3, 4, 5, 6, 7}};
        CHECK(wrapper.handle()->data1 == 11);
        CHECK(wrapper.handle()->data2 == 22);
        CHECK(wrapper.handle()->data3 == 33);
        CHECK(wrapper.handle()->data4[0] == 0);
        CHECK(wrapper.handle()->data4[1] == 1);
        CHECK(wrapper.handle()->data4[2] == 2);
        CHECK(wrapper.handle()->data4[3] == 3);
        CHECK(wrapper.handle()->data4[4] == 4);
        CHECK(wrapper.handle()->data4[5] == 5);
        CHECK(wrapper.handle()->data4[6] == 6);
        CHECK(wrapper.handle()->data4[7] == 7);
    }

    SECTION("Construct from single array") {
        std::array<UA_Byte, 16> data{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        const Guid wrapper{data};
        CHECK(wrapper.handle()->data1 == 0x00010203);
        CHECK(wrapper.handle()->data2 == 0x0405);
        CHECK(wrapper.handle()->data3 == 0x0607);
        CHECK(wrapper.handle()->data4[0] == 8);
        CHECK(wrapper.handle()->data4[1] == 9);
        CHECK(wrapper.handle()->data4[2] == 10);
        CHECK(wrapper.handle()->data4[3] == 11);
        CHECK(wrapper.handle()->data4[4] == 12);
        CHECK(wrapper.handle()->data4[5] == 13);
        CHECK(wrapper.handle()->data4[6] == 14);
        CHECK(wrapper.handle()->data4[7] == 15);
    }

    SECTION("random") {
        CHECK(Guid::random() != Guid());
        CHECK(Guid::random() != Guid::random());
    }

#if UAPP_HAS_PARSING
    SECTION("parse") {
        const auto guid = Guid::parse("12345678-1234-5678-1234-567890ABCDEF");
        CHECK(guid->data1 == 0x12345678);
        CHECK(guid->data2 == 0x1234);
        CHECK(guid->data3 == 0x5678);
        CHECK(guid->data4[0] == 0x12);
        CHECK(guid->data4[1] == 0x34);
        CHECK(guid->data4[2] == 0x56);
        CHECK(guid->data4[3] == 0x78);
        CHECK(guid->data4[4] == 0x90);
        CHECK(guid->data4[5] == 0xAB);
        CHECK(guid->data4[6] == 0xCD);
        CHECK(guid->data4[7] == 0xEF);
    }
#endif
}

TEST_CASE("DateTime") {
    SECTION("Empty") {
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

    SECTION("Static methods") {
        CHECK_NOTHROW(DateTime::localTimeUtcOffset());
    }

    SECTION("From std::chrono::time_point") {
        using namespace std::chrono;

        const auto now = system_clock::now();
        const int64_t secSinceEpoch = duration_cast<seconds>(now.time_since_epoch()).count();
        const int64_t nsecSinceEpoch = duration_cast<nanoseconds>(now.time_since_epoch()).count();

        const DateTime dt{now};
        CHECK(dt.get() == (nsecSinceEpoch / 100) + UA_DATETIME_UNIX_EPOCH);
        CHECK(dt.toUnixTime() == secSinceEpoch);
    }

    SECTION("Format") {
        CHECK(DateTime{}.format("%Y-%m-%d %H:%M:%S") == "1970-01-01 00:00:00");
    }

    SECTION("Comparison") {
        const auto zero = DateTime{0};
        const auto now = DateTime::now();
        CHECK(zero != now);
        CHECK(zero < now);
    }
}

TEST_CASE("NodeId") {
    SECTION("Numeric identifier") {
        NodeId id{1, 123};
        CHECK(id.identifierType() == NodeIdType::Numeric);
        CHECK(id.namespaceIndex() == 1);
        CHECK(id.identifier<uint32_t>() == 123);
        CHECK(id.identifierIf<uint32_t>() != nullptr);
        CHECK(*id.identifierIf<uint32_t>() == 123);
    }

    SECTION("String identifier") {
        String str{"Test456"};
        NodeId id{2, str};
        CHECK(id.identifierType() == NodeIdType::String);
        CHECK(id.namespaceIndex() == 2);
        // CHECK(id.identifier<String>() == str);
        CHECK(id.identifierIf<String>() != nullptr);
        // CHECK(*id.identifierIf<String>() == str);
    }

    SECTION("Guid identifier") {
        Guid guid = Guid::random();
        NodeId id{3, guid};
        CHECK(id.identifierType() == NodeIdType::Guid);
        CHECK(id.namespaceIndex() == 3);
        CHECK(id.identifier<Guid>() == guid);
        CHECK(id.identifierIf<Guid>() != nullptr);
        CHECK(*id.identifierIf<Guid>() == guid);
    }

    SECTION("ByteString identifier") {
        ByteString byteStr{"Test789"};
        NodeId id{4, byteStr};
        CHECK(id.identifierType() == NodeIdType::ByteString);
        CHECK(id.namespaceIndex() == 4);
        CHECK(id.identifier<ByteString>() == byteStr);
        CHECK(id.identifierIf<ByteString>() != nullptr);
        CHECK(*id.identifierIf<ByteString>() == byteStr);
    }

    SECTION("Construct from node id enums") {
        CHECK(NodeId{DataTypeId::Boolean} == NodeId{0, UA_NS0ID_BOOLEAN});
        CHECK(NodeId{ReferenceTypeId::References} == NodeId{0, UA_NS0ID_REFERENCES});
        CHECK(NodeId{ObjectTypeId::BaseObjectType} == NodeId{0, UA_NS0ID_BASEOBJECTTYPE});
        CHECK(NodeId{VariableTypeId::BaseVariableType} == NodeId{0, UA_NS0ID_BASEVARIABLETYPE});
        CHECK(NodeId{ObjectId::RootFolder} == NodeId{0, UA_NS0ID_ROOTFOLDER});
        CHECK(NodeId{VariableId::LocalTime} == NodeId{0, UA_NS0ID_LOCALTIME});
        CHECK(NodeId{MethodId::AddCommentMethodType} == NodeId{0, UA_NS0ID_ADDCOMMENTMETHODTYPE});
    }

#if UAPP_HAS_PARSING
    SECTION("parse") {
        const auto id = NodeId::parse("ns=2;s=Test");
        CHECK(id.namespaceIndex() == 2);
        CHECK(id.identifier<String>() == "Test");
    }
#endif

    SECTION("identifierIf/identifier") {
        NodeId id{1, 123};
        CHECK(id.identifierIf<uint32_t>() != nullptr);
        CHECK(id.identifierIf<String>() == nullptr);
        CHECK(id.identifierIf<Guid>() == nullptr);
        CHECK(id.identifierIf<ByteString>() == nullptr);
        CHECK_NOTHROW(id.identifier<uint32_t>());
        CHECK_THROWS_AS(id.identifier<String>(), TypeError);
        CHECK_THROWS_AS(id.identifier<Guid>(), TypeError);
        CHECK_THROWS_AS(id.identifier<ByteString>(), TypeError);
    }

    SECTION("isNull") {
        CHECK(NodeId{}.isNull());
        CHECK_FALSE(NodeId{0, 1}.isNull());
    }

    SECTION("hash") {
        CHECK(NodeId{0, 1}.hash() == NodeId{0, 1}.hash());
        CHECK(NodeId{0, 1}.hash() != NodeId{0, 2}.hash());
        CHECK(NodeId{0, 1}.hash() != NodeId{1, 1}.hash());
    }

    SECTION("Comparison") {
        CHECK(NodeId{0, 1} == NodeId{0, 1});
        CHECK(NodeId{0, 1} <= NodeId{0, 1});
        CHECK(NodeId{0, 1} >= NodeId{0, 1});
        CHECK(NodeId{0, 1} != NodeId{0, 2});
        CHECK(NodeId{0, 1} != NodeId{1, 1});
        CHECK(NodeId{0, "a"} == NodeId{0, "a"});
        CHECK(NodeId{0, "a"} != NodeId{0, "b"});
        CHECK(NodeId{0, "a"} != NodeId{1, "a"});

        // namespace index is compared before identifier
        CHECK(NodeId{0, 1} < NodeId{1, 0});
        CHECK(NodeId{0, 1} < NodeId{0, 2});

        CHECK(NodeId{1, "a"} < NodeId{1, "b"});
        CHECK(NodeId{1, "b"} > NodeId{1, "a"});
    }

    SECTION("std::hash specialization") {
        const NodeId id{1, "Test123"};
        CHECK(std::hash<NodeId>{}(id) == id.hash());
    }
}

TEST_CASE("ExpandedNodeId") {
    ExpandedNodeId idLocal{{1, "local"}, {}, 0};
    CHECK(idLocal.isLocal());
    CHECK(idLocal.nodeId() == NodeId{1, "local"});
    CHECK(idLocal.nodeId().handle() == &idLocal.handle()->nodeId);  // return ref
    CHECK(idLocal.namespaceUri().empty());
    CHECK(idLocal.serverIndex() == 0);

    ExpandedNodeId idFull{{1, "full"}, "namespace", 1};
    CHECK(idFull.nodeId() == NodeId{1, "full"});
    CHECK(std::string(idFull.namespaceUri()) == "namespace");
    CHECK(idFull.serverIndex() == 1);

    CHECK(idLocal == idLocal);
    CHECK(idLocal != idFull);

#if UAPP_HAS_PARSING
    SECTION("parse") {
        const auto id = ExpandedNodeId::parse("svr=1;nsu=http://example.org/UA/;i=1234");
        CHECK(id.serverIndex() == 1);
        CHECK(id.namespaceUri() == "http://example.org/UA/");
        CHECK(id.nodeId().namespaceIndex() == 0);
        CHECK(id.nodeId().identifier<uint32_t>() == 1234);
    }
#endif

    SECTION("hash") {
        CHECK(ExpandedNodeId{}.hash() == ExpandedNodeId{}.hash());
        CHECK(ExpandedNodeId{}.hash() != idLocal.hash());
        CHECK(ExpandedNodeId{}.hash() != idFull.hash());
    }

    SECTION("std::hash specialization") {
        CHECK(std::hash<ExpandedNodeId>()(idLocal) == idLocal.hash());
    }
}

TEST_CASE("Variant") {
    SECTION("Empty") {
        Variant var;
        CHECK(var.empty());
        CHECK_FALSE(var.isScalar());
        CHECK_FALSE(var.isArray());
        CHECK(var.type() == nullptr);
        CHECK(var.data() == nullptr);
        CHECK(std::as_const(var).data() == nullptr);
        CHECK(var.arrayLength() == 0);
        CHECK(var.arrayDimensions().empty());
        CHECK_THROWS(var.scalar<int>());
        CHECK_THROWS(var.array<int>());
    }

    SECTION("From native") {
        // assignment operator is overloaded for non-variant types -> test original overloads
        float value = 5;
        UA_Variant native{};
        native.type = &UA_TYPES[UA_TYPES_FLOAT];
        native.storageType = UA_VARIANT_DATA_NODELETE;
        native.data = &value;

        SECTION("lvalue (copy)") {
            Variant var{native};
            CHECK(var.type() == &UA_TYPES[UA_TYPES_FLOAT]);
            CHECK(var.data() != &value);
            CHECK(var.scalar<float>() == value);
        }
        SECTION("rvalue (move)") {
            Variant var{std::move(native)};
            CHECK(var.type() == &UA_TYPES[UA_TYPES_FLOAT]);
            CHECK(var.data() == &value);
        }
    }

    SECTION("From scalar") {
        double value = 11.11;
        const auto& type = UA_TYPES[UA_TYPES_DOUBLE];
        Variant var;

        SECTION("Pointer") {
            SECTION("Constructor") {
                var = Variant{&value};
            }
            SECTION("Constructor with type") {
                var = Variant{&value, type};
            }
            CHECK(var.isScalar());
            CHECK(var.type() == &type);
            CHECK(var.data() == &value);
            CHECK(var.scalar<double>() == value);
        }

        SECTION("Copy") {
            SECTION("Constructor lvalue") {
                var = Variant{value};
            }
            SECTION("Constructor rvalue") {
                var = Variant{std::move(value)};
            }
            SECTION("Constructor lvalue with type") {
                var = Variant{value, type};
            }
            SECTION("Constructor rvalue with type") {
                var = Variant{std::move(value), type};
            }
            CHECK(var.isScalar());
            CHECK(var.type() == &type);
            CHECK(var.data() != &value);
            CHECK(var.scalar<double>() == value);
        }
    }

    SECTION("From array") {
        std::vector<double> array{11.11, 22.22, 33.33};
        const auto& type = UA_TYPES[UA_TYPES_DOUBLE];
        Variant var;

        SECTION("Pointer") {
            SECTION("Constructor") {
                var = Variant{&array};
            }
            SECTION("Constructor with type") {
                var = Variant{&array, type};
            }
            CHECK(var.isArray());
            CHECK(var.type() == &type);
            CHECK(var.data() == array.data());
            CHECK(var.to<std::vector<double>>() == array);
        }

        SECTION("Copy") {
            SECTION("Constructor") {
                var = Variant{array};
            }
            SECTION("Constructor with type") {
                var = Variant{array, type};
            }
            SECTION("Constructor with iterator pair") {
                var = Variant{array.begin(), array.end()};
            }
            SECTION("Constructor with iterator pair and type") {
                var = Variant{array.begin(), array.end(), type};
            }
            CHECK(var.isArray());
            CHECK(var.type() == &type);
            CHECK(var.data() != array.data());
            CHECK(var.to<std::vector<double>>() == array);
        }
    }

    SECTION("Type checks") {
        Variant var;
        CHECK_FALSE(var.isType(&UA_TYPES[UA_TYPES_STRING]));
        CHECK_FALSE(var.isType(UA_TYPES[UA_TYPES_STRING]));
        CHECK_FALSE(var.isType(DataTypeId::String));
        CHECK_FALSE(var.isType<String>());
        CHECK(var.type() == nullptr);

        var->type = &UA_TYPES[UA_TYPES_STRING];
        CHECK(var.isType(&UA_TYPES[UA_TYPES_STRING]));
        CHECK(var.isType(UA_TYPES[UA_TYPES_STRING]));
        CHECK(var.isType(DataTypeId::String));
        CHECK(var.isType<String>());
        CHECK(var.type() == &UA_TYPES[UA_TYPES_STRING]);
    }

    SECTION("Set nullptr") {
        Variant var{42};
        float* ptr{nullptr};
        SECTION("assign") {
            var.assign(ptr);
        }
        SECTION("assign with type") {
            var.assign(ptr, UA_TYPES[UA_TYPES_FLOAT]);
        }
        SECTION("assignment operator") {
            var = ptr;
        }
        CHECK(var.empty());
        CHECK(var.type() == nullptr);
        CHECK(var.data() == nullptr);
    }

    SECTION("Set/get scalar (pointer)") {
        Variant var;
        int32_t value = 5;
        SECTION("assign") {
            var.assign(&value);
        }
        SECTION("assignment operator") {
            var = &value;
        }
        CHECK(var.isScalar());
        CHECK(var.data() == &value);
        CHECK(&var.scalar<int32_t>() == &value);
        CHECK(&std::as_const(var).scalar<int32_t>() == &value);
        CHECK(var.to<int32_t>() == value);
    }

    SECTION("Set/get scalar wrapper (pointer)") {
        Variant var;
        LocalizedText value{"en-US", "text"};
        SECTION("assign") {
            var.assign(&value);
        }
        SECTION("assignment operator") {
            var = &value;
        }
        CHECK(var.isScalar());
        CHECK(&var.scalar<LocalizedText>() == &value);
        CHECK(var.scalar<LocalizedText>() == value);
        CHECK(var.to<LocalizedText>() == value);
    }

    SECTION("Set/get scalar (copy)") {
        Variant var;
        double value = 11.11;
        SECTION("assign") {
            var.assign(value);
        }
        SECTION("assignment operator") {
            var = value;
        }
        CHECK(&var.scalar<double>() != &value);
        CHECK(var.scalar<double>() == value);
        CHECK(var.to<double>() == value);
    }

    SECTION("Set/get scalar (move)") {
        Variant var;
        String value{"test"};
        auto* data = value->data;
        SECTION("assign") {
            var.assign(std::move(value));
        }
        SECTION("assignment operator") {
            var = std::move(value);
        }
        CHECK(var.scalar<String>()->data == data);
        CHECK(var.scalar<String>() == "test");
        CHECK(var.to<String>() == "test");
    }

    SECTION("Set/get std::string (copy & convert)") {
        Variant var;
        std::string value{"test"};
        SECTION("assign") {
            var.assign(value);
        }
        SECTION("assignment operator") {
            var = value;
        }
        CHECK(var.scalar<String>().data() != value.data());
        CHECK(var.scalar<String>() == value);
        CHECK(var.to<std::string>() == value);
    }

    SECTION("Set/get array (pointer)") {
        Variant var;
        std::vector<float> array{0, 1, 2};
        SECTION("assign") {
            var.assign(&array);
        }
        SECTION("assignment operator") {
            var = &array;
        }
        CHECK(var.data() == array.data());
        CHECK(var.array<float>().data() == array.data());
        CHECK(std::as_const(var).array<float>().data() == array.data());
        CHECK(var.to<std::vector<float>>() == array);
    }

    SECTION("Set array of native strings (pointer)") {
        Variant var;
        std::array array{
            detail::toNativeString("item1"),
            detail::toNativeString("item2"),
            detail::toNativeString("item3"),
        };
        var.assign(&array, UA_TYPES[UA_TYPES_STRING]);
        CHECK(var.isArray());
        CHECK(var.data() == array.data());
        CHECK(var.arrayLength() == array.size());
    }

    SECTION("Set array of string wrapper (pointer)") {
        Variant var;
        std::vector<String> array{String{"item1"}, String{"item2"}, String{"item3"}};
        SECTION("assign") {
            var.assign(&array);
        }
        SECTION("assignment operator") {
            var = &array;
        }
        CHECK(var.data() == array.data());
        CHECK(var.arrayLength() == array.size());
        CHECK(var.array<String>().data() == array.data());
    }

    SECTION("Set/get empty array (copy)") {
        Variant var;
        std::vector<float> array;
        SECTION("assign") {
            var.assign(array);
        }
        SECTION("assign with iterator pair") {
            var.assign(array.begin(), array.end());
        }
        SECTION("assignment operator") {
            var = array;
        }
        CHECK(var.isArray());
        CHECK(var.type() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(var.data() == nullptr);
        CHECK(var.arrayLength() == 0);
        CHECK(var.array<float>().data() == nullptr);
    }

    SECTION("Set/get array (copy)") {
        Variant var;
        std::vector<float> array{0, 1, 2, 3, 4, 5};
        SECTION("assign") {
            var.assign(array);
        }
        SECTION("assign with iterator pair") {
            var.assign(array.begin(), array.end());
        }
        SECTION("assignment operator") {
            var = array;
        }
        CHECK(var.isArray());
        CHECK(var.type() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(var.data() != array.data());
        CHECK(var.arrayLength() == array.size());
        CHECK(var.to<std::vector<float>>() == array);
    }

    SECTION("Set array from initializer list (copy)") {
        Variant var;
        var.assign(Span<const int>{1, 2, 3});  // TODO: avoid manual template types
        CHECK(var.isArray());
        CHECK(var.type() == &UA_TYPES[UA_TYPES_INT32]);
        CHECK(var.arrayLength() == 3);
    }

    SECTION("Set/get array of std::string (copy & convert)") {
        Variant var;
        std::vector<std::string> array{"a", "b", "c"};
        SECTION("assign") {
            var.assign(array);
        }
        SECTION("assign with iterator pair") {
            var.assign(array.begin(), array.end());
        }
        SECTION("assignment operator") {
            var = array;
        }
        CHECK(var.isArray());
        CHECK(var.type() == &UA_TYPES[UA_TYPES_STRING]);
        CHECK(var.arrayLength() == array.size());
        CHECK_NOTHROW(var.array<String>());
        CHECK(var.to<std::vector<std::string>>() == array);
    }

    SECTION("Set/get array with std::vector<bool> (copy)") {
        // std::vector<bool> is a possibly space optimized template specialization which caused
        // several problems: https://github.com/open62541pp/open62541pp/issues/164
        Variant var;
        std::vector<bool> array{true, false, true};
        SECTION("assign") {
            var.assign(array);
        }
        SECTION("assign with iterator pair") {
            var.assign(array.begin(), array.end());
        }
        SECTION("assignment operator") {
            var = array;
        }
        CHECK(var.arrayLength() == array.size());
        CHECK(var.type() == &UA_TYPES[UA_TYPES_BOOLEAN]);
        CHECK(var.to<std::vector<bool>>() == array);
    }

    SECTION("Set/get custom type") {
        // same memory layout as int32_t
        struct Custom {
            int32_t number;
        };

        const auto& type = UA_TYPES[UA_TYPES_INT32];

        Variant var;
        Custom value{11};

        SECTION("Scalar (pointer)") {
            var.assign(&value, type);
            CHECK(var.isScalar());
            CHECK(var.type() == &type);
            CHECK(var.data() == &value);
        }

        SECTION("Scalar (copy)") {
            var.assign(value, type);
            CHECK(var.isScalar());
            CHECK(var.type() == &type);
            CHECK(var.data() != &value);
            CHECK(var.data() != nullptr);
            CHECK(static_cast<Custom*>(var.data())->number == 11);
        }

        std::vector<Custom> array{Custom{11}, Custom{22}, Custom{33}};

        SECTION("Array (pointer)") {
            var.assign(&array, type);
            CHECK(var.isArray());
            CHECK(var.type() == &type);
            CHECK(var.data() == array.data());
            CHECK(var.arrayLength() == 3);
        }

        SECTION("Array (copy)") {
            SECTION("Container") {
                var.assign(array, type);
            }
            SECTION("Iterator pair") {
                var.assign(array.begin(), array.end(), type);
            }
            CHECK(var.isArray());
            CHECK(var.type() == &type);
            CHECK(var.data() != array.data());
            CHECK(var.data() != nullptr);
            CHECK(static_cast<Custom*>(var.data())[0].number == 11);
            CHECK(static_cast<Custom*>(var.data())[1].number == 22);
            CHECK(static_cast<Custom*>(var.data())[2].number == 33);
            CHECK(var.arrayLength() == 3);
        }
    }

    SECTION("scalar ref qualifiers") {
        Variant var{"test"};
        const void* data = var.scalar<String>()->data;

        SECTION("lvalue") {
            const auto dst = var.scalar<String>();
            CHECK(dst->data != data);  // copy
        }
        SECTION("const lvalue") {
            const auto dst = std::as_const(var).scalar<String>();
            CHECK(dst->data != data);  // copy
        }
        SECTION("rvalue") {
            const auto dst = std::move(var).scalar<String>();
            CHECK(dst->data == data);  // move
        }
        SECTION("const rvalue") {
            const auto dst = std::move(std::as_const(var)).scalar<String>();
            CHECK(dst->data != data);  // can not move const -> copy
        }
    }

    SECTION("scalar ref qualifiers with borrowed value") {
        String str{"test"};
        Variant var{&str};
        const void* data = var.scalar<String>()->data;

        SECTION("rvalue") {
            const auto dst = std::move(var).scalar<String>();
            CHECK(dst->data != data);  // copy
        }

        CHECK(str == "test");
    }

    SECTION("to ref qualifiers") {
        Variant var{"test"};
        void* data = var.scalar<String>()->data;

        SECTION("lvalue") {
            const auto dst = var.to<String>();
            CHECK(dst->data != data);  // copy
        }
        SECTION("rvalue") {
            const auto dst = std::move(var).to<String>();
            CHECK(dst->data == data);  // move
        }
    }
}

TEST_CASE("DataValue") {
    SECTION("Create from scalar") {
        CHECK(DataValue{Variant{5}}.value().to<int>() == 5);
    }

    SECTION("Create from array") {
        std::vector<int> vec{1, 2, 3};
        CHECK(DataValue{Variant{vec}}.value().to<std::vector<int>>() == vec);
    }

    SECTION("Empty") {
        DataValue dv{};
        CHECK_FALSE(dv.hasValue());
        CHECK_FALSE(dv.hasSourceTimestamp());
        CHECK_FALSE(dv.hasServerTimestamp());
        CHECK_FALSE(dv.hasSourcePicoseconds());
        CHECK_FALSE(dv.hasServerPicoseconds());
        CHECK_FALSE(dv.hasStatus());
    }

    SECTION("Constructor with all optional parameter empty") {
        DataValue dv{{}, {}, {}, {}, {}, {}};
        CHECK(dv.hasValue());
        CHECK_FALSE(dv.hasSourceTimestamp());
        CHECK_FALSE(dv.hasServerTimestamp());
        CHECK_FALSE(dv.hasSourcePicoseconds());
        CHECK_FALSE(dv.hasServerPicoseconds());
        CHECK_FALSE(dv.hasStatus());
    }

    SECTION("Constructor with all optional parameter specified") {
        DataValue dv(
            Variant{5},
            DateTime{1},
            DateTime{2},
            uint16_t{3},
            uint16_t{4},
            UA_STATUSCODE_BADINTERNALERROR
        );
        CHECK(dv.value().isScalar());
        CHECK(dv.sourceTimestamp() == DateTime{1});
        CHECK(dv.serverTimestamp() == DateTime{2});
        CHECK(dv.sourcePicoseconds() == 3);
        CHECK(dv.serverPicoseconds() == 4);
        CHECK(dv.status() == UA_STATUSCODE_BADINTERNALERROR);
    }

    SECTION("Setter methods") {
        DataValue dv;
        SECTION("Value (move)") {
            float value = 11.11f;
            Variant var;
            var.assign(&value);
            CHECK(var->data == &value);
            dv.setValue(std::move(var));
            CHECK(dv.hasValue());
            CHECK(dv.value().scalar<float>() == value);
            CHECK(dv->value.data == &value);
        }
        SECTION("Value (copy)") {
            float value = 11.11f;
            Variant var;
            var.assign(value);
            dv.setValue(var);
            CHECK(dv.hasValue());
            CHECK(dv.value().scalar<float>() == value);
        }
        SECTION("Source timestamp") {
            DateTime dt{123};
            dv.setSourceTimestamp(dt);
            CHECK(dv.hasSourceTimestamp());
            CHECK(dv.sourceTimestamp() == dt);
        }
        SECTION("Server timestamp") {
            DateTime dt{456};
            dv.setServerTimestamp(dt);
            CHECK(dv.hasServerTimestamp());
            CHECK(dv.serverTimestamp() == dt);
        }
        SECTION("Source picoseconds") {
            const uint16_t ps = 123;
            dv.setSourcePicoseconds(ps);
            CHECK(dv.hasSourcePicoseconds());
            CHECK(dv.sourcePicoseconds() == ps);
        }
        SECTION("Server picoseconds") {
            const uint16_t ps = 456;
            dv.setServerPicoseconds(ps);
            CHECK(dv.hasServerPicoseconds());
            CHECK(dv.serverPicoseconds() == ps);
        }
        SECTION("Status") {
            const UA_StatusCode statusCode = UA_STATUSCODE_BADALREADYEXISTS;
            dv.setStatus(statusCode);
            CHECK(dv.hasStatus());
            CHECK(dv.status() == statusCode);
        }
    }

    SECTION("getValue (lvalue & rvalue)") {
        DataValue dv{Variant{11}};
        void* data = dv.value().data();

        Variant var;
        SECTION("rvalue") {
            var = dv.value();
            CHECK(var.data() != data);  // copy
        }
        SECTION("const rvalue") {
            var = std::as_const(dv).value();
            CHECK(var.data() != data);  // copy
        }
        SECTION("lvalue") {
            var = std::move(dv).value();
            CHECK(var.data() == data);  // move
        }
        SECTION("const lvalue") {
            var = std::move(std::as_const(dv)).value();
            CHECK(var.data() != data);  // can not move const -> copy
        }
        CHECK(var.scalar<int>() == 11);
    }
}

TEST_CASE("ExtensionObject") {
    SECTION("Empty") {
        ExtensionObject obj;
        CHECK(obj.empty());
        CHECK_FALSE(obj.isEncoded());
        CHECK_FALSE(obj.isDecoded());
        CHECK(obj.encoding() == ExtensionObjectEncoding::EncodedNoBody);
        CHECK(obj.encodedTypeId() == nullptr);
        CHECK(obj.encodedBinary() == nullptr);
        CHECK(obj.encodedXml() == nullptr);
        CHECK(obj.decodedType() == nullptr);
        CHECK(obj.decodedData() == nullptr);
    }

    SECTION("From nullptr") {
        int* value{nullptr};
        ExtensionObject obj{value};
        CHECK(obj.empty());
    }

    SECTION("From decoded (pointer)") {
        ExtensionObject obj;
        String value{"test123"};
        SECTION("Deduce data type") {
            obj = ExtensionObject(&value);
        }
        SECTION("Custom data type") {
            obj = ExtensionObject(&value, UA_TYPES[UA_TYPES_STRING]);
        }
        CHECK(obj.encoding() == ExtensionObjectEncoding::DecodedNoDelete);
        CHECK(obj.isDecoded());
        CHECK(obj.decodedType() == &UA_TYPES[UA_TYPES_STRING]);
        CHECK(obj.decodedData() == value.handle());
        CHECK(obj.decodedData<String>() == &value);
        CHECK(obj.decodedData<int>() == nullptr);
        CHECK(obj.decodedData<double>() == nullptr);
    }

    SECTION("From decoded (copy)") {
        ExtensionObject obj;
        const auto value = Variant(11.11);
        SECTION("Deduce data type") {
            obj = ExtensionObject(value);
        }
        SECTION("Custom data type") {
            obj = ExtensionObject(value, UA_TYPES[UA_TYPES_VARIANT]);
        }
        CHECK(obj.encoding() == ExtensionObjectEncoding::Decoded);
        CHECK(obj.isDecoded());
        CHECK(obj.decodedType() == &UA_TYPES[UA_TYPES_VARIANT]);
        CHECK(obj.decodedData() != nullptr);
        CHECK(obj.decodedData<Variant>() != nullptr);
        CHECK(obj.decodedData<Variant>()->scalar<double>() == 11.11);
    }

    SECTION("Encoded binary") {
        NodeId typeId{1, 1000};
        ExtensionObject obj;
        obj->encoding = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
        obj->content.encoded.typeId = typeId;
        obj->content.encoded.body = UA_STRING_ALLOC("binary");
        CHECK(obj.isEncoded());
        CHECK(obj.encoding() == ExtensionObjectEncoding::EncodedByteString);
        CHECK(obj.encodedTypeId() != nullptr);
        CHECK(*obj.encodedTypeId() == typeId);
        CHECK(obj.encodedBinary() != nullptr);
        CHECK(*obj.encodedBinary() == ByteString{"binary"});
        CHECK(obj.encodedXml() == nullptr);
    }

    SECTION("Encoded XML") {
        NodeId typeId{1, 1000};
        ExtensionObject obj;
        obj->encoding = UA_EXTENSIONOBJECT_ENCODED_XML;
        obj->content.encoded.typeId = typeId;
        obj->content.encoded.body = UA_STRING_ALLOC("xml");
        CHECK(obj.isEncoded());
        CHECK(obj.encoding() == ExtensionObjectEncoding::EncodedXml);
        CHECK(obj.encodedTypeId() != nullptr);
        CHECK(*obj.encodedTypeId() == typeId);
        CHECK(obj.encodedBinary() == nullptr);
        CHECK(obj.encodedXml() != nullptr);
        CHECK(*obj.encodedXml() == XmlElement("xml"));
    }
}

TEST_CASE("NumericRangeDimension") {
    CHECK((NumericRangeDimension{} == NumericRangeDimension{}));
    CHECK((NumericRangeDimension{1, 2} == NumericRangeDimension{1, 2}));
    CHECK((NumericRangeDimension{1, 2} != NumericRangeDimension{1, 3}));
}

TEST_CASE("NumericRange") {
    SECTION("Empty") {
        const NumericRange nr;
        CHECK(nr.empty());
        CHECK(nr.dimensions().size() == 0);
    }

    SECTION("From encoded range (invalid)") {
        CHECK_THROWS(NumericRange("abc"));
    }

    SECTION("From encoded range") {
        const NumericRange nr{"1:2,0:3,5"};
        CHECK(nr.dimensions().size() == 3);
        CHECK((nr.dimensions()[0] == NumericRangeDimension{1, 2}));
        CHECK((nr.dimensions()[1] == NumericRangeDimension{0, 3}));
        CHECK((nr.dimensions()[2] == NumericRangeDimension{5, 5}));
    }

    SECTION("From span") {
        std::vector<NumericRangeDimension> dimensions{{1, 2}, {3, 4}};
        const NumericRange nr{dimensions};
        CHECK_FALSE(nr.empty());
        CHECK(nr.dimensions().size() == 2);
        CHECK((nr.dimensions()[0] == NumericRangeDimension{1, 2}));
        CHECK((nr.dimensions()[1] == NumericRangeDimension{3, 4}));
    }

    SECTION("From native") {
        UA_NumericRange native{};
        std::vector<NumericRangeDimension> dimensions{{1, 2}, {3, 4}};
        native.dimensionsSize = dimensions.size();
        native.dimensions = dimensions.data();
        const NumericRange nr{native};
        CHECK_FALSE(nr.empty());
        CHECK(nr.dimensions().size() == 2);
        CHECK((nr.dimensions()[0] == NumericRangeDimension{1, 2}));
        CHECK((nr.dimensions()[1] == NumericRangeDimension{3, 4}));
    }

    SECTION("Copy & move") {
        std::vector<NumericRangeDimension> dimensions{{1, 2}, {3, 4}};
        NumericRange src{dimensions};

        SECTION("Copy constructor") {
            const NumericRange dst{src};
            CHECK(dst.dimensions().size() == 2);
            CHECK((dst.dimensions()[0] == NumericRangeDimension{1, 2}));
            CHECK((dst.dimensions()[1] == NumericRangeDimension{3, 4}));
        }

        SECTION("Move constructor") {
            const NumericRange dst{std::move(src)};
            CHECK(src.dimensions().size() == 0);
            CHECK(dst.dimensions().size() == 2);
            CHECK((dst.dimensions()[0] == NumericRangeDimension{1, 2}));
            CHECK((dst.dimensions()[1] == NumericRangeDimension{3, 4}));
        }

        SECTION("Copy assignment") {
            NumericRange dst;
            dst = src;
            CHECK(dst.dimensions().size() == 2);
            CHECK(dst.dimensions().size() == 2);
            CHECK((dst.dimensions()[0] == NumericRangeDimension{1, 2}));
            CHECK((dst.dimensions()[1] == NumericRangeDimension{3, 4}));
        }

        SECTION("Move assignment") {
            NumericRange dst;
            dst = std::move(src);
            CHECK(src.dimensions().size() == 0);
            CHECK(dst.dimensions().size() == 2);
            CHECK((dst.dimensions()[0] == NumericRangeDimension{1, 2}));
            CHECK((dst.dimensions()[1] == NumericRangeDimension{3, 4}));
        }
    }

    SECTION("toString") {
        CHECK(toString(NumericRange({{1, 1}})) == "1");
        CHECK(toString(NumericRange({{1, 2}})) == "1:2");
        CHECK(toString(NumericRange({{1, 2}, {0, 3}, {5, 5}})) == "1:2,0:3,5");
    }
}

#if UAPP_HAS_TOSTRING
TEST_CASE("toString") {
    const auto toStringStl = [](auto... args) { return std::string{toString(args...)}; };

    CHECK_THAT(toStringStl(11), ContainsSubstring("11"));
    CHECK_THAT(toStringStl(11, UA_TYPES[UA_TYPES_INT32]), ContainsSubstring("11"));

    CHECK_THAT(toStringStl(String{"test"}), ContainsSubstring("test"));
    CHECK_THAT(toStringStl(String{"test"}, UA_TYPES[UA_TYPES_STRING]), ContainsSubstring("test"));
}
#endif

#if UAPP_HAS_ORDER
TEST_CASE("Comparison operators") {
    // use simple registered type without custom comparison overloads
    STATIC_REQUIRE(IsRegistered<UA_Range>::value);
    const UA_Range r1{1, 2};
    const UA_Range r2{3, 4};
    const UA_Range r3{5, 6};

    CHECK((r1 == r1));
    CHECK_FALSE((r1 == r2));

    CHECK((r1 != r2));
    CHECK_FALSE((r1 != r1));

    CHECK((r1 < r2));
    CHECK_FALSE((r2 < r2));
    CHECK_FALSE((r3 < r2));

    CHECK((r1 <= r2));
    CHECK((r2 <= r2));
    CHECK_FALSE((r3 <= r2));

    CHECK((r3 > r2));
    CHECK_FALSE((r2 > r2));
    CHECK_FALSE((r1 > r2));

    CHECK((r3 >= r2));
    CHECK((r2 >= r2));
    CHECK_FALSE((r1 >= r2));
}
#endif

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "open62541pp/config.hpp"
#include "open62541pp/ua/types.hpp"

using namespace opcua;

TEST_CASE("EnumValueType") {
    const EnumValueType enumValueType(1, {"", "Name"}, {"", "Description"});
    CHECK(enumValueType.value() == 1);
    CHECK(enumValueType.displayName() == LocalizedText("", "Name"));
    CHECK(enumValueType.description() == LocalizedText("", "Description"));
}

TEST_CASE("ApplicationDescription") {
    const ApplicationDescription description(
        "applicationUri",
        "productUri",
        {"", "applicationName"},
        opcua::ApplicationType::ClientAndServer,
        "gatewayServerUri",
        "discoveryProfileUri",
        {String{"discoveryUrl"}}
    );
    CHECK(description.applicationUri() == "applicationUri");
    CHECK(description.productUri() == "productUri");
    CHECK(description.applicationName() == LocalizedText("", "applicationName"));
    CHECK(description.applicationType() == opcua::ApplicationType::ClientAndServer);
    CHECK(description.gatewayServerUri() == "gatewayServerUri");
    CHECK(description.discoveryProfileUri() == "discoveryProfileUri");
    CHECK(description.discoveryUrls().size() == 1);
    CHECK(description.discoveryUrls().at(0) == "discoveryUrl");
}

TEST_CASE("RequestHeader") {
    const auto now = DateTime::now();
    const RequestHeader header({1, 1000}, now, 1, 2, "auditEntryId", 10, {});
    CHECK(header.authenticationToken() == NodeId(1, 1000));
    CHECK(header.timestamp() == now);
    CHECK(header.requestHandle() == 1);
    CHECK(header.returnDiagnostics() == 2);
    CHECK(header.auditEntryId() == "auditEntryId");
    CHECK(header.additionalHeader().empty());
}

TEST_CASE("UserTokenPolicy") {
    UserTokenPolicy token(
        "policyId",
        UserTokenType::Username,
        "issuedTokenType",
        "issuerEndpointUrl",
        "securityPolicyUri"
    );
    CHECK(token.policyId() == "policyId");
    CHECK(token.tokenType() == UserTokenType::Username);
    CHECK(token.issuedTokenType() == "issuedTokenType");
    CHECK(token.issuerEndpointUrl() == "issuerEndpointUrl");
    CHECK(token.securityPolicyUri() == "securityPolicyUri");
}

TEST_CASE("NodeAttributes") {
    // getters/setters are generated by specialized macros
    // just test the macros with VariableAttributes here
    VariableAttributes attr;
    CHECK(attr.specifiedAttributes() == UA_NODEATTRIBUTESMASK_NONE);

    SECTION("Primitive type") {
        attr.setWriteMask(WriteMask::DataType);
        CHECK(attr.writeMask() == UA_WRITEMASK_DATATYPE);
        CHECK(attr.specifiedAttributes() == UA_NODEATTRIBUTESMASK_WRITEMASK);
    }

    SECTION("Cast type") {
        attr.setValueRank(ValueRank::TwoDimensions);
        CHECK(attr.valueRank() == ValueRank::TwoDimensions);
        CHECK(attr.specifiedAttributes() == UA_NODEATTRIBUTESMASK_VALUERANK);
    }

    SECTION("Wrapper type") {
        attr.setDisplayName({"", "Name"});
        CHECK(attr.displayName() == LocalizedText{"", "Name"});
        CHECK(attr.specifiedAttributes() == UA_NODEATTRIBUTESMASK_DISPLAYNAME);
    }

    SECTION("Array type") {
        CHECK(attr.arrayDimensions().empty());
        // assign twice to check deallocation
        attr.setArrayDimensions({1});
        attr.setArrayDimensions({1, 2});
        CHECK(attr.arrayDimensions() == Span<const uint32_t>{1, 2});
        CHECK(attr.specifiedAttributes() == UA_NODEATTRIBUTESMASK_ARRAYDIMENSIONS);
    }
}

TEST_CASE("NodeAttributes fluent interface") {
    const auto attr = NodeAttributes{}.setDisplayName({"", "displayName"}).setWriteMask(0xFFFFFFFF);
    CHECK(attr.displayName() == LocalizedText("", "displayName"));
    CHECK(attr.writeMask() == 0xFFFFFFFF);
}

TEMPLATE_TEST_CASE("NodeAttributes setDataType", "", VariableAttributes, VariableTypeAttributes) {
    CHECK(TestType{}.setDataType(DataTypeId::Boolean).dataType() == NodeId(DataTypeId::Boolean));
    CHECK(TestType{}.template setDataType<bool>().dataType() == NodeId(DataTypeId::Boolean));
}

TEST_CASE("UserNameIdentityToken") {
    const UserNameIdentityToken token("userName", "password", "encryptionAlgorithm");
    CHECK(token.policyId().empty());
    CHECK(token.userName() == "userName");
    CHECK(token.password() == ByteString("password"));
    CHECK(token.encryptionAlgorithm() == "encryptionAlgorithm");
}

TEST_CASE("X509IdentityToken") {
    const X509IdentityToken token(ByteString("certificateData"));
    CHECK(token.policyId().empty());
    CHECK(token.certificateData() == ByteString("certificateData"));
}

TEST_CASE("IssuedIdentityToken") {
    const IssuedIdentityToken token(ByteString("tokenData"), "encryptionAlgorithm");
    CHECK(token.policyId().empty());
    CHECK(token.tokenData() == ByteString("tokenData"));
    CHECK(token.encryptionAlgorithm() == "encryptionAlgorithm");
}

TEST_CASE("AddNodesItem / AddNodesRequest") {
    const AddNodesItem item(
        ExpandedNodeId({1, 1000}),
        {1, 1001},
        ExpandedNodeId({1, 1002}),
        {1, "item"},
        NodeClass::Object,
        ExtensionObject(ObjectAttributes{}),
        ExpandedNodeId({1, 1003})
    );
    CHECK(item.parentNodeId().nodeId() == NodeId(1, 1000));
    CHECK(item.referenceTypeId() == NodeId(1, 1001));
    CHECK(item.requestedNewNodeId().nodeId() == NodeId(1, 1002));
    CHECK(item.browseName() == QualifiedName(1, "item"));
    CHECK(item.nodeClass() == NodeClass::Object);
    CHECK(item.nodeAttributes().decodedType() == &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]);
    CHECK(item.typeDefinition().nodeId() == NodeId(1, 1003));

    const AddNodesRequest request({}, {item});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.nodesToAdd().size() == 1);
}

TEST_CASE("AddReferencesItem / AddReferencesRequest") {
    const AddReferencesItem item(
        {1, 1000}, {1, 1001}, true, {}, ExpandedNodeId({1, 1002}), NodeClass::Object
    );
    CHECK(item.sourceNodeId() == NodeId(1, 1000));
    CHECK(item.referenceTypeId() == NodeId(1, 1001));
    CHECK(item.isForward() == true);
    CHECK(item.targetServerUri().empty());
    CHECK(item.targetNodeId().nodeId() == NodeId(1, 1002));
    CHECK(item.targetNodeClass() == NodeClass::Object);

    const AddReferencesRequest request({}, {item});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.referencesToAdd().size() == 1);
}

TEST_CASE("DeleteNodesItem / DeleteNodesRequest") {
    const DeleteNodesItem item({1, 1000}, true);
    CHECK(item.nodeId() == NodeId(1, 1000));
    CHECK(item.deleteTargetReferences() == true);

    const DeleteNodesRequest request({}, {item});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.nodesToDelete().size() == 1);
}

TEST_CASE("DeleteReferencesItem / DeleteReferencesRequest") {
    const DeleteReferencesItem item({1, 1000}, {1, 1001}, true, ExpandedNodeId({1, 1002}), true);
    CHECK(item.sourceNodeId() == NodeId(1, 1000));
    CHECK(item.referenceTypeId() == NodeId(1, 1001));
    CHECK(item.isForward() == true);
    CHECK(item.targetNodeId().nodeId() == NodeId(1, 1002));
    CHECK(item.deleteBidirectional() == true);

    const DeleteReferencesRequest request({}, {item});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.referencesToDelete().size() == 1);
}

TEST_CASE("ViewDescription") {
    const ViewDescription vd({1, 1000}, 12345U, 2U);
    CHECK(vd.viewId() == NodeId(1, 1000));
    CHECK(vd.timestamp() == 12345U);
    CHECK(vd.viewVersion() == 2U);
}

TEST_CASE("BrowseDescription") {
    const BrowseDescription bd(NodeId(1, 1000), BrowseDirection::Forward);
    CHECK(bd.nodeId() == NodeId(1, 1000));
    CHECK(bd.browseDirection() == BrowseDirection::Forward);
    CHECK(bd.referenceTypeId() == NodeId(0, UA_NS0ID_REFERENCES));
    CHECK(bd.includeSubtypes() == true);
    CHECK(bd.nodeClassMask() == UA_NODECLASS_UNSPECIFIED);
    CHECK(bd.resultMask() == UA_BROWSERESULTMASK_ALL);
}

TEST_CASE("RelativePathElement") {
    const RelativePathElement rpe(ReferenceTypeId::HasComponent, false, false, {0, "test"});
    CHECK(rpe.referenceTypeId() == NodeId{0, UA_NS0ID_HASCOMPONENT});
    CHECK(rpe.isInverse() == false);
    CHECK(rpe.includeSubtypes() == false);
    CHECK(rpe.targetName() == QualifiedName(0, "test"));
}

TEST_CASE("RelativePath") {
    const RelativePath rp{
        {ReferenceTypeId::HasComponent, false, false, {0, "child1"}},
        {ReferenceTypeId::HasComponent, false, false, {0, "child2"}},
    };
    const auto elements = rp.elements();
    CHECK(elements.size() == 2);
    CHECK(elements[0].targetName() == QualifiedName(0, "child1"));
    CHECK(elements[1].targetName() == QualifiedName(0, "child2"));
}

TEST_CASE("BrowsePath") {
    const BrowsePath bp(
        ObjectId::ObjectsFolder, {{ReferenceTypeId::HasComponent, false, false, {0, "child"}}}
    );
    CHECK(bp.startingNode() == NodeId(0, UA_NS0ID_OBJECTSFOLDER));
    CHECK(bp.relativePath().elements().size() == 1);
}

TEST_CASE("BrowseRequest") {
    const BrowseRequest request({}, {{1, 1000}, {}, 1}, 11U, {});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.view().viewId() == NodeId(1, 1000));
    CHECK(request.view().viewVersion() == 1);
    CHECK(request.requestedMaxReferencesPerNode() == 11U);
    CHECK(request.nodesToBrowse().empty());
}

TEST_CASE("BrowseNextRequest") {
    const BrowseNextRequest request({}, true, {ByteString("123")});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.releaseContinuationPoints() == true);
    CHECK(request.continuationPoints().size() == 1);
    CHECK(request.continuationPoints()[0] == ByteString("123"));
}

TEST_CASE("TranslateBrowsePathsToNodeIdsRequest") {
    const TranslateBrowsePathsToNodeIdsRequest request({}, {});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.browsePaths().empty());
}

TEST_CASE("RegisterNodesRequest") {
    const RegisterNodesRequest request({}, {{1, 1000}});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.nodesToRegister().size() == 1);
    CHECK(request.nodesToRegister()[0] == NodeId(1, 1000));
}

TEST_CASE("UnregisterNodesRequest") {
    const UnregisterNodesRequest request({}, {{1, 1000}});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.nodesToUnregister().size() == 1);
    CHECK(request.nodesToUnregister()[0] == NodeId(1, 1000));
}

TEST_CASE("ReadValueId") {
    const ReadValueId rvid(NodeId(1, 1000), AttributeId::Value);
    CHECK(rvid.nodeId() == NodeId(1, 1000));
    CHECK(rvid.attributeId() == AttributeId::Value);
    CHECK(rvid.indexRange().empty());
    CHECK(rvid.dataEncoding() == QualifiedName());
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
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.maxAge() == 111.11);
    CHECK(request.timestampsToReturn() == TimestampsToReturn::Both);
    CHECK(request.nodesToRead().size() == 1);
    CHECK(request.nodesToRead()[0].nodeId() == NodeId(1, 1000));
    CHECK(request.nodesToRead()[0].attributeId() == AttributeId::Value);
}

TEST_CASE("WriteValue") {
    const WriteValue wv({1, 1000}, AttributeId::Value, {}, DataValue(Variant(11.11)));
    CHECK(wv.nodeId() == NodeId(1, 1000));
    CHECK(wv.attributeId() == AttributeId::Value);
    CHECK(wv.indexRange().empty());
    CHECK(wv.value().value().scalar<double>() == 11.11);
}

TEST_CASE("WriteRequest") {
    const WriteRequest request(
        {},
        {
            {{1, 1000}, AttributeId::Value, {}, DataValue(Variant(11.11))},
        }
    );
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.nodesToWrite().size() == 1);
    CHECK(request.nodesToWrite()[0].nodeId() == NodeId(1, 1000));
    CHECK(request.nodesToWrite()[0].attributeId() == AttributeId::Value);
    CHECK(request.nodesToWrite()[0].value().value().scalar<double>() == 11.11);
}

TEST_CASE("WriteResponse") {
    const WriteResponse response;
    CHECK_NOTHROW(response.responseHeader());
    CHECK(response.results().empty());
    CHECK(response.diagnosticInfos().empty());
}

TEST_CASE("BuildInfo") {
    const BuildInfo buildInfo(
        "productUri",
        "manufacturerName",
        "productName",
        "softwareVersion",
        "buildNumber",
        DateTime(1234)
    );
    CHECK(buildInfo.productUri() == "productUri");
    CHECK(buildInfo.manufacturerName() == "manufacturerName");
    CHECK(buildInfo.productName() == "productName");
    CHECK(buildInfo.softwareVersion() == "softwareVersion");
    CHECK(buildInfo.buildNumber() == "buildNumber");
    CHECK(buildInfo.buildDate().get() == 1234);
}

#ifdef UA_ENABLE_METHODCALLS

TEST_CASE("Argument") {
    const Argument argument(
        "name", {"", "description"}, DataTypeId::Int32, ValueRank::TwoDimensions, {2, 3}
    );
    CHECK(argument.name() == "name");
    CHECK(argument.description() == LocalizedText("", "description"));
    CHECK(argument.dataType() == NodeId(DataTypeId::Int32));
    CHECK(argument.valueRank() == ValueRank::TwoDimensions);
    CHECK(argument.arrayDimensions().size() == 2);
    CHECK(argument.arrayDimensions()[0] == 2);
    CHECK(argument.arrayDimensions()[1] == 3);
}

TEST_CASE("CallMethodRequest / CallRequest") {
    const CallMethodRequest item({1, 1000}, {1, 1001}, {Variant(11)});
    const CallRequest request({}, {item});
    CHECK(request.methodsToCall().size() == 1);
    CHECK(request.methodsToCall()[0].objectId() == NodeId(1, 1000));
    CHECK(request.methodsToCall()[0].methodId() == NodeId(1, 1001));
    CHECK(request.methodsToCall()[0].inputArguments().size() == 1);
}

#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS

TEST_CASE("ElementOperand") {
    CHECK(ElementOperand(11).index() == 11);
}

TEST_CASE("LiteralOperand") {
    CHECK(LiteralOperand(Variant(11)).value().scalar<int>() == 11);
    CHECK(LiteralOperand(11).value().scalar<int>() == 11);
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
    CHECK(operand.nodeId() == NodeId(ObjectTypeId::BaseEventType));
    CHECK(operand.alias() == "alias");
    CHECK(operand.browsePath().elements().size() == 2);
    CHECK(operand.attributeId() == AttributeId::Value);
    CHECK(operand.indexRange().empty());
}

TEST_CASE("SimpleAttributeOperand") {
    const SimpleAttributeOperand operand(
        ObjectTypeId::BaseEventType, {{0, "child1"}, {0, "child2"}}, AttributeId::Value, {}
    );
    CHECK(operand.typeDefinitionId() == NodeId(ObjectTypeId::BaseEventType));
    CHECK(operand.browsePath().size() == 2);
    CHECK(operand.attributeId() == AttributeId::Value);
    CHECK(operand.indexRange().empty());
}

TEST_CASE("ContentFilter(Element)") {
    const ContentFilter contentFilter{
        {FilterOperator::And, {ElementOperand(1), ElementOperand(2)}},
        {FilterOperator::OfType, {LiteralOperand(NodeId(ObjectTypeId::BaseEventType))}},
        {FilterOperator::Equals, {LiteralOperand(99), LiteralOperand(99)}},
    };

    auto elements = contentFilter.elements();
    CHECK(elements.size() == 3);
    CHECK(elements[0].filterOperator() == FilterOperator::And);
    CHECK(elements[0].filterOperands().size() == 2);
    CHECK(elements[0].filterOperands()[0].decodedData<ElementOperand>()->index() == 1);
    CHECK(elements[0].filterOperands()[1].decodedData<ElementOperand>()->index() == 2);
    CHECK(elements[1].filterOperator() == FilterOperator::OfType);
    CHECK(elements[1].filterOperands().size() == 1);
    CHECK(elements[1].filterOperands()[0].decodedData<LiteralOperand>() != nullptr);
    CHECK(elements[2].filterOperator() == FilterOperator::Equals);
    CHECK(elements[2].filterOperands().size() == 2);
    CHECK(elements[2].filterOperands()[0].decodedData<LiteralOperand>() != nullptr);
    CHECK(elements[2].filterOperands()[1].decodedData<LiteralOperand>() != nullptr);
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
        return operand.decodedData<ElementOperand>()->index();
    };

    auto firstOperator = [](const ContentFilter& contentFilter) {
        return contentFilter.elements()[0].filterOperator();
    };

    SECTION("Not") {
        const ContentFilter filterElementNot = !filterElement;
        CHECK(filterElementNot.elements().size() == 2);
        CHECK(firstOperator(filterElementNot) == FilterOperator::Not);
        CHECK(elementOperandIndex(filterElementNot.elements()[0].filterOperands()[0]) == 1);
        CHECK(filterElementNot.elements()[1].filterOperator() == FilterOperator::GreaterThan);

        const ContentFilter filterNot = !filter;
        CHECK(filterNot.elements().size() == 4);
        CHECK(firstOperator(filterNot) == FilterOperator::Not);
        CHECK(elementOperandIndex(filterNot.elements()[0].filterOperands()[0]) == 1);
        CHECK(elementOperandIndex(filterNot.elements()[1].filterOperands()[0]) == 2);
        CHECK(elementOperandIndex(filterNot.elements()[1].filterOperands()[1]) == 3);
    }

    SECTION("And") {
        CHECK((filterElement && filterElement).elements().size() == 3);
        CHECK((filterElement && filter).elements().size() == 5);
        CHECK((filter && filterElement).elements().size() == 5);
        CHECK((filter && filter).elements().size() == 7);

        CHECK(firstOperator(filterElement && filterElement) == FilterOperator::And);
        CHECK(firstOperator(filterElement && filter) == FilterOperator::And);
        CHECK(firstOperator(filter && filterElement) == FilterOperator::And);
        CHECK(firstOperator(filter && filter) == FilterOperator::And);

        SECTION("Increment operand indexes") {
            const ContentFilter filterAdd = filter && filter;
            CHECK(filterAdd.elements().size() == 7);
            // and
            CHECK(elementOperandIndex(filterAdd.elements()[0].filterOperands()[0]) == 1);
            CHECK(elementOperandIndex(filterAdd.elements()[0].filterOperands()[1]) == 4);
            // lhs and
            CHECK(elementOperandIndex(filterAdd.elements()[1].filterOperands()[0]) == 2);
            CHECK(elementOperandIndex(filterAdd.elements()[1].filterOperands()[1]) == 3);
            // rhs and
            CHECK(elementOperandIndex(filterAdd.elements()[4].filterOperands()[0]) == 5);
            CHECK(elementOperandIndex(filterAdd.elements()[4].filterOperands()[1]) == 6);
        }
    }

    SECTION("Or") {
        CHECK((filterElement || filterElement).elements().size() == 3);
        CHECK((filterElement || filter).elements().size() == 5);
        CHECK((filter || filterElement).elements().size() == 5);
        CHECK((filter || filter).elements().size() == 7);

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

    CHECK(dataChangeFilter.trigger() == DataChangeTrigger::StatusValue);
    CHECK(dataChangeFilter.deadbandType() == DeadbandType::Percent);
    CHECK(dataChangeFilter.deadbandValue() == 11.11);
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

    CHECK(eventFilter.selectClauses().size() == 3);
    CHECK(eventFilter.whereClause().elements().size() == 1);
}

TEST_CASE("AggregateFilter") {
    const DateTime startTime = DateTime::now();
    AggregateConfiguration aggregateConfiguration{};
    aggregateConfiguration.useSlopedExtrapolation = true;

    const AggregateFilter aggregateFilter(
        startTime, ObjectId::AggregateFunction_Average, 11.11, aggregateConfiguration
    );

    CHECK(aggregateFilter.startTime() == startTime);
    CHECK(aggregateFilter.aggregateType() == NodeId(ObjectId::AggregateFunction_Average));
    CHECK(aggregateFilter.processingInterval() == 11.11);
    CHECK(aggregateFilter.aggregateConfiguration().useSlopedExtrapolation == true);
}

TEST_CASE("MonitoringParameters") {
    const MonitoringParameters params(11.11, {}, 10, false);
    CHECK(params.samplingInterval() == 11.11);
    CHECK(params.filter().empty());
    CHECK(params.queueSize() == 10);
    CHECK(params.discardOldest() == false);
}

TEST_CASE("MonitoredItemCreateRequest / CreateMonitoredItemsRequest") {
    const MonitoredItemCreateRequest item({{1, 1000}, AttributeId::Value});
    CHECK(item.itemToMonitor().nodeId() == NodeId(1, 1000));
    CHECK(item.itemToMonitor().attributeId() == AttributeId::Value);
    CHECK(item.monitoringMode() == MonitoringMode::Reporting);

    const CreateMonitoredItemsRequest request({}, 1U, TimestampsToReturn::Both, {item});
    CHECK(request.subscriptionId() == 1U);
    CHECK(request.timestampsToReturn() == TimestampsToReturn::Both);
    CHECK(request.itemsToCreate().size() == 1);
}

TEST_CASE("MonitoredItemModifyRequest / ModifyMonitoredItemsRequest") {
    const MonitoredItemModifyRequest item(1U, MonitoringParameters(11.11));
    CHECK(item.monitoredItemId() == 1U);
    CHECK(item.requestedParameters().samplingInterval() == 11.11);

    const ModifyMonitoredItemsRequest request({}, 1U, TimestampsToReturn::Both, {item});
    CHECK(request.subscriptionId() == 1U);
    CHECK(request.timestampsToReturn() == TimestampsToReturn::Both);
    CHECK(request.itemsToModify().size() == 1);
}

TEST_CASE("SetMonitoringModeRequest") {
    const SetMonitoringModeRequest request({}, 1U, MonitoringMode::Reporting, {0U, 1U});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.subscriptionId() == 1U);
    CHECK(request.monitoringMode() == MonitoringMode::Reporting);
    CHECK(request.monitoredItemIds().size() == 2);
    CHECK(request.monitoredItemIds()[0] == 0U);
    CHECK(request.monitoredItemIds()[1] == 1U);
}

TEST_CASE("SetTriggeringRequest") {
    const SetTriggeringRequest request({}, 1U, 2U, {3U}, {4U, 5U});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.subscriptionId() == 1U);
    CHECK(request.triggeringItemId() == 2U);
    CHECK(request.linksToAdd().size() == 1);
    CHECK(request.linksToAdd()[0] == 3U);
    CHECK(request.linksToRemove().size() == 2);
    CHECK(request.linksToRemove()[0] == 4U);
    CHECK(request.linksToRemove()[1] == 5U);
}

TEST_CASE("DeleteMonitoredItemsRequest") {
    const DeleteMonitoredItemsRequest request({}, 1U, {0U, 1U});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.subscriptionId() == 1U);
    CHECK(request.monitoredItemIds().size() == 2);
    CHECK(request.monitoredItemIds()[0] == 0U);
    CHECK(request.monitoredItemIds()[1] == 1U);
}

TEST_CASE("CreateSubscriptionRequest") {
    const CreateSubscriptionRequest request({}, 11.11, 2, 3, 4, true, 5);
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.requestedPublishingInterval() == 11.11);
    CHECK(request.requestedLifetimeCount() == 2);
    CHECK(request.requestedMaxKeepAliveCount() == 3);
    CHECK(request.maxNotificationsPerPublish() == 4);
    CHECK(request.publishingEnabled() == true);
    CHECK(request.priority() == 5);
}

TEST_CASE("ModifySubscriptionRequest") {
    const ModifySubscriptionRequest request({}, 1, 11.11, 2, 3, 4, 5);
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.subscriptionId() == 1);
    CHECK(request.requestedPublishingInterval() == 11.11);
    CHECK(request.requestedLifetimeCount() == 2);
    CHECK(request.requestedMaxKeepAliveCount() == 3);
    CHECK(request.maxNotificationsPerPublish() == 4);
    CHECK(request.priority() == 5);
}

TEST_CASE("SetPublishingModeRequest") {
    const SetPublishingModeRequest request({}, true, {1, 2, 3});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.publishingEnabled() == true);
    CHECK(request.subscriptionIds().size() == 3);
    CHECK(request.subscriptionIds()[0] == 1);
    CHECK(request.subscriptionIds()[1] == 2);
    CHECK(request.subscriptionIds()[2] == 3);
}

TEST_CASE("DeleteSubscriptionsRequest") {
    const DeleteSubscriptionsRequest request({}, {1, 2, 3});
    CHECK_NOTHROW(request.requestHeader());
    CHECK(request.subscriptionIds().size() == 3);
    CHECK(request.subscriptionIds()[0] == 1);
    CHECK(request.subscriptionIds()[1] == 2);
    CHECK(request.subscriptionIds()[2] == 3);
}

#endif

#if UAPP_HAS_DATAACCESS

TEST_CASE("Range") {
    const Range range(1.1, 2.2);
    CHECK(range.low() == 1.1);
    CHECK(range.high() == 2.2);
}

TEST_CASE("EUInformation") {
    const EUInformation info("namespaceUri", 1, {"", "displayName"}, {"", "description"});
    CHECK(info.namespaceUri() == "namespaceUri");
    CHECK(info.unitId() == 1);
    CHECK(info.displayName() == LocalizedText("", "displayName"));
    CHECK(info.description() == LocalizedText("", "description"));
}

TEST_CASE("ComplexNumberType") {
    const ComplexNumberType complex(1.1f, 2.2f);
    CHECK(complex.real() == 1.1f);
    CHECK(complex.imaginary() == 2.2f);
}

TEST_CASE("DoubleComplexNumberType") {
    const DoubleComplexNumberType complex(1.1, 2.2);
    CHECK(complex.real() == 1.1);
    CHECK(complex.imaginary() == 2.2);
}

TEST_CASE("AxisInformation") {
    const AxisInformation axis(
        EUInformation("namespaceUri", 1, {}, {}),
        Range(1.1, 3.3),
        {"", "title"},
        AxisScaleEnumeration::Log,
        {1.1, 2.2, 3.3}
    );
    CHECK(axis.engineeringUnits().namespaceUri() == "namespaceUri");
    CHECK(axis.eURange().low() == 1.1);
    CHECK(axis.eURange().high() == 3.3);
    CHECK(axis.title() == LocalizedText("", "title"));
    CHECK(axis.axisScaleType() == AxisScaleEnumeration::Log);
    CHECK(axis.axisSteps().size() == 3);
    CHECK(axis.axisSteps()[0] == 1.1);
}

TEST_CASE("XVType") {
    const XVType xv(1.1, 2.2f);
    CHECK(xv.x() == 1.1);
    CHECK(xv.value() == 2.2f);
}

#endif

#ifdef UA_ENABLE_TYPEDESCRIPTION

TEST_CASE("EnumField / EnumDefinition") {
    const EnumDefinition enumDefinition{{0, "Zero"}, {1, "One"}};
    CHECK(enumDefinition.fields().size() == 2);
    CHECK(enumDefinition.fields()[0].value() == 0);
    CHECK(enumDefinition.fields()[0].displayName() == LocalizedText("", "Zero"));
    CHECK(enumDefinition.fields()[0].description() == LocalizedText());
    CHECK(enumDefinition.fields()[0].name() == "Zero");
    CHECK(enumDefinition.fields()[1].value() == 1);
    CHECK(enumDefinition.fields()[1].displayName() == LocalizedText("", "One"));
    CHECK(enumDefinition.fields()[1].description() == LocalizedText());
    CHECK(enumDefinition.fields()[1].name() == "One");
}

#endif

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    // Create object types `MammalType` and `DogType`:
    // (OT) BaseObjectType
    // + (OT) MammalType
    //   + (V) Age
    //   + (OT) DogType
    //      + (V) Name
    auto nodeBaseObjectType = server.getNode(opcua::ObjectTypeId::BaseObjectType);
    auto nodeMammalType = nodeBaseObjectType.addObjectType({1, 10000}, "MammalType");
    nodeMammalType.writeDisplayName({"en-US", "MammalType"})
        .writeDescription({"en-US", "A mammal"});

    nodeMammalType.addVariable({1, 10001}, "Age")
        .addModellingRule(opcua::ModellingRule::Mandatory)  // create on new instance
        .writeDisplayName({"en-US", "Age"})
        .writeDescription({"en-US", "This mammals age in months"})
        .writeScalar<uint32_t>(0);  // default age

    auto nodeDogType = nodeMammalType.addObjectType({1, 10002}, "DogType");
    nodeDogType.writeDisplayName({"en-US", "DogType"})
        .writeDescription({"en-US", "A dog, subtype of mammal"});

    nodeDogType.addVariable({1, 10003}, "Name")
        .addModellingRule(opcua::ModellingRule::Mandatory)  // create on new instance
        .writeDisplayName({"en-US", "Name"})
        .writeDescription({"en-US", "This dogs name"})
        .writeScalar(opcua::String("unnamed dog"));  // default name

    // Instatiate a dog named Bello:
    // (O) Objects
    //   + (O) Bello <DogType>
    //     + (V) Age
    //     + (V) Name
    auto nodeObjects = server.getObjectsNode();
    auto nodeBello = nodeObjects.addObject({1, 20000}, "Bello", nodeDogType.getNodeId());
    nodeBello.writeDisplayName({"en-US", "Bello"}).writeDescription({"en-US", "A dog named Bello"});

    // Set variables Age and Name
    nodeBello.browseChild({{1, "Age"}}).writeScalar(3U);
    nodeBello.browseChild({{1, "Name"}}).writeScalar(opcua::String("Bello"));

    server.run();
}

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    // Create object types `MammalType` and `DogType`:
    // (ObjectType) BaseObjectType
    // └─ (ObjectType) MammalType
    //    ├─ (Variable) Age
    //    └─ (ObjectType) DogType
    //       └─ (Variable) Name
    auto nodeBaseObjectType = server.getNode(opcua::ObjectTypeId::BaseObjectType);
    auto nodeMammalType = nodeBaseObjectType.addObjectType(
        {1, 10000},
        "MammalType",
        opcua::ObjectTypeAttributes{}
            .setDisplayName({"en-US", "MammalType"})
            .setDescription({"en-US", "A mammal"})
    );

    nodeMammalType
        .addVariable(
            {1, 10001},
            "Age",
            opcua::VariableAttributes{}
                .setDisplayName({"en-US", "Age"})
                .setDescription({"en-US", "This mammals age in months"})
                .setValueScalar(0U)  // default age
        )
        .addModellingRule(opcua::ModellingRule::Mandatory);  // create on new instance

    auto nodeDogType = nodeMammalType.addObjectType(
        {1, 10002},
        "DogType",
        opcua::ObjectTypeAttributes{}
            .setDisplayName({"en-US", "DogType"})
            .setDescription({"en-US", "A dog, subtype of mammal"})
    );

    nodeDogType
        .addVariable(
            {1, 10003},
            "Name",
            opcua::VariableAttributes{}
                .setDisplayName({"en-US", "Name"})
                .setDescription({"en-US", "This dogs name"})
                .setValueScalar("unnamed dog")  // default name
        )
        .addModellingRule(opcua::ModellingRule::Mandatory);  // create on new instance

    // Instantiate a dog named Bello:
    // (Object) Objects
    // └─ (Object) Bello <DogType>
    //    ├─ (Variable) Age
    //    └─ (Variable) Name
    auto nodeBello = server.getObjectsNode().addObject(
        {1, 20000},
        "Bello",
        opcua::ObjectAttributes{}
            .setDisplayName({"en-US", "Bello"})
            .setDescription({"en-US", "A dog named Bello"}),
        nodeDogType.getNodeId()
    );

    // Set variables Age and Name
    nodeBello.browseChild({{1, "Age"}}).writeValueScalar(3U);
    nodeBello.browseChild({{1, "Name"}}).writeValueScalar("Bello");

    server.run();
}

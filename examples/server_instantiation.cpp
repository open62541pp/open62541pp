#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    /* Create object types `MamalType` and `DogType`:
     * (OT) BaseObjectType
     * + (OT) MamalType
     *   + (V) Age
     *   + (OT) DogType
     *     + (V) Name
     */
    auto nodeBaseObjectType = server.getBaseObjectTypeNode();
    auto nodeMamalType = nodeBaseObjectType.addObjectType({1, 10000}, "MamalType");
    nodeMamalType.writeDisplayName("en-US", "MamalType");
    nodeMamalType.writeDescription("en-US", "A mamal");

    auto nodeMamalTypeAge = nodeMamalType.addVariable({1, 10001}, "Age");
    nodeMamalTypeAge.writeDisplayName("en-US", "Age");
    nodeMamalTypeAge.writeDescription("en-US", "This mamals age in months");
    nodeMamalTypeAge.writeModellingRule(opcua::ModellingRule::Mandatory);  // create on new instance
    nodeMamalTypeAge.writeScalar<uint32_t>(0);  // default age

    auto nodeDogType = nodeMamalType.addObjectType({1, 10002}, "DogType");
    nodeDogType.writeDisplayName("en-US", "DogType");
    nodeDogType.writeDescription("en-US", "A dog, subtype of mamal");

    auto nodeDogTypeName = nodeDogType.addVariable({1, 10003}, "Name");
    nodeDogTypeName.writeDisplayName("en-US", "Name");
    nodeDogTypeName.writeDescription("en-US", "This dogs name");
    nodeDogTypeName.writeModellingRule(opcua::ModellingRule::Mandatory);  // create on new instance
    nodeDogTypeName.writeScalar(opcua::String("unnamed dog"));  // default name

    /* Instatiate a dog named Bello:
     * (O) Objects
     *   + (O) Bello <DogType>
     *     + (V) Age
     *     + (V) Name
     */
    auto nodeObjects = server.getObjectsNode();
    auto nodeBello = nodeObjects.addObject({1, 20000}, "Bello", nodeDogType.getNodeId());
    nodeBello.writeDisplayName("en-US", "Bello");
    nodeBello.writeDescription("en-US", "A dog named Bello");

    // Set variables Age and Name
    nodeBello.getChild({{1, "Age"}}).writeScalar(3U);
    nodeBello.getChild({{1, "Name"}}).writeScalar(opcua::String("Bello"));

    server.run();
}

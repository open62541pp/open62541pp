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
    nodeMamalType.setDescription("en-US", "A mamal");
    nodeMamalType.setDisplayName("en-US", "MamalType");

    auto nodeMamalTypeAge = nodeMamalType.addVariable({1, 10001}, "Age");
    nodeMamalTypeAge.setDescription("en-US", "This mamals age in months");
    nodeMamalTypeAge.setDisplayName("en-US", "Age");
    nodeMamalTypeAge.setModellingRule(opcua::ModellingRule::Mandatory);  // create for new instance
    nodeMamalTypeAge.writeScalar<uint32_t>(0);  // default age

    auto nodeDogType = nodeMamalType.addObjectType({1, 10002}, "DogType");
    nodeDogType.setDescription("en-US", "A dog, subtype of mamal");
    nodeDogType.setDisplayName("en-US", "DogType");

    auto nodeDogTypeName = nodeDogType.addVariable({1, 10003}, "Name");
    nodeDogTypeName.setDescription("en-US", "This dogs name");
    nodeDogTypeName.setDisplayName("en-US", "Name");
    nodeDogTypeName.setModellingRule(opcua::ModellingRule::Mandatory);  // create for new instance
    nodeDogTypeName.writeScalar(opcua::String("unnamed dog"));  // default name

    /* Instatiate a dog named Bello:
     * (O) Objects
     *   + (O) Bello <DogType>
     *     + (V) Age
     *     + (V) Name
     */
    auto nodeObjects = server.getObjectsNode();
    auto nodeBello = nodeObjects.addObject({1, 20000}, "Bello", nodeDogType.getNodeId());
    nodeBello.setDescription("en-US", "A dog named Bello");
    nodeBello.setDisplayName("en-US", "Bello");

    // Set variables Age and Name
    nodeBello.getChild({{1, "Age"}}).writeScalar(3U);
    nodeBello.getChild({{1, "Name"}}).writeScalar(opcua::String("Bello"));

    server.run();
}

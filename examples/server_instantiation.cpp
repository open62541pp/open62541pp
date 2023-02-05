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
    nodeMamalType.setDescription("A mamal", "en-US");
    nodeMamalType.setDisplayName("MamalType", "en-US");

    auto nodeMamalTypeAge = nodeMamalType.addVariable({1, 10001}, "Age");
    nodeMamalTypeAge.setDescription("This mamals age in months", "en-US");
    nodeMamalTypeAge.setDisplayName("Age", "en-US");
    nodeMamalTypeAge.setModellingRule(opcua::ModellingRule::Mandatory);  // create for new instance
    nodeMamalTypeAge.writeScalar<uint32_t>(0);  // default age

    auto nodeDogType = nodeMamalType.addObjectType({1, 10002}, "DogType");
    nodeDogType.setDescription("A dog, subtype of mamal", "en-US");
    nodeDogType.setDisplayName("DogType", "en-US");

    auto nodeDogTypeName = nodeDogType.addVariable({1, 10003}, "Name");
    nodeDogTypeName.setDescription("This dogs name", "en-US");
    nodeDogTypeName.setDisplayName("Name", "en-US");
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
    nodeBello.setDescription("A dog named Bello", "en-US");
    nodeBello.setDisplayName("Bello", "en-US");

    // Set variables Age and Name
    nodeBello.getChild({{1, "Age"}}).writeScalar(3U);
    nodeBello.getChild({{1, "Name"}}).writeScalar(opcua::String("Bello"));

    server.run();
}

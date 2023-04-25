#include <iomanip>
#include <iostream>

#include "open62541pp/open62541pp.h"

// separate definition for recursion
void printNodeTree(opcua::Node<opcua::Client>& node, int indent);

/**
 * Browse and print the server's node tree recursively.
 *
 * - Objects (Object)
 *   - Server (Object)
 *     - Auditing (Variable)
 *     - ServiceLevel (Variable)
 *     - NamespaceArray (Variable)
 *     - ServerArray (Variable)
 *     - ServerRedundancy (Object)
 *       - RedundancySupport (Variable)
 *     - VendorServerInfo (Object)
 * ...
 */
void printNodeTree(opcua::Node<opcua::Client>& node, int indent) {  // NOLINT
    for (auto&& child : node.browseChildren()) {
        std::cout << std::setw(indent) << "- " << child.readBrowseName() << " ("
                  << opcua::getNodeClassName(child.readNodeClass()) << ")\n";
        printNodeTree(child, indent + 2);
    }
}

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    auto nodeRoot = client.getRootNode();

    // browse all nodes recursively and print node tree to console
    printNodeTree(nodeRoot, 0);

    // get child node by relative path using browse names
    auto nodeServer = nodeRoot.browseChild({{0, "Objects"}, {0, "Server"}});
    // get parent
    auto nodeServerParent = nodeServer.browseParent();

    std::cout << nodeServer.readDisplayName().getText() << "'s parent node is "
              << nodeServerParent.readDisplayName().getText() << "\n";
}

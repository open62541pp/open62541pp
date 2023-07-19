#pragma once

#include <functional>

namespace opcua {

// forward declaration
class NodeId;
class DataValue;

/**
 * Value callbacks allow to synchronize a variable value with an external representation.
 * The attached callbacks are executed before every read and after every write operation.
 * @see https://www.open62541.org/doc/1.3/tutorial_server_datasource.html
 */
struct ValueCallback {
    /**
     * Called before the value attribute is read. It is possible to write into the value attribute
     * during onBeforeRead (using e.g. services::writeValue or Node::writeValue). The node is
     * re-opened afterwards so that changes are considered in the following read operation.
     */
    std::function<void(const NodeId&, const DataValue&)> onBeforeRead;

    /**
     * Called after writing the value attribute. The node is re-opened after
     * writing so that the new value is visible in the callback.
     */
    std::function<void(const NodeId&, const DataValue&)> onAfterWrite;
};

}  // namespace opcua

#pragma once

#include <functional>

namespace opcua {

// forward declaration
class NodeId;
class DataValue;

/**
 * Value callbacks for variable nodes.
 *
 * Value callbacks allow to synchronize a variable value with an external representation.
 * The attached callbacks are executed before every read and after every write operation.
 * @see https://www.open62541.org/doc/1.3/tutorial_server_datasource.html
 */
struct ValueCallback {
    /**
     * Called before the value attribute is read.
     *
     * It is possible to write into the value attribute during onBeforeRead (using e.g.
     * services::writeValue or Node::writeValue). The node is re-opened afterwards so that changes
     * are considered in the following read operation.
     *
     * @param value Current value before the read operation
     */
    std::function<void(const DataValue& value)> onBeforeRead;

    /**
     * Called after writing the value attribute.
     *
     * The node is re-opened after writing so that the new value is visible in the callback.
     *
     * @param value New value after the write operation
     */
    std::function<void(const DataValue& value)> onAfterWrite;
};

/**
 * Data source backend for variable nodes.
 *
 * The server redirects every read and write request to callback functions.
 * Internally, the data source needs to implement its own memory management.
 * @see https://www.open62541.org/doc/1.3/tutorial_server_datasource.html
 */
struct ValueBackendDataSource {
    /**
     * Callback to set the read value, the result status and optionally a source timestamp.
     *
     * @note Zero-copy operations are possible. It is not required to return a copy of the actual
     * content data. You can return a pointer to memory owned by the user. Memory can be reused
     * between read callbacks of a data source, as the result is already encoded on the network
     * buffer between each read operation.
     * To use zero-copy reads, set the value of the Variant (DataValue::getValue) without copying,
     * e.g. with Variant::setScalar or Variant::setArray.
     *
     * @param value The DataValue that is returned to the reader
     * @param range If not empty, then the data source shall return only a selection of the
     *              (nonscalar) data.
     *              Set `UA_STATUSCODE_BADINDEXRANGEINVALID` in `value` if this does not apply
     * @param timestamp Set the source timestamp of `value` if `true`
     * @return StatusCode
     */
    std::function<StatusCode(DataValue& value, const NumericRange& range, bool timestamp)> read;

    /**
     * Callback to write the value into a data source.
     * This function can be empty if the operation is unsupported.
     *
     * @param value The DataValue that has been written by the writer
     * @param range If not empty, then only this selection of (non-scalar) data should be written
     *              into the data source
     * @return StatusCode
     */
    std::function<StatusCode(const DataValue& value, const NumericRange& range)> write;
};

}  // namespace opcua

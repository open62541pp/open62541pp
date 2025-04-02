#pragma once

#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/open62541/server.h"  // nodestore plugins defined in server.h before v1.2
#include "open62541pp/plugin/pluginadapter.hpp"
#include "open62541pp/session.hpp"
#include "open62541pp/types.hpp"

namespace opcua {

/**
 * Value callback base class for variable nodes.
 *
 * Value callbacks allow to synchronize a variable value with an external representation.
 * The attached callbacks are executed before every read and after every write operation.
 * @see https://www.open62541.org/doc/1.3/tutorial_server_datasource.html
 */
class ValueCallbackBase : public PluginAdapter<UA_ValueCallback> {
public:
    /**
     * Called before the value attribute is read.
     *
     * It is possible to write into the value attribute during onBeforeRead (using e.g.
     * services::writeValue or Node::writeValue). The node is re-opened afterwards so that changes
     * are considered in the following read operation.
     *
     * @param session Current session
     * @param id The identifier of the node being read from
     * @param range Optional numeric range the client wants to read from
     * @param value Current value before the read operation
     */
    virtual void onRead(
        Session& session, const NodeId& id, const NumericRange* range, const DataValue& value
    ) = 0;

    /**
     * Called after writing the value attribute.
     *
     * The node is re-opened after writing so that the new value is visible in the callback.
     *
     * @param session Current session
     * @param id The identifier of the node being written to
     * @param range Optional numeric range the client wants to write to
     * @param value New value after the write operation
     */
    virtual void onWrite(
        Session& session, const NodeId& id, const NumericRange* range, const DataValue& value
    ) = 0;

    UA_ValueCallback create(bool ownsAdapter) override;
};

/**
 * Data source base class for variable nodes.
 *
 * The server redirects every read and write request to callback functions.
 * Internally, the data source needs to implement its own memory management.
 * @see https://www.open62541.org/doc/1.3/tutorial_server_datasource.html
 */
class DataSourceBase : public PluginAdapter<UA_DataSource> {
public:
    /**
     * Callback to set the read value, the result status and optionally a source timestamp.
     *
     * @note Zero-copy operations are possible. It is not required to return a copy of the actual
     * content data. You can return a pointer to memory owned by the user. Memory can be reused
     * between read callbacks of a data source, as the result is already encoded on the network
     * buffer between each read operation.
     * To use zero-copy reads, set the value of the Variant (DataValue::setValue) without copying,
     * e.g. with Variant::assign.
     *
     * @param session Current session
     * @param id The identifier of the node being read from
     * @param range If not `nullptr`, then the data source shall return only a selection of the
     *              (non-scalar) data.
     *              Set `UA_STATUSCODE_BADINDEXRANGEINVALID` in `value` if this does not apply.
     * @param value The DataValue that is returned to the reader
     * @param timestamp Set the source timestamp of `value` if `true`
     * @return StatusCode
     */
    virtual StatusCode read(
        Session& session,
        const NodeId& id,
        const NumericRange* range,
        DataValue& value,
        bool timestamp
    ) = 0;

    /**
     * Callback to write the value into a data source.
     *
     * @param session Current session
     * @param id The identifier of the node being written to
     * @param range If not `nullptr`, then only this selection of (non-scalar) data should be
     *              written into the data source.
     * @param value The DataValue that has been written by the writer
     * @return StatusCode
     */
    virtual StatusCode write(
        Session& session, const NodeId& id, const NumericRange* range, const DataValue& value
    ) = 0;

    UA_DataSource create(bool ownsAdapter) override;
};

}  // namespace opcua

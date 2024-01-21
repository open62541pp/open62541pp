from dataclasses import dataclass
from pathlib import Path
from typing import Optional

HERE = Path(__file__).parent
HEADER_FILE = HERE.parent / "include/open62541pp/services/Attribute_highlevel.h"


@dataclass
class Attribute:
    name: str
    type_value: str
    type_view: Optional[str] = None
    copy: bool = False
    writeable: bool = True


ATTRIBUTES = [
    Attribute(name="NodeId", type_value="opcua::NodeId", copy=False, writeable=False),
    Attribute(name="NodeClass", type_value="opcua::NodeClass", copy=True, writeable=False),
    Attribute(name="BrowseName", type_value="opcua::QualifiedName", copy=False, writeable=True),
    Attribute(name="DisplayName", type_value="opcua::LocalizedText", copy=False, writeable=True),
    Attribute(name="Description", type_value="opcua::LocalizedText", copy=False, writeable=True),
    Attribute(name="WriteMask", type_value="uint32_t", copy=True, writeable=True),
    Attribute(name="UserWriteMask", type_value="uint32_t", copy=True, writeable=True),
    Attribute(name="IsAbstract", type_value="bool", copy=True, writeable=True),
    Attribute(name="Symmetric", type_value="bool", copy=True, writeable=True),
    Attribute(name="InverseName", type_value="opcua::LocalizedText", copy=False, writeable=True),
    Attribute(name="ContainsNoLoops", type_value="bool", copy=False, writeable=True),
    Attribute(name="EventNotifier", type_value="uint8_t ", copy=True, writeable=True),
    Attribute(name="Value", type_value="opcua::Variant", copy=False, writeable=True),
    Attribute(name="DataType", type_value="opcua::NodeId", copy=False, writeable=True),
    Attribute(name="ValueRank", type_value="opcua::ValueRank", copy=True, writeable=True),
    Attribute(name="ArrayDimensions", type_value="std::vector<uint32_t>", type_view="Span<const uint32_t>", copy=False, writeable=True),
    Attribute(name="AccessLevel", type_value="uint8_t", copy=True, writeable=True),
    Attribute(name="UserAccessLevel", type_value="uint8_t", copy=True, writeable=True),
    Attribute(name="MinimumSamplingInterval", type_value="double", copy=True, writeable=True),
    # Attribute(name="Historizing", type_value="bool", copy=True, writeable=True),
    # Attribute(name="Executable", type_value="bool", copy=True, writeable=True),
    # Attribute(name="UserExecutable", type_value="bool", copy=True, writeable=True),
]

TEMPLATE_HEADER = """
/* ---------------------------------------------------------------------------------------------- */
/*                                   Generated - do not modify!                                   */
/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "open62541pp/services/Attribute.h"

namespace opcua::services {{

/**
 * @addtogroup Attribute
 * @{{
 */

{body}

/**
 * @}}
 */

}}  // namespace opcua::services
""".lstrip()

TEMPLATE_READ = """
/**
 * Read the AttributeId::{attr} attribute of a node.
 */
template <typename T>
inline {type_without_ns} read{attr}(T& serverOrClient, const NodeId& id) {{
    return detail::readAttributeImpl<AttributeId::{attr}>(serverOrClient, id);
}}

/**
 * Asynchronously read the AttributeId::{attr} attribute of a node.
 * @param token @completiontoken{{void(opcua::StatusCode, {type_completion})}}
*/
template <typename CompletionToken = DefaultCompletionToken>
inline auto read{attr}Async(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {{
    return detail::readAttributeAsyncImpl<AttributeId::{attr}>(
        client, id, std::forward<CompletionToken>(token)
    );
}}
""".lstrip()

TEMPLATE_WRITE = """
/**
 * Write the AttributeId::{attr} attribute of a node.
 */
template <typename T>
inline void write{attr}(T& serverOrClient, const NodeId& id, {type_parameter} {parameter_name}) {{
    detail::writeAttributeImpl<AttributeId::{attr}>(serverOrClient, id, {parameter_name});
}}

/**
 * Asynchronously write the AttributeId::{attr} attribute of a node.
 * @param token @completiontoken{{void(opcua::StatusCode)}}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline void write{attr}Async(
    Client& client, const NodeId& id, {type_parameter} {parameter_name}, CompletionToken&& token
) {{
    detail::writeAttributeAsyncImpl<AttributeId::{attr}>(
        client, id, {parameter_name}, std::forward<CompletionToken>(token)
    );
}}
""".lstrip()

def gen_functions():
    for attr in ATTRIBUTES:
        pass_by_value = attr.copy
        type_completion = attr.type_value if pass_by_value else f"{attr.type_value}&"
        type_without_ns = attr.type_value.removeprefix("opcua::")
        type_parameter = type_without_ns if pass_by_value else f"const {type_without_ns}&"
        parameter_name = attr.name[0].lower() + attr.name[1:]
        format_args = {
            "attr": attr.name,
            "type_without_ns": type_without_ns,
            "type_completion": type_completion,
            "type_parameter": attr.type_view or type_parameter,
            "parameter_name": parameter_name,
        }
        yield TEMPLATE_READ.format(**format_args)
        if attr.writeable is True:
            yield TEMPLATE_WRITE.format(**format_args)


def main():
    body = "\n".join(gen_functions()).strip()
    content = TEMPLATE_HEADER.format(body=body)
    HEADER_FILE.write_text(content)


if __name__ == "__main__":
    main()

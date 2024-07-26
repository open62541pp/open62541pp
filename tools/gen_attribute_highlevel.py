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


# fmt: off
ATTRIBUTES = [
    Attribute(name="NodeId", type_value="NodeId", copy=False, writeable=False),
    Attribute(name="NodeClass", type_value="NodeClass", copy=True, writeable=False),
    Attribute(name="BrowseName", type_value="QualifiedName", copy=False, writeable=True),
    Attribute(name="DisplayName", type_value="LocalizedText", copy=False, writeable=True),
    Attribute(name="Description", type_value="LocalizedText", copy=False, writeable=True),
    Attribute(name="WriteMask", type_value="Bitmask<WriteMask>", copy=True, writeable=True),
    Attribute(name="UserWriteMask", type_value="Bitmask<WriteMask>", copy=True, writeable=True),
    Attribute(name="IsAbstract", type_value="bool", copy=True, writeable=True),
    Attribute(name="Symmetric", type_value="bool", copy=True, writeable=True),
    Attribute(name="InverseName", type_value="LocalizedText", copy=False, writeable=True),
    Attribute(name="ContainsNoLoops", type_value="bool", copy=False, writeable=True),
    Attribute(name="EventNotifier", type_value="Bitmask<EventNotifier>", copy=True, writeable=True),
    Attribute(name="Value", type_value="Variant", copy=False, writeable=True),
    Attribute(name="DataType", type_value="NodeId", copy=False, writeable=True),
    Attribute(name="ValueRank", type_value="ValueRank", copy=True, writeable=True),
    Attribute(name="ArrayDimensions", type_value="std::vector<uint32_t>", type_view="Span<const uint32_t>", copy=False, writeable=True),
    Attribute(name="AccessLevel", type_value="Bitmask<AccessLevel>", copy=True, writeable=True),
    Attribute(name="UserAccessLevel", type_value="Bitmask<AccessLevel>", copy=True, writeable=True),
    Attribute(name="MinimumSamplingInterval", type_value="double", copy=True, writeable=True),
    Attribute(name="Historizing", type_value="bool", copy=True, writeable=True),
    Attribute(name="Executable", type_value="bool", copy=True, writeable=True),
    Attribute(name="UserExecutable", type_value="bool", copy=True, writeable=True),
    Attribute(name="DataTypeDefinition", type_value="Variant", copy=False, writeable=False),
]
# fmt: on

TEMPLATE_HEADER = """
/* ---------------------------------------------------------------------------------------------- */
/*                                   Generated - do not modify!                                   */
/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "open62541pp/services/Attribute.h"

namespace opcua::services {{

{body}

}}  // namespace opcua::services
""".lstrip()

TEMPLATE_READ = """
/**
 * Read the AttributeId::{attr} attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
inline Result<{type}> read{attr}(T& connection, const NodeId& id) noexcept {{
    return detail::readAttributeImpl<AttributeId::{attr}>(connection, id);
}}

/**
 * Asynchronously read the AttributeId::{attr} attribute of a node.
 * @copydetails read{attr}
 * @param token @completiontoken{{void({token_type})}}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto read{attr}Async(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {{
    return detail::readAttributeAsyncImpl<AttributeId::{attr}>(
        connection, id, std::forward<CompletionToken>(token)
    );
}}
""".lstrip()

TEMPLATE_WRITE = """
/**
 * Write the AttributeId::{attr} attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param {param_name} Value to write
 * @ingroup Write
 */
template <typename T>
inline Result<void> write{attr}(T& connection, const NodeId& id, {param_type} {param_name}) noexcept {{
    return detail::writeAttributeImpl<AttributeId::{attr}>(connection, id, {param_name});
}}

/**
 * Asynchronously write the AttributeId::{attr} attribute of a node.
 * @copydetails write{attr}
 * @param token @completiontoken{{void(Result<void>)}}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto write{attr}Async(
    Client& connection,
    const NodeId& id,
    {param_type} {param_name},
    CompletionToken&& token = DefaultCompletionToken()
) {{
    return detail::writeAttributeAsyncImpl<AttributeId::{attr}>(
        connection, id, {param_name}, std::forward<CompletionToken>(token)
    );
}}
""".lstrip()


def gen_functions():
    for attr in ATTRIBUTES:
        type_value = attr.type_value
        token_type = f"Result<{type_value}>" if attr.copy else f"Result<{type_value}>&"
        param_type = type_value if attr.copy else f"const {type_value}&"
        param_name = attr.name[0].lower() + attr.name[1:]
        format_args = {
            "attr": attr.name,
            "type": attr.type_value,
            "token_type": token_type,
            "param_type": attr.type_view or param_type,
            "param_name": param_name,
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

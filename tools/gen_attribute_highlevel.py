from dataclasses import dataclass
from pathlib import Path
from typing import Optional
from subprocess import check_call

HERE = Path(__file__).parent
HEADER_FILE = HERE.parent / "include/open62541pp/services/attribute_highlevel.hpp"


@dataclass
class Attribute:
    name: str
    type: str
    type_view: Optional[str] = None
    copy: bool = False
    writeable: bool = True
    details: Optional[str] = None


# fmt: off
ATTRIBUTES = [
    Attribute(name="NodeId", type="NodeId", copy=False, writeable=False),
    Attribute(name="NodeClass", type="NodeClass", copy=True, writeable=False),
    Attribute(name="BrowseName", type="QualifiedName", copy=False, writeable=True),
    Attribute(name="DisplayName", type="LocalizedText", copy=False, writeable=True),
    Attribute(name="Description", type="LocalizedText", copy=False, writeable=True),
    Attribute(name="WriteMask", type="Bitmask<WriteMask>", copy=True, writeable=True),
    Attribute(name="UserWriteMask", type="Bitmask<WriteMask>", copy=True, writeable=True),
    Attribute(name="IsAbstract", type="bool", copy=True, writeable=True),
    Attribute(name="Symmetric", type="bool", copy=True, writeable=True),
    Attribute(name="InverseName", type="LocalizedText", copy=False, writeable=True),
    Attribute(name="ContainsNoLoops", type="bool", copy=False, writeable=True),
    Attribute(name="EventNotifier", type="Bitmask<EventNotifier>", copy=True, writeable=True),
    Attribute(name="Value", type="Variant", copy=False, writeable=True),
    Attribute(name="DataType", type="NodeId", copy=False, writeable=True),
    Attribute(name="ValueRank", type="ValueRank", copy=True, writeable=True),
    Attribute(name="ArrayDimensions", type="std::vector<uint32_t>", type_view="Span<const uint32_t>", copy=False, writeable=True),
    Attribute(name="AccessLevel", type="Bitmask<AccessLevel>", copy=True, writeable=True),
    Attribute(name="UserAccessLevel", type="Bitmask<AccessLevel>", copy=True, writeable=True),
    Attribute(name="MinimumSamplingInterval", type="double", copy=True, writeable=True),
    Attribute(name="Historizing", type="bool", copy=True, writeable=True),
    Attribute(name="Executable", type="bool", copy=True, writeable=True),
    Attribute(name="UserExecutable", type="bool", copy=True, writeable=True),
    Attribute(name="DataTypeDefinition", type="Variant", copy=False, writeable=False, details="The attribute value can be of type EnumDefinition or StructureDefinition."),
]
# fmt: on

TEMPLATE_HEADER = """
/* ---------------------------------------------------------------------------------------------- */
/*                                   Generated - do not modify!                                   */
/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "open62541pp/services/attribute.hpp"

namespace opcua::services {{

{body}

}}  // namespace opcua::services
""".lstrip()

TEMPLATE_READ = """
/**
 * Read the AttributeId::{attr} attribute of a node.
 * {details}
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<{type}> read{attr}(T& connection, const NodeId& id) noexcept {{
    return detail::readAttributeImpl<AttributeId::{attr}>(connection, id);
}}

/**
 * @copydoc read{attr}
 * @param token @completiontoken{{void({param_type_callback})}}
 * @return @asyncresult{{Result<{type}>}}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto read{attr}Async(
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
 * {details}
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param {param_name} Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode write{attr}(T& connection, const NodeId& id, {param_type} {param_name}) noexcept {{
    return detail::writeAttributeImpl<AttributeId::{attr}>(connection, id, {param_name});
}}

/**
 * @copydoc write{attr}
 * @param token @completiontoken{{void(StatusCode)}}
 * @return @asyncresult{{StatusCode}}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto write{attr}Async(
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




def postprocess(code: str) -> str:
    def strip_empty_doc_lines(code: str) -> str:
        lines = code.splitlines()
        lines_filtered = filter(lambda line: line.strip() != "*", lines)
        return "\n".join(lines_filtered)

    return strip_empty_doc_lines(code)


def gen_functions():
    for attr in ATTRIBUTES:
        format_args = {
            "attr": attr.name,
            "type": attr.type,
            "param_type_callback": f"Result<{attr.type}>" if attr.copy else f"Result<{attr.type}>&",
            "param_type": attr.type_view or (attr.type if attr.copy else f"const {attr.type}&"),
            "param_name": attr.name[0].lower() + attr.name[1:],
            "details": attr.details or "",
        }
        yield postprocess(TEMPLATE_READ.format(**format_args))
        if attr.writeable is True:
            yield postprocess(TEMPLATE_WRITE.format(**format_args))


def main():
    body = "\n".join(gen_functions()).strip()
    content = TEMPLATE_HEADER.format(body=body)
    HEADER_FILE.write_text(content)
    check_call(("clang-format", "-i", HEADER_FILE))


if __name__ == "__main__":
    main()

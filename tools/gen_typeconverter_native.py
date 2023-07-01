from pathlib import Path
from typing import Optional

HERE = Path(__file__).parent
SCHEMA_DIR = HERE.parent / "3rdparty" / "open62541" / "tools" / "schema"
HEADER_FILE = HERE.parent / "include" / "open62541pp" / "TypeConverterNative.h"

FILENAMES_AND_GUARDS = {
    "datatypes_minimal.txt": None,
    "datatypes_method.txt": "UA_ENABLE_METHODCALLS",
    "datatypes_subscriptions.txt": "UA_ENABLE_SUBSCRIPTIONS",
    "datatypes_events.txt": "UA_ENABLE_SUBSCRIPTIONS_EVENTS",
    "datatypes_historizing.txt": "UA_ENABLE_HISTORIZING",
    "datatypes_discovery.txt": "UA_ENABLE_DISCOVERY",
    "datatypes_query.txt": "UA_ENABLE_QUERY",
    "datatypes_pubsub.txt": "UA_ENABLE_PUBSUB",
    "datatypes_dataaccess.txt": "UA_ENABLE_DA",
}

TEMPLATE_HEADER = """
/* ---------------------------------------------------------------------------------------------- */
/*                                   Generated - do not modify!                                   */
/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "open62541pp/TypeConverter.h"
#include "open62541pp/open62541.h"

namespace opcua {{

// clang-format off

{body}

// clang-format on

}}  // namespace opcua
""".lstrip()

TEMPLATE_MACRO = (
    "#ifdef {typeindex_define}\n"
    "UAPP_TYPECONVERTER_NATIVE({type}, {typeindex_define})\n"
    "#endif"
)

EXCLUDE_TYPES = [
    # builtin types
    "Boolean",
    "SByte",
    "Byte",
    "Int16",
    "UInt16",
    "Int32",
    "UInt32",
    "Int64",
    "UInt64",
    "Float",
    "Double",
    "String",
    "DateTime",
    "Guid",
    "ByteString",
    "XmlElement",
    "NodeId",
    "ExpandedNodeId",
    "StatusCode",
    "QualifiedName",
    "LocalizedText",
    "ExtensionObject",
    "DataValue",
    "Variant",
    "DiagnosticInfo",
    # type aliases
    "Duration",  # Double
    "UtcTime",  # Int64
    "LocaleId",  # String
]


def gen_by_file(filename: str, guard: Optional[str]):
    typenames = (SCHEMA_DIR / filename).read_text().strip().splitlines()
    typenames = list(dict.fromkeys(typenames))  # remove duplicates, keep order
    if guard:
        yield f"#ifdef {guard}"
    for typename in typenames:
        if typename in EXCLUDE_TYPES:
            continue
        type = f"UA_{typename}"
        typeindex_define = f"UA_TYPES_{typename.upper()}"
        yield TEMPLATE_MACRO.format(type=type, typeindex_define=typeindex_define)
    if guard:
        yield f"#endif  // {guard}"


def gen_body():
    for filename, guard in FILENAMES_AND_GUARDS.items():
        yield from gen_by_file(filename, guard)
        yield ""


def main():
    body = "\n".join(gen_body()).strip()
    header = TEMPLATE_HEADER.format(body=body)
    HEADER_FILE.write_text(header)


if __name__ == "__main__":
    main()

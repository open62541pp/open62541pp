from pathlib import Path

HERE = Path(__file__).parent
SCHEMA_DIR = HERE.parent / "3rdparty" / "open62541" / "tools" / "schema"
HEADER_FILE = HERE.parent / "include" / "open62541pp" / "TypeConverterNative.h"

FILES_DATATYPES = [
    SCHEMA_DIR / "datatypes_minimal.txt",
    SCHEMA_DIR / "datatypes_method.txt",
    SCHEMA_DIR / "datatypes_subscriptions.txt",
    SCHEMA_DIR / "datatypes_events.txt",
    SCHEMA_DIR / "datatypes_historizing.txt",
    SCHEMA_DIR / "datatypes_discovery.txt",
    SCHEMA_DIR / "datatypes_query.txt",
    SCHEMA_DIR / "datatypes_pubsub.txt",
    SCHEMA_DIR / "datatypes_dataaccess.txt",
]

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

TEMPLATE_MACRO = """
#ifdef {typeindex_define}
UAPP_TYPECONVERTER_NATIVE({type}, {typeindex_define})
#endif
""".strip()

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
    "DiscoveryConfiguration",  # void*
    "FilterOperand",  # void*
    "DataSetFieldContentMask",  # UInt32
    "DataSetFieldFlags",  # UInt16
    "JsonDataSetMessageContentMask",  # UInt32
    "JsonNetworkMessageContentMask",  # UInt32
    "UadpDataSetMessageContentMask",  # UInt32
    "UadpNetworkMessageContentMask",  # UInt32
    "PermissionType",  # UInt32
]


def main():
    # remove duplicates to prevent redefinitions
    typenames = set.union(*(
        set(f.read_text().strip().splitlines())
        for f in FILES_DATATYPES
    ))
    typenames -= set(EXCLUDE_TYPES)
    body = "\n".join(
        TEMPLATE_MACRO.format(
            type=f"UA_{typename}",
            typeindex_define = f"UA_TYPES_{typename.upper()}"
        )
        for typename in sorted(typenames)
    )
    header = TEMPLATE_HEADER.format(body=body)
    HEADER_FILE.write_text(header)


if __name__ == "__main__":
    main()

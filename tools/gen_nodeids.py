import urllib.request
from collections import defaultdict
from pathlib import Path

URL = "http://www.opcfoundation.org/UA/schemas/1.04/NodeIds.csv"
HERE = Path(__file__).parent
HEADER_FILE = HERE.parent / "include" / "open62541pp" / "NodeIds.h"

TEMPLATE_HEADER = """
/* ---------------------------------------------------------------------------------------------- */
/*                                   Generated - do not modify!                                   */
/* ---------------------------------------------------------------------------------------------- */

#pragma once

// ignore (false-positive?) warning of GCC:
// declaration of ‘MonitoringParameters’ shadows a global declaration
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace opcua {{

/**
 * @defgroup NodeIds Generated NodeIds
 * Numeric NodeIds defined by the OPC UA specification.
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/A.3
 */

// clang-format off

{body}

// clang-format on

}}  // namespace opcua

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
""".lstrip()

TEMPLATE_ENUM = """
/**
 * {nodeclass} node ids defined by the OPC UA specification (generated).
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/A.3
 * @ingroup NodeIds
 */
enum class {enum_name} : uint32_t {{
{body}
}};
""".strip()


def main():
    opener = urllib.request.build_opener()
    opener.addheaders = [("User-agent", "Mozilla/5.0")]

    with opener.open(URL) as f:
        content = f.read().decode("utf-8")

    nodeids = defaultdict(list)  # node class as key

    for line in content.splitlines():
        name, id, nodeclass = line.split(",")
        nodeids[nodeclass].append((name, int(id)))

    def gen_enum_definition(enum_name: str, nodeclass: str) -> str:
        body = "\n".join(f"    {name} = {id}," for name, id in nodeids[nodeclass])
        return TEMPLATE_ENUM.format(
            nodeclass=nodeclass,
            enum_name=enum_name,
            body=body,
        )

    enums = [
        gen_enum_definition(f"{nodeclass}Id", nodeclass)
        for nodeclass in (
            "DataType",
            "ReferenceType",
            "ObjectType",
            "VariableType",
            "Object",
            "Variable",
            "Method",
        )
    ]
    header = TEMPLATE_HEADER.format(body="\n\n".join(enums))
    HEADER_FILE.write_text(header)


if __name__ == "__main__":
    main()

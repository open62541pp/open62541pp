#!/usr/bin/env python

import sys
import re
from pathlib import Path


REMOVE_KEYWORDS = [
    " UA_EXPORT",
    " UA_INLINE",
    " UA_FUNC_ATTR_WARN_UNUSED_RESULT",
    " UA_FUNC_ATTR_MALLOC",
    " UA_RESTRICT ",
]


def process_header(headerfile: Path):
    """
    Remove reStructuredText documentation from open62541 header file.

    All text in /** */ comments is reStructuredText.
    """
    content = headerfile.read_text("utf-8")

    # remove rst comment blocks
    content = re.sub("/\*\*[\s\S]+?\*/$", "", content, flags = re.M)  # /** ... */

    # transform /* ... */ comments to doxygen style /** ... */
    content = re.sub("^/\* ", "/** ", content, flags = re.M)

    for keyword in REMOVE_KEYWORDS:
        content = content.replace(keyword, "")

    headerfile.write_text(content, "utf-8")


if __name__ == "__main__":
    if len(sys.argv) == 2:
        for headerfile in Path(sys.argv[1]).rglob("*.h"):
            process_header(headerfile)
    else:
        print("Usage: python prepare_open62541_header.py <dir>")

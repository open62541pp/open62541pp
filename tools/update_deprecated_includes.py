from __future__ import annotations

import re
from argparse import ArgumentParser
from pathlib import Path

HERE = Path(__file__).parent
CMAKELISTS = HERE.parent / "CMakeLists.txt"
REGEX = r"deprecated_header\((\S+)\s+(\S+)\)"


def full_include(include_path: str) -> str:
    return f"open62541pp/{include_path}"


def get_deprecation_dict() -> dict[str, str]:
    content = CMAKELISTS.read_text(encoding="utf-8")
    matches = re.finditer(REGEX, content, re.MULTILINE)
    return {
        full_include(match.group(1)): full_include(match.group(2))
        for match in matches
    }

def main():
    parser = ArgumentParser(description="Update deprecated open62541pp includes.")
    parser.add_argument("path", help="path to source file", type=Path, nargs="*")
    args = parser.parse_args()

    paths = args.path
    print("Paths:")
    for path in paths:
        print(f"- {path!s}")

    deprecation_dict = get_deprecation_dict()
    print("Deprecated an new headers:")
    for header_old, header_new in deprecation_dict.items():
        print(f"- {header_old:40} -> {header_new}")

    print("Replace includes...")
    for path in paths:
        content = path.read_text(encoding="utf-8")
        for header_old, header_new in deprecation_dict.items():
            content = content.replace(header_old, header_new)
        path.write_text(content, encoding="utf-8")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3

from __future__ import annotations

import os
import shutil
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parent.parent
TOOLS_DIR = ROOT_DIR / ".tools"

def get_clang_format_version() -> str:
    config = ROOT_DIR / ".pre-commit-config.yaml"
    in_mirrors_block = False
    for line in config.read_text().splitlines():
        if "pre-commit/mirrors-clang-format" in line:
            in_mirrors_block = True
        if in_mirrors_block and line.strip().startswith("rev:"):
            return line.split(":", 1)[1].strip().lstrip("v")
    raise RuntimeError("clang-format version not found in .pre-commit-config.yaml")

def main() -> int:
    clang_format_version = get_clang_format_version()
    TOOLS_DIR.mkdir(exist_ok=True)

    with tempfile.TemporaryDirectory(prefix="clang-format-") as temp_dir:
        temp_path = Path(temp_dir)

        print(f"Downloading clang-format {clang_format_version}...")
        subprocess.run(
            [
                sys.executable,
                "-m",
                "pip",
                "download",
                "--disable-pip-version-check",
                "--no-deps",
                "--only-binary=:all:",
                "--dest",
                str(temp_path),
                f"clang-format=={clang_format_version}",
            ],
            check=True,
        )

        wheels = list(temp_path.glob("*.whl"))
        if len(wheels) != 1:
            raise RuntimeError(
                f"Expected exactly one clang-format wheel, found {len(wheels)}"
            )

        wheel = wheels[0]

        with zipfile.ZipFile(wheel) as archive:
            executables = [
                name
                for name in archive.namelist()
                if name.startswith("clang_format/data/bin/")
                and Path(name).name in ("clang-format", "clang-format.exe")
            ]

            if len(executables) != 1:
                raise RuntimeError(
                    f"Expected exactly one clang-format executable, found: {executables}"
                )

            executable = executables[0]
            destination = TOOLS_DIR / Path(executable).name

            print(f"Installing {destination}")
            with archive.open(executable) as source, destination.open("wb") as target:
                shutil.copyfileobj(source, target)

            if os.name != "nt":
                destination.chmod(0o755)

    print(f"Installed clang-format {clang_format_version}:")
    subprocess.run([str(destination), "--version"], check=True)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

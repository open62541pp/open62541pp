"""
Update examples in README.

Install cog with `pip install cogapp`.
"""

from pathlib import Path
from subprocess import check_call

HERE = Path(__file__).parent
ROOT = HERE.parent
README = ROOT / "README.md"

check_call(("cog", "-r", str(README)), cwd=ROOT)

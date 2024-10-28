from pathlib import Path
from subprocess import check_call

HERE = Path(__file__).parent
DIRS = (
    HERE.parent / "examples",
    HERE.parent / "include",
    HERE.parent / "src",
    HERE.parent / "tests",
)
PATTERNS = ("**/*.cpp", "**/*.h", "**/*.hpp")


def main():
    for dir_path in DIRS:
        for pattern in PATTERNS:
            for file_path in dir_path.glob(pattern):
                print(file_path.resolve())
                check_call(("clang-format", "-i", str(file_path)))


if __name__ == "__main__":
    main()

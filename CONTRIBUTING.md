# Contributing to open62541++

Thank you for your interest in contributing to open62541++! There are many ways to contribute, and all of them are appreciated.

1. [Fork](https://github.com/open62541pp/open62541pp/fork) the open62541pp repository on GitHub
2. Clone your fork of open62541pp `git clone --recursive https://github.com/<your-username>/open62541pp.git`
3. Create a new branch for the feature or fix
4. Commit to the new branch
   - It's OK to have multiple small commits as you work on the PR - GitHub can automatically squash them before merging.
   - Commit messages must follow the [conventional commits specification](https://www.conventionalcommits.org/en/v1.0.0/) so that changelogs can be automatically generated.
5. [Create a pull request (PR) on GitHub](https://github.com/open62541pp/open62541pp/pulls)
   - If adding a new feature:
     - Provide a convincing reason to add this feature. Ideally, you should open a suggestion issue first and have it approved before working on it.
   - If fixing a bug:
     - Provide a detailed description of the bug in the PR.

## Development setup

```shell
# create build directory
mkdir build
cd build

# configure with recommended project options for development (sanitizers require GCC or Clang)
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DUAPP_BUILD_DOCUMENTATION=ON \
      -DUAPP_BUILD_EXAMPLES=ON \
      -DUAPP_BUILD_TESTS=ON \
      -DUAPP_ENABLE_CLANG_TIDY=ON \
      -DUAPP_ENABLE_SANITIZER_ADDRESS=ON \
      -DUAPP_ENABLE_SANITIZER_LEAK=ON \
      -DUAPP_ENABLE_SANITIZER_UNDEFINED_BEHAVIOR=ON \
      ..
# or use ccmake/cmake-gui to toggle the project options

# build
cmake --build .

# run tests
ctest --output-on-failure
```

Defined checks can be automatically executed before committing with [pre-commit](https://pre-commit.com):

```shell
# install pre-commit
pip install pre-commit
# install git hook scripts defined in `.pre-commit-config.yaml`
pre-commit install
# (optionally) run against all the files
pre-commit run --all-files
```

## Code style

Please use both and `clang-tidy` and `clang-format` during development.
The provided configs `.clang-tidy` and `.clang-format` will enforce modern C++, best practises and uniform formatting.

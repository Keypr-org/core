# core
> Repository containing the code of the core library that is responsible of handling encryption/decryption/parsing of the vault files.

## Build the project

You will need to install `vcpkg` in order to compile the project. Follow [these instructions](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash) to install it.

To build the project you must first fetch the dependencies using the following command:

```bash
cmake -S . -B build   -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Where `$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake` is the path to the vcpkg package root.

Then to build the project run the following command:

```bash
cmake --build build
```

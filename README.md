# Keypr Password Manager Core Library

Keypr Core is the C++20 library used by the [Keypr Password Manager](https://github.com/Keypr-org).
It manages encrypted vault files and the decrypted data they contain: categories, website
credentials, Wi-Fi credentials, credit cards, and personas.

The library uses [libsodium](https://doc.libsodium.org/) for password-based key derivation,
authentication, and authenticated encryption, and [nlohmann/json](https://github.com/nlohmann/json)
for the in-memory vault representation and serialization.

## Requirements

- CMake 4.0 or newer (the checked-in preset file uses schema version 10)
- A C++20-compatible compiler
- Git
- vcpkg

The repository's vcpkg manifest declares the required dependencies:

- `libsodium`
- `nlohmann-json`
- `gtest` (used by the test target)

## Install vcpkg

Clone vcpkg, bootstrap it, and export `VCPKG_ROOT` so that the CMake presets can find its
toolchain file. The following commands work on Linux and macOS:

```bash
git clone https://github.com/microsoft/vcpkg.git "$HOME/vcpkg"
"$HOME/vcpkg/bootstrap-vcpkg.sh"
export VCPKG_ROOT="$HOME/vcpkg"
```

To make `VCPKG_ROOT` available in future shells, add the export to your shell profile:

```bash
echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.bashrc
```

On macOS using zsh, use `~/.zshrc` instead. On Windows, clone the repository and run
`bootstrap-vcpkg.bat` from PowerShell:

```powershell
git clone https://github.com/microsoft/vcpkg.git "$env:USERPROFILE\vcpkg"
& "$env:USERPROFILE\vcpkg\bootstrap-vcpkg.bat"
$env:VCPKG_ROOT = "$env:USERPROFILE\vcpkg"
```

Set `VCPKG_ROOT` permanently in the system or user environment if it should persist across
PowerShell sessions. See the [official vcpkg installation guide](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started)
for platform-specific prerequisites and troubleshooting.

The first CMake configure step installs the manifest dependencies automatically into the build
directory. No separate `vcpkg install` command is required for this repository.

## Build with CMake presets

Run these commands from the repository root. The presets use
`$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake` as the CMake toolchain file and keep Debug and
Release artifacts in separate directories.

### Debug build and tests

The `default` workflow configures a Debug build, builds the library and tests, and runs the test
suite:

```bash
cmake --workflow default
```

The equivalent individual steps are:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset default
```

The resulting library is `build/debug/libkeypr-core.a` on Unix-like systems. The exact library
filename varies by platform.

### Release build

The `release` workflow configures and builds an optimized Release library without tests:

```bash
cmake --workflow release
```

The equivalent individual steps are:

```bash
cmake --preset release
cmake --build --preset release
```

The resulting library is written to `build/release/`. To start over with a different
configuration, remove only the corresponding build directory and rerun its workflow.

## Use the library from another CMake project

Keypr Core currently exposes its CMake target through the source tree; it does not define an
installed package or an exported CMake config. The simplest integration method is to add this
repository as a subdirectory (for example, as `third_party/keypr-core`) and link the
`keypr-core` target:

```cmake
cmake_minimum_required(VERSION 3.21)
project(my_password_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The library's tests are enabled by default by CTest; consumers generally do not need them.
set(BUILD_TESTING OFF CACHE BOOL "Disable Keypr Core tests" FORCE)
add_subdirectory(third_party/keypr-core)

add_executable(my_password_manager main.cpp)
target_link_libraries(my_password_manager PRIVATE keypr-core)
```

The consuming project must also declare the runtime dependencies in its own `vcpkg.json` (a
`vcpkg.json` inside an added subdirectory is not automatically used as the consuming manifest):

```json
{
  "dependencies": [
    "libsodium",
    "nlohmann-json"
  ]
}
```

Configure the consuming project with the same vcpkg toolchain:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```

Because `keypr-core` declares its include directory and dependencies as `PUBLIC`, linking the
target makes the headers, libsodium, and nlohmann-json available to the consuming target.
Include public headers by name:

```cpp
#include "VaultRepository.h"
#include "VaultSession.h"
#include "entities/Category.h"
#include "entities/Website.h"
```

## Vault lifecycle

`VaultRepository` is the normal entry point for file-backed use:

1. `createVault` creates an in-memory session and derives encryption and authentication keys from
   the master password.
2. The application changes the session through its categories, entries, and personas.
3. `lockVault` serializes the session, authenticates the header, encrypts the JSON body, and writes
   the complete vault atomically.
4. `unlockVault` reads and validates the file, derives the keys from the header salt and parameters,
   verifies the header, decrypts the body, and reconstructs a `VaultSession`.

Example:

```cpp
#include "VaultRepository.h"
#include "VaultSession.h"
#include "entities/Category.h"
#include "entities/Persona.h"
#include "entities/Website.h"

#include <chrono>
#include <iostream>
#include <memory>

int main() {
    VaultRepository repository;
    const std::string filename = "personal.kvdb";
    const std::string masterPassword = "a sufficiently long password";
    const DateTime dateOfBirth = std::chrono::system_clock::now();

    auto session = repository.createVault(masterPassword, "Personal vault");
    auto category = std::make_unique<Category>("Websites");
    const auto categoryId = category->getId();
    session->addCategory(std::move(category));

    auto website = std::make_unique<Website>(
        "Primary account", "Example", "alice", "secret",
        "https://example.com");
    const auto websiteId = website->getId();
    session->addEntryToCategory(categoryId, std::move(website));

    if (!repository.lockVault(*session, filename)) {
        std::cerr << "Could not save the vault\n";
        return 1;
    }

    auto reopened = repository.unlockVault(masterPassword, filename);
    if (!reopened) {
        std::cerr << "The password did not unlock the vault\n";
        return 1;
    }

    const auto* savedWebsite = reopened->getWebsiteById(websiteId);
    if (savedWebsite != nullptr) {
        std::cout << savedWebsite->getUsername() << '\n';
    }
}
```

`createVault` and successful `unlockVault` return `std::unique_ptr<VaultSession>`. An incorrect
master password returns `nullptr`; malformed, unreadable, or corrupted vault files raise
`UnlockVaultError`. `lockVault` returns `true` on success and `false` when serialization or writing
fails. Keep a session in memory only for as long as it is needed, since it contains decrypted
secrets.

## Working with sessions

### Categories and entries

Categories own entries through `std::unique_ptr`. Add and remove entries using the session methods,
and retain IDs when later operations need to refer to an object:

```cpp
const auto& categories = session->getCategories();
const auto& entries = session->getEntriesInCategory(categoryId);

session->removeEntryFromCategory(categoryId, websiteId);
```

The supported concrete entry types are:

| Type | Constructor data | Useful accessors |
| --- | --- | --- |
| `Website` | notes, title, username, password, URL, optional comments/persona/alias | `getTitle`, `getUsername`, `getPassword`, `getUrl` |
| `Wifi` | network name, password, optional notes | `getNetworkName`, `getPassword` |
| `CreditCard` | cardholder, card number, expiration, security code, optional notes | `getCardHolderName`, `getCardNumber`, `getExpiration` |

All entries inherit `Entry`, so common notes are available through `getNotes`. Use
`dynamic_cast` when iterating over `Entry` pointers and needing type-specific fields. IDs are
generated by the library when objects are constructed and can be read with `getId()`.

### Personas, links, and search

Personas are owned by the session and can be linked to website entries:

```cpp
auto persona = std::make_unique<Persona>(
    "Alice", "Example", dateOfBirth, "Example Street 1", "+41 79 123 45 67");
const auto personaId = persona->getId();
session->addPersona(std::move(persona));
session->linkPersonaToEntry(personaId, categoryId, websiteId);

const auto matches = session->searchEntriesInCategory(categoryId, "Example");
const auto websites = session->getWebsiteByUrl("example.com");
```

`searchEntriesInCategory` searches notes and type-specific display fields. Category, entry, and
persona lookup methods throw `CategoryNotFoundError`, `EntryNotFoundError`, or
`PersonaNotFoundError` when an ID does not exist. Linking a persona or setting a website alias on
the wrong entry type throws `EntryNotGoodTypeError`. Removing a persona also unlinks it from
websites that referenced it.

## Serialization and lower-level APIs

Most applications should use `VaultRepository`, but lower-level components are available:

- `VaultSession::serialize` and `VaultSession::parse` convert the decrypted session body to and
  from UTF-8 JSON bytes.
- `RawVault::parse` and `RawVault::serialize` handle the on-disk binary container.
- `VaultHeader::parse` and `VaultHeader::serialize` handle the fixed-size vault header.
- `CryptoService` exposes Argon2id key derivation, authentication, verification, encryption, and
  decryption for integrations that need the same primitives.
- `FileHandler` provides file reads, atomic writes, and existence checks.

These APIs use `Bytes` (`std::span<const uint8_t>`) and the fixed-size key and MAC types declared
in `Types.h`. Their failures are reported with specific runtime-error-derived exceptions such as
`ParseError`, `RawVaultParsingError`, `KeyDerivationError`, `DecryptionError`, and
`FileWriteError`; catch the relevant exception at an application boundary rather than ignoring
errors.

## How to contribute

Contributions are welcome. Please use the repository's [GitHub Issues](https://github.com/Keypr-org/core/issues)
to report bugs, suggest improvements, or discuss proposed changes before starting substantial work.
Include enough context to reproduce a problem, such as the platform, compiler, CMake version, and
relevant error output.

Submit changes through a [GitHub pull request](https://github.com/Keypr-org/core/pulls):

1. Create a branch for your change.
2. Make focused changes and update the documentation or tests when appropriate.
3. Run the Debug workflow (`cmake --workflow default`) and verify that all tests are passing before opening the pull request.
4. Describe the motivation, implementation, and validation performed in the pull request.

Keep pull requests focused on a single issue where possible. Reviewers may request changes before
the pull request is merged.

## AI usage

AI tools were used during the development of this project to:

- Generate some unit tests.
- Assist with the configuration of the build tools, including the CMake configuration and CI/CD
  pipeline.
- Help document the code and write this README.

All generated content was reviewed and integrated by the project contributors.

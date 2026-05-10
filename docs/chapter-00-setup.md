# Chapter 00 — Project Setup & Toolchain

## What we built
A CMake-based C++26 project using Homebrew's Clang 22, with a thin Makefile
wrapper for everyday commands.

---

## Compiler choice

macOS ships with **Apple Clang** (via Xcode), but Homebrew provides a more
up-to-date upstream build:

```bash
# Check Apple Clang
clang++ --version

# Check Homebrew Clang
$(brew --prefix llvm)/bin/clang++ --version

# Install if missing
brew install llvm
brew install cmake
```

We chose **Homebrew Clang 22** for its superior C++26 support. Apple Clang 21
supports C++23 well, but Homebrew Clang 22 gets us closer to the cutting edge.

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_COMPILER   "/opt/homebrew/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")

project(dungeon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(dungeon ${SOURCES})
target_include_directories(dungeon PRIVATE include)

target_compile_options(dungeon PRIVATE
    -Wall -Wextra -Wpedantic
    -Wshadow -Wconversion -Wnull-dereference
)
```

Key decisions:
- `CMAKE_CXX_EXTENSIONS OFF` — pure ISO C++, no GNU extensions
- `CMAKE_EXPORT_COMPILE_COMMANDS ON` — lets clangd and editors understand
  the project structure
- Warnings as noise detectors: `-Wshadow`, `-Wconversion`, `-Wnull-dereference`
  catch real bugs early

---

## Makefile

```makefile
.PHONY: init build start clean

BUILD_DIR := build
BINARY    := $(BUILD_DIR)/dungeon

init:
	cmake -B $(BUILD_DIR)

build:
	cmake --build $(BUILD_DIR)

start: build
	./$(BINARY)

clean:
	rm -rf $(BUILD_DIR)
```

> ⚠️ Makefile recipe lines must use **Tab characters**, not spaces.

| Command      | Effect                              |
|--------------|-------------------------------------|
| `make init`  | Configure project, generate build system |
| `make build` | Compile everything                  |
| `make start` | Build if needed, then launch        |
| `make clean` | Wipe the build folder               |

---

## Project structure

```
dungeon/
├── CMakeLists.txt
├── Makefile
├── docs/               ← you are here
├── include/
│   ├── overloaded.hpp
│   ├── terminal.hpp
│   ├── entities/
│   │   └── player.hpp
│   └── world/
│       ├── bsp.hpp
│       ├── grid.hpp
│       ├── renderer.hpp
│       └── tile.hpp
└── src/
    ├── main.cpp
    ├── entities/
    │   └── player.cpp
    └── world/
        ├── bsp.cpp
        └── renderer.cpp
```

---

## C++ concepts introduced

### `std::println` (C++23)
Replaces `printf` and `std::cout`. Type-safe, always appends a newline,
uses `std::format` under the hood.

```cpp
std::println("Hello, {}!", name);   // formatted + newline
std::print("no newline here");      // formatted, no newline
```

### `std::expected<T, E>` (C++23)
Returns either a value or a typed error — no exceptions, no output params.

```cpp
std::expected<void, InitError> init_terminal() {
    return {};                              // success
    return std::unexpected(InitError::TerminalTooSmall);  // failure
}

if (auto r = init_terminal(); !r) {
    // handle r.error()
}
```

### `enum class`
Scoped, type-safe enums. Values don't leak into surrounding scope.
The compiler warns if a `switch` doesn't cover all cases.

```cpp
enum class GameState { MainMenu, Playing, Paused, GameOver };
```

### `using` — type alias
Modern replacement for `typedef`. Reads left-to-right naturally and works
with templates.

```cpp
using Tile = std::variant<Floor, Wall, Door>;
// equivalent old form:
typedef std::variant<Floor, Wall, Door> Tile;
```

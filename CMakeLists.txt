cmake_minimum_required(VERSION 4.3.2)

# ── Toolchain: Homebrew LLVM 22 ───────────────────────────────────────────────
# Must be set BEFORE project() so CMake uses it for all compiler detection.
set(CMAKE_C_COMPILER   "/opt/homebrew/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")

project(dungeon LANGUAGES CXX)

# ── C++26 ─────────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)   # No GNU extensions; pure ISO C++

# ── Useful during development ──────────────────────────────────────────────────
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # Lets clangd / your editor understand the project

# ── Sources ───────────────────────────────────────────────────────────────────
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(dungeon ${SOURCES})

target_include_directories(dungeon PRIVATE include)

# ── Warnings ──────────────────────────────────────────────────────────────────
target_compile_options(dungeon PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wnull-dereference
)

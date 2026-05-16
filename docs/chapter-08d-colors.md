# Chapter 08d — Color System

## What we built

A two-layer color system extracted into a single header,
replacing scattered ANSI string literals across multiple files.

## The problem

Colors were declared locally in three files with three different
naming conventions — no shared truth, no consistency.

## Two-layer design

### Layer 1 — `Palette` namespace

Raw ANSI escape codes. What colors physically exist.
Named after the color itself.

```cpp
namespace Palette {
    inline constexpr std::string_view CYAN   = "\033[36m";
    inline constexpr std::string_view RED    = "\033[31m";
    // ... full ZX Spectrum palette
    inline constexpr std::string_view DIM    = "\033[2m";
    inline constexpr std::string_view BOLD   = "\033[1m";
    inline constexpr std::string_view RESET  = "\033[0m";
}
```

### Layer 2 — `Color` namespace

Semantic aliases. What colors *mean* in this game.
Named after intent, not implementation.

```cpp
namespace Color {
    inline constexpr std::string_view DANGER  = Palette::RED;
    inline constexpr std::string_view LOOT    = Palette::YELLOW;
    inline constexpr std::string_view STAIRS  = Palette::CYAN;
    inline constexpr std::string_view DOOR    = Palette::GREEN;
    // ...
}
```

## Why two layers?

Changing "danger" from red to magenta = one line in `Color`.
Without layers it would require a grep across the whole codebase.

`Color::DANGER` tells you *why* something is that color.
`"\033[31m"` tells you nothing.

## `inline constexpr std::string_view`

The right type for compile-time string constants in headers:

- `constexpr`  — evaluated at compile time, zero runtime cost
- `inline`     — safe to define in headers, no duplicate symbols
- `string_view` — non-owning, no allocation, no copy

## ZX Spectrum palette

8 colors × 2 brightness levels = 15 unique colors:

| Color   | Normal     | Bright     |
|---------|------------|------------|
| Black   | `\033[30m` | `\033[90m` |
| Red     | `\033[31m` | `\033[91m` |
| Green   | `\033[32m` | `\033[92m` |
| Yellow  | `\033[33m` | `\033[93m` |
| Blue    | `\033[34m` | `\033[94m` |
| Magenta | `\033[35m` | `\033[95m` |
| Cyan    | `\033[36m` | `\033[96m` |
| White   | `\033[37m` | `\033[97m` |

## Principle applied

**Single source of truth** — one definition, referenced everywhere.
Changing the palette means changing one file.
Every file that uses colors gets the update for free.

# Chapter 08a — New Tile Types

## What we added

Four new tile types extending the existing variant system,
plus richer rendering with ANSI colours and Unicode glyphs.

---

## New tile types

```cpp
// include/world/tile.hpp
 
enum class StairsDirection { Up, Down };
 
struct Stairs {
    StairsDirection direction;
};
 
struct Chest {
    bool is_open       = false;
    bool is_locked     = false;
    bool is_mimic      = false;
    bool found_open    = false;  // was already open when discovered
};
 
using Tile = std::variant<Floor, Wall, Door, Stairs, Chest>;
```

---

## Design decisions

### One `Stairs` struct instead of two

A staircase physically connects two levels — it exists on both floors
at the same position. One struct with a direction property models this
cleanly and avoids duplicated logic.

### `Chest::found_open`

Distinguishes player-opened chests (already looted) from
pre-opened chests (suspicious — possible mimic).

### `Chest::is_mimic`

A mimic is a chest that attacks instead of giving loot.
The glyph is identical to a normal chest — the player never
knows until they interact. Pure information asymmetry.

### Single source of truth — `base_glyph`

Adding new tile types required updating exactly one function.
Every `std::visit` elsewhere got the new types for free.

---

## `base_glyph` — returns `std::string`

Changed from `char` to `std::string` to support:

- Multi-byte Unicode characters (`≠` for locked chest)
- ANSI colour codes wrapping each glyph
- Future emoji support (wide character rendering TBD)

```cpp
static std::string base_glyph(const Tile& tile) {
    return std::visit(overloaded{
        [](const Floor&)    -> std::string { return WHITE + "."; },
        [](const Wall&)     -> std::string { return WHITE + "#"; },
        [](const Door& d)   -> std::string { ... },
        [](const Stairs& s) -> std::string { ... },
        [](const Chest& c)  -> std::string { ... },
    }, tile);
}
```

The explicit `-> std::string` return type on branching lambdas
helps the compiler deduce that all branches return the same type.

---

## Colour palette — ZX Spectrum inspired

```cpp
static constexpr std::string_view WHITE  = "\033[37m";  // floor, walls
static constexpr std::string_view GREEN  = "\033[32m";  // doors
static constexpr std::string_view YELLOW = "\033[33m";  // chests, loot
static constexpr std::string_view CYAN   = "\033[36m";  // stairs, special
static constexpr std::string_view RED    = "\033[31m";  // locked, danger
static constexpr std::string_view DIM    = "\033[2m";   // fog of war
static constexpr std::string_view RESET  = "\033[0m";   // always last
```

Colour as a communication layer — the player reads intent before
reading the glyph. Red means danger. Yellow means value. Cyan means
something special is here.

### ANSI sequence ownership

`base_glyph` sets the colour but does NOT append RESET.
`glyph_for` always appends RESET exactly once at the end:

```cpp
// visible tile:
return base + std::string(RESET);
 
// remembered tile (fog):
return std::string(DIM) + base + std::string(RESET);
```

This prevents inner RESET from cancelling DIM prematurely.

---

## Glyph reference

| Tile           | Glyph | Colour |
|----------------|-------|--------|
| Floor          | `.`   | White  |
| Wall           | `#`   | White  |
| Door open      | `_`   | Green  |
| Door closed    | `+`   | Green  |
| Door locked    | `x`   | Red    |
| Stairs down    | `>`   | Cyan   |
| Stairs up      | `<`   | Cyan   |
| Chest closed   | `=`   | Yellow |
| Chest open     | `~`   | Yellow |
| Chest locked   | `=`   | Red    |

---

## Placing landmarks — `place_landmarks`

Runs after `carve_corridors` in `generate()`. Places:

- `StairsUp` in the first leaf room (player start)
- `StairsDown` in the last leaf room (furthest from start)
- 1–3 `Chest`s in random middle rooms

### Preventing overlap — `std::unordered_set<std::size_t>`

Tracks used room indices. Same pattern as fog of war visited tiles.

```cpp
std::unordered_set<std::size_t> used_rooms;
used_rooms.insert(0);                  // stairs up room
used_rooms.insert(leaves.size() - 1); // stairs down room
 
// Skip used rooms when placing chests
while (used_rooms.contains(index))
    index = pick(rng);
used_rooms.insert(index);
```

### Defensive programming

Guard against infinite loops when all rooms are exhausted:

```cpp
if (used_rooms.size() >= leaves.size()) break;
```

### Random chest properties

```cpp
std::uniform_int_distribution<int> one_in_three{0, 2};
std::uniform_int_distribution<int> one_in_five{0, 4};
 
bool found_open = one_in_three(rng) == 0;  // 33% pre-opened
bool is_mimic   = one_in_five(rng)  == 0;  // 20% mimic
```

---

## `FogMode` enum class — avoiding bool parameters

```cpp
enum class FogMode { Enabled, Disabled };
```

Replaces `bool fog_enabled` parameter. Call sites become self-documenting:

```cpp
render(grid, player, FogMode::Disabled);  // clear intent
render(grid, player, false);              // what does false mean here?
```

> **Rule:** avoid `bool` parameters. A named `enum class` makes
> call sites readable without consulting the function signature.

## Debug flags

```cpp
// main.cpp
static constexpr FogMode DEBUG_FOG_MODE = FogMode::Disabled;
```

`static constexpr` at file scope:

- `constexpr` — evaluated at compile time, zero runtime cost
- `static` — private to this translation unit
- The compiler eliminates disabled code paths entirely

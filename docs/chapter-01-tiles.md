# Chapter 01 — Tile System & `std::variant`

## What we built
A type-safe tile system where each tile is one of several distinct types,
each carrying only its own relevant data.

---

## The problem with classic approaches

### The char array approach
```cpp
char grid[HEIGHT][WIDTH];
// '#' = wall, '.' = floor, '+' = door
```
Nothing stops you putting `'z'` in there. Adding data to a tile type
(e.g. "is this door locked?") means ugly parallel arrays.

### The enum approach
```cpp
enum class TileKind { Floor, Wall, Door };

struct TileData {
    TileKind kind;
    bool door_is_open = false;  // meaningless for Floor and Wall
    int  wall_hp      = 0;      // meaningless for Floor and Door
};
```
Every tile carries fields that only make sense for one specific kind.
Wasteful and error-prone.

---

## The modern solution: `std::variant`

`std::variant<A, B, C>` is a **type-safe union** — holds exactly one of
several possible types at a time, and the compiler always knows which one.

```cpp
// include/world/tile.hpp
#pragma once

#include <variant>

struct Floor {};

struct Wall {};

struct Door {
    bool is_open = false;
};

using Tile = std::variant<Floor, Wall, Door>;
```

Each alternative is its own struct and carries **only its own relevant data**.
A `Door` knows if it's open. A `Wall` doesn't pretend to have that field.

This pattern — variant where each alternative is a struct — is called a
**sum type**. Rust's `enum` works the same way.

---

## `std::visit` and exhaustiveness checking

`std::visit` takes a callable and a variant, and calls the callable with
whatever type the variant currently holds. The compiler forces you to handle
**every possible type** — forget one and it won't compile.

```cpp
char glyph = std::visit(overloaded{
    [](const Floor&) { return '.'; },
    [](const Wall&)  { return '#'; },
    [](const Door& d){ return d.is_open ? '_' : '+'; },
}, tile);
```

If you add a new tile type to the variant and forget to handle it in a
`std::visit`, the compiler refuses to build. This turns a silent runtime
bug into a loud compile error.

---

## The `overloaded` helper

`std::visit` needs a single callable that handles all variant types.
The `overloaded` pattern merges multiple lambdas into one:

```cpp
// include/overloaded.hpp
#pragma once

template<typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
```

- `typename... Ts` — a **parameter pack**: zero or more types
- `: Ts...` — `overloaded` inherits from all of them
- `using Ts::operator()...` — pulls every parent's `operator()` into scope

This is a **header-only** template by necessity — templates must be fully
visible at the point of instantiation, so they always live in headers.

---

## Variant access utilities

```cpp
// Check which type a variant currently holds
std::holds_alternative<Wall>(tile)      // → bool

// Extract a value (throws if wrong type)
std::get<Door>(tile).is_open            // → bool

// Extract safely (returns nullptr if wrong type)
auto* d = std::get_if<Door>(&tile);
if (d) { /* use d->is_open */ }
```

---

## Designated initialisers (C++20)

Name the fields you're initialising instead of relying on positional order:

```cpp
Door d{ .is_open = true };   // C++20 designated initialiser
```

Clearer, safer when structs gain new fields, and self-documenting.

---

## Key principles introduced

### Don't store what you can derive
Every extra piece of stored state is a new opportunity for inconsistency.
Derive values from existing state on demand instead.

```cpp
// ❌ fragile — must remember to keep in sync
bool is_leaf;
std::unique_ptr<BSPNode> left, right;

// ✅ derived — always correct by construction
bool is_leaf() const { return left == nullptr && right == nullptr; }
```

### Sum types force exhaustiveness
`std::variant` + `std::visit` give you compile-time proof that every
case is handled, everywhere, forever. Adding a new tile type breaks every
unhandled `std::visit` at compile time — not at runtime.

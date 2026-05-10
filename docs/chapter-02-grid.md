# Chapter 02 — World Grid & `std::mdspan`

## What we built
A memory-efficient 2D dungeon grid using a flat `std::vector` viewed through
`std::mdspan` — contiguous memory with clean 2D indexing.

---

## Why not a 2D array or vector of vectors?

```cpp
// Option A — fixed size, must be known at compile time
Tile grid[HEIGHT][WIDTH];

// Option B — each row is a separate heap allocation
std::vector<std::vector<Tile>> grid;
```

Option B is the common instinct but has a hidden cost: each inner `vector`
is a **separate allocation** scattered around memory. The CPU fetches memory
in cache lines — scattered allocations cause cache misses on every row
boundary, which is measurable on large maps.

---

## The modern solution: flat + mdspan

Store data in **one contiguous block**, view it as 2D:

```
Memory (flat):  [T][T][T][T][T][T][T][T][T]   ← one allocation
Logical view:   row 0: [T][T][T]
                row 1: [T][T][T]
                row 2: [T][T][T]
```

`std::mdspan` (C++23) is a **non-owning multidimensional view** over
contiguous memory. It owns no memory itself — it just gives you clean
multi-dimensional indexing over memory that lives somewhere else.

---

## The Grid struct

```cpp
// include/world/grid.hpp
#pragma once

#include <vector>
#include <mdspan>
#include "world/tile.hpp"

struct Grid {
    std::size_t rows;
    std::size_t cols;
    std::vector<Tile> cells;

    Grid(std::size_t rows, std::size_t cols)
        : rows{rows}, cols{cols}, cells(rows * cols, Tile{Floor{}})
    {}

    auto view() {
        return std::mdspan(cells.data(), rows, cols);
    }
};
```

`cells` owns the memory. `view()` hands out a window into it.

---

## Member initialiser lists

The `: rows{rows}, cols{cols}, cells(...)` syntax is a **member initialiser
list** — the proper C++ way to initialise members at construction time.

```cpp
// ❌ assignment in body — member is default-initialised first, then overwritten
Grid(std::size_t rows, std::size_t cols) {
    this->rows  = rows;
    this->cells = std::vector<Tile>(...);  // wasted default construction
}

// ✅ initialiser list — members are initialised directly, no wasted step
Grid(std::size_t rows, std::size_t cols)
    : rows{rows}, cols{cols}, cells(rows * cols, Tile{Floor{}})
{}
```

The vector fill constructor `vector(count, value)` produces `count` copies
of `value`. `Tile{Floor{}}` constructs a `variant` holding a `Floor`.

---

## `std::mdspan` indexing

```cpp
auto v = grid.view();

// C++23 multi-index syntax — comma inside the brackets
v[1, 3] = Tile{Wall{}};         // set row 1, col 3
auto t  = v[0, 0];              // read row 0, col 0
```

The `v[1, 3]` syntax with a comma inside the brackets is new in C++23.
Older C++ required `v[1][3]` with nested indexing.

The explicit full return type of `view()` would be:
```cpp
std::mdspan<Tile, std::dextents<std::size_t, 2>>
```
`auto` saves us from writing this every time — the type is an implementation
detail the caller doesn't need to care about.

---

## `std::size_t`

An **unsigned integer type** guaranteed to be large enough to hold the size
of any object in memory. On 64-bit Apple Silicon it is 64 bits wide.

The `_t` suffix is a convention meaning "this is a typedef".

| Type              | Use when                                          |
|-------------------|---------------------------------------------------|
| `std::size_t`     | Sizes, counts, indices into containers            |
| `std::int32_t`    | Exact bit width matters (file formats, network)   |
| `std::ptrdiff_t`  | Signed size arithmetic (differences, negatives)   |
| `int`             | Everyday arithmetic with no special constraints   |

Rule of thumb: indexing into a container or describing memory size → `std::size_t`.

---

## `auto` — when to use it

```cpp
auto grid  = Grid{10, 10};   // ✅ type obvious from right-hand side
auto v     = grid.view();    // ✅ type is a mouthful, caller just uses it
std::size_t n = v.size();    // ✅ explicit: communicates intent to reader
```

> Use `auto` when the type is verbose, obvious from context, or an
> implementation detail. Be explicit when the type itself communicates intent.

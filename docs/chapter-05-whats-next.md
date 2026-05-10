# Chapter 05 — What's Next

## Where we are

At this point the project is a fully playable terminal roguelike foundation:

```
✅ CMake + Makefile toolchain     Homebrew Clang 22, C++26
✅ Tile system                    std::variant, std::visit, overloaded
✅ World grid                     std::mdspan, flat memory layout
✅ BSP dungeon generation         recursion, std::unique_ptr, std::mt19937
✅ Renderer                       std::print, ANSI escape codes
✅ Terminal raw mode              RAII, = delete, termios
✅ Player + movement              std::expected, std::ptrdiff_t, std::move
```

---

## Planned chapters

---

### Chapter 06 — Fog of War
**C++ concepts:** `std::unordered_set`, custom hash, `std::pair`

The player only sees tiles they have visited. Unvisited tiles render as
empty space. Visited but not currently visible tiles render dimly.

Design sketch:
```cpp
struct Player {
    // ...
    std::unordered_set<std::pair<std::size_t, std::size_t>, PairHash> visited;
};
```

`std::unordered_set` requires a hash function for the key type.
`std::pair` has no built-in hash, so we write a small `PairHash` functor —
a great excuse to learn about hash combining and `operator()` on structs.

---

### Chapter 07 — Enemies & AI
**C++ concepts:** `std::ranges` pipelines, `std::views::filter`,
`std::views::transform`, polymorphism with `std::variant`

Enemies wander the dungeon and chase the player when nearby.

Design sketch:
```cpp
struct Goblin { int health; int attack; };
struct Troll  { int health; int attack; int armor; };

using Enemy = std::variant<Goblin, Troll>;
std::vector<Enemy> enemies;

// Find all enemies near the player using ranges
auto nearby = enemies
    | std::views::filter([&](const Enemy& e) {
          return distance(e, player) < 5;
      });
```

Pathfinding via BFS (breadth-first search) — a natural fit for
`std::queue` and the grid structure we already have.

---

### Chapter 08 — Combat & Inventory
**C++ concepts:** `std::variant` for items, `std::optional`,
`std::vector` algorithms

Weapons, potions and armour as items on the floor. Player picks them up,
equips weapons, uses potions. Combat resolves via shield-then-health damage.

Design sketch:
```cpp
struct Sword  { int damage; };
struct Bow    { int damage; int range; };
struct Potion { int heal_amount; };

using Item   = std::variant<Sword, Bow, Potion>;
using Weapon = std::variant<Fists, Sword, Bow>;

struct Player {
    // ...
    Weapon equipped = Fists{};
    std::vector<Item> inventory;
    std::optional<Item> held_item;  // item under the player's feet
};
```

`std::optional<T>` is the right type for "a value that may or may not
be present" — cleaner than a raw pointer or a sentinel value.

---

### Chapter 09 — Double Buffering
**C++ concepts:** `std::string` building, `std::string_view`,
single-write rendering

Fix the flickering by building the entire frame into a `std::string`
first, then flushing it to stdout in one `write()` syscall.

```cpp
void render(Grid& grid, const Player& player) {
    std::string frame;
    frame.reserve(grid.rows * (grid.cols + 1));  // pre-allocate

    for (std::size_t row = 0; row < grid.rows; ++row) {
        for (std::size_t col = 0; col < grid.cols; ++col)
            frame += glyph_for(grid, player, row, col);
        frame += '\n';
    }

    // One syscall — no visible blank frame
    ::write(STDOUT_FILENO, frame.data(), frame.size());
}
```

---

### Chapter 10 — Modules (C++20)
**C++ concepts:** `export module`, `import`, BMI files, CMake module support

Migrate one subsystem (e.g. the tile system) from headers to a C++20 module
and compare build times, encapsulation, and tooling experience.

```cpp
// world_tile.cppm
export module world.tile;

import std;

export struct Floor {};
export struct Wall  {};
export struct Door  { bool is_open = false; };
export using  Tile = std::variant<Floor, Wall, Door>;

// internal helper — NOT exported, invisible to importers
static char default_glyph() { return '.'; }
```

```cpp
// main.cpp
import std;
import world.tile;
```

Modules eliminate header inclusion order bugs, macro leakage across
translation units, and repeated parsing of the same declarations.
They are the future of C++ code organisation.

---

### Chapter 11 — Save & Load
**C++ concepts:** `std::format`, file I/O with `std::fstream`,
serialisation design

Save the current dungeon seed, player position, health and visited tiles
to a file. Load and reconstruct the session on next launch.

Because the dungeon is procedurally generated from a seed, we only need
to save the seed (not the entire grid) — then replay generation identically
to reconstruct the world. Only player state and visited tiles need
explicit serialisation.

---

## Key C++ principles from this project

| Principle | Where we saw it |
|---|---|
| Single source of truth | `is_leaf()`, default member initialisers |
| Don't store what you derive | `is_leaf()` vs `bool is_leaf` |
| Explicit over implicit | `static_cast`, `= delete`, named parameters |
| RAII for all resources | `Terminal`, `std::unique_ptr` |
| Errors as values | `std::expected` in `try_move` |
| Exhaustive case handling | `std::visit` + `overloaded` |
| Contiguous memory | flat `vector` + `std::mdspan` |
| Zero-cost abstractions | `auto`, ranges views, `std::move` |

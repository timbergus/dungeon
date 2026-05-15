# Chapter 06 — Fog of War

## What we built

A three-state visibility system: dark (never seen), remembered (seen but
not currently visible), and visible (within radius and line of sight).

## The three states

| State      | Condition                        | Render          |
|------------|----------------------------------|-----------------|
| Dark       | never seen                       | ` ` (space)     |
| Remembered | visited, outside radius or LOS   | dimmed glyph    |
| Visible    | within radius + line of sight    | normal glyph    |

## `std::unordered_set` for visited tiles

Stores only tiles the player has actually seen.
Lookup and insertion are both O(1) average.

```cpp
std::unordered_set
    std::pair,
    PairHash
> visited;
```

`std::pair` has no built-in hash — we write PairHash ourselves.

## Custom hash — `PairHash`

```cpp
struct PairHash {
    template
    std::size_t operator()(const std::pair& p) const {
        std::size_t h1 = std::hash{}(p.first);
        std::size_t h2 = std::hash{}(p.second);
        return h1 ^ (h2 << 32 | h2 >> 32);
    }
};
```

`operator()` makes a struct callable like a function — a **functor**.
Lambdas are compiler-generated functors. `std::hash`, `overloaded`,
and `PairHash` are all the same pattern.

## Euclidean distance for circular visibility

```cpp
auto dr = static_cast(r) - static_cast(row);
auto dc = static_cast(c) - static_cast(col);
return std::sqrt(dr * dr + dc * dc) <= static_cast(VISIBILITY_RADIUS);
```

Chebyshev distance (max of dr, dc) produces a square.
Euclidean distance produces a natural circle.

## Raycasting — Bresenham's line algorithm

For each tile in radius, trace a line from the player.
If the line passes through a wall before reaching the tile,
the tile is not visible.

```cpp
bool has_line_of_sight(const Grid& grid,
                        std::size_t tr, std::size_t tc) const;
```

Bresenham's algorithm traces a line using only integer arithmetic —
no floating point, no rounding errors, very fast.

## `const` correctness

`const` on a method promises the compiler the method won't modify
any member. Two overloads differing only in `const` are both valid —
the compiler picks the right one automatically:

```cpp
auto view()       { return std::mdspan(cells.data(), rows, cols); }
auto view() const { return std::mdspan(cells.data(), rows, cols); }
```

`const_cast` removes const and is a code smell — it signals incomplete
const correctness somewhere in the design. Always fix the root cause
instead of casting.

## `constexpr` vs `#define`

```cpp
inline constexpr std::size_t VISIBILITY_RADIUS = 5;
```

- **Typed** — compiler enforces the type
- **Scoped** — lives where you declare it
- **Debugger visible** — the name exists at runtime
- **`inline`** — safe to define in headers included multiple times

`#define` does blind text substitution before the compiler runs.
No type, no scope, no safety. Never use it for constants.

## Key bugs encountered

### `||` vs `&&` in visibility logic

```cpp
// ❌ wrong — returns dark if EITHER condition fails
if (!visible || !remembered) return " ";

// ✅ correct — returns dark only if BOTH conditions fail
if (!visible && !remembered) return " ";
```

### Brace initialization for pairs

```cpp
visited.insert(r, c);    // ❌ two arguments — wrong overload
visited.insert({r, c});  // ✅ one pair — correct
```

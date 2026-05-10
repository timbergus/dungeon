# Chapter 03 — BSP Dungeon Generation

## What we built
A procedural dungeon generator using Binary Space Partitioning (BSP),
producing unique room-and-corridor layouts on every run.

---

## The algorithm

Binary Space Partitioning recursively divides a rectangle into two halves,
then places a room inside each indivisible region and connects siblings
with corridors.

```
Step 1 — whole grid:
┌─────────────────┐

Step 2 — split vertically:
┌────────┬────────┐

Step 3 — split each half:
┌────────┬────────┐
│        │        │
├────────┤        │
│        │        │
└────────┴────────┘

Step 4 — place rooms in leaves:
┌────────┬────────┐
│  ┌──┐  │  ┌─┐  │
│  └──┘  │  └─┘  │
└────────┴────────┘

Step 5 — connect siblings:
┌────────┬────────┐
│  ┌──┐  │  ┌─┐  │
│  └──┘──┴──└─┘  │
└────────┴────────┘
```

---

## Data structures

### `Rect` — a rectangular region
```cpp
struct Rect {
    std::size_t row, col, height, width;

    std::size_t center_row() const { return row + height / 2; }
    std::size_t center_col() const { return col + width  / 2; }
};
```

### `BSPNode` — a node in the binary tree
```cpp
struct BSPNode {
    Rect region;
    std::unique_ptr<BSPNode> left  = nullptr;
    std::unique_ptr<BSPNode> right = nullptr;

    bool is_leaf() const { return left == nullptr && right == nullptr; }
};
```

`is_leaf()` is derived from existing state rather than stored as a separate
`bool` — making inconsistency impossible. This is the **single source of
truth** principle.

---

## `std::unique_ptr` — smart ownership

`std::unique_ptr<T>` owns the memory it points to and **automatically frees
it** when destroyed. The tree cleans itself up recursively at zero cost.

```cpp
// ❌ manual — easy to leak or double-free
BSPNode* left = new BSPNode{...};
delete left;

// ✅ modern — ownership is automatic
std::unique_ptr<BSPNode> left = std::make_unique<BSPNode>(...);
// freed automatically when the parent BSPNode is destroyed
```

> **Rule:** never write `new` or `delete` by hand.
> Use `std::unique_ptr` for single ownership,
> `std::shared_ptr` for shared ownership.

Default member initialisers on `left` and `right` mean every `BSPNode`
construction site gets `nullptr` automatically — no need to spell it out:

```cpp
BSPNode root{.region = {0, 0, grid.rows, grid.cols}};
// left and right are nullptr — set by default member initialiser
```

---

## Random number generation

```cpp
std::mt19937 rng{std::random_device{}()};
std::uniform_int_distribution<std::size_t> dist{min, max};
std::size_t value = dist(rng);
```

The engine (`std::mt19937`) and the distribution are **separate responsibilities**:

- **Engine** — produces raw random bits. `mt19937` is fast and high-quality.
- **Distribution** — shapes those bits into a useful range with guaranteed
  statistical uniformity.

The naive `rand() % range` approach is subtly biased when `RAND_MAX` isn't
perfectly divisible by the range. `std::uniform_int_distribution` avoids this.

We pass `rng` by reference so the whole tree shares one generator and its
state advances consistently across all splits.

---

## The splitting logic

```cpp
void split(BSPNode& node, std::mt19937& rng,
           std::size_t min_size, int depth) {
    // Base cases — stop recursing
    if (depth == 0
     || node.region.height < min_size * 2
     || node.region.width  < min_size * 2) {
        return;
    }

    bool split_horizontally = node.region.width > node.region.height;
    // ... pick random split point within safe bounds ...
    // ... create left and right children with make_unique ...
    // ... recurse into both children ...
}
```

Splitting wider regions horizontally and taller regions vertically produces
naturally varied dungeon layouts.

---

## Caller owns the tree — good ownership design

The BSP root is created by the caller and passed by reference to functions
that operate on it. No function owns the tree — they just use it:

```cpp
// main.cpp
auto root = make_tree(grid, rng);   // caller owns root
generate(root, grid, rng);          // operates on it
Rect start = first_room(root);      // reads from it
```

This is cleaner than having `generate` own the tree internally and throw it
away — the caller can keep querying the tree after generation.

---

## `std::ranges` — pipelines over collections (C++20)

Ranges let you express transformations over collections as **lazy pipelines**
with no intermediate allocations:

```cpp
// old way — allocates an intermediate vector
std::vector<Region> filtered;
for (auto& r : regions)
    if (r.is_leaf()) filtered.push_back(r);

// new way — lazy, no intermediate allocation
auto leaves = regions | std::views::filter(&Region::is_leaf);
```

The `|` pipe operator chains views together. Nothing is computed until you
actually iterate — evaluation is **on demand, one element at a time**.

---

## Key functions

```cpp
// Build and split the BSP tree
BSPNode make_tree(Grid& grid, std::mt19937& rng);

// Fill grid with walls, carve rooms and corridors into it
void generate(BSPNode& root, Grid& grid, std::mt19937& rng);

// Find the first leaf room (used for player starting position)
Rect first_room(BSPNode& node);
```

# Chapter 08a — New Tile Types

## What we added

Four new tile types extending the existing variant system:

- `Stairs` — directional, connects dungeon levels
- `Chest` — openable, contains loot, can be a mimic
- `StairsDirection` — enum class driving stair behaviour

## Key decisions

### One `Stairs` struct instead of two

A staircase physically connects two levels — it exists on both floors
at the same position. One struct with a direction property models this
cleanly and avoids duplicated logic.

### `Chest` with `is_mimic`

A chest that attacks instead of giving loot. The player never knows
until they interact — pure information asymmetry creating tension.

### Single source of truth — `base_glyph`

Adding two new tile types required updating exactly one place.
Every `std::visit` elsewhere got the new types for free.

## Placing landmarks — `place_landmarks`

### Raw pointers for non-owning observation

```cpp
std::vector leaves;  // observes, does not own
```

`unique_ptr` owns memory. Raw pointers just look at it.
Non-owning observation is a valid and safe use of raw pointers.

### Defensive programming

Guard against infinite loops when all rooms are exhausted:

```cpp
if (used_rooms.size() >= leaves.size()) break;
```

The compiler can't catch logical exhaustion — we guard manually.

### `std::unordered_set` for used rooms

Same pattern as fog of war visited tiles — fast O(1) insertion
and lookup, no duplicates by design.

## Glyph conventions

```text
< — StairsUp     (roguelike standard)
> — StairsDown   (roguelike standard)
= — Chest closed (looks like a chest from above)
~ — Chest open   (contents spilling out)
```

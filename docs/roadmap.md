# Project Roadmap & Open Developments

## Current state — what's complete

```text
✅ Toolchain & CMake          Homebrew Clang 22, C++26, Makefile
✅ Tile system                std::variant, std::visit, overloaded
✅ World grid                 std::mdspan, flat memory layout
✅ BSP dungeon generation     recursion, std::unique_ptr, std::mt19937
✅ Fog of war + raycasting    std::unordered_set, Bresenham, Euclidean
✅ Double buffered rendering  single write syscall, cursor home
✅ Terminal RAII              termios, = delete, full_clear
✅ Player movement            std::expected, std::ptrdiff_t
✅ Debug fog toggle           FogMode enum class, constexpr flag
✅ New tile types             Stairs, Chest (mimic, found_open, locked)
✅ Color palette              ZX Spectrum ANSI colors
✅ Interaction system         context-sensitive dialogs, press 'e'
✅ ASCII art library          raw string literals, extern declarations
✅ Death screen               Pratchett-inspired, cause-of-death flavour
✅ Victory screen             Death with coffee, score summary
✅ Being base struct          inheritance, damage model, stamina/weight
✅ Enemy struct               EnemyKind variant, factory functions
✅ Stats tracking             levels, mimics, chests counters
```

---

## Partially implemented — open placeholders

### Combat system

```text
✅ Being/Enemy/Player structure
✅ take_damage, stamina, resistance, weight model
✅ can_double_attack, exhaust, recover
❌ Combat loop — MimicFight is a placeholder in main.cpp
❌ Enemy AI turn — attack calculation not wired up
❌ Combat screen — no UI for fight sequence
❌ mimics_defeated counter not incremented
```

### Chest interactions

```text
✅ Dialog system (open, locked, found_open, mimic states)
✅ Mimic detection (MimicAte instant death)
❌ Loot not spawned when chest opened
❌ Key system not implemented
❌ Locked chest key consumption not wired up
```

### Door interactions

```text
✅ Dialog system
✅ Locked door blocks movement
❌ Key consumption not implemented
❌ Force door mechanic not implemented
❌ Door art placeholder (uses STAIRS_DOWN art)
```

### Stairs

```text
✅ Dialog system (ascend/descend options)
✅ StairsUp/StairsDown placed in dungeon
❌ Level transition not implemented
❌ levels_descended counter not incremented
❌ New level not generated on descent
```

---

## Designed but not started

### Inventory system (Chapter 08d)

```text
Item types:
  - Torch    { int radius_bonus }     — modifies visibility radius
  - Dagger   { int damage }
  - Sword    { int damage }           — one-handed
  - Greatsword { int damage }         — two-handed
  - Shield   { int defense }
  - Potion   { int heal_amount }
  - Key      { }                      — opens locked doors and chests
  - Coins    { int amount }           — economy currency
 
using Item = std::variant<Torch, Dagger, Sword, Greatsword,
                           Shield, Potion, Key, Coins>;
 
Hand system:
  std::optional<Item> left_hand
  std::optional<Item> right_hand
  Two-handed items occupy both slots
 
Inventory (backpack):
  std::vector<Item> items
  int current_weight()
  bool can_add(const Item& item)
  Weight limit affects carry_weight in Being
 
ItemTile on the floor:
  struct ItemTile { Item item; }
  Added to Tile variant in Chapter 08d
  Becomes Floor when picked up
```

### Level system (Chapter 08e)

```text
struct Level {
    Grid        grid;
    int         depth;
    int         enemy_count;
    int         chest_count;
    int         item_count;
};
 
Level generation:
  - BSP generates grid
  - Scatter enemies by difficulty tier
  - Scatter chests (mimic chance scales with depth)
  - Scatter items (quality scales with depth)
 
Multi-level dungeon:
  std::vector<Level> dungeon_stack;
  Descend → push new Level
  Ascend  → pop current Level
  Level 1 → items only, no enemies (tutorial floor)
```

### Economy (Chapter 08f)

```text
Coins dropped by enemies (random amount)
Repair stations on certain floors
Item shops on certain floors
Key shops — buy keys with coins
 
Decision: spend coins on repairs or save for keys?
```

### Enemies on the map (Chapter 07)

```text
std::vector<Enemy> enemies per level
Rendered as glyphs (g=goblin, m=mage, M=mimic walking)
Visibility-aware — hidden outside player radius
BFS pathfinding — enemies chase player when nearby
Turn-based movement — enemies move after player acts
std::ranges pipelines for filtering nearby enemies
```

---

## Suggested build order

```text
Step 1 — Combat loop
  Wire up MimicFight in main.cpp
  Turn-based: player attacks → enemy attacks → repeat
  Combat screen showing HP bars and turn log
  Increment mimics_defeated on win
        ↓
Step 2 — Inventory + items
  Item types as variant
  Hand slots (std::optional<Item>)
  Backpack with weight
  ItemTile on floor — pick up with 'e'
  Torch modifies VISIBILITY_RADIUS dynamically
        ↓
Step 3 — Key system
  Key item in inventory
  Locked door/chest consumes one key
  Key count shown in HUD
        ↓
Step 4 — Level transitions
  Level struct wrapping Grid
  New BSP dungeon on descent
  levels_descended incremented
  Difficulty scales with depth
        ↓
Step 5 — Enemies on the map
  Enemy positions tracked per level
  BFS pathfinding
  Turn-based movement
  Visibility-aware rendering
        ↓
Step 6 — Economy
  Coin drops
  Repair/shop stations
  Full resource management loop
```

---

## Future ideas (captured for later)

```text
Fog of war improvements
  — Torch radius as dynamic VISIBILITY_RADIUS
  — Different lighting per tile type (torches on walls)
 
Double buffering polish
  — Wide character support for emoji
  — Reserve accounting for ANSI code bytes
 
Modules migration (Chapter 10)
  — Migrate tile system to C++20 modules
  — Compare build times and encapsulation
 
Save & load (Chapter 11)
  — Serialize dungeon seed, player state, visited tiles
  — std::fstream, std::format for file output
 
Equipment degradation
  — Items lose durability in combat
  — Broken items must be repaired or dropped
  — Adds weight to economy decisions
 
Pratchett easter eggs
  — Luggage (chest with legs) as rare enemy
  — Death appears randomly in deep levels
  — "YOU SEEM SURPRISED TO SEE ME" dialogue
```

---

## Core design philosophy

> The dungeon always wins eventually.
> Winning means NOT YET — not SAFE FOREVER.
> Death is not a punishment. He's a constant.
> He will be waiting in the lobby.
> He always is.

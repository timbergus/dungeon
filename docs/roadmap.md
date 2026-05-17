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
✅ New tile types             Stairs, Chest, Mimic (from enemy.hpp)
✅ Color system               ZX Spectrum palette, two-layer design
✅ Interaction system         context-sensitive dialogs, press 'e'
✅ ASCII art library          raw string literals, extern declarations
✅ Death screen               Pratchett-inspired, cause-of-death flavour
✅ Victory screen             Death with coffee, score summary
✅ Being base struct          inheritance, damage model, stamina/weight
✅ Enemy struct               EnemyKind variant, factory functions
✅ Stats tracking             levels, mimics, chests counters
✅ Dice system                D4/D6/D8/D10/D12/D20, roll(sides, count)
✅ Combat system              turn-based, initiative, parry, double attack
✅ Mimic separation           Mimic is an enemy tile, not a Chest property
✅ Broken chest               attacking innocent chests has consequences
✅ Context-aware dialogs      broken chest messages vary by chest state
```

---

## Partially implemented — open placeholders

### Combat system

```text
✅ Being/Enemy/Player structure
✅ take_damage, stamina, resistance, weight model
✅ can_double_attack, exhaust, recover
✅ Interactive dice rolls — press R to roll
✅ Parry mechanic — monster-specific, off-balance penalty
✅ Combat screen with HP bars and combat log
✅ MimicFight wired up in main.cpp
✅ mimics_defeated counter incremented
❌ Loot dropped after combat — not yet implemented
❌ Goblin cowardly flee mechanic — flees below 20% health
❌ Mage fireball mechanic — uses mana for ranged attack
```

### Chest interactions

```text
✅ Dialog system (open, locked, found_open, broken states)
✅ Mimic now a separate tile type
✅ Broken chest dialog — context-aware insults
✅ Attacking open chests destroys them
❌ Loot not yet spawned when chest opened
❌ Key system not implemented
❌ Locked chest key consumption not wired up
```

### Door interactions

```text
✅ Dialog system
✅ Locked door blocks movement
❌ Key consumption not implemented
❌ Force door mechanic not implemented
❌ Door art (currently uses STAIRS_DOWN as placeholder)
```

### Stairs

```text
✅ Dialog system (ascend/descend options)
✅ StairsUp/StairsDown placed in dungeon
❌ Level transition not implemented
❌ levels_descended counter not incremented
❌ New level not generated on descent
❌ Surface death not implemented (see below)
```

---

## Designed but not started

### Surface death mechanic ⭐

When the player climbs StairsUp on level 1 (depth = 0),
they reach the surface — which is worse than the dungeon.

```text
Narrative:
"You climb the stairs toward the surface.
 Fresh air. Sunlight. Freedom.
 
 Also Orcs. Lots of Orcs.
 Now you remember why you came down here.
 
 Well. Best of luck next time."
 
Implementation:
  - Check current depth in stair handler
  - depth == 0 + StairsUp → surface_dialog()
  - Ends game with InteractionResult::SurfaceDeath
  - Death screen quote:
    "THE ORCS SEND THEIR REGARDS.
     I DID TRY TO WARN YOU.
     WELL. NO. I DIDN'T.
     BUT I THOUGHT ABOUT IT.
     'SIGHT'.
     PLEASE, FOLLOW ME TO THE LOBBY."
 
Game philosophy:
  The dungeon isn't a trap — it's a refuge.
  The surface is the real danger.
  At least the dungeon has chests.
  Death is waiting below, Orcs are waiting above.
  The dungeon is all you have.
```

### Inventory system (Chapter 08e)

```text
Item types:
  using Item = std::variant<
      Torch,       { int radius_bonus }
      Dagger,      { int damage }
      Sword,       { int damage }          one-handed
      Greatsword,  { int damage }          two-handed
      Shield,      { int defense }
      Potion,      { int heal_amount }
      Key,         {}
      Coins,       { int amount }
  >
 
Hand system:
  std::optional<Item> left_hand
  std::optional<Item> right_hand
  Two-handed items occupy both slots
  Torch in one hand → can carry shield in other
  Two-handed weapon → both hands full → no torch → darkness
 
Weight system (already in Being):
  carry_weight    updated when items picked up or dropped
  max_carry_weight varies by player build
  Overencumbered → reduced effective stamina → weaker in combat
 
Backpack:
  std::vector<Item> items
  int  current_weight() const
  bool can_add(const Item& item) const
 
ItemTile on the floor:
  struct ItemTile { Item item; };
  Added to Tile variant
  Becomes Floor when picked up
  Spawned by:
    - Chest opening (loot)
    - Mimic defeat (remains)
    - Enemy death (drops)
 
Torch mechanic:
  VISIBILITY_RADIUS becomes a runtime method on Player:
  int visibility_radius() const {
      int bonus = 0;
      if (holds torch in either hand) bonus = torch.radius_bonus;
      return BASE_RADIUS + bonus;
  }
```

### Level system (Chapter 08f)

```text
struct Level {
    Grid    grid;
    BSPNode bsp_root;
    int     depth;
    int     enemy_count;
    int     chest_count;
    int     mimic_count;
};
 
Multi-level dungeon:
  std::vector<Level> dungeon_stack
  Descend → generate new Level, push to stack
  Ascend  → pop current Level (or surface death if depth == 0)
 
Level 1 (depth 0) — The Stash:
  Items only, no enemies
  Tutorial floor — learn mechanics safely
  StairsUp leads to surface (Orcs) not safety
 
Difficulty scaling per depth:
  enemy_count  = 2 + depth
  chest_count  = 3 + depth / 2
  mimic_chance = 0.10 + depth * 0.05  (max 0.40)
  item_quality scales with depth
  enemy health and attack scale with depth
 
Surface death:
  Triggered when ascending from depth 0
  InteractionResult::SurfaceDeath → death screen
  "The dungeon is all you have"
```

### Economy (Chapter 08g)

```text
Coins dropped by enemies (random, scales with depth)
Repair stations on certain floors
Item shops on certain floors
 
Decision matrix:
  Spend coins on repairs → better combat survivability
  Save coins for keys    → access to locked chests and doors
  Save coins for shops   → better equipment deeper down
 
Equipment degradation:
  Items lose durability in combat
  Broken items: halved stats, must repair or drop
  Adds weight to economy — carry broken gear or drop it?
```

### Enemies on the map (Chapter 07)

```text
std::vector<Enemy> enemies per level
Rendered as glyphs:
  g = Goblin (green)
  m = Mage   (magenta)
  M = Mimic walking (red — only visible when adjacent)
 
Visibility-aware:
  Enemies outside player radius are hidden
  "Lurking in the dark" tension
 
BFS pathfinding:
  std::queue<std::pair<row,col>> for frontier
  Enemies chase player when within detection range
  Detection range < visibility radius
  (you see them before they see you — usually)
 
Turn-based movement:
  Enemies move after every player action
  Goblin flees below 20% health
  Mage keeps distance, uses fireball when in range
 
std::ranges pipelines:
  auto nearby = enemies
      | std::views::filter([&](const Enemy& e) {
            return distance(e, player) < DETECTION_RANGE;
        });
```

---

## Suggested build order

```text
Step 1 — Inventory + items          ← next
  Item variant, hand slots, backpack
  ItemTile on floor
  Torch modifies visibility radius
  Loot spawns from chests and mimic remains
        ↓
Step 2 — Key system
  Key item in inventory
  Locked door and chest consume one key
  Key count shown in HUD
        ↓
Step 3 — Level system + surface death
  Level struct wrapping Grid
  New BSP dungeon on descent
  Surface death on ascending from depth 0
  Difficulty scaling per depth
        ↓
Step 4 — Enemies on the map
  Enemy positions per level
  BFS pathfinding
  Turn-based movement
  Goblin flee, Mage fireball
        ↓
Step 5 — Economy
  Coin drops
  Repair and shop stations
  Full resource management loop
        ↓
Step 6 — Polish
  Door art
  Goblin and Mage combat mechanics
  Equipment degradation
  Pratchett easter eggs
```

---

## Future ideas (captured for later)

```text
Modules migration (Chapter 10)
  — Migrate tile system to C++20 modules
  — Compare build times and encapsulation
 
Save & load (Chapter 11)
  — Serialize dungeon seed, player state, visited tiles
  — std::fstream, std::format for file output
  — Only seed + player state needed (world is deterministic)
 
Pratchett easter eggs
  — Luggage (chest with legs) as rare enemy
  — Death appears randomly in deep levels
  — "YOU SEEM SURPRISED TO SEE ME" dialogue
  — Death plays chess between levels (very classic)
 
Fog of war improvements
  — Torch flickers (random radius variance per frame)
  — Wall torches cast light independently
  — Darkness deepens with each level
 
Wide character rendering
  — Account for emoji being 2 cells wide
  — Emoji items and enemies on the map
 
ZX Spectrum loading screen
  — Coloured stripes animation on game start
  — Pure nostalgia, zero gameplay value, 100% necessary
```

---

## Core design philosophy

```text
The dungeon isn't a trap — it's a refuge.
The surface is worse. Much worse.
(Orcs. Lots of Orcs.)
 
Death is not a punishment. He's a constant.
Winning means NOT YET — not SAFE FOREVER.
Every run ends at the lobby.
He always has coffee ready.
 
"SIGHT."
```

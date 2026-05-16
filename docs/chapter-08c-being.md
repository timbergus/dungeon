# Chapter 08c — Being Inheritance & Combat Foundation

## What we built

A shared `Being` base struct inherited by both `Player` and `Enemy`,
with a physics-inspired damage model incorporating resistance, stamina,
weight, and double attack chances.

---

## Inheritance in C++

When two types share data and behavior, extract the common parts
into a **base struct** and inherit from it:

```cpp
struct Being {
    std::string name;
    int         health;
    // ... shared fields and methods
};
 
struct Player : Being {
    std::size_t row;   // Player-specific
    std::size_t col;
};
 
struct Enemy : Being {
    EnemyKind   kind;  // Enemy-specific
    std::string art;
};
```

`Player : Being` means Player **inherits** everything from Being —
all fields and methods — and adds its own on top.

The subtype constructor calls the base constructor explicitly
in the initializer list:

```cpp
Player(std::string name, std::size_t row, std::size_t col)
    : Being{std::move(name), 100, 8, 5, 100, 80}
    , row{row}
    , col{col}
{}
```

---

## The `Being` struct

```cpp
struct Being {
    std::string name;
    int         health;
    int         max_health;
    int         resistance;    // innate damage mitigation
    int         stamina;
    int         max_stamina;
    int         attack;
    int         shield;        // 0 at creation, added by items
    int         carry_weight;
    int         max_carry_weight;
};
```

### Default parameters

```cpp
Being(std::string name,
      int         max_health,
      int         resistance       = 5,
      int         attack           = 3,
      int         max_stamina      = 100,
      int         max_carry_weight = 100)
```

Default parameters must always trail from right to left.
Once a parameter has a default, every parameter after it must too:

```cpp
// ✅ valid — defaults trail right
Being(std::string name, int max_health, int resistance = 5);
 
// ❌ invalid — non-default after default
Being(std::string name, int resistance = 5, int max_health);
```

---

## The damage model

### Weight affects stamina capacity

```text
stamina_capacity = max_stamina × (1 - carry_weight / max_carry_weight)
```

A heavily encumbered being starts fights already partially exhausted.

### Stamina affects effective resistance (percentage model)

```text
effective_resistance = resistance × (stamina / stamina_capacity)
```

Percentage scaling chosen over flat reduction because it:

- Scales cleanly regardless of item bonuses
- Never produces negative resistance
- Gives a smooth curve rather than a cliff edge

### Full damage pipeline

```text
1. effective_resistance = resistance × (stamina / capacity)
2. after_resist  = max(0, incoming - effective_resistance)
3. absorbed      = min(after_resist, shield)
4. shield       -= absorbed
5. health       -= (after_resist - absorbed)
```

### Double attack chance

```text
chance = stamina / stamina_capacity
```

High stamina → likely double attack.
Low stamina → single attack, barely swinging.

---

## Key methods on `Being`

```cpp
bool   is_alive() const;
double stamina_capacity() const;
double effective_resistance() const;
void   take_damage(int incoming);
void   exhaust(int cost = 10);       // stamina cost per attack
bool   can_double_attack(std::mt19937& rng) const;
void   recover(int amount = 5);      // stamina recovery per turn
```

---

## `std::uniform_real_distribution<double>`

The floating-point cousin of `std::uniform_int_distribution`.
Generates a `double` in [0.0, 1.0] for probability checks:

```cpp
std::uniform_real_distribution<double> roll{0.0, 1.0};
double chance = static_cast<double>(stamina) / capacity;
return roll(rng) < chance;  // true with probability = chance
```

---

## Enemy types — variant for unique data

Shared data lives in `Being`. Only unique per-type data lives in the variant:

```cpp
struct Mimic  { std::string taunt; };
struct Goblin { bool is_cowardly; };
struct Mage   { int mana; int fireball_dmg; };
 
using EnemyKind = std::variant<Mimic, Goblin, Mage>;
 
struct Enemy : Being {
    EnemyKind   kind;
    std::string description;
    std::string art;
};
```

Adding a new enemy type (Dragon, Troll, Necromancer) means:

- Adding a new struct with its unique fields
- Adding it to `EnemyKind`
- Every unhandled `std::visit` refuses to compile — exhaustiveness for free

### Factory functions

```cpp
Enemy make_mimic();   // 40hp, 8atk — hits hard, low resistance
Enemy make_goblin();  // 25hp, 4atk — quick, cowardly below 20% health
Enemy make_mage();    // 35hp, 6atk — balanced, has fireball
```

---

## Stats tracking

Three counters in `main.cpp` feed the victory screen:

```cpp
int levels_descended = 0;  // incremented on stair descent
int mimics_defeated  = 0;  // incremented on MimicFight win
int chests_looted    = 0;  // incremented on ChestLooted
```

Displayed by Death on the victory screen with `std::format`:

```cpp
std::format("  Levels descended : {:>3}\n", levels_descended)
```

---

## Open placeholders (next steps)

```text
MimicFight     → combat loop not yet implemented
ChestLooted    → loot not yet spawned
levels_descended → stairs not yet transitioning levels
mimics_defeated  → combat win not yet detected
```

## File structure added

```text
include/entities/
├── being.hpp    — Being base struct
└── enemy.hpp    — Enemy : Being, EnemyKind variant, factory declarations
 
src/entities/
└── enemy.cpp    — make_mimic, make_goblin, make_mage implementations
```

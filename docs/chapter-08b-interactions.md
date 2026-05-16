# Chapter 08b — Interactions, Dialogs & Death Screen

## What we built

A context-sensitive interaction system triggered by pressing `e` on
special tiles, with a full dialog renderer, consequence resolution,
a Pratchett-inspired death screen, and a victory screen.

---

## New C++ concepts

### `std::get_if<T>` — safe variant access

Returns a pointer to the held value if the type matches, nullptr otherwise.
Perfect for "check type AND access data" in one step:

```cpp
if (auto* d = std::get_if<Door>(&target))
    if (!d->is_open)
        return std::unexpected(MoveError::HitWall);
```

### if-with-initializer (C++17)

Declare a variable scoped to the if block:

```cpp
if (auto* d = std::get_if<Door>(&target)) {
    // d only exists here — no pollution of surrounding scope
}
```

### Generic lambda — `[](const auto&) {}`

A lambda with an `auto` parameter — a template under the hood.
Catches any type not handled by earlier overloads in a `std::visit`:

```cpp
std::visit(overloaded{
    [&](Chest& c)      { /* handle chest */ },
    [&](Stairs& s)     { /* handle stairs */ },
    [](const auto&) {} // everything else — do nothing
}, tile);
```

### `std::nullopt` — explicit empty optional

```cpp
return std::nullopt;  // deliberately returning nothing
```

Clearer than `return {}` — the intent is obvious.

### Dereferencing `std::optional`

```cpp
if (!result) return;    // guard against empty
auto choice = *result;  // safe to dereference
```

Always guard before dereferencing — like a pointer.

### `extern` — declaration without definition

```cpp
// art.hpp — declaration
extern const std::string CHEST_CLOSED;
 
// art.cpp — definition
const std::string Art::CHEST_CLOSED = R"(...)";
```

One definition, many declarations. The linker connects them.
Prevents duplicate symbol errors when headers are included multiple times.

### Raw string literals — `R"(...)"`

Inside a raw string, backslashes and special characters are literal.
Perfect for multiline ASCII art:

```cpp
const std::string Art::CHEST_CLOSED = R"(
  ╔══════════╗
 ▓║▓▓▓▓▓▓▓▓▓║
▓▓║  ══════ ║▓
  ╚══════════╝)";
```

### `std::string(count, char)` — fill constructor

```cpp
std::string(40, '~')  // 40 tilde characters
```

Same constructor as the grid fill from Chapter 02 — consistent pattern.

### `std::to_string` vs `std::format`

```cpp
// Simple — no formatting control
std::to_string(42)              // "42"
 
// Powerful — full control
std::format("{:>3}", 42)        // " 42" — right-aligned in 3 chars
std::format("{:0>5}", 42)       // "00042" — zero-padded
std::format("{:.2f}", 3.14159)  // "3.14" — 2 decimal places
```

Use `std::to_string` for simple integer conversion.
Use `std::format` when alignment or precision matters.

---

## Design decisions

### Context-sensitive action menus

Different tiles produce different dialogs with different options.
The consequence (`InteractionResult`) is separated from the display
(`Dialog`) — one function decides what to show, another resolves
what it means.

### Open mimic — no escape

An open chest mimic cannot be escaped — only fought or inspected
(fatally). This creates genuine tension with no safe exit.

```text
Found open chest → inspect → CHOMP → dead (MimicAte)
                → attack  → MimicFight (initiative advantage)
                → leave   → safe (if it hasn't spotted you)
```

### `found_open` property on `Chest`

Distinguishes between:

- Chest the player opened themselves → already looted, walk away
- Chest found open by someone else → suspicious, possible mimic

### Death screen as a distinct experience

- `full_clear()` — blank canvas, not dungeon underneath
- Cyan art, dim white text — visually distinct from every screen
- No numbered options — Death doesn't offer choices
- Cause-of-death flavour text — different insults for different mistakes

### Victory screen — Death with coffee

Same character, opposite context, same message underneath:
you can't escape Death, only postpone the meeting.
Winning means "NOT YET" — not "SAFE FOREVER".

---

## The consequence chain

```text
Chest interaction
├── Already looted by player → walk away
├── Found open
│   ├── Inspect → mimic? → MimicAte → death screen
│   │           → normal? → ChestLooted
│   ├── Attack  → MimicFight → combat
│   └── Leave   → None
├── Locked
│   ├── Use key → open (TODO: key system)
│   ├── Attack  → MimicFight
│   └── Leave   → None
└── Closed
    ├── Open    → mimic? → MimicFight
    │           → normal? → ChestLooted
    ├── Attack  → MimicFight
    └── Leave   → None
```

---

## File structure added

```text
include/ui/
├── art.hpp          — ASCII art library (extern declarations)
├── dialog.hpp       — Dialog, DialogOption, DialogResult
├── death_screen.hpp — show_death_screen, show_victory_screen
└── interactions.hpp — InteractionResult, dialog factories, resolve_chest
 
src/ui/
├── art.cpp          — ASCII art definitions (raw string literals)
├── dialog.cpp       — Dialog renderer
├── death_screen.cpp — Death and victory screens
└── interactions.cpp — Consequence resolution logic
```

---

## Terry Pratchett's Death

DEATH speaks in small caps. He is not unkind, merely matter-of-fact.
He says "SIGHT" instead of "SIGH" because he has never quite understood
the living. He drinks coffee at victory screens because even Death
needs a moment to collect himself. He will wait for you in the lobby.
He always does.

# Chapter 08b — Interactions & Dialogs

## What we built

A context-sensitive interaction system triggered by pressing `e` on
special tiles, with a full dialog renderer, consequence resolution,
and a Pratchett-inspired death screen.

## New C++ concepts

### `std::get_if<T>` — safe variant access

Returns a pointer to the held value if the type matches, nullptr otherwise.
Perfect for "check type AND access data" in one step:

```cpp
if (auto* d = std::get_if(&target))
    if (!d->is_open)
        return std::unexpected(MoveError::HitWall);
```

### if-with-initializer (C++17)

Declare a variable scoped to the if block:

```cpp
if (auto* d = std::get_if(&target)) {
    // d only exists here
}
// d is gone — no pollution of surrounding scope
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
if (!result) return;   // guard against empty
auto choice = *result; // safe to dereference
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
  ╚══════════╝
)";
```

### `std::string(count, char)` — fill constructor

```cpp
std::string(40, '~')  // 40 tilde characters
```

Same constructor as the grid fill from Chapter 02 — consistent pattern.

## Design decisions

### Context-sensitive action menus

Different tiles produce different dialogs with different options.
The consequence (`InteractionResult`) is separated from the display
(`Dialog`) — one function decides what to show, another resolves
what it means.

### Open mimic — no escape

An open chest mimic cannot be escaped — only fought or inspected
(fatally). This creates genuine tension: the right choice is never
obvious, and the wrong one is permanent.

### Death screen as a distinct experience

- `full_clear()` instead of `clear()` — blank canvas, not dungeon
- Cyan art, dim white text — visually distinct from every other screen
- No numbered options — Death doesn't offer choices
- Cause-of-death flavour text — different insults for different mistakes

### `found_open` property on `Chest`

Distinguishes between a chest the player opened themselves
(already looted, walk away) and one found open by someone else
(suspicious, possibly a mimic waiting for a second victim).

## The consequence chain

```text
Chest interaction
├── Already looted by player → walk away
├── Found open
│   ├── Inspect → mimic? → MimicAte → death screen
│   │           → normal? → ChestLooted
│   ├── Attack  → MimicFight → TODO: combat
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

## Terry Pratchett's Death

DEATH speaks in small caps. He is not unkind, merely matter-of-fact.
He says "SIGHT" instead of "SIGH" because he has never quite understood
the living. He will wait for you in the lobby.

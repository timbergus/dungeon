# Chapter 04 — Player, RAII & Movement

## What we built
A player entity with position, health and shield, raw terminal input via
RAII-managed terminal settings, and validated movement using `std::expected`.

---

## The Player struct

```cpp
// include/entities/player.hpp
#pragma once

#include <string>
#include <cstddef>

struct Player {
    std::string name;
    std::size_t row;
    std::size_t col;
    int         health;
    int         shield;

    Player(std::string name, std::size_t start_row, std::size_t start_col)
        : name{std::move(name)}
        , row{start_row}
        , col{start_col}
        , health{100}
        , shield{30}
    {}
};
```

### Type choices

- `std::size_t` for `row` and `col` — matches the grid's index type,
  prevents signed/unsigned comparison warnings
- `int` for `health` and `shield` — these can meaningfully cross zero
  (shield breaking, overkill damage), so signed is correct
- `std::string` for `name` — owns its text, no lifetime concerns

### `std::move` in constructors

```cpp
: name{std::move(name)}
```

Without `move`: the string is **copied** — new allocation, every character
copied byte by byte.

With `move`: ownership of the string's internal buffer is **transferred**
from the parameter into the member — zero copying, zero extra allocation.

> Rule: when a constructor takes a parameter **by value** and stores it as
> a member, always `std::move` it into the member. The caller already paid
> for the copy when passing the argument.

---

## RAII — Resource Acquisition Is Initialisation

RAII ties a resource's lifetime to an object's lifetime:

- **Constructor** → acquire the resource
- **Destructor** → release the resource, automatically, always

The terminal is a resource. Changing it to raw mode and forgetting to restore
it leaves the terminal broken after the game exits. With RAII, restoration is
**guaranteed** — even if an exception is thrown or the program exits early.

```cpp
// include/terminal.hpp
#pragma once

#include <termios.h>
#include <unistd.h>
#include <print>

struct Terminal {
    Terminal() {
        tcgetattr(STDIN_FILENO, &original);
        termios raw = original;
        raw.c_lflag &= ~static_cast<tcflag_t>(ECHO | ICANON);
        raw.c_cc[VMIN]  = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    ~Terminal() {
        // Called automatically when Terminal goes out of scope
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
    }

    // Prevent copying — a terminal handle must not be duplicated
    Terminal(const Terminal&)            = delete;
    Terminal& operator=(const Terminal&) = delete;

    static char read_key() {
        char c{};
        ::read(STDIN_FILENO, &c, 1);
        return c;
    }

    static void clear() {
        // ANSI escape: clear screen + move cursor home
        std::print("\033[2J\033[H");
    }

private:
    termios original{};
};
```

### The cast on `c_lflag`
`ECHO` and `ICANON` are `int` constants but `c_lflag` is `tcflag_t`
(unsigned long on macOS). The explicit cast makes the conversion intentional
and silences `-Wconversion`:

```cpp
raw.c_lflag &= ~static_cast<tcflag_t>(ECHO | ICANON);
```

---

## `= delete` — explicitly forbidden operations

`= delete` makes calling a function a **compile error**. It is not a default
value — it is a prohibition.

```cpp
Terminal(const Terminal&)            = delete;  // copy construction forbidden
Terminal& operator=(const Terminal&) = delete;  // copy assignment forbidden
```

```cpp
Terminal t1{};
Terminal t2 = t1;   // ❌ compile error: use of deleted function
t1 = t2;            // ❌ compile error: use of deleted function
```

`= delete` vs `= default`:
- `= default` — *"generate the standard implementation"*
- `= delete`  — *"this operation does not exist"*

`std::unique_ptr` uses the same pattern for the same reason: sole ownership
must not be accidentally duplicated.

---

## `std::expected` for movement validation

`std::expected<T, E>` returns either a value (`T`) or a typed error (`E`).
No exceptions, no output parameters, no magic error codes.

```cpp
enum class MoveError { HitWall, OutOfBounds };

std::expected<void, MoveError> try_move(
    Player& player, Grid& grid, int d_row, int d_col
);
```

### Signed arithmetic with `std::ptrdiff_t`

Movement deltas (`d_row`, `d_col`) can be negative. Subtracting from
`std::size_t` without care wraps around catastrophically:

```cpp
std::size_t row = 0;
row + (-1);  // wraps to 18446744073709551615 — undefined territory
```

Solution: convert to `std::ptrdiff_t` (the signed counterpart of `size_t`)
for arithmetic, check bounds, then convert back:

```cpp
auto new_row = static_cast<std::ptrdiff_t>(player.row) + d_row;
if (new_row < 0 || static_cast<std::size_t>(new_row) >= grid.rows)
    return std::unexpected(MoveError::OutOfBounds);
```

### Full implementation
```cpp
std::expected<void, MoveError> try_move(
    Player& player, Grid& grid, int d_row, int d_col
) {
    auto new_row = static_cast<std::ptrdiff_t>(player.row) + d_row;
    auto new_col = static_cast<std::ptrdiff_t>(player.col) + d_col;

    if (new_row < 0 || new_col < 0
     || static_cast<std::size_t>(new_row) >= grid.rows
     || static_cast<std::size_t>(new_col) >= grid.cols)
        return std::unexpected(MoveError::OutOfBounds);

    auto v = grid.view();
    if (std::holds_alternative<Wall>(v[
            static_cast<std::size_t>(new_row),
            static_cast<std::size_t>(new_col)]))
        return std::unexpected(MoveError::HitWall);

    player.row = static_cast<std::size_t>(new_row);
    player.col = static_cast<std::size_t>(new_col);
    return {};
}
```

---

## The game loop — putting it together

```cpp
// main.cpp (Playing state)
case GameState::Playing: {
    render(grid, player);

    char key = Terminal::read_key();
    int dr{}, dc{};

    if      (key == 'w') { dr = -1; dc =  0; }
    else if (key == 's') { dr =  1; dc =  0; }
    else if (key == 'a') { dr =  0; dc = -1; }
    else if (key == 'd') { dr =  0; dc =  1; }
    else if (key == 'q') { state = GameState::GameOver; break; }

    if (dr != 0 || dc != 0)
        try_move(player, grid, dr, dc);  // wall hits silently ignored

    break;
}
```

Movement direction as deltas:
| Key | `dr` | `dc` | Direction |
|-----|------|------|-----------|
| `w` |  -1  |   0  | Up        |
| `s` |  +1  |   0  | Down      |
| `a` |   0  |  -1  | Left      |
| `d` |   0  |  +1  | Right     |

---

## ANSI escape codes

Special byte sequences that terminals understand natively — no library needed.

```cpp
std::print("\033[2J");   // clear entire screen
std::print("\033[H");    // move cursor to top-left (home)
```

Used in `Terminal::clear()` to prevent the dungeon from scrolling on every
frame. The remaining flicker is from the gap between clear and redraw —
fixable later with double buffering.

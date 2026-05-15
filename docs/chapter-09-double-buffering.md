# Chapter 09 — Double Buffering

## The problem

Clearing the screen then printing line by line leaves a brief blank gap
— visible as flicker. The CPU is fast but the eye catches the gap.

## The solution

Build the entire frame into a `std::string` in memory, then flush it
in one `write()` syscall. The screen is never partially drawn.

## `std::string::reserve()`

Pre-allocate exactly the memory needed upfront:

```cpp
frame.reserve(grid.rows * (grid.cols + 1));
```

Without reserve, the string reallocates multiple times as it grows.
With it, one allocation covers the whole frame.

## Raw POSIX `write()` over `std::print`

```cpp
::write(STDOUT_FILENO, frame.data(), frame.size());
```

- `::` — call the global POSIX symbol, bypass any local shadow
- `frame.data()` — raw `const char*` to the string's buffer
- One syscall = one atomic screen update

## Cursor home instead of clear

```cpp
::write(STDOUT_FILENO, "\033[H", 3);
```

`\033[H` moves the cursor to top-left without blanking the screen.
The new frame overwrites the old one character by character.
No blank gap, no flicker.

## `std::string_view`

A lightweight non-owning view into a string — the right type for
string constants you only need to read:

```cpp
static constexpr std::string_view DIM   = "\033[2m";
static constexpr std::string_view RESET = "\033[0m";
```

Far cheaper than `std::string` — no allocation, no ownership.

## Separation of concerns

- `glyph_for()` — static, private to renderer.cpp,
  answers "what character represents this cell?"
- `render()` — builds the frame string
- `Terminal::clear()` — repositions the cursor
- `main()` — calls clear then render: two explicit sequential steps

`static` on a free function means private to this translation unit —
an implementation detail invisible to other files.

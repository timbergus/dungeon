#include <cstdlib>  // std::exit
#include <expected> // C++23 — explicit error handling without exceptions
#include <print> // C++23 — std::println (replaces printf/cout for formatted output)
#include <random>

#include "entities/player.hpp"
#include "terminal.hpp"
#include "world/bsp.hpp"
#include "world/grid.hpp"
#include "world/renderer.hpp"

// ── A tiny taste of C++23 error handling ─────────────────────────────────────
//
// std::expected<T, E> is the modern C++ way to return "either a value or an
// error" — no exceptions, no output params, no magic error codes buried in
// ints.
//
// Think of it as Rust's Result<T, E> or Haskell's Either, built into the STL.

enum class InitError { TerminalTooSmall, CannotReadInput };

std::expected<void, InitError> init_terminal() {
  // TODO: check terminal size via sys/ioctl.h in a later step.
  // For now we just signal success.
  return {}; // An empty expected<void,E> means "success".
}

// ── Game states
// ───────────────────────────────────────────────────────────────
//
// We'll use a plain enum class (scoped, type-safe) to drive the game loop.
// In a later step this becomes a proper state machine with std::variant.

enum class GameState { MainMenu, Playing, Paused, GameOver };

// ── Entry point
// ───────────────────────────────────────────────────────────────
int main() {
  // std::println is C++23: std::format + newline, no '\n' ceremony needed.
  std::println("╔══════════════════════════════╗");
  std::println("║   🗡  DUNGEON CRAWLER  🗡     ║");
  std::println("╚══════════════════════════════╝");
  std::println("");

  Terminal terminal{};

  // Pattern: handle the expected/unexpected at the call site.
  if (auto result = init_terminal(); !result) {
    switch (result.error()) {
    case InitError::TerminalTooSmall:
      std::println(stderr, "Error: terminal window is too small.");
      break;
    case InitError::CannotReadInput:
      std::println(stderr, "Error: cannot read from stdin.");
      break;
    }
    return EXIT_FAILURE;
  }

  std::println("Terminal OK. Starting game loop…");
  std::println("(Press Ctrl-C to quit for now)\n");

  Grid grid(40, 80);

  std::mt19937 rng{std::random_device{}()};
  auto root = make_tree(grid, rng);
  generate(root, grid, rng);

  Rect start = first_room(root);

  Player player{"Hero", start.center_row(), start.center_col()};

  player.mark_visited(player.row, player.col);
  player.mark_visible_as_visited(grid);

  // ── Bare-bones game loop
  // ──────────────────────────────────────────────────
  //
  // Every roguelike is built around a loop: read input → update world →
  // render. We'll flesh each part out over the coming steps.

  GameState state = GameState::MainMenu;
  bool running = true;

  while (running) {
    switch (state) {
    case GameState::MainMenu:
      std::println("[MENU] Press Enter to start, q to quit.");
      if (char c = '\0'; (c = static_cast<char>(std::getchar())) == 'q') {
        running = false;
      } else {
        state = GameState::Playing;
      }
      break;

    case GameState::Playing: {
      Terminal::clear();

      render(grid, player);

      char key = Terminal::read_key();
      int dr{}, dc{};

      if (key == 'w') {
        dr = -1;
        dc = 0;
      } else if (key == 's') {
        dr = 1;
        dc = 0;
      } else if (key == 'a') {
        dr = 0;
        dc = -1;
      } else if (key == 'd') {
        dr = 0;
        dc = 1;
      } else if (key == 'q') {
        state = GameState::GameOver;
        break;
      }

      if (dr != 0 || dc != 0) {
        auto result = try_move(player, grid, dr, dc);
        player.mark_visible_as_visited(grid);
        if (!result && result.error() == MoveError::HitWall) {
          // Silently ignore wall collisions — just don't move
        }
      }

      break;
    }

    case GameState::GameOver:
      std::println("[GAME OVER] Thanks for playing!");
      running = false;
      break;

    case GameState::Paused:
      // Not reachable yet — added to silence the compiler warning
      // about unhandled enum values. We'll wire it up later.
      break;
    }
  }

  return EXIT_SUCCESS;
}

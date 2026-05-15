#include <cstddef>
#include <string>
#include <unistd.h>

#include "overloaded.hpp"
#include "world/renderer.hpp"

static constexpr std::string_view DIM = "\033[2m";
static constexpr std::string_view RESET = "\033[0m";
static constexpr std::string_view YELLOW = "\033[33m";
static constexpr std::string_view CYAN = "\033[36m";
static constexpr std::string_view RED = "\033[31m";
static constexpr std::string_view GREEN = "\033[32m";
static constexpr std::string_view WHITE = "\033[37m";

static std::string base_glyph(const Tile &tile) {
  return std::visit(
      overloaded{
          [](const Floor &) -> std::string { return std::string{WHITE} + "."; },
          [](const Wall &) -> std::string { return std::string{WHITE} + "#"; },
          [](const Door &d) -> std::string {
            if (d.is_locked) {
              return std::string(RED) + "x";
            }
            return d.is_open ? std::string(GREEN) + "_"
                             : std::string(GREEN) + "+";
          },
          [](const Stairs &s) -> std::string {
            return s.direction == StairsDirection::Down
                       ? std::string(CYAN) + ">"
                       : std::string(CYAN) + "<";
          },
          [](const Chest &c) -> std::string {
            if (c.is_locked) {
              return std::string(RED) + "≠";
            } // watch the return type here!
            return c.is_open ? std::string(YELLOW) + "~"
                             : std::string(YELLOW) + "=";
          },
      },
      tile);
}

static std::string glyph_for(const Tile &tile, const Player &player,
                             const Grid &grid, std::size_t row, std::size_t col,
                             FogMode fog_mode) {
  bool fog_on = fog_mode == FogMode::Enabled;
  bool fog_off = fog_mode == FogMode::Disabled;

  // Player character — always visible
  if (row == player.row && col == player.col) {
    return "@";
  }

  bool visible = player.can_see(row, col, grid);
  bool remembered = player.has_visited(row, col);

  // Never seen — pure darkness
  if (!visible && !remembered && fog_on)
    return " ";

  // Decide the base glyph
  std::string base = base_glyph(tile);

  // Visible — full brightness
  if (visible || fog_off) {
    return base + std::string(RESET);
  }

  // Remembered but not visible — dimmed
  // only remaining case: !visible && remembered && fog_enabled
  return std::string(DIM) + base + std::string(RESET);
}

void render(Grid &grid, const Player &player, FogMode fog_mode) {
  auto v = grid.view();

  std::string frame;
  // frame.reserve(grid.rows * (grid.cols + 1));

  for (std::size_t row = 0; row < grid.rows; ++row) {
    for (std::size_t col = 0; col < grid.cols; ++col) {
      frame += glyph_for(v[row, col], player, grid, row, col, fog_mode);
    }
    frame += '\n';
  }

  ::write(STDOUT_FILENO, frame.data(), frame.size());
}

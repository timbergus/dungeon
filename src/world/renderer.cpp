#include <cstddef>
#include <unistd.h>

#include "overloaded.hpp"
#include "world/renderer.hpp"

static constexpr std::string_view DIM = "\033[2m";
static constexpr std::string_view RESET = "\033[0m";

static char base_glyph(const Tile &tile) {
  return std::visit(overloaded{
                        [](const Floor &) { return '.'; },
                        [](const Wall &) { return '#'; },
                        [](const Door &d) { return d.is_open ? '_' : '+'; },
                        [](const Stairs &s) {
                          return s.direction == StairsDirection::Down ? '>'
                                                                      : '<';
                        },
                        [](const Chest &c) { return c.is_open ? '~' : '='; },
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
  char base = base_glyph(tile);

  // Visible — full brightness
  if (visible || fog_off) {
    return std::string(1, base);
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

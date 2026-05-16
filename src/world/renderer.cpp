#include "world/renderer.hpp"
#include "overloaded.hpp"
#include "ui/color.hpp"
#include <cstddef>
#include <string>
#include <unistd.h>

static std::string base_glyph(const Tile &tile) {
  return std::visit(overloaded{
                        [](const Floor &) -> std::string {
                          return std::string{Color::FLOOR} + ".";
                        },
                        [](const Wall &) -> std::string {
                          return std::string{Color::WALL} + "#";
                        },
                        [](const Door &d) -> std::string {
                          if (d.is_locked) {
                            return std::string(Color::DANGER) + "x";
                          }
                          return d.is_open ? std::string(Color::DOOR) + "_"
                                           : std::string(Color::DOOR) + "+";
                        },
                        [](const Stairs &s) -> std::string {
                          return s.direction == StairsDirection::Down
                                     ? std::string(Color::STAIRS) + ">"
                                     : std::string(Color::STAIRS) + "<";
                        },
                        [](const Chest &c) -> std::string {
                          if (c.is_locked) {
                            return std::string(Color::DANGER) + "≠";
                          } // watch the return type here!
                          return c.is_open ? std::string(Color::LOOT) + "~"
                                           : std::string(Color::LOOT) + "=";
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
    return base + std::string(Color::RESET);
  }

  // Remembered but not visible — dimmed
  // only remaining case: !visible && remembered && fog_enabled
  return std::string(Color::FOG) + base + std::string(Color::RESET);
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

#include <print>

#include "overloaded.hpp"
#include "terminal.hpp"
#include "world/renderer.hpp"

void render(Grid &grid, const Player &player) {
  Terminal::clear();

  auto v = grid.view();

  for (std::size_t row = 0; row < grid.rows; ++row) {
    for (std::size_t col = 0; col < grid.cols; ++col) {
      if (row == player.row && col == player.col) {
        std::print("@");
        continue;
      }

      char glyph = std::visit(
          overloaded{[](const Floor &) { return '.'; },
                     [](const Wall &) { return '#'; },
                     [](const Door &d) { return d.is_open ? '_' : '+'; }},
          v[row, col]);

      std::print("{}", glyph);
    }
    std::println();
  }
}

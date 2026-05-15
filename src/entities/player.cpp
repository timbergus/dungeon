#include "entities/player.hpp"
#include "world/tile.hpp"
#include <cstddef>
#include <expected>
#include <variant>

std::expected<void, MoveError> try_move(Player &player, Grid &grid, int d_row,
                                        int d_col) {
  // Cast carefully — size_t + negative int needs explicit handling
  auto new_row = static_cast<std::ptrdiff_t>(player.row) + d_row;
  auto new_col = static_cast<std::ptrdiff_t>(player.col) + d_col;

  // Bounds check
  if (new_row < 0 || new_col < 0 ||
      static_cast<std::size_t>(new_row) >= grid.rows ||
      static_cast<std::size_t>(new_col) >= grid.cols) {
    return std::unexpected(MoveError::OutOfBounds);
  }

  auto v = grid.view();

  auto &target =
      v[static_cast<std::size_t>(new_row), static_cast<std::size_t>(new_col)];

  // Wall check
  if (std::holds_alternative<Wall>(target)) {
    return std::unexpected(MoveError::HitWall);
  }

  // Door check
  if (auto *d = std::get_if<Door>(&target)) {
    if (!d->is_open) {
      return std::unexpected(MoveError::HitDoor);
    }
  }

  player.row = static_cast<std::size_t>(new_row);
  player.col = static_cast<std::size_t>(new_col);
  player.mark_visited(player.row, player.col);

  return {};
}

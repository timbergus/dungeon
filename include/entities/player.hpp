#pragma once

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <expected>
#include <string>
#include <unordered_set>
#include <utility>

#include "utils/hash.hpp"
#include "world/grid.hpp"

inline constexpr std::size_t VISIBILITY_RADIUS = 5;

struct Player {
  std::string name;
  std::size_t row;
  std::size_t col;
  int health;
  int shield;

  std::unordered_set<std::pair<std::size_t, std::size_t>, PairHash> visited;

  void mark_visited(std::size_t r, std::size_t c) { visited.insert({r, c}); }

  bool has_visited(std::size_t r, std::size_t c) const {
    return visited.contains({r, c});
  }

  bool has_line_of_sight(const Grid &grid, std::size_t tr,
                         std::size_t tc) const {
    // Bresenham's line from (row,col) to (tr,tc)
    std::ptrdiff_t r0 = static_cast<std::ptrdiff_t>(row);
    std::ptrdiff_t c0 = static_cast<std::ptrdiff_t>(col);
    std::ptrdiff_t r1 = static_cast<std::ptrdiff_t>(tr);
    std::ptrdiff_t c1 = static_cast<std::ptrdiff_t>(tc);

    std::ptrdiff_t dr = std::abs(r1 - r0);
    std::ptrdiff_t dc = std::abs(c1 - c0);
    std::ptrdiff_t sr = r0 < r1 ? 1 : -1;
    std::ptrdiff_t sc = c0 < c1 ? 1 : -1;
    std::ptrdiff_t err = dr - dc;

    auto v = grid.view();

    while (true) {
      // If this tile is a wall and it's not the target, line is blocked
      if (std::holds_alternative<Wall>(
              v[static_cast<std::size_t>(r0), static_cast<std::size_t>(c0)]) &&
          !(r0 == r1 && c0 == c1)) {
        return false;
      }

      if (r0 == r1 && c0 == c1)
        return true;

      std::ptrdiff_t e2 = 2 * err;
      if (e2 > -dc) {
        err -= dc;
        r0 += sr;
      }
      if (e2 < dr) {
        err += dr;
        c0 += sc;
      }
    }
  }

  bool can_see(std::size_t r, std::size_t c, const Grid &grid) const {
    auto dr = static_cast<double>(r) - static_cast<double>(row);
    auto dc = static_cast<double>(c) - static_cast<double>(col);

    auto dist = std::sqrt(std::pow(dr, 2) + std::pow(dc, 2));

    bool in_radius = dist <= static_cast<double>(VISIBILITY_RADIUS);

    return in_radius && has_line_of_sight(grid, r, c);
  }

  void mark_visible_as_visited(const Grid &grid) {
    for (std::size_t r = 0; r < grid.rows; ++r) {
      for (std::size_t c = 0; c < grid.cols; ++c) {
        if (can_see(r, c, grid)) {
          mark_visited(r, c);
        }
      }
    }
  }

  Player(std::string name, std::size_t start_row, std::size_t start_col)
      : name{std::move(name)}, row{start_row}, col{start_col}, health{100},
        shield{30} {}
};

enum class MoveError { HitDoor, HitWall, OutOfBounds };

std::expected<void, MoveError> try_move(Player &player, Grid &grid, int d_row,
                                        int d_col);

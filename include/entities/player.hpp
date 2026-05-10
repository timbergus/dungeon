#pragma once

#include "world/grid.hpp"
#include <cstddef>
#include <expected>
#include <string>

struct Player {
  std::string name;
  std::size_t row;
  std::size_t col;
  int health;
  int shield;

  Player(std::string name, std::size_t start_row, std::size_t start_col)
      : name{std::move(name)}, row{start_row}, col{start_col}, health{100},
        shield{30} {}
};

enum class MoveError { HitWall, OutOfBounds };

std::expected<void, MoveError> try_move(Player &player, Grid &grid, int d_row,
                                        int d_col);

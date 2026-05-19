#pragma once

#include <cstddef>
#include <memory>
#include <random>

#include "world/grid.hpp"

struct Rect {
  std::size_t row;
  std::size_t col;
  std::size_t height;
  std::size_t width;

  std::size_t center_row() const { return row + height / 2; }
  std::size_t center_col() const { return col + width / 2; }
};

struct BSPNode {
  Rect region;
  std::unique_ptr<BSPNode> left = nullptr;
  std::unique_ptr<BSPNode> right = nullptr;

  bool is_leaf() const { return left == nullptr && right == nullptr; }
};

void split(BSPNode &node, std::mt19937 &rng, std::size_t min_size, int depth);

void place_rooms(BSPNode &node, Grid &grid, std::mt19937 &rng,
                 std::size_t min_size);

void carve_corridors(BSPNode &node, Grid &grid);

BSPNode make_tree(Grid &grid, std::mt19937 &rng);

void generate(BSPNode &root, Grid &grid, std::mt19937 &rng);

Rect first_room(BSPNode &node);

Rect last_room(BSPNode &node);

void place_landmarks(BSPNode &root, Grid &grid, std::mt19937 &rng);

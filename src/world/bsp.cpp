#include <algorithm>
#include <cstddef>
#include <memory>
#include <random>

#include "world/bsp.hpp"
#include "world/tile.hpp"

void split(BSPNode &node, std::mt19937 &rng, std::size_t min_size, int depth) {
  if (depth == 0 || node.region.height < min_size * 2 ||
      node.region.width < min_size * 2) {
    return;
  }

  // Decide split direction based on the region's shape
  bool split_horizontally = node.region.width > node.region.height;

  if (split_horizontally) {
    std::uniform_int_distribution<std::size_t> dist(
        min_size, node.region.width - min_size);

    std::size_t split_col = node.region.col + dist(rng);

    node.left = std::make_unique<BSPNode>(
        BSPNode{.region = {node.region.row, node.region.col, node.region.height,
                           split_col - node.region.col}});

    node.right = std::make_unique<BSPNode>(
        BSPNode{.region = {node.region.row, split_col, node.region.height,
                           node.region.col + node.region.width - split_col}});
  } else {
    std::uniform_int_distribution<std::size_t> dist(
        min_size, node.region.height - min_size);

    std::size_t split_row = node.region.row + dist(rng);

    node.left = std::make_unique<BSPNode>(
        BSPNode{.region = {node.region.row, node.region.col,
                           split_row - node.region.row, node.region.width}});

    node.right = std::make_unique<BSPNode>(
        BSPNode{.region = {split_row, node.region.col,
                           node.region.row + node.region.height - split_row,
                           node.region.width}});
  }

  split(*node.left, rng, min_size, depth - 1);
  split(*node.right, rng, min_size, depth - 1);
}

void place_rooms(BSPNode &node, Grid &grid, std::mt19937 rng,
                 std::size_t min_size) {
  if (node.is_leaf()) {
    // Pick a random room size that fits inside this region
    std::uniform_int_distribution<std::size_t> h_dist(min_size,
                                                      node.region.height - 1);
    std::uniform_int_distribution<std::size_t> w_dist(min_size,
                                                      node.region.width - 1);

    std::size_t room_h = h_dist(rng);
    std::size_t room_w = w_dist(rng);

    // Pick a random position for the room inside the region
    std::uniform_int_distribution<std::size_t> r_dist(
        node.region.row, node.region.row + node.region.height - room_h - 1);

    std::uniform_int_distribution<std::size_t> c_dist(
        node.region.col, node.region.col + node.region.width - room_w - 1);

    std::size_t room_row = r_dist(rng);
    std::size_t room_col = c_dist(rng);

    // Carve the room into the grid
    auto v = grid.view();

    for (std::size_t r = room_row; r < room_row + room_h; ++r) {
      for (std::size_t c = room_col; c < room_col + room_w; ++c) {
        v[r, c] = Tile{Floor{}};
      }
    }

    // Store the room rect back into the node so carve_corridors can find it
    node.region = {room_row, room_col, room_h, room_w};

    return;
  }

  place_rooms(*node.left, grid, rng, min_size);
  place_rooms(*node.right, grid, rng, min_size);
}

void carve_corridors(BSPNode &node, Grid &grid) {
  if (node.is_leaf()) {
    return;
  }

  // Recurse first so children have their final room positions
  carve_corridors(*node.left, grid);
  carve_corridors(*node.right, grid);

  // Connect the centres of the two children
  auto [r1, c1] =
      std::pair{node.left->region.center_row(), node.left->region.center_col()};
  auto [r2, c2] = std::pair{node.right->region.center_row(),
                            node.right->region.center_col()};

  auto v = grid.view();

  // Carve horizontally then vertically (L-shaped corridor)
  std::size_t c_min = std::min(c1, c2);
  std::size_t c_max = std::max(c1, c2);

  for (std::size_t c = c_min; c <= c_max; ++c) {
    v[r1, c] = Tile{Floor{}};
  }

  std::size_t r_min = std::min(r1, r2);
  std::size_t r_max = std::max(r1, r2);

  for (std::size_t r = r_min; r <= r_max; ++r) {
    v[r, c2] = Tile{Floor{}};
  }
}

BSPNode make_tree(Grid &grid, std::mt19937 &rng) {
  BSPNode root{.region = {0, 0, grid.rows, grid.cols}};
  split(root, rng, 4, 4);
  return root;
}

void generate(BSPNode &root, Grid &grid, std::mt19937 &rng) {
  // Fill everything with walls first
  auto v = grid.view();

  for (std::size_t r = 0; r < grid.rows; ++r) {
    for (std::size_t c = 0; c < grid.cols; ++c) {
      v[r, c] = Tile{Wall{}};
    }
  }

  // Place rooms and carve corridors
  place_rooms(root, grid, rng, 3);
  carve_corridors(root, grid);
}

Rect first_room(BSPNode &node) {
  if (node.is_leaf()) {
    return node.region;
  }

  return first_room(*node.left);
}

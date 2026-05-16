#include "world/bsp.hpp"
#include "world/tile.hpp"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <random>
#include <unordered_set>
#include <vector>

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

Rect first_room(BSPNode &node) {
  if (node.is_leaf()) {
    return node.region;
  }

  return first_room(*node.left);
}

Rect last_room(BSPNode &node) {
  if (node.is_leaf()) {
    return node.region;
  }

  return last_room(*node.right);
}

static void collect_leaves(BSPNode &node, std::vector<BSPNode *> &leaves) {
  if (node.is_leaf()) {
    leaves.push_back(&node);
    return;
  }

  collect_leaves(*node.left, leaves);
  collect_leaves(*node.right, leaves);
}

void place_landmarks(BSPNode &root, Grid &grid, std::mt19937 &rng) {
  std::vector<BSPNode *> leaves;
  collect_leaves(root, leaves);

  // Need at least 2 rooms — one for stairs up, one for stairs down
  if (leaves.size() < 2) {
    return;
  }

  std::unordered_set<std::size_t> used_rooms;
  used_rooms.insert(0);
  used_rooms.insert(leaves.size() - 1);

  auto v = grid.view();

  // StairsUp in the first room (player start)
  auto &start = leaves.front()->region;
  v[start.center_row(), start.center_col()] = Tile{Stairs{StairsDirection::Up}};

  // StairsDown in the last room
  auto &end = leaves.back()->region;
  v[end.center_row(), end.center_col()] = Tile{Stairs{StairsDirection::Down}};

  // Chests in random middle rooms
  std::uniform_int_distribution<std::size_t> pick(1, leaves.size() - 2);

  std::size_t chest_count = std::min(leaves.size() / 3, std::size_t{3});

  std::uniform_int_distribution<int> one_in_three{0, 2};
  std::uniform_int_distribution<int> one_in_five{0, 4};

  for (std::size_t i = 0; i < chest_count; ++i) {
    // If we have more chests than available rooms, we need to stop adding them.
    if (used_rooms.size() >= leaves.size()) {
      break;
    }

    size_t index = pick(rng);

    while (used_rooms.contains(index)) {
      index = pick(rng);
    }

    bool found_open = one_in_three(rng) == 0;
    bool is_mimic = one_in_five(rng) == 0;

    auto &room = leaves[index]->region;
    v[room.center_row(), room.center_col()] = Tile{Chest{
        .is_open = found_open,
        .is_mimic = is_mimic,
        .is_locked = false,
        .found_open = found_open,
    }};
    used_rooms.insert(index);
  }
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
  place_landmarks(root, grid, rng);
}

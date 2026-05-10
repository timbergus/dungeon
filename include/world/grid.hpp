#pragma once

#include <cstddef>
#include <mdspan>
#include <vector>

#include "tile.hpp"

struct Grid {
  std::size_t rows;
  std::size_t cols;
  std::vector<Tile> cells;

  Grid(std::size_t rows, std::size_t cols)
      : rows{rows}, cols{cols}, cells(rows * cols, Tile{Floor{}}) {}

  auto view() { return std::mdspan(cells.data(), rows, cols); }
};

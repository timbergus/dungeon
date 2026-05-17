#pragma once

#include <random>
#include <stdexcept>
namespace Dice {

// Roll a single die with 'sides' faces - e.g. roll(20) == D20
inline int roll(int sides, std::mt19937 &rng) {
  if (sides < 2) {
    throw std::invalid_argument("A die needs at least two sides.");
  }

  std::uniform_int_distribution<int> die{1, sides};

  return die(rng);
}

// Roll multiple dice and sum - e.g. roll(6, 3) == 3 * D6
inline int roll(int sides, std::mt19937 &rng, int count) {
  int total = 0;

  for (int i = 0; i < count; ++i) {
    total += roll(sides, rng);
  }

  return total;
}

// Named dice - readable at call sites
inline int D4(std::mt19937 &rng) { return roll(4, rng); }
inline int D6(std::mt19937 &rng) { return roll(6, rng); }
inline int D8(std::mt19937 &rng) { return roll(8, rng); }
inline int D10(std::mt19937 &rng) { return roll(10, rng); }
inline int D12(std::mt19937 &rng) { return roll(12, rng); }
inline int D20(std::mt19937 &rng) { return roll(20, rng); }
} // namespace Dice

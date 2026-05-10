#pragma once

#include <cstdint>
#include <variant>

struct Floor {};

struct Wall {};

struct Door {
  bool is_open = false; // Doors have a state - variants can hold data!
};

using Tile = std::variant<Floor, Wall, Door>;

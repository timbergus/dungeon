#pragma once

#include <variant>

enum class StairsDirection { Up, Down };

struct Floor {};

struct Wall {};

struct Door {
  bool is_open = false; // Doors have a state - variants can hold data!
  bool is_locked = false;
};

struct Stairs {
  StairsDirection direction;
};

struct Chest {
  bool is_open = false;
  bool is_mimic = false;
  bool is_locked = false;
};

using Tile = std::variant<Floor, Wall, Door, Stairs, Chest>;

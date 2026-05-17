#pragma once

#include "entities/enemy.hpp"
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
  bool is_locked = false;
  bool found_open = false; // was already open when discovered
  bool is_broken = false;  // destroyed by attacking
};

using Tile = std::variant<Floor, Wall, Door, Stairs, Chest, Mimic>;

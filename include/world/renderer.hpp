#pragma once

#include "entities/player.hpp"
#include "grid.hpp"

enum class FogMode { Enabled, Disabled };

void render(Grid &grid, const Player &player, FogMode fog_mode);

#pragma once

#include "ui/interactions.hpp"

void show_death_screen(InteractionResult cause);
void show_victory_screen(int levels_descended, int mimics_defeated,
                         int chests_looted);

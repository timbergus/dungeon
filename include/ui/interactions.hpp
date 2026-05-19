#pragma once

#include "ui/dialog.hpp"
#include "world/tile.hpp"

enum class InteractionResult {
  None,
  ChestLooted,
  MimicFight,
  MimicAte,
  PlayerDied,
  SurfaceDeath,
  Descended,
  Ascended,
};

Dialog chest_dialog(const Chest &chest);
Dialog stairs_dialog(const Stairs &stairs);
Dialog door_dialog(const Door &door);
Dialog mimic_dialog(const Mimic &mimic);
Dialog broken_chest_dialog(const Chest &chest);
Dialog surface_dialog();

InteractionResult resolve_chest(Chest &chest, std::size_t choice);

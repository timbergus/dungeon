#pragma once

#include "ui/art.hpp"
#include "ui/dialog.hpp"
#include "world/tile.hpp"

enum class InteractionResult {
  None,
  ChestLooted,
  MimicFight,
  MimicAte,
  Descended,
  Ascended,
};

Dialog chest_dialog(const Chest &chest);
Dialog stairs_dialog(const Stairs &stairs);
Dialog door_dialog(const Door &door);

InteractionResult resolve_chest(Chest &chest, std::size_t choice);

#include "ui/interactions.hpp"

Dialog chest_dialog(const Chest &chest) {
  // Already looted by the player
  if (chest.is_open && !chest.found_open) {
    return Dialog{.title = "You already looted this chest.",
                  .art = Art::CHEST_OPEN,
                  .options = {
                      {"1", "Walk away", "Nothing left here"},
                  }};
  }

  // Found already open — suspicious
  if (chest.is_open && chest.found_open) {
    return Dialog{.title =
                      "An open chest... someone got here first. Or did they?",
                  .art = Art::CHEST_OPEN,
                  .options = {
                      {"1", "Inspect it", "Maybe something was missed..."},
                      {"2", "Attack it", "Strike first — just in case"},
                      {"3", "Leave it", "Something feels very wrong..."},
                  }};
  }

  // Locked
  if (chest.is_locked) {
    return Dialog{.title = "A locked chest sits before you...",
                  .art = Art::CHEST_LOCKED,
                  .options = {
                      {"1", "Use a key", "Unlock and open the chest"},
                      {"2", "Attack it", "What if it's a mimic?"},
                      {"3", "Leave it", "Walk away — for now"},
                  }};
  }

  // Closed — normal
  return Dialog{.title = "A chest! Could be treasure... or a trap.",
                .art = Art::CHEST_CLOSED,
                .options = {
                    {"1", "Open it", "Reach for the lid"},
                    {"2", "Attack it", "Strike first, ask questions later"},
                    {"3", "Leave it", "Something feels wrong..."},
                }};
}

Dialog stairs_dialog(const Stairs &stairs) {
  if (stairs.direction == StairsDirection::Down) {
    return Dialog{.title = "A dark passage leads downward...",
                  .art = Art::STAIRS_DOWN,
                  .options = {
                      {"1", "Descend", "Into the darkness below"},
                      {"2", "Wait", "Not yet — there's more to explore"},
                  }};
  }
  return Dialog{.title = "The way back up...",
                .art = Art::STAIRS_UP,
                .options = {
                    {"1", "Ascend", "Return to the level above"},
                    {"2", "Stay", "There's still more to find here"},
                }};
}

Dialog door_dialog(const Door &door) {
  if (door.is_locked) {
    return Dialog{.title = "A locked door blocks your path.",
                  .art = Art::STAIRS_DOWN, // placeholder until we have door art
                  .options = {
                      {"1", "Use a key", "Unlock the door"},
                      {"2", "Force it", "Try to break it down"},
                      {"3", "Leave it", "Find another way"},
                  }};
  }
  return Dialog{.title = "A closed door.",
                .art = Art::STAIRS_DOWN, // placeholder
                .options = {
                    {"1", "Open it", "Push the door open"},
                    {"2", "Leave it", "Keep it closed"},
                }};
}

InteractionResult resolve_chest(Chest &chest, std::size_t choice) {
  // Already looted — only option is walk away
  if (chest.is_open && !chest.found_open)
    return InteractionResult::None;

  // Found open chest
  if (chest.is_open && chest.found_open) {
    if (choice == 0) {
      // Inspect — no escape from an open mimic
      if (chest.is_mimic) {
        // Show the Pratchett moment 😈
        show_dialog(Dialog{
            .title = "The lid snaps shut. Were those teeth there before?",
            .art = Art::MIMIC,
            .options = {{"1", "...", "Your last thought fades"}}});
        return InteractionResult::MimicAte;
      }
      chest.found_open = false; // no longer suspicious
      return InteractionResult::ChestLooted;
    }
    if (choice == 1) {
      // Attack — win or lose, no loot from an open mimic
      return InteractionResult::MimicFight;
    }
    return InteractionResult::None; // leave it
  }

  // Closed chest — choice 0: open, 1: attack, 2: leave
  if (choice == 0) {
    if (chest.is_mimic) {
      chest.is_open = true;
      return InteractionResult::MimicFight; // mimic reveals itself
    }
    chest.is_open = true;
    return InteractionResult::ChestLooted;
  }
  if (choice == 1)
    return InteractionResult::MimicFight; // attack regardless

  return InteractionResult::None; // leave it
}

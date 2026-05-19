#include "ui/interactions.hpp"

Dialog chest_dialog(const Chest &chest) {
  // Broken — always check this first
  if (chest.is_broken) {
    return broken_chest_dialog(chest);
  }

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

// In stairs_dialog — or a new surface_dialog
Dialog surface_dialog() {
  return Dialog{.title = "You climb the stairs toward the surface.\n\n"
                         "Fresh air. Sunlight. Freedom.\n\n"
                         "Also Orcs. Lots of Orcs.\n"
                         "Now you remember why you came down here.\n\n"
                         "Well. Best of luck next time.",
                .art = Art::STAIRS_UP,
                .options = {
                    {"1", "...", "Death nods sympathetically"},
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

Dialog mimic_dialog(const Mimic &mimic) {
  if (mimic.is_defeated) {
    return Dialog{
        .title = "The mimic lies still. "
                 "Its teeth are less charming now.",
        .art = Art::MIMIC,
        .options = {
            {"1", "Search the remains", "Maybe it dropped something useful"},
            {"2", "Walk away", "You've seen enough teeth today"},
        }};
  }

  // Pre-opened mimic — player thinks it's an already-looted chest
  if (mimic.is_open && !mimic.is_defeated) {
    return Dialog{.title = "An open chest... someone got here first. "
                           "Or did they?",
                  .art = Art::CHEST_OPEN, // still deceiving with chest art!
                  .options = {
                      {"1", "Inspect it", "Maybe something was missed..."},
                      {"2", "Attack it", "Strike first — just in case"},
                      {"3", "Leave it", "Something feels very wrong..."},
                  }};
  }

  // Disguised — player thinks it's a closed chest
  return Dialog{.title = "A chest... probably. Almost certainly. Most likely.",
                .art = Art::CHEST_CLOSED,
                .options = {
                    {"1", "Open it", "Reach for the lid"},
                    {"2", "Attack it", "Strike first, ask questions later"},
                    {"3", "Leave it", "Something feels... bitey"},
                }};
}

Dialog broken_chest_dialog(const Chest &chest) {

  // Second interaction with an already broken chest
  if (chest.is_broken && chest.is_open) {
    return Dialog{.title = "A pile of splinters.\n"
                           "A monument to your paranoia.\n\n"
                           "Nothing left here.",
                  .art = Art::CHEST_BROKEN,
                  .options = {
                      {"1", "Walk away", "Nothing to see here"},
                  }};
  }

  std::string title;

  if (chest.found_open) {
    title = "You attack the already-open chest.\n"
            "Your paranoia has reached new heights.\n\n"
            "It was not a mimic.\n"
            "It was not even suspicious.\n"
            "It was just a chest.";
  } else if (chest.is_open) {
    title = "You attack the chest you just opened.\n"
            "A bold strategy.\n\n"
            "The loot you were about to pick up\n"
            "is now decorating the walls.";
  } else {
    title = "You smash the innocent chest.\n\n"
            "It was not a mimic.\n"
            "The loot is... everywhere.\n"
            "And somehow also nowhere.";
  }

  return Dialog{.title = title,
                .art = Art::CHEST_BROKEN,
                .options = {
                    {"1", "Sigh deeply", "At least no one saw that"},
                }};
}

InteractionResult resolve_chest(Chest &chest, std::size_t choice) {
  // Already looted by the player — walk away
  if (chest.is_open && !chest.found_open)
    return InteractionResult::None;

  // Found open — could have remaining loot
  if (chest.is_open && chest.found_open) {
    if (choice == 0) {
      // Inspect — just a chest, might have something left
      chest.found_open = false;
      return InteractionResult::ChestLooted;
    }
    if (choice == 1) {
      // Attack — destroy it
      show_dialog(broken_chest_dialog(chest));
      chest.is_broken = true;
      return InteractionResult::None;
    }
    return InteractionResult::None; // leave it
  }

  // Closed chest — choice 0: open, 1: attack, 2: leave
  if (choice == 0) {
    chest.is_open = true;
    return InteractionResult::ChestLooted;
  }

  if (choice == 1) {
    show_dialog(broken_chest_dialog(chest));
    // Attacking a real chest — destroys it, no loot
    chest.is_open = true;
    chest.is_broken = true;
    return InteractionResult::None;
  }

  return InteractionResult::None; // leave it
}

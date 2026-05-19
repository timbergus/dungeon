#include "ui/death_screen.hpp"
#include "terminal.hpp"
#include "ui/art.hpp"
#include "ui/color.hpp"
#include <format>

void show_death_screen(InteractionResult cause) {
  std::string art = Art::DEATH;
  std::string quote = "";

  switch (cause) {
  case InteractionResult::MimicAte:
    art = Art::DEATH_MIMIC_ATE;
    quote = "WELL, AT LEAST NO ONE WAS THERE\n"
            "TO SEE YOU CRAP YOUR PANTS.\n\n"
            "\"SIGHT\"\n\n"
            "PLEASE, FOLLOW ME TO THE LOBBY.";
    break;

  case InteractionResult::MimicFight:
    art = Art::DEATH_MIMIC_FIGHT;
    quote = "YOU FOUGHT BRAVELY. WELL.\n"
            "ADEQUATELY. WELL, YOU FOUGHT.\n\n"
            "\"SIGHT\"\n\n"
            "PLEASE, FOLLOW ME TO THE LOBBY.";
    break;

  case InteractionResult::PlayerDied:
    quote = "YOU FOUGHT. THAT IS SOMETHING.\n"
            "NOT MUCH, BUT SOMETHING.\n\n"
            "\"SIGHT\"\n\n"
            "PLEASE, FOLLOW ME TO THE LOBBY.";
    break;

  case InteractionResult::SurfaceDeath:
    quote = "THE ORCS SEND THEIR REGARDS.\n\n"
            "I DID TRY TO WARN YOU.\n"
            "WELL. NO. I DIDN'T.\n"
            "BUT I THOUGHT ABOUT IT.\n\n"
            "\"SIGHT\"\n\n"
            "PLEASE, FOLLOW ME TO THE LOBBY.";
    break;

  default:
    quote = "EVERYONE DIES.\n"
            "MOST PEOPLE MANAGE IT\n"
            "WITH MORE DIGNITY THAN THIS.\n\n"
            "\"SIGHT\"\n\n"
            "PLEASE, FOLLOW ME TO THE LOBBY.";
    break;
  }

  Terminal::full_clear();

  // Build the death screen as one frame
  std::string frame;

  // Separator — dim cyan line across the top
  frame += std::string(Color::DEATH_DIM) + std::string(Color::DEATH_ART) +
           std::string(40, '~') + std::string(Color::RESET) + "\n\n";

  // Art — cyan
  frame +=
      std::string(Color::DEATH_ART) + art + std::string(Color::RESET) + "\n\n";

  // Separator
  frame += std::string(Color::DEATH_DIM) + std::string(Color::DEATH_ART) +
           std::string(40, '~') + std::string(Color::RESET) + "\n\n";

  // Death's words — white, slightly dim for gravitas
  frame += std::string(Color::DEATH_DIM) + std::string(Color::DEATH_QUOTE) +
           quote + std::string(Color::RESET) + "\n\n";

  // Separator
  frame += std::string(Color::DEATH_DIM) + std::string(Color::DEATH_ART) +
           std::string(40, '~') + std::string(Color::RESET) + "\n\n";

  // Prompt — dim, unobtrusive
  frame += std::string(Color::DEATH_DIM) + "[ Press any key to follow... ]" +
           std::string(Color::RESET) + "\n";

  ::write(STDOUT_FILENO, frame.data(), frame.size());

  // Wait for any key — no choice needed, Death is not optional
  Terminal::read_key();
}

void show_victory_screen(int levels_descended, int mimics_defeated,
                         int chests_looted) {
  Terminal::full_clear();

  std::string frame;

  // Top border
  frame += std::string(Color::DEATH_DIM) + std::string(Color::DEATH_ART) +
           std::string(40, '~') + std::string(Color::RESET) + "\n\n";

  // Art — Death with coffee
  frame += std::string(Color::DEATH_ART) + Art::DEATH_VICTORY +
           std::string(Color::RESET) + "\n\n";

  // Middle border
  frame += std::string(Color::DEATH_DIM) + std::string(Color::DEATH_ART) +
           std::string(40, '~') + std::string(Color::RESET) + "\n\n";

  // Death's words
  frame += std::string(Color::DEATH_DIM) + std::string(Color::DEATH_QUOTE) +
           "NO NEED TO GET COMFY.\n"
           "WE WILL MEET AGAIN SOON.\n\n"
           "BUT SINCE YOU ARE HERE...\n\n" +
           std::string(Color::RESET);

  // Score summary — yellow for visibility
  frame += std::string(Color::DEATH_SUMMARY) +
           std::format("  Levels descended : {:>3}\n", levels_descended) +
           std::format("  Mimics defeated  : {:>3}\n", mimics_defeated) +
           std::format("  Chests looted    : {:>3}\n", chests_looted) +
           std::string(Color::RESET) + "\n";

  // Death's closing remark
  frame += std::string(Color::DEATH_DIM) + std::string(Color::DEATH_QUOTE) +
           "\"SIGHT\"\n\n"
           "THE DUNGEON WILL BE READY\n"
           "WHEN YOU ARE.\n" +
           std::string(Color::RESET) + "\n\n";

  // Bottom border
  frame += std::string(Color::DEATH_DIM) + std::string(Color::DEATH_ART) +
           std::string(40, '~') + std::string(Color::RESET) + "\n\n";

  // Prompt
  frame += std::string(Color::DEATH_DIM) +
           "[ Press any key to face the dungeon again... ]" +
           std::string(Color::RESET) + "\n";

  ::write(STDOUT_FILENO, frame.data(), frame.size());

  Terminal::read_key();
}

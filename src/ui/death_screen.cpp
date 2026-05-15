#include "ui/death_screen.hpp"
#include "terminal.hpp"
#include "ui/art.hpp"

static constexpr std::string_view DEATH_ART =
    "\033[36m"; // cyan — Death himself
static constexpr std::string_view DEATH_QUOTE = "\033[37m"; // white — his words
static constexpr std::string_view DEATH_DIM = "\033[2m";    // dim — the void
static constexpr std::string_view RESET = "\033[0m";

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
            "\"SIGH\"\n\n"
            "PLEASE, FOLLOW ME TO THE LOBBY.";
    break;

  default:
    quote = "EVERYONE DIES.\n"
            "MOST PEOPLE MANAGE IT\n"
            "WITH MORE DIGNITY THAN THIS.\n\n"
            "\"SIGH\"\n\n"
            "PLEASE, FOLLOW ME TO THE LOBBY.";
    break;
  }

  Terminal::full_clear();

  // Build the death screen as one frame
  std::string frame;

  // Separator — dim cyan line across the top
  frame += std::string(DEATH_DIM) + std::string(DEATH_ART) +
           std::string(40, '~') + std::string(RESET) + "\n\n";

  // Art — cyan
  frame += std::string(DEATH_ART) + art + std::string(RESET) + "\n\n";

  // Separator
  frame += std::string(DEATH_DIM) + std::string(DEATH_ART) +
           std::string(40, '~') + std::string(RESET) + "\n\n";

  // Death's words — white, slightly dim for gravitas
  frame += std::string(DEATH_DIM) + std::string(DEATH_QUOTE) + quote +
           std::string(RESET) + "\n\n";

  // Separator
  frame += std::string(DEATH_DIM) + std::string(DEATH_ART) +
           std::string(40, '~') + std::string(RESET) + "\n\n";

  // Prompt — dim, unobtrusive
  frame += std::string(DEATH_DIM) + "[ Press any key to follow... ]" +
           std::string(RESET) + "\n";

  ::write(STDOUT_FILENO, frame.data(), frame.size());

  // Wait for any key — no choice needed, Death is not optional
  Terminal::read_key();
}

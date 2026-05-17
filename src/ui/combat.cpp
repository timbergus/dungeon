#include "ui/combat.hpp"

#include "terminal.hpp"
#include "ui/color.hpp"
#include "utils/dice.hpp"
#include <format>
#include <unistd.h>

static std::string repeat(std::string_view s, int count) {
  std::string result;
  for (int i = 0; i < count; ++i)
    result += s;
  return result;
}

// ── Screen renderer
// ───────────────────────────────────────────────────────────

static void render_combat(const CombatState &state) {
  Terminal::full_clear();

  std::string frame;

  // Header
  frame += std::string(Color::DEATH_ART) + repeat("═", 40) +
           std::string(Color::RESET) + "\n";

  frame += std::string(Color::TITLE) +
           std::format("  ⚔  {} vs {}\n", state.player.name, state.enemy.name) +
           std::string(Color::RESET);

  frame += std::string(Color::DEATH_ART) + repeat("═", 40) +
           std::string(Color::RESET) + "\n\n";

  // Enemy art
  frame += std::string(Color::DEATH_ART) + state.enemy.art +
           std::string(Color::RESET) + "\n\n";

  // HP bars
  auto hp_bar = [](const Being &b) -> std::string {
    int filled = (b.health * 20) / b.max_health;
    std::string bar = "[";
    for (int i = 0; i < 20; ++i)
      bar += (i < filled) ? "█" : "░";
    bar += "]";
    return bar;
  };

  frame +=
      std::string(Color::DANGER) +
      std::format("  {} HP: {} {}/{}\n", state.enemy.name, hp_bar(state.enemy),
                  state.enemy.health, state.enemy.max_health) +
      std::string(Color::RESET);

  frame += std::string(Color::SPECIAL) +
           std::format("  {} HP: {} {}/{}\n", state.player.name,
                       hp_bar(state.player), state.player.health,
                       state.player.max_health) +
           std::string(Color::RESET);

  // Stamina bars
  frame += std::string(Color::LOOT) +
           std::format("  Stamina: {}/{}\n\n", state.player.stamina,
                       state.player.max_stamina) +
           std::string(Color::RESET);

  // Separator
  frame += std::string(Color::BORDER) + repeat("─", 40) +
           std::string(Color::RESET) + "\n\n";

  // Combat log — last 8 events
  std::size_t start = state.log.size() > 8 ? state.log.size() - 8 : 0;

  for (std::size_t i = start; i < state.log.size(); ++i) {
    frame += std::string(state.log[i].colour) + "  " + state.log[i].message +
             "\n" + std::string(Color::RESET);
  }

  frame += "\n";
  ::write(STDOUT_FILENO, frame.data(), frame.size());
}

static int interactive_roll(CombatState &state, int sides,
                            const std::string &label, std::mt19937 &rng) {
  render_combat(state);

  // Prompt
  std::string prompt = std::string(Color::BORDER) + "  [ Press R to roll " +
                       label + " ] " + std::string(Color::RESET);
  ::write(STDOUT_FILENO, prompt.data(), prompt.size());

  // Wait for R
  char key = '\0';
  while (key != 'r' && key != 'R') {
    key = Terminal::read_key();
  }

  int result = Dice::roll(sides, rng);

  state.add_event(std::format("🎲 {} → {}", label, result), Color::SPECIAL);

  return result;
}

static bool check_parry(CombatState &state, const std::string &defender_name,
                        bool defender_can_parry, BalanceState &attacker_balance,
                        std::mt19937 &rng);

static bool resolve_turn(CombatState &state, Being &attacker, Being &defender,
                         const std::string &attacker_name,
                         const std::string &defender_name,
                         bool defender_can_parry,
                         BalanceState &attacker_balance, std::mt19937 &rng) {
  // Check if attacker is off balance — apply penalty
  int balance_penalty = 0;
  if (attacker_balance == BalanceState::OffBalance) {
    balance_penalty = state.modifiers.balance_bonus;
    attacker_balance = BalanceState::Stable; // reset regardless
    state.modifiers.balance_bonus = 0;
    state.add_event(std::format("{} is still off balance! +{} damage incoming.",
                                attacker_name, balance_penalty),
                    Color::DANGER);
  }

  // Attack roll
  int atk = interactive_roll(state, 20, attacker_name + " D20 attack", rng) +
            attacker.attack;
  int def = interactive_roll(state, 20, defender_name + " D20 defense", rng) +
            static_cast<int>(defender.effective_resistance());

  if (atk > def) {
    // Check parry before applying damage
    if (check_parry(state, defender_name, defender_can_parry, attacker_balance,
                    rng)) {
      // Parried — no damage, attacker is now off balance
      attacker.exhaust(15); // parry costs more stamina
      return defender.is_alive();
    }

    // Hit — roll damage + balance penalty
    int dmg = Dice::D6(rng) + balance_penalty;

    if (balance_penalty > 0) {
      state.add_event(std::format("🎲 D6 damage + {} balance penalty = {}",
                                  balance_penalty, dmg),
                      Color::DANGER);
    }

    defender.take_damage(dmg);
    state.add_event(std::format("{} hits {} for {} damage! (HP: {})",
                                attacker_name, defender_name, dmg,
                                defender.health),
                    Color::DANGER);

    // Double attack check
    if (attacker.can_double_attack(rng)) {
      state.add_event(attacker_name + " strikes again!", Color::LOOT);

      int atk2 = interactive_roll(state, 20,
                                  attacker_name + " D20 (second strike)", rng) +
                 attacker.attack;
      int def2 =
          interactive_roll(state, 20, defender_name + " D20 defense", rng) +
          static_cast<int>(defender.effective_resistance());

      if (atk2 > def2) {
        // Check parry on second strike too
        if (!check_parry(state, defender_name, defender_can_parry,
                         attacker_balance, rng)) {
          int dmg2 = Dice::D6(rng);
          defender.take_damage(dmg2);
          state.add_event(std::format("Second strike hits for {}! (HP: {})",
                                      dmg2, defender.health),
                          Color::DANGER);
        }
      } else {
        state.add_event(defender_name + " deflects the second strike!",
                        Color::SPECIAL);
      }
    }
  } else {
    state.add_event(std::format("{} swings but {} deflects the blow!",
                                attacker_name, defender_name),
                    Color::SPECIAL);
  }

  attacker.exhaust();
  return defender.is_alive();
}

InteractionResult run_combat(Player &player, Enemy &enemy, std::mt19937 &rng,
                             bool player_has_initiative) {
  CombatState state{player, enemy};

  // Mimic taunt from variant data
  std::visit(
      overloaded{
          [&](const Mimic &m) { state.add_event(m.taunt, Color::DANGER); },
          [&](const Goblin &) {
            state.add_event("The Goblin snarls at you!", Color::DANGER);
          },
          [&](const Mage &) {
            state.add_event("The Mage raises their staff...", Color::DANGER);
          },
      },
      enemy.kind);

  // Initiative roll
  state.add_event("Rolling for initiative...", Color::TITLE);

  int player_init = interactive_roll(state, 6, "Your D6 initiative", rng);
  int enemy_init = interactive_roll(state, 6, "Enemy D6 initiative", rng);

  if (player_has_initiative || player_init >= enemy_init) {
    state.add_event("You have initiative! You strike first.", Color::LOOT);
  } else {
    state.add_event("The enemy has initiative! They strike first.",
                    Color::DANGER);
    player_has_initiative = false;
  }

  // Combat loop
  while (player.is_alive() && enemy.is_alive()) {
    if (player_has_initiative) {
      // Player turn
      state.add_event("── YOUR TURN ──", Color::TITLE);
      bool enemy_alive =
          resolve_turn(state, player, enemy, player.name, enemy.name,
                       enemy.has_parry(), state.modifiers.player_balance, rng);
      if (!enemy_alive)
        break;

      // Enemy turn
      state.add_event("── ENEMY'S TURN ──", Color::DANGER);
      resolve_turn(state, enemy, player, enemy.name, player.name,
                   false, // player can't parry yet — future ability?
                   state.modifiers.enemy_balance, rng);
    } else {
      // Enemy goes first
      state.add_event("── ENEMY'S TURN ──", Color::DANGER);
      bool player_alive =
          resolve_turn(state, enemy, player, enemy.name, player.name,
                       false, // player can't parry yet — future ability?
                       state.modifiers.enemy_balance, rng);
      if (!player_alive)
        break;

      state.add_event("── YOUR TURN ──", Color::TITLE);
      resolve_turn(state, player, enemy, player.name, enemy.name,
                   enemy.has_parry(), state.modifiers.player_balance, rng);

      // After first round initiative resets
      player_has_initiative = true;
    }

    // Stamina recovery between rounds
    player.recover();
    enemy.recover();
  }

  // Outcome
  render_combat(state);

  if (player.is_alive()) {
    state.add_event("You defeated the " + enemy.name + "!", Color::LOOT);
    render_combat(state);

    std::string msg = std::string(Color::LOOT) + "\n  [ " + enemy.name +
                      " defeated! Press any key... ]" +
                      std::string(Color::RESET);
    ::write(STDOUT_FILENO, msg.data(), msg.size());
    Terminal::read_key();

    return InteractionResult::MimicFight; // player won
  }

  // Player died — show death screen from main
  return InteractionResult::PlayerDied;
}

static bool check_parry(CombatState &state, const std::string &defender_name,
                        bool defender_can_parry, BalanceState &attacker_balance,
                        std::mt19937 &rng) {
  if (!defender_can_parry)
    return false;

  // 2D20 both over 14 — about 12% chance
  int r1 = interactive_roll(state, 20, defender_name + " parry D20 (1/2)", rng);
  int r2 = interactive_roll(state, 20, defender_name + " parry D20 (2/2)", rng);

  if (r1 > 14 && r2 > 14) {
    state.add_event(
        std::format("⚡ PARRY! {} deflects the blow perfectly!", defender_name),
        Palette::BRIGHT_CYAN);
    state.add_event("You stumble — off balance! Next attack is punished.",
                    Color::DANGER);

    // Roll balance penalty damage now — applied on next hit
    int penalty = Dice::D6(rng);
    state.modifiers.balance_bonus = penalty;
    attacker_balance = BalanceState::OffBalance;

    state.add_event(
        std::format("Balance penalty: {} bonus damage on next hit.", penalty),
        Color::DANGER);
    return true;
  }

  return false;
}

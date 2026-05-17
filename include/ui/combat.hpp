#pragma once

#include "entities/enemy.hpp"
#include "entities/player.hpp"
#include "ui/color.hpp"
#include "ui/interactions.hpp"
#include <random>

enum class BalanceState { Stable, OffBalance };

struct CombatModifiers {
  BalanceState player_balance = BalanceState::Stable;
  BalanceState enemy_balance = BalanceState::Stable;
  int balance_bonus = 0; // extra damage while off balance
};

// A single line in the combat log
struct CombatEvent {
  std::string message;
  std::string_view colour = Color::OPTION;
};

// The full combat screen state
struct CombatState {
  Player &player;
  Enemy &enemy;
  std::vector<CombatEvent> log = {};
  CombatModifiers modifiers = {};
  bool player_turn = true;

  void add_event(std::string msg, std::string_view colour = Color::OPTION) {
    log.push_back({std::move(msg), colour});
  }
};

// Run a full combat encounter
// Returns PlayerDied or MimicFight(won) via InteractionResult
InteractionResult run_combat(Player &player, Enemy &enemy, std::mt19937 &rng,
                             bool player_has_initiative);

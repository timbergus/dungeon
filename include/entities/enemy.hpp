#pragma once

#include "entities/being.hpp"
#include "overloaded.hpp"
#include <string>
#include <utility>
#include <variant>

// Unique per-enemy data -----------------------------------------------

struct Mimic {
  std::string taunt = "Were those teeth there before?";
  bool can_parry = true;    // Mimics are surprisingly nimble
  bool is_open = false;     // lid open — revealed
  bool is_defeated = false; // combat over
};

struct Goblin {
  bool is_cowardly = true;
  bool can_parry = false; // Goblins just flail wildly
};

struct Mage {
  int mana = 30;
  int fireball_dmg = 15;
  bool can_parry = false; // Mages use magic, not swordsmanship
};

using EnemyKind = std::variant<Mimic, Goblin, Mage>;

// Enemy inherits Being ------------------------------------------------

struct Enemy : Being {
  EnemyKind kind;

  std::string description;
  std::string art;

  Enemy(std::string name, std::string description, EnemyKind kind,
        int max_health, int resistance = 3, int attack = 4,
        int max_stamina = 80, int max_carried_weight = 50)
      : Being(std::move(name), max_health, resistance, attack, max_stamina,
              max_carried_weight),
        kind{std::move(kind)}, description{std::move(description)}, art{} {}

  bool has_parry() const {
    return std::visit(overloaded{
                          [](const Mimic &m) { return m.can_parry; },
                          [](const Goblin &g) { return g.can_parry; },
                          [](const Mage &m) { return m.can_parry; },
                      },
                      kind);
  }
};

// Factory functions - create named enemy types ------------------------

Enemy make_mimic();
Enemy make_goblin();
Enemy make_mage();

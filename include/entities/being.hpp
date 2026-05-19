#pragma once

#include <algorithm>
#include <random>
#include <string>
#include <utility>

struct Being {
  std::string name;
  int health;
  int max_health;
  int resistance; // Innate damage mitigation.
  int stamina;
  int max_stamina;
  int attack;
  int shield;                   // Always 0 at creation. Added by items.
  int carried_weight = 0;       // Current carried weight.
  int max_carried_weight = 100; // Max carried weight.

  Being(std::string name, int max_health, int resistance = 5, int attack = 3,
        int max_stamina = 100, int max_carried_weight = 100)
      : name{std::move(name)}, health{max_health}, max_health{max_health},
        resistance{resistance}, stamina{max_stamina}, max_stamina{max_stamina},
        attack{attack}, shield{0}, carried_weight{0},
        max_carried_weight{max_carried_weight} {}

  bool is_alive() const { return health > 0; }

  // Effective max stamina - reduced by carried weight.
  double stamina_capacity() const {
    double weight_ratio = static_cast<double>(carried_weight) /
                          static_cast<double>(max_carried_weight);
    return static_cast<double>(max_stamina) * (1.0 - weight_ratio);
  }

  // Effective resistance - scales with stamina percentage.
  double effective_resistance() const {
    double capacity = stamina_capacity();
    if (capacity <= 0) {
      return 0; // Completely encumbered.
    }
    double stamina_ratio = static_cast<double>(stamina) / capacity;
    stamina_ratio = std::min(1.0, stamina_ratio); // Cap at 100%.
    return static_cast<double>(resistance) * stamina_ratio;
  }

  // Apply incoming damage through the full pipeline.
  void take_damage(int incoming) {
    // Step 1 - resistance reduces incoming.
    int after_resist =
        std::max(0, incoming - static_cast<int>(effective_resistance()));

    // Step 2 - Shield absorbs remainder.
    int absorbed = std::min(after_resist, shield);
    shield = std::max(0, shield - absorbed);

    // Step 3 - Health takes the rest.
    health = std::max(0, health - (after_resist - absorbed));
  }

  // Consume stamina for an attack.
  void exhaust(int cost = 10) { stamina = std::max(0, stamina - cost); }

  // Check if stamina allows a double attack.
  bool can_double_attack(std::mt19937 &rng) const {
    double capacity = stamina_capacity();
    if (capacity <= 0.0) {
      return false;
    }
    std::uniform_real_distribution<double> roll{0.0, 1.0};
    double chance = static_cast<double>(stamina) / capacity;
    chance = std::min(1.0, chance);
    return roll(rng) < chance;
  }

  // Recover stamina slightly each turn.
  void recover(int amount = 5) {
    int cap = static_cast<int>(stamina_capacity());
    stamina = std::min(cap, stamina + amount);
  }
};

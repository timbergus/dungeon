#include "entities/enemy.hpp"
#include "ui/art.hpp"

Enemy make_mimic() {
  Enemy e{
      "Mimic", "A chest that bites back.", Mimic{},
      40, // Max health
      2,  // Resistance - chest aren't agile
      8,  // But they hit hard
      60, // Max stamina
      20  // Max carried weight
  };

  e.art = Art::MIMIC;

  return e;
};

Enemy make_goblin() {
  Enemy e{"Goblin", "Small, green, and surprisingly irritating.",
          Goblin{}, 25,
          3,        4,
          90,       30};

  return e;
};

Enemy make_mage() {
  Enemy e{"Mage", "Probably failed his exams at wizard school.",
          Mage{}, 35,
          5,      6,
          70,     40};

  return e;
};

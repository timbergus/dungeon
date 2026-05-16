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
  Enemy e{
      "Goblin", "Small, green, and surprisingly irritating.", Goblin{},
      25, // Max health
      3,  // Resistance - chest aren't agile
      4,  // But they hit hard
      90, // Max stamina
      30  // Max carried weight
  };

  return e;
};

Enemy make_mage() {
  Enemy e{
      "Mage", "Probably failed his exams at wizard school.", Mage{},
      35, // Max health
      5,  // Resistance - chest aren't agile
      6,  // But they hit hard
      70, // Max stamina
      40  // Max carried weight
  };

  return e;
};

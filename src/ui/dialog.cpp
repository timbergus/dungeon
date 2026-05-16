#include "ui/dialog.hpp"
#include "terminal.hpp"
#include "ui/color.hpp"
#include <string>

DialogResult show_dialog(const Dialog &dialog) {
  Terminal::full_clear();

  std::string frame;

  // Art
  frame += dialog.art + "\n";

  // Title
  frame += std::string(Color::TITLE) + dialog.title +
           std::string(Color::RESET) + "\n\n";

  // Options
  for (const auto &opt : dialog.options) {
    frame += std::string(Color::BORDER) + "[" + std::string(Color::RESET) +
             std::string(Color::OPTION) + opt.key + std::string(Color::RESET) +
             std::string(Color::BORDER) + "] " + std::string(Color::RESET) +
             opt.label + " — " + opt.description + "\n";
  }

  frame += "\n" + std::string(Color::BORDER) +
           "Your choice: " + std::string(Color::RESET);

  ::write(STDOUT_FILENO, frame.data(), frame.size());

  // Read choice
  char key = Terminal::read_key();

  for (std::size_t i = 0; i < dialog.options.size(); ++i) {
    if (dialog.options[i].key == std::string(1, key))
      return i;
  }

  return std::nullopt; // Escape or unrecognized key
}

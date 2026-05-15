#include "ui/dialog.hpp"
#include "terminal.hpp"
#include <string>
#include <string_view>

static constexpr std::string_view BORDER = "\033[36m"; // cyan border
static constexpr std::string_view TITLE = "\033[33m";  // yellow title
static constexpr std::string_view OPTION = "\033[37m"; // white options
static constexpr std::string_view RESET = "\033[0m";

DialogResult show_dialog(const Dialog &dialog) {
  Terminal::full_clear();

  std::string frame;

  // Art
  frame += dialog.art + "\n";

  // Title
  frame += std::string(TITLE) + dialog.title + std::string(RESET) + "\n\n";

  // Options
  for (const auto &opt : dialog.options) {
    frame += std::string(BORDER) + "[" + std::string(RESET) +
             std::string(OPTION) + opt.key + std::string(RESET) +
             std::string(BORDER) + "] " + std::string(RESET) + opt.label +
             " — " + opt.description + "\n";
  }

  frame += "\n" + std::string(BORDER) + "Your choice: " + std::string(RESET);

  ::write(STDOUT_FILENO, frame.data(), frame.size());

  // Read choice
  char key = Terminal::read_key();

  for (std::size_t i = 0; i < dialog.options.size(); ++i) {
    if (dialog.options[i].key == std::string(1, key))
      return i;
  }

  return std::nullopt; // Escape or unrecognized key
}

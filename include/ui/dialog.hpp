#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

struct DialogOption {
  std::string key;
  std::string label;
  std::string description;
};

struct Dialog {
  std::string title;
  std::string art;
  std::vector<DialogOption> options;
};

using DialogResult = std::optional<std::size_t>;

DialogResult show_dialog(const Dialog &dialog);

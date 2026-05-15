#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

struct DialogOptions {
  std::string key;
  std::string label;
  std::string description;
};

struct Dialog {
  std::string title;
  std::string art;
  std::vector<DialogOptions> options;
};

using DialogResult = std::optional<std::size_t>;

DialogResult show_dialog(const Dialog &dialog);

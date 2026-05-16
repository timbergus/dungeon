#pragma once

#include <string_view>

// Layer 1 - ZX Spectrum raw palette --------------------------------------

namespace Palette {
// Normal intensity
inline constexpr std::string_view BLACK = "\033[30m";
inline constexpr std::string_view RED = "\033[31m";
inline constexpr std::string_view GREEN = "\033[32m";
inline constexpr std::string_view YELLOW = "\033[33m";
inline constexpr std::string_view BLUE = "\033[34m";
inline constexpr std::string_view MAGENTA = "\033[35m";
inline constexpr std::string_view CYAN = "\033[36m";
inline constexpr std::string_view WHITE = "\033[37m";

// Bright intensity
inline constexpr std::string_view BRIGHT_BLACK = "\033[90m";
inline constexpr std::string_view BRIGHT_RED = "\033[91m";
inline constexpr std::string_view BRIGHT_GREEN = "\033[92m";
inline constexpr std::string_view BRIGHT_YELLOW = "\033[93m";
inline constexpr std::string_view BRIGHT_BLUE = "\033[94m";
inline constexpr std::string_view BRIGHT_MAGENTA = "\033[95m";
inline constexpr std::string_view BRIGHT_CYAN = "\033[96m";
inline constexpr std::string_view BRIGHT_WHITE = "\033[97m";

// Modifiers
inline constexpr std::string_view DIM = "\033[2m";
inline constexpr std::string_view BOLD = "\033[1m";
inline constexpr std::string_view RESET = "\033[0m";
} // namespace Palette

// Layer 2 - Semantic aliases ---------------------------------------------

namespace Color {
// UI chrome
inline constexpr std::string_view BORDER = Palette::CYAN;
inline constexpr std::string_view TITLE = Palette::YELLOW;
inline constexpr std::string_view OPTION = Palette::WHITE;
inline constexpr std::string_view PROMPT = Palette::CYAN;

// Map tiles
inline constexpr std::string_view FLOOR = Palette::WHITE;
inline constexpr std::string_view WALL = Palette::WHITE;
inline constexpr std::string_view DOOR = Palette::GREEN;
inline constexpr std::string_view STAIRS = Palette::CYAN;
inline constexpr std::string_view LOOT = Palette::YELLOW;
inline constexpr std::string_view DANGER = Palette::RED;
inline constexpr std::string_view SPECIAL = Palette::BRIGHT_CYAN;

// Death screen
inline constexpr std::string_view DEATH_ART = Palette::CYAN;
inline constexpr std::string_view DEATH_QUOTE = Palette::WHITE;
inline constexpr std::string_view DEATH_SUMMARY = Palette::YELLOW;
inline constexpr std::string_view DEATH_DIM = Palette::DIM;

// Fog of war
inline constexpr std::string_view FOG = Palette::DIM;

// Always needed
inline constexpr std::string_view RESET = Palette::RESET;
inline constexpr std::string_view DIM = Palette::DIM;
inline constexpr std::string_view BOLD = Palette::BOLD;
} // namespace Color

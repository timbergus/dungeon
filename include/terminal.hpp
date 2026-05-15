#pragma once

#include <termios.h>
#include <unistd.h>

struct Terminal {
  Terminal() {
    // Save the current terminal settings
    tcgetattr(STDIN_FILENO, &original);

    // Build the raw mode settings
    termios raw = original;
    raw.c_lflag &= ~static_cast<tcflag_t>(
        ECHO | ICANON);  // disable echo and line buffering
    raw.c_cc[VMIN] = 1;  // read blocks until 1 char is available
    raw.c_cc[VTIME] = 0; // no timeout

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  }

  ~Terminal() {
    // Automatically restore original settings when Terminal is destroyed
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
  }

  // Delete copy and move — this resource should not be duplicated
  Terminal(const Terminal &) = delete;
  Terminal &operator=(const Terminal &) = delete;

  static char read_key() {
    char c{};
    ::read(STDIN_FILENO, &c, 1);
    return c;
  }

  static void clear() { ::write(STDOUT_FILENO, "\033[H", 3); }

private:
  termios original{};
};


#include <iostream>
#include "ui/cli.hpp"

int main() {
  try {
    CLI::demo();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

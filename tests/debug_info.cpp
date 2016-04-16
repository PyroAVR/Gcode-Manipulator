#include "../RS274.hpp"
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>

int main(int argc, char* argv[])  {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <input> " << std::endl;
    return -1;
  }
  RS274 *g = new RS274(argv[1], "");
  std::cout << "initialized" << std::endl;
  std::cout << g->run() << std::endl;
}

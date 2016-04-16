#include "../RS274.hpp"
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[])  {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <input> <lineno1> <lineno2>" << std::endl;
    return 0;
  }
  
}

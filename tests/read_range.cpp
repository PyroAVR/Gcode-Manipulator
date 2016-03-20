#include <RS274/RS274.hpp>
#include <cstdlib>
int main(int argc, char* argv[])  {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <input> <lineno1> <lineno2>" << std::endl;
    return 0;
  }
  RS274 *g = new RS274(std::string(argv[1]), "");
  g->parse();
  for(int i = atoi(argv[2]); i <= atoi(argv[3]); i++) {
    std::cout << g->readElement(i-1) << std::endl;
  }
  return 0;
}

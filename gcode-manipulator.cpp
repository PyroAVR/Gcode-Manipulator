#include <cstdlib>
#include <string>
#include "RS274.hpp"
using namespace std;
int main(int argc, char *argv[]) {
 if (argc < 5)	{
  cout << "Usage: " << argv[0] << " <input> <output> <dX> <dY> <dZ>" << endl;
  return 0;
  }
  RS274 *g(std::string(argv[1]), std::string(argv[2]));
  g->run();
  g->shift(atoi(argv[3]),atoi(argv[4]),atoi(argv[5]));
  g->write();
  return 0;
}

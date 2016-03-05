#include <cstdlib>
#include "RS274.hpp"
using namespace std;
int main(int argc, char const *argv[]) {
 if (argc < 5)	{
  cout << "Usage: " << argv[0] << " <input> <output> <dX> <dY> <dZ>" << endl;
  return 0;
}
 RS274 g;
  g.parse(argv[1]);
  g.shift(atoi(argv[3]),atoi(argv[4]),atoi(argv[5]));
  g.write(argv[2]);
  return 0;
}

#include "RS274.hpp"
using namespace std;
int main(int argc, char const *argv[]) {
  RS274 g;
  cout << g.parseLine("G90 Y50 Y20") << endl;
  return 0;
}

#include "RS274.hpp"
using namespace std;
int main(int argc, char const *argv[]) {
  RS274 g;
  g.parse("test.ngc");
  g.shift(10,5,-2.53);
  g.write("out.ngc");
  return 0;
}

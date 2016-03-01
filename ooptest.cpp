#include "RS274.hpp"
using namespace std;
int main(int argc, char const *argv[]) {
  RS274 g;
  g.parse("test.ngc");
  g.write("out.ngc");
  return 0;
}

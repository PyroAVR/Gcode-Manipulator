#include <cstdlib>
#include <string>
#include "RS274.hpp"
using namespace std;
int main(int argc, char *argv[]) {
 if (argc < 6)	{
  cout << "Usage: " << argv[0] << " <input> <output> <dX> <dY> <dZ>" << endl;
  return 0;
  }
  RS274 *g = new RS274(std::string(argv[1]), std::string(argv[2]));
  g->run();
  threadWorkerData tw;
  tw.instructionMatrix = g->getInstructionMatrix();
  for(auto a : tw.instructionMatrix) {
  std::cout << "Line " << a.lineno << ": Command/Special : "
   << a.command << "/"
   << a.specialCommand
   << " X: " << std::to_string(a.xCoord)
   << " Y: " << std::to_string(a.yCoord)
   << " Z: " << std::to_string(a.zCoord)
   << "Comment: " << a.comment
   << "Modal:" << std::to_string(a.isModal)
   << "Motion:" << std::to_string(a.isMotion)
   << std::endl;
 }
  g->shift(atoi(argv[3]),atoi(argv[4]),atoi(argv[5]));
  g->write();
  return 0;
}

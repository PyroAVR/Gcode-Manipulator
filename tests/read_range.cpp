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
  std::cout << "Working... please wait.";
  threadWorkerData tw;
  std::ifstream input;
  std::string line;
  tw.id = 0;
  input.open(argv[1], std::fstream::in);
  while(!input.eof() && !input.bad()) {
    std::getline(input, line, '\n');
    tw.lines.push_back(line);
  }
  RS274Worker *rw = new RS274Worker(tw);
  rw->parseRange(atoi(argv[2]), atoi(argv[3]));
  tw = rw->getParsedData();
  std::cout << " Done!" << std::endl;
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
}

#include "../RS274.hpp"
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>

std::mutex datamutex;

//this locking scheme WILL NOT WORK, data MUST be copied out, worked on, and then re-inserted.
void rangeParseThread(RS274* g, int start, int end, int& finished) {
  finished = 0;
  std::lock_guard<std::mutex> guard(datamutex);
  g->parseRange(start, end);
  finished = 1;
}



int main(int argc, char* argv[])  {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <input> <lineno1> <lineno2>" << std::endl;
    return 0;
  }
  RS274 *g = new RS274(std::string(argv[1]), "");
  std::cout << g->prepareThreadWorkers() << std::endl;
  std::vector<threadWorkerData> t;
  for(int i = 1; i <=4; i++)  {
    t.push_back(g->getThreadWorkerInstance(i));
    //for(auto a : t.lines) std::cout << a << std::endl;
    std::cout << t.end()->size << std::endl;
    //std::cout << t.id << std::endl;
  }

  return 0;
}

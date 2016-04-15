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
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <input> " << std::endl;
    return 0;
  }
  return 0;
}

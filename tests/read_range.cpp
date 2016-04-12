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
  int maxLinesPerJob = 99;
  std::vector<std::thread> threads;
  std::vector<int> tlock;

  RS274 *g = new RS274(std::string(argv[1]), "");
/*  if ((atoi(argv[3]) - atoi(argv[2])) < 10) {
    //g->parse();
  }*/
  //else  {
    int start = atoi(argv[2]), end = atoi(argv[3]);
    int range = end-start;
    int numjobs = range/maxLinesPerJob;
    int nthreads = std::thread::hardware_concurrency();
    std::cout << nthreads << " Threads will be used" << " with " << numjobs << " total jobs" << std::endl
    << "range: " << range << "start: " << start << "end: " << end << std::endl;
    tlock.push_back(0);
    threads.push_back(
      std::thread(rangeParseThread, g, start, maxLinesPerJob, std::ref(tlock[0]))
    );
    int lastjend = maxLinesPerJob;
    if(numjobs > 1) {
    for(int i = 1; i <= range; i++) {
      int jstart = lastjend + 1;
      int jend = jstart + maxLinesPerJob;
      lastjend = jend;
      tlock.push_back(0);
      threads.push_back(
        std::thread(rangeParseThread, g, jstart, jend, std::ref(tlock[i]))
      );
  }
}
  for(int i = 0; i < threads.size(); i++) { threads[i].detach();}
  /*int tcount = 0;
  while(tcount < numjobs)  {
    tcount = 0;
    for(auto i : tlock) {
      tcount += i;
    }

  }*/
  std::this_thread::sleep_for(std::chrono::seconds(5));
  for(int i = 1; i < g->size(); i++) {
    std::cout << g->readElement(i-1) << std::endl;
  }
  return 0;
}

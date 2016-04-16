#include "../RS274.hpp"
#include <thread>
#include <fstream>
#include <string>
#include <cstdlib>

void runtw(RS274Worker& w) {
  w.run();
  w.parseRange(0,12);

}


int main(int argc, char* argv[])  {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <input> <lineno1> <lineno2>" << std::endl;
    return 0;
  }
  int blocksize = atoi(argv[3]) - atoi(argv[2]);
  char* line = new char[255];
  std::ifstream input;
  input.open(argv[1], std::fstream::in);
  std::vector<std::string> l;
  std::cout << "opening file" << std::endl;
  while(!input.eof()) {
    input.getline(line, 255, '\n');
    l.push_back(std::string(line));
  }
  std::cout << "closing file" << std::endl;
  input.close();
  std::vector<threadWorkerData> twds;
  int hwt = std::thread::hardware_concurrency();
  if(l.size() > 100)  {
    int nl = blocksize/hwt;
    std::cout << "lines per job: " << nl << std::endl;
    std::vector<std::string>::iterator it = l.begin() + nl;
    threadWorkerData t;
    t.id = 0;
    std::copy(l.begin(), it, back_inserter(t.lines));
    twds.push_back(t);
    for(int i = 1; i < hwt - 1; i++)  {
      threadWorkerData tw;
      tw.id = i;
      it++;
      std::copy(it, it + nl, back_inserter(t.lines));
      it += nl;
      twds.push_back(tw);
    }
    t.id = hwt;
    t.lines.clear();
    std::copy(it, l.end(), back_inserter(t.lines));
    twds.push_back(t);

  }
  std::vector<RS274Worker> workers;
  for(auto i : twds) workers.push_back(RS274Worker(i));
  //std::cout << twds.size() << std::endl;
  //std::cout << twds[0].id << std::endl;
  runtw(workers[0]);
  threadWorkerData ret = workers[0].getParsedData();
  std::cout << "size of returned mat: " << ret.instructionMatrix.size() << std::endl;
  for(auto i : ret.instructionMatrix) std::cout << i.xCoord << std::endl;
  /*
  for(auto a : t.instructionMatrix) {
    std::cout << "X:" << a.xCoord << "Y:" << a.yCoord << "Z:" << a.zCoord << std::endl;
  }*/
  return 0;
}

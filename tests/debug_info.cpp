#include "../RS274.hpp"
#include <thread>
#include <string>
#include <chrono>
#include <cstdlib>

std::string prompt(std::string question, std::string prompt) {
  std::cout << question << prompt;
  std::string response;
  std::cin >> response;
  std::cout << std::endl;
  return response;
}


int main(int argc, char* argv[])  {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <input> [start] [end]" << std::endl;
    return -1;
  }
  std::cout << "Working... please wait.";
  threadWorkerData tw;
  std::ifstream input;
  std::string line;
  tw.id = 0;
  input.open(argv[1], std::fstream::in);
  if(argc < 4)  {
    while(!input.eof() && !input.bad()) {
      std::getline(input, line, '\n');
      tw.lines.push_back(line);
    }
  } else  {
    input.seekg(atoi(argv[2]));
    while(!input.eof() && !input.bad() && tw.lines.size() <= atoi(argv[3])) {
      std::getline(input, line, '\n');
      tw.lines.push_back(line);
    }
  }
  RS274Worker *rw = new RS274Worker(tw);
  rw->run();
  tw = rw->getParsedData();
  std::cout << " Done!" << std::endl;
  std::cout << tw.instructionMatrix.size() << std::endl;

}

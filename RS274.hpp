#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cmath>
#pragma once
enum {
  _crash_,
  _exit_
} exitCodes;
enum {
  _stop_,
  _working_,
  _done_
} statusCodes;
/*This struct defines what a single line of gcode looks like, but it separates
* some of the things out.
* To demonstrate:
* N042 G01 X0.5197 Y52.385 Z-3.32153 ;random comment
* Turns into:
* N042 G01 0.5197 52.385 -3.31253 ;random comment
* This is because we need the numbers to work with, not strings.
*/
//std::string Usage = "Usage: gcmanip <input> <output> <X> <Y> <Z>";
struct gInstruction {
  std::string lineno;
  std::string command;
  std::string specialCommand;
  double xCoord;
  double yCoord;
  double zCoord;
  std::string comment;
  bool isModal = false;
  bool isMotion = false;
};

struct threadWorkerData {
  int id;
  int size;
  std::vector<std::string> lines;
  std::vector<gInstruction> instructionMatrix;
};

class RS274Tokenizer  {
private:
    //Regular expressions for the various selections needed in order to parse
    //a line of gcode.
    std::regex linenoRegex = std::regex("N[\\s]?[0-9]+", std::regex::icase);                          //N<numbers>
    std::regex commandRegex = std::regex("([G][\\s]?[0-9]+[.]?[0-9]*[\\s]?)+", std::regex::icase);   //G<numbers>.<numbers> or G <numbers>.<numbers> Now supports multiple commands, ie: G91 G01
    std::regex Xregex = std::regex("X[\\s]?[\\+-]?[0-9]*[.]?[0-9]*", std::regex::icase);           //X<numbers>.<numbers> or X <numbers>.<numbers>
    std::regex Yregex = std::regex("Y[\\s]?[\\+-]?[0-9]*[.]?[0-9]*", std::regex::icase);           //Y<numbers>.<numbers> or Y <numbers>.<numbers>
    std::regex Zregex = std::regex("Z[\\s]?[\\+-]?[0-9]*[.]?[0-9]*", std::regex::icase);           //Z<numbers>.<numbers> or Z <numbers>.<numbers>
    std::regex specialRegex = std::regex("[SFPMT][\\s]?[0-9]*[.]?[0-9]*", std::regex::icase);      //SFP<numbers>.<numbers> or SFP <numbers>.<numbers>
    std::regex commentRegex = std::regex("[\(;]");                                 //comments start with ; or (
    std::regex modalRegex = std::regex("G0?(([0-3]+)?|([7-8]+)?)\\D|G(33|38\\.[1-3]|73|76|8[0-9])+\\D|G(17|18|19)\\D|G(9[0-28-9])+\\D|G(2[0-1])\\D|G(4[1-3][\\.1]+)\\D|G(4[0-39])|G(59\\.[1-3]?)\\D|G(5[3-9])", std::regex::icase);
    std::regex motionRegex = std::regex("(G38\\.[2-5])|(G0?5\\.[1-2])|(G0?[1-35]?)", std::regex::icase);
public:
  RS274Tokenizer();
  gInstruction parseLine(std::string line);
  ~RS274Tokenizer();
};

class RS274Worker  {
private:
  RS274Tokenizer rt;
  threadWorkerData twd;
  int status = _stop_;
  int setStatus(int s);
public:
  RS274Worker(int id);
  RS274Worker(threadWorkerData _twd);
  RS274Worker(int id, std::vector<std::string> _lines);
  int parseRange(int start, int end);
  int run();
  int getStatus();
  threadWorkerData getParsedData();

};

class RS274 {
private:
  std::vector<RS274Worker> workers;
  std::vector<threadWorkerData> twds;
  std::vector<std::string> linestoparse;
  std::vector<std::thread> workerthreads;
  std::vector<gInstruction> instructionMatrix;
  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::chrono::duration<double> loadtime, parsetime;
  int bufferSize = 255;
  char* inputBuffer = new char[bufferSize];               //Input buffer for one line
  char* outputBuffer = new char[bufferSize];              //Output buffer for one line
  double  Xshift, Yshift, Zshift;
  char *inputFile, outputFile;
  std::ifstream input;
  std::ofstream output;
  int hardwarethreads, jobs, linecount;
  void runWorker(RS274Worker& w);
public:
  RS274(std::string inputFile, std::string outputFile);
  int run();
  std::string readElement(int lineno);
  int shiftElement(int lineno);
  int shift(double X, double Y, double Z);
  int writeLine(int lineno);
  int write();
  int write(std::string &filename);
  int write(const char *filename);
  std::vector<gInstruction> getInstructionMatrix();
  int size();
  ~RS274();
};

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <cmath>
#pragma once
enum {
  _crash_,
  _exit_
} exitCodes;

/*This struct defines what a single line of gcode looks like, but it separates
* some of the things out.
* To demonstrate:
* N042 G01 X0.5197 Y52.385 Z-3.32153 ;random comment
* Turns into:
* N042 G01 0.5197 52.385 -3.31253 ;random comment
* This is because we need the numbers to work with, not strings.
*/
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
//send help, I don't know what I'm doing

class RS274 {
private:
  //Regular expressions for the various selections needed in order to parse
  //a line of gcode.
  std::regex linenoRegex = std::regex("N[\\s]?[0-9]+");                          //N<numbers>
  std::regex commandRegex = std::regex("([Gg][\\s]?[0-9]+[.]?[0-9]*[\\s]?)+");   //G<numbers>.<numbers> or G <numbers>.<numbers> Now supports multiple commands, ie: G91 G01
  std::regex Xregex = std::regex("[Xx][\\s]?[\\+-]?[0-9]*[.]?[0-9]*");           //X<numbers>.<numbers> or X <numbers>.<numbers>
  std::regex Yregex = std::regex("[Yy][\\s]?[\\+-]?[0-9]*[.]?[0-9]*");           //Y<numbers>.<numbers> or Y <numbers>.<numbers>
  std::regex Zregex = std::regex("[Zz][\\s]?[\\+-]?[0-9]*[.]?[0-9]*");           //Z<numbers>.<numbers> or Z <numbers>.<numbers>
  std::regex specialRegex = std::regex("[SFPMTsfpmt][\\s]?[0-9]*[.]?[0-9]*");      //SFP<numbers>.<numbers> or SFP <numbers>.<numbers>
  std::regex commentRegex = std::regex("[\(;]");                                 //comments start with ; or (
  std::regex modalRegex = std::regex("G0?(([1-3]+)?|([7-8]+)?)\\D|G(33|38\\.[1-3]|73|76|8[0-9])+\\D|G(17|18|19)\\D|G(9[0-28-9])+\\D|G(2[0-1])\\D|G(4[1-3][\\.1]+)\\D|G(4[0-39])|G(59\\.[1-3]?)\\D|G(5[3-9])", std::regex::icase);
  std::regex motionRegex = std::regex("(G38\\.[2-5])|(G0?5\\.[1-2])|(G0?[1-35]?)", std::regex::icase);
  int bufferSize = 255;                                   //Lines cannot be longer than this many characters
  int linecount = 0;                                      //size of gInstruction vector
  std::string Usage = "Usage: gcmanip <input> <output> <X> <Y> <Z>";
  char* inputBuffer = new char[bufferSize];               //Input buffer for one line
  char* outputBuffer = new char[bufferSize];              //Output buffer for one line
  double  Xshift, Yshift, Zshift;
  char *inputFile, outputFile;
  std::ifstream input;
  std::ofstream output;
  //std::vector<int> modalDomains;                  //Regions where modal commands are in effect
  //A dynamic array to hold our file in.
  std::vector<gInstruction> instructionMatrix;
  std::vector<threadWorkerData> threadDataBlocks;
  int hardwarethreads;
  void error(int status, std::string message);
public:
  RS274();
  RS274(std::string input, std::string output);
  int parseLine(std::string line);
  int parse();
  int parse(std::string &filename);
  int parse(const char* filename);
  int parseRange(int start, int end);
  std::string readElement(int lineno);
  int shiftElement(int lineno);
  int shift(double X, double Y, double Z);
  int writeLine(int lineno);
  int write();
  int write(std::string &filename);
  int write(const char *filename);
  std::vector<gInstruction> getInstructionMatrix();
  int size();
  int prepareThreadWorkers();
  int prepareThreadWorkers(std::string &filename);
  int prepareThreadWorkers(const char* filename);
  threadWorkerData getThreadWorkerInstance(int id);
  int destroyThreads();
  ~RS274();
};

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
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
};

class RS274 {
private:
  //Regular expressions for the various selections needed in order to parse
  //a line of gcode.
  std::regex linenoRegex = std::regex("N[\\s]?[0-9]+");                          //N<numbers>
  std::regex commandRegex = std::regex("([Gg][\\s]?[0-9]+[.]?[0-9]*[\\s]?)+");   //G<numbers>.<numbers> or G <numbers>.<numbers> Now supports multiple commands, ie: G91 G01
  std::regex Xregex = std::regex("[Xx][\\s]?[\\+-]?[0-9]*[.]?[0-9]*");           //X<numbers>.<numbers> or X <numbers>.<numbers>
  std::regex Yregex = std::regex("[Yy][\\s]?[\\+-]?[0-9]*[.]?[0-9]*");           //Y<numbers>.<numbers> or Y <numbers>.<numbers>
  std::regex Zregex = std::regex("[Zz][\\s]?[\\+-]?[0-9]*[.]?[0-9]*");           //Z<numbers>.<numbers> or Z <numbers>.<numbers>
  std::regex specialRegex = std::regex("[SFPMsfpm][\\s]?[0-9]*[.]?[0-9]*");      //SFP<numbers>.<numbers> or SFP <numbers>.<numbers>
  std::regex commentRegex = std::regex("[\(;]");                                 //comments start with ; or (
  int bufferSize = 255;                                   //Lines cannot be longer than this many characters
  std::string Usage = "Usage: gcmanip <input> <output> <X> <Y> <Z>";
  char* inputBuffer = new char[bufferSize];               //Input buffer for one line
  char* outputBuffer = new char[bufferSize];              //Output buffer for one line
  double  Xshift, Yshift, Zshift;
  std::ifstream input;
  std::ofstream output;
  //A dynamic array to hold our file in.
  std::vector<gInstruction> instructionMatrix;
  void error(int status, std::string message);
public:
  RS274();
  RS274(std::string filename);
  int parseLine(std::string line);
  int parse(std::string &filename);
  int parse(const char* filename);
  int shiftElement(int lineno);
  int writeLine(int lineno);
  ~RS274();
};

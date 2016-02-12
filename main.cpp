/*GCode Manipulator
 *Can shift and transform GCode.
 *Reads lines one at a time from file in order to minimize memory usage.
 *
 */

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include <cstdlib>
#include <cstring>

enum {
  _crash_,
  _exit_
} exitCodes;
//Regular expressions for the various selections needed in order to parse
//a line of gcode.
std::regex linenoRegex("N[\\s]?[0-9]+");                        //N<numbers>
std::regex commandRegex("[GM][\\s]?[0-9]+[.]?[0-9]*");         //GM<numbers>.<numbers> or GM <numbers>.<numbers>
std::regex Xregex("X[\\s]?[\\+-]?[0-9]*[.]?[0-9]*");           //X<numbers>.<numbers> or X <numbers>.<numbers>
std::regex Yregex("Y[\\s]?[\\+-]?[0-9]*[.]?[0-9]*");           //Y<numbers>.<numbers> or Y <numbers>.<numbers>
std::regex Zregex("Z[\\s]?[\\+-]?[0-9]*[.]?[0-9]*");           //Z<numbers>.<numbers> or Z <numbers>.<numbers>
std::regex specialRegex("[SFP][\\s]?[0-9]*[.]?[0-9]*");        //SFP<numbers>.<numbers> or SFP <numbers>.<numbers>
std::string commentDelimiter = ";";                     //Like assembly, gcode comments are denoted with a ;
int bufferSize = 255;                                   //Lines cannot be longer than this
std::string Usage = "Usage: gcmanip <input> <output> <X> <Y> <Z>";
char* inputBuffer = new char[bufferSize];               //Input buffer for one line
char* outputBuffer = new char[bufferSize];              //Output buffer for one line
double  Xshift, Yshift, Zshift;
std::ifstream input;
std::ofstream output;

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

//A dynamic array to hold our file in.
std::vector<gInstruction> instructionMatrix;

//Handy for easy crashing and termination.  Takes care of C-allocated resources.
void cleanup(int status, std::string message = "") {
  if(message == "") {
    delete[] inputBuffer;
    delete[] outputBuffer;
    std::exit(0);
  }
  if(status == _crash_) {
    std::cout << "Error: " << message << std::endl;
    delete[] inputBuffer;
    delete[] outputBuffer;
    std::exit(-1);
  }
  std::cout << message << std::endl;
  delete[] inputBuffer;
  delete[] outputBuffer;
}

/*
* This is the complicated one. The function takes a single line of gcode
* as an argument and splits it into many pieces.
* The first step looks for comments and separates them out, as they can
* trip up the command search step.
* The second step searches for commands, G or M commands specifically.
* The third step searches for coordinates, separates them, strips their
* designating letter <X, Y, or Z>, and finally,
* The fourth step initializes a new gInstruction, adds it to the matrix, clears
* the temporary buffer, and moves returns, waiting for the next line.
*/
int parseLine(std::string line)  {
  std::smatch linenoMatch;
  std::smatch commandMatch;
  std::smatch Xmatch;
  std::smatch Ymatch;
  std::smatch Zmatch;
  std::smatch specialMatch;
  std::string comment;
  if(line.find(commentDelimiter) <= line.length()) {
    comment = line.substr(line.find(commentDelimiter), std::string::npos);
    line = line.substr(0,line.find(commentDelimiter));
  }

  std::regex_search(line, linenoMatch, linenoRegex);
  std::regex_search(line, commandMatch, commandRegex);
  std::regex_search(line, specialMatch, specialRegex);
  std::regex_search(line, Xmatch, Xregex);
  std::regex_search(line, Ymatch, Yregex);
  std::regex_search(line, Zmatch, Zregex);
  //specials excluded from syntax checker
  if(linenoMatch.size() > 1 || commandMatch.size() > 1 || Xmatch.size() > 1 || Ymatch.size() > 1 || Zmatch.size() > 1)  {
    cleanup(_crash_, "Something's wrong with the syntax on this line:\n" + line);
  }
  /*
  * Brief explanation of the string-reassignments:
  * Something fishy is going on in the standard library whereby std::regex_match cannot be called with
  * an std::sub_match[n] as an argument, even though it should evaluate to a std::string.
  * Unfortunately this means that we have to instruct the compiler to do this another way,
  * namely by assigning the contents of an std::sub_match[n] to an std::string.
  * This has to be done many times, hopefully it gets optimized out by the compiler later
  * after it realizes its own foolishness.
  */
  std::regex coordRegex("[\\+-][0-9]*.?[0-9]*|[0-9]+.?[0-9]*");      //<numbers>.<numbers>
  std::smatch Xcoord, Ycoord, Zcoord;
  std::string XmatchString = Xmatch[0];
  std::string YmatchString = Ymatch[0];
  std::string ZmatchString = Zmatch[0];
  std::regex_search(XmatchString, Xcoord, coordRegex);
  std::regex_search(YmatchString, Ycoord, coordRegex);
  std::regex_search(ZmatchString, Zcoord, coordRegex);
  gInstruction currentLine = {linenoMatch[0], commandMatch[0], specialMatch[0], atof(Xcoord[0].str().c_str()), atof(Ycoord[0].str().c_str()), atof(Zcoord[0].str().c_str()), comment};
  instructionMatrix.push_back(currentLine);
  memset(&currentLine, NULL, sizeof(currentLine));
  return 0;
}

//Simple, apply the shifts!
int shiftElement(int lineno)  {
  /*
  * Weird, seemingly unnessary or backwards if statements:
  * Some CAM processors do not write a coordinate at all if there is no change from the previous
  * position in that axis.  Therefore, we have to not apply any shifts if there is nothing to shift.
  * To keep the program short, we check if there IS something, and then write, rather than check all
  * possible cases in which there might NOT be something. Efficiency, HAYO!
  * This also applies to writeLine.  It doesn't have to check for null commands due to this architecture.
  */
  if(instructionMatrix[lineno].command == "") return 0;          //Don't write coords if there isn't a command.
  if(instructionMatrix[lineno].xCoord != NULL) instructionMatrix[lineno].xCoord = instructionMatrix[lineno].xCoord + Xshift;
  if(instructionMatrix[lineno].yCoord != NULL) instructionMatrix[lineno].yCoord = instructionMatrix[lineno].yCoord + Yshift;
  instructionMatrix[lineno].zCoord = instructionMatrix[lineno].zCoord + Zshift; //Some transformations are skewed if Z-coordinates not present.
  return 0;
}
//Reverse of the parseLine() function, but this is much simpler.
int writeLine(int lineno) {
  std::string newLine;
  if(instructionMatrix[lineno].lineno != "") newLine += instructionMatrix[lineno].lineno + " ";                                     //N042
  newLine += instructionMatrix[lineno].command + " ";                                                                               //N042 G01 (Already checked above!)
  newLine += instructionMatrix[lineno].specialCommand + " ";
  if(instructionMatrix[lineno].xCoord != NULL) newLine += "X" + std::to_string(instructionMatrix[lineno].xCoord) + " ";             //N042 G01 X0.0525
  if(instructionMatrix[lineno].yCoord != NULL) newLine += "Y" + std::to_string(instructionMatrix[lineno].yCoord) + " ";            //N042 G01 X0.0525 Y2
  if(instructionMatrix[lineno].zCoord != NULL) newLine += "Z" + std::to_string(instructionMatrix[lineno].zCoord) + " ";            //N042 G01 X0.0525 Y2 Z-1.25
  if(instructionMatrix[lineno].comment != "") newLine += instructionMatrix[lineno].comment;
  output << newLine << std::endl;
  return 0;
}


int main(int argc, char *argv[])  {
  if(argc < 6)  cleanup(_crash_, "Not enough arguments!\n" + Usage);
  Xshift = atof(argv[3]);
  Yshift = atof(argv[4]);
  Zshift = atof(argv[5]);
  input.open(argv[1], std::fstream::in);
  output.open(argv[2], std::fstream::out);

  int linecount = 0;
  while(! input.eof())  {
    input.getline(inputBuffer, bufferSize, '\n');
    linecount++;
    std::cout << "[parse] line " << linecount << std::endl;
    parseLine(inputBuffer);
    memset(inputBuffer, NULL, bufferSize);
  }

  input.close();
  std::cout << "Input read successfully: " << linecount << " lines parsed." << std::endl;
  for(int i = 0; i <= linecount; i++) {
    std::cout << "[translate] (" << i << "/" << linecount << ") " << (static_cast<float>(i)/static_cast<float>(linecount))*100 << "%"  << std::endl;
    shiftElement(i);
    writeLine(i);
  }
  output.close();
  cleanup(_exit_, "Transformation applied!");
  return 0;

}

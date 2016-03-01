#include "RS274.hpp"

RS274::RS274()  {

}

void RS274::error(int status, std::string message) {
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

int RS274::parseLine(std::string line)  {
  std::smatch linenoMatch;
  std::smatch commandMatch;
  std::smatch Xmatch;
  std::smatch Ymatch;
  std::smatch Zmatch;
  std::smatch specialMatch;
  std::smatch commentMatch;
  std::string comment;
  std::regex_search(line, commentMatch, commentRegex);                          //Comment removal.
  if(!(commentMatch.size() < 1))  {
    comment = line.substr(line.find(commentMatch[0].str()), line.length());
    line = line.substr(0,line.find(commentMatch[0].str()));
  }
  std::regex_search(line, linenoMatch, linenoRegex);
  std::regex_search(line, commandMatch, commandRegex);
  std::regex_search(line, specialMatch, specialRegex);
  std::regex_search(line, Xmatch, Xregex);
  std::regex_search(line, Ymatch, Yregex);
  std::regex_search(line, Zmatch, Zregex);
  //specials excluded from syntax checker.  Multiple commands can be on one line, ie: G91 G00 <x><y><z>
  if(linenoMatch.size() > 1 || commandMatch.size() > 2 || Xmatch.size() > 1 || Ymatch.size() > 1 || Zmatch.size() > 1)  {
    error(_crash_, "Something's wrong with the syntax on this line:\n" + line);
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
  /* This really, REALLY needs a re-write, but it works for now.  Basically, write in a value, and then check if it was empty to account for atof() returning 0.0 on error.
   * We write in NaN if the match is blank.
   */
  if(Xcoord[0] == "") currentLine.xCoord = std::nan("1");
  if(Ycoord[0] == "") currentLine.yCoord = std::nan("1");
  if(Zcoord[0] == "") currentLine.zCoord = std::nan("1");
  instructionMatrix.push_back(currentLine);
  memset(&currentLine, 0, sizeof(currentLine));
  return 0;
}

int RS274::parse(const char* filename)  {
  input.open(filename, std::fstream::in);
  int linecount = 0;
  while(! input.eof())  {
    input.getline(inputBuffer, bufferSize, '\n');
    linecount++;
    std::cout << "[parse] line " << linecount << std::endl;
    parseLine(inputBuffer);
    memset(inputBuffer, 0, bufferSize);
  }
  return 0;
}
//Dummy!
int RS274::parse(std::string &filename)  {
  return RS274::parse(filename.c_str());
}
int RS274::shiftElement(int lineno) {
  /*
  * Weird, seemingly unnessary or backwards if statements:
  * Some CAM processors do not write a coordinate at all if there is no change from the previous
  * position in that axis.  Therefore, we have to not apply any shifts if there is nothing to shift.
  * To keep the program short, we check if there IS something, and then write, rather than check all
  * possible cases in which there might NOT be something. Efficiency, HAYO!
  */
  if(instructionMatrix[lineno].command == "") return 0;          //Don't write coords if there isn't a command.
  if(!std::isnan(instructionMatrix[lineno].xCoord)) instructionMatrix[lineno].xCoord = instructionMatrix[lineno].xCoord + Xshift;
  if(!std::isnan(instructionMatrix[lineno].yCoord)) instructionMatrix[lineno].yCoord = instructionMatrix[lineno].yCoord + Yshift;
  instructionMatrix[lineno].zCoord = instructionMatrix[lineno].zCoord + Zshift; //Some transformations are skewed if Z-coordinates not present.
  return 0;
}

int RS274::writeLine(int lineno)  {
  std::string newLine;
  if(instructionMatrix[lineno].lineno != "") newLine += instructionMatrix[lineno].lineno + " ";                                              //N042
  if(instructionMatrix[lineno].command != "") newLine += instructionMatrix[lineno].command + " ";                                           //N042 G01 The empty check is for my sanity.
  newLine += instructionMatrix[lineno].specialCommand + " ";
  if(instructionMatrix[lineno].command != "") {
    if(!std::isnan(instructionMatrix[lineno].xCoord)) newLine += "X" + std::to_string(instructionMatrix[lineno].xCoord) + " ";             //N042 G01 X0.0525
    if(!std::isnan(instructionMatrix[lineno].yCoord)) newLine += "Y" + std::to_string(instructionMatrix[lineno].yCoord) + " ";            //N042 G01 X0.0525 Y2
    if(!std::isnan(instructionMatrix[lineno].zCoord)) newLine += "Z" + std::to_string(instructionMatrix[lineno].zCoord) + " ";            //N042 G01 X0.0525 Y2 Z-1.25
  }
  if(instructionMatrix[lineno].comment != "") newLine += instructionMatrix[lineno].comment;
  output << newLine << std::endl;
  return 0;
}

RS274::~RS274() {
    delete[] inputBuffer;
    delete[] outputBuffer;
}

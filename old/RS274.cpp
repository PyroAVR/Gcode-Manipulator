#include "RS274.hpp"

RS274::RS274()  {

}
RS274::RS274(std::string inputFile, std::string outputFile) {
  input.open(inputFile.c_str(), std::fstream::in);
  output.open(outputFile.c_str(), std::fstream::out);
  hardwarethreads = std::thread::hardware_concurrency();
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
  std::smatch modalMatch;
  std::smatch motionMatch;
  std::smatch commentMatch;
  std::string comment;
  std::regex_search(line, commentMatch, commentRegex);                          //Comment removal.
  if(!(commentMatch.size() < 1))  {
    comment = line.substr(line.find(commentMatch[0].str()), line.length());
    line = line.substr(0,line.find(commentMatch[0].str()));
  }
  std::regex_search(line, linenoMatch, linenoRegex);
  std::regex_search(line, commandMatch, commandRegex);
  std::regex_search(line, modalMatch, modalRegex);
  std::regex_search(line, motionMatch, motionRegex);
  std::regex_search(line, specialMatch, specialRegex);
  std::regex_search(line, Xmatch, Xregex);
  std::regex_search(line, Ymatch, Yregex);
  std::regex_search(line, Zmatch, Zregex);
  /*//specials excluded from syntax checker.  Multiple commands can be on one line, ie: G91 G00 <x><y><z>
  if(linenoMatch.size() > 1 || commandMatch.size() > 2 || Xmatch.size() > 1 || Ymatch.size() > 1 || Zmatch.size() > 1)  {
    error(_crash_, "Something's wrong with the syntax on this line:\n" + line);
  }*/
  //This block determines if a line is either setting a mode, or part of a previously set mode.
  //chances are, this needs some work

  bool isCommandModal = false;
  bool isCommandBlank = false;
  bool isLineModal = false;
  (modalMatch[0] != "") ? isCommandModal = true : isCommandModal = false;
  (commandMatch[0] == "") ? isCommandBlank = true : isCommandBlank = false;

  static bool isModalBlock = false;
  static int modalBlockLine;

  if(!isModalBlock) (modalMatch[0] != "") ? isModalBlock = true : isModalBlock = false;
  //if no command, assume still in block
  if(isModalBlock)  (commandMatch[0] == "") ? isModalBlock = true : (modalMatch[0] != "") ? isModalBlock = true : isModalBlock = false;
  isLineModal = isModalBlock;

  bool isCommandMotion = false;
  bool isMotion = false;
  static bool isMotionBlock;
  //if a motion command was found
  (motionMatch[0] != "") ? isCommandMotion = true : isCommandMotion = false;
  //if the command is modal and motion, we're starting a block
  (isLineModal & isCommandMotion) ? isMotionBlock = true : (isLineModal | motionMatch[0] == "") ? isMotionBlock = true : isMotionBlock = false;
  isMotion = isMotionBlock;
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
  gInstruction currentLine = {linenoMatch[0], commandMatch[0], specialMatch[0], atof(Xcoord[0].str().c_str()), atof(Ycoord[0].str().c_str()), atof(Zcoord[0].str().c_str()), comment, isLineModal, isMotion};
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

int RS274::parse()  {
  if(!input.is_open()) return -1;
  while(! input.eof())  {
    input.getline(inputBuffer, bufferSize, '\n');
    linecount++;
    std::cout << "[parse] line " << linecount << std::endl;
    parseLine(inputBuffer);
    memset(inputBuffer, 0, bufferSize);
  }
  return 0;
}

int RS274::parse(const char* filename)  {
  if(input.is_open()) input.close();
  input.open(filename, std::fstream::in);
  parse();
  return 0;
}
//Dummy!
int RS274::parse(std::string &filename)  {
  return RS274::parse(filename.c_str());
}
int RS274::parseRange(const int start, const int end) {
  if(!input.is_open()) return -1;
  if(start > end) return -1;
  for(int i = start; i <= end; i++) {
    if(input.eof()) break;
    input.getline(inputBuffer, bufferSize, '\n');
    linecount++;
    std::cout << "[parse] line " << linecount << std::endl;
    parseLine(inputBuffer);
    memset(inputBuffer, 0, bufferSize);
  }
  return end - linecount;
}
std::string RS274::readElement(int lineno) {
  return std::string("Line " + instructionMatrix[lineno].lineno + ": Command/Special : "
   + instructionMatrix[lineno].command + "/"
   + instructionMatrix[lineno].specialCommand
   + " X: " + std::to_string(instructionMatrix[lineno].xCoord)
   + " Y: " + std::to_string(instructionMatrix[lineno].yCoord)
   + " Z: " + std::to_string(instructionMatrix[lineno].zCoord)
   + "Comment: " + instructionMatrix[lineno].comment
   + "Modal:" + std::to_string(instructionMatrix[lineno].isModal)
   + "Motion:" + std::to_string(instructionMatrix[lineno].isMotion));
}
int RS274::shiftElement(int lineno) {
  /*
  * Weird, seemingly unnessary or backwards if statements:
  * Some CAM processors do not write a coordinate at all if there is no change from the previous
  * position in that axis.  Therefore, we have to not apply any shifts if there is nothing to shift.
  * To keep the program short, we check if there IS something, and then write, rather than check all
  * possible cases in which there might NOT be something. Efficiency, HAYO!
  */
  //if(instructionMatrix[lineno].command == "") return 0;          //Don't write coords if there isn't a command.
  if(!std::isnan(instructionMatrix[lineno].xCoord)) instructionMatrix[lineno].xCoord = instructionMatrix[lineno].xCoord + Xshift;
  if(!std::isnan(instructionMatrix[lineno].yCoord)) instructionMatrix[lineno].yCoord = instructionMatrix[lineno].yCoord + Yshift;
  instructionMatrix[lineno].zCoord = instructionMatrix[lineno].zCoord + Zshift; //Some transformations are skewed if Z-coordinates not present.
  return 0;
}
int RS274::shift(double X, double Y, double Z)  {
  Xshift = X;
  Yshift = Y;
  Zshift = Z;
  for(int i = 0; i < linecount; i++)  {
    shiftElement(i);
  }
  return 0;
}

int RS274::writeLine(int lineno)  {
  static bool isModal = false;
  std::string newLine;
  if(instructionMatrix[lineno].lineno != "") newLine += instructionMatrix[lineno].lineno + " ";                                              //N042
  //If not modal and special command, write sp command and return, else write command, special, continue
  if(!instructionMatrix[lineno].isModal) newLine += instructionMatrix[lineno].specialCommand + " ";
  if(instructionMatrix[lineno].isModal) {                        //N042 G01 The empty check is for my sanity.
    newLine += instructionMatrix[lineno].command + " ";
    newLine += instructionMatrix[lineno].specialCommand + " ";
    if(!std::isnan(instructionMatrix[lineno].xCoord)) newLine += "X" + std::to_string(instructionMatrix[lineno].xCoord) + " ";             //N042 G01 X0.0525
    if(!std::isnan(instructionMatrix[lineno].yCoord)) newLine += "Y" + std::to_string(instructionMatrix[lineno].yCoord) + " ";            //N042 G01 X0.0525 Y2
    if(!std::isnan(instructionMatrix[lineno].zCoord)) newLine += "Z" + std::to_string(instructionMatrix[lineno].zCoord) + " ";            //N042 G01 X0.0525 Y2 Z-1.25
  }
  if(instructionMatrix[lineno].comment != "") newLine += instructionMatrix[lineno].comment;
  output << newLine << std::endl;
  return 0;
}
int RS274::write()  {
  if(!output.is_open()) return -1;
  std::cout << "Input read successfully: " << linecount << " lines parsed." << std::endl;
  for(int i = 0; i < linecount; i++) {
    //std::cout << "[translate] (" << i << "/" << linecount << ") " << (static_cast<float>(i)/static_cast<float>(linecount))*100 << "%"  << std::endl;
    writeLine(i);
  }
  return 0;
}
int RS274::write(const char* filename) {
  if(output.is_open()) output.close();
  output.open(filename, std::fstream::out);
  return write();
}
int RS274::write(std::string &filename) {
  return write(filename.c_str());
}
std::vector<gInstruction> RS274::getInstructionMatrix()  {
  return instructionMatrix;
}
int RS274::size() {
  return linecount;
}
int RS274::prepareThreadWorkers() {
  std::vector<std::string> lines;
  while(!input.eof()) {
    input.getline(inputBuffer, bufferSize, '\n');
    linecount++;
    lines.push_back(inputBuffer);
  }
  int blockSize = linecount/hardwarethreads;
  int numjobs = hardwarethreads;
  if(linecount < blockSize) numjobs = 1;
  //std::move??
  //also what the hell is all this iterator crap
  std::vector<std::string>::iterator separator = lines.begin();
  separator += blockSize;
  threadWorkerData t;
  t.id = 0;
  t.size = blockSize;
  std::copy(lines.begin(), separator, back_inserter(t.lines));
  threadDataBlocks.push_back(t);
  if(numjobs > 1) {
    for(int i = 1; i < hardwarethreads -1; i++) {
        separator++;
        threadWorkerData t;
        t.id = i;
        t.size = blockSize;
        std::copy(separator, separator + blockSize, back_inserter(t.lines));
        separator += blockSize;
        threadDataBlocks.push_back(t);
    }
    //ensure that all lines are fed to the threads
    threadWorkerData t;
    t.id = (threadDataBlocks.end()->id + 1);
    std::copy(separator, lines.end(), back_inserter(t.lines));
    t.size = t.lines.size();
    threadDataBlocks.push_back(t);
  }
  return numjobs;
}
int RS274::prepareThreadWorkers(std::string& filename)  {
  if(input.is_open()) input.close();
  input.open(filename.c_str(), std::fstream::in);
  return prepareThreadWorkers();
}
int RS274::prepareThreadWorkers(const char* filename) {
  if(input.is_open()) input.close();
  input.open(filename, std::fstream::in);
  return prepareThreadWorkers();
}
threadWorkerData RS274::getThreadWorkerInstance(int id)  {
  return threadDataBlocks[id - 1];
}
int RS274::destroyThreads() {
  //uhhhhhhhh
  //delete thread instances too;
  return 0;
}
RS274::~RS274() {
    delete[] inputBuffer;
    delete[] outputBuffer;
    destroyThreads();
    input.close();
    output.close();
}

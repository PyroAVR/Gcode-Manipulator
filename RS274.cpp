#include "RS274.hpp"
RS274Tokenizer::RS274Tokenizer()  {

}
gInstruction RS274Tokenizer::parseLine(std::string line)  {
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
  return currentLine;
}
RS274Tokenizer::~RS274Tokenizer() {

}




RS274Worker::RS274Worker(int id)  {
  twd.id = id;
}
RS274Worker::RS274Worker(threadWorkerData _twd)  {
  twd = _twd;
}
RS274Worker::RS274Worker(int id, std::vector<std::string> _lines)  {
  twd.id = id;
  std::copy(_lines.begin(), _lines.end(), back_inserter(twd.lines));
}
int RS274Worker::parseRange(int start, int end)  {
  if(start > end) return -1;
  setStatus(_working_);
  for(int i = start; i < end; i++) {
    //std::cout << twd.lines[i] << std::endl;
    twd.instructionMatrix.push_back(rt.parseLine(twd.lines[i]));
  }
  twd.size = twd.instructionMatrix.size();
  setStatus(_done_);
  return twd.size;
}
int RS274Worker::run() {
  return parseRange(0, twd.lines.size());
}
int RS274Worker::setStatus(int s)  {
  status = s;
  return status;
}
int RS274Worker::getStatus()  {
  return status;
}
threadWorkerData RS274Worker::getParsedData()  {
  return twd;
}






RS274::RS274()  {
  hardwarethreads = std::thread::hardware_concurrency();

}
RS274::RS274(std::string inputFile, std::string outputFile) {
  start = std::chrono::system_clock::now();
  input.open(inputFile.c_str(), std::fstream::in);
  output.open(outputFile.c_str(), std::fstream::out);
  hardwarethreads = std::thread::hardware_concurrency();
  while(!input.eof()) {
    input.getline(inputBuffer, bufferSize, '\n');
    std::cout << inputBuffer << std::endl;
    linestoparse.push_back(std::string(inputBuffer));
  }
  //this should be adjustable by the user
  int blockSize = linestoparse.size()/hardwarethreads;
  jobs = hardwarethreads; //possibly change later for batched work
  if(blockSize <= 1) jobs = 1;
  //std::move??
  //also what the hell is all this iterator crap
  std::vector<std::string>::iterator separator = linestoparse.begin() + blockSize;
  threadWorkerData t;
  t.id = 0;
  t.size = blockSize;
  std::copy(linestoparse.begin(), separator, back_inserter(t.lines));
  twds.push_back(t);
  if(jobs > 1) {
    for(int i = 1; i < jobs -1; i++) {
        separator++;
        t.lines.clear();  //make sure there's no data stored
        t.id = i;
        t.size = blockSize;
        std::copy(separator, separator + blockSize, back_inserter(t.lines));
        separator += blockSize;
        twds.push_back(t);
    }
    //ensure that all lines are fed to the threads
    t.lines.clear();
    t.id = (twds.end()->id + 1);
    separator++;
    std::copy(separator, linestoparse.end(), back_inserter(t.lines));
    t.size = t.lines.size();    //could be larger or smaller than blockSize
    twds.push_back(t);
  }
  for(auto a : twds)  {
    workers.push_back(RS274Worker(a));
  }
  twds.clear();   //don't unncessarily store data
  for(int i = 0; i < workers.size(); i++) {
    workerthreads.push_back(std::move(std::thread(&RS274::runWorker, this, std::ref(workers[i]))));
  }
  end = std::chrono::system_clock::now();
  loadtime = end-start;
}
void RS274::runWorker(RS274Worker& w) {
  w.run();
}

int RS274::run()  {
  start = std::chrono::system_clock::now();
  for(int i = 0; i < workerthreads.size(); i++) {
    std::cout << i << std::endl;
    workerthreads[i].detach();
  }
  bool isFinished = false;
  int sum;
  while(!isFinished)  {
    for(int i = 0; i < workers.size(); i++)  {
      sum += workers[i].getStatus();
      if(sum >= _done_*hardwarethreads) isFinished = true;  //total number of threads * val of _done_ flag
    }
    sum = 0;
  }
  for(auto a : workers)  {
    std::vector<gInstruction> imat = a.getParsedData().instructionMatrix;
    std::copy(imat.begin(), imat.end(), back_inserter(instructionMatrix));
  }
  end = std::chrono::system_clock::now();
  parsetime = end - start;
  return instructionMatrix.size();
}
std::string RS274::readElement(int lineno)  {

}
int RS274::shiftElement(int lineno) {

}
int RS274::shift(double X, double Y, double Z)  {

}
int RS274::writeLine(int lineno)  {

}
int RS274::write(std::string &filename) {

}
int RS274::write(const char* filename)  {

}
std::vector<gInstruction> RS274::getInstructionMatrix()  {

}
int RS274::size() {

}
RS274::~RS274() {

}

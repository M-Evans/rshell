#include <string>
#include <vector>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>



#define GETWORD     0
#define TRIMSPACE   1
#define HANDLESEMI  2
#define GETREDIR    3
#define GETSTRING   4
#define ENDQUOTE    5

#define NONE 0
#define AND  1
#define OR   2
#define SEMI 3
#define PIPE 4

#define debug(A) std::cout << #A << ": " << A << std::endl


#define UNDEFINED -1
#define FROMSTR   -2
#define DEFAULT   -3

#define INPUT  0
#define OUTPUT 1

struct fdChange_t {
  int orig;
  int moveTo;
  int type;
  std::string s;
  fdChange_t() : orig(UNDEFINED), moveTo(UNDEFINED), type(UNDEFINED) {}
};


// struct for holding a command and what its connector is
struct Command_t {
  int connector;
  std::vector<char*> args;
  std::vector<fdChange_t> fdChanges;
  // constructor
  Command_t() : connector(0) {}
};



bool isRedir(char c) {
  switch(c) {
    case '>': case '<':
      return true;
      break;
    default:
      return false;
  }
}

bool isConn(char c) {
  switch(c) {
    case ';':
    case '&':
    case '|':
      return true;
      break;
    default:
      return false;
  }
}



void handleCon(std::vector<Command_t>& cmds, Command_t& cmd, const std::string& line,
               int& mode, unsigned& begin, unsigned& i, bool& se) {
  switch(line[i]) {
    case '&':
      // make sure we don't access past what we're supposed to
      if (i + 1 < line.size() && line[i+1] == '&') {
        i++;
        // check if there's more to parse after this command
        if (i + 1 < line.size()) {
          // there is
          if (line[i+1] == '&' || line[i+1] == '|') {
            // unfortunately, the next thing is a connector. This is a syntax error.
            se = true;
            break;
          } else if (isspace(line[i+1])) { // it's a space. go into TRIMSPACE mode
            mode = TRIMSPACE;
          } else { // it's neither a connector nor a space: must be a word
            mode = GETWORD;
            begin = i + 1;
          }
        }
        cmd.connector = AND; // the command has an "and" connector
        cmds.push_back(cmd); // add the command to the vector of commands that need to be executed
        // prep cmd for next use
        cmd.args.clear(); // remove command and arguments for the next bit of parsing
        cmd.connector = NONE;
      } else { // if the next thing isn't another &, syntax error
        se = true;
      }
      break;
    case '|':
      // make sure we don't access past what we're supposed to
      if (i + 1 < line.size() && line[i+1] == '|') {
        i++;
        // check if there's more to parse after this command
        if (i + 1 < line.size()) {
          // there is
          if (line[i+1] == '&' || line[i+1] == '|') {
            // unfortunately, the next thing is a connector. This is a syntax error.
            se = true;
            break;
          } else if (isspace(line[i+1])) { // it's a space. go into TRIMSPACE mode
            mode = TRIMSPACE;
          } else { // it's neither a connector nor a space: must be a word
            mode = GETWORD;
            begin = i + 1;
          }
        }
        cmd.connector = OR; // the command has an "or" connector
        cmds.push_back(cmd); // add the command to the vector of commands that need to be executed
        // prep cmd for next use
        cmd.args.clear(); // remove command and arguments for the next bit of parsing
        cmd.connector = NONE;
      } else { // if the next thing isn't another |, syntax error (for now. :P)
        se = true;
      }
      break;
    case ';':
      // there is always more to parse
      if (line[i+1] == '&' || line[i+1] == '|') {
        // unfortunately, the next thing is a continuation connector. This is a syntax error.
        se = true;
        break;
      } else if (isspace(line[i+1])) { // it's a space. go into TRIMSPACE mode
        mode = TRIMSPACE;
      } else if (line[i+1] == ';') {
        mode = HANDLESEMI;
      } else { // it's neither a connector nor a space: must be a word
        mode = GETWORD;
        begin = i + 1;
      }

      if (!cmd.args.empty()) { // if there are commands in the command vector
        cmd.connector = SEMI; // the current command has an semicolon connector
        cmds.push_back(cmd); // add the command to the vector of commands that need to be executed
        // prep cmd for next use
        cmd.args.clear(); // remove command and arguments for the next bit of parsing
        cmd.connector = NONE;
      }
      break;
    default:
      printf("There's a connector, but I don't know what it is. help.\n");
      se = true;
  }
}


void preProcessLine(std::string& line) {
  getline(std::cin, line);

  if (std::cin.fail()) {
    printf("\nGoodbye!\n");
    exit(0);
  }

  // look for comments
  if (line.find("#") != std::string::npos) {
    // remove them if necessary (they're useless)
    line = line.substr(0, line.find("#"));
  }

  // remove leading whitespace
  while (line.size() > 0 && (line[0] == ' ' || line[0] == '\t' || line[0] == ';')) {
    line = line.substr(1, line.size() - 1);
  }

  // adding this makes parsing easier
  line += " ; ";
}


void getPrompt(std::string& prompt) {
  char* cbuff = getlogin();

  if (cbuff == NULL) {
    perror("getlogin failed");
  } else {
    prompt += cbuff;

    cbuff = new char[4096];
    if (gethostname(cbuff, (unsigned)4096) == -1) {
      perror("gethostname failed");
    } else {
      prompt += '@';
      prompt += cbuff;
    }
    delete[] cbuff;
  }
  prompt += " > ";
}


void addArg(Command_t& cmd, const std::string& s, unsigned begin, unsigned i) {
  if (i == begin) return;
  // allocate memory for the cstring (+1 for the NULL char)
  char* c = new char[i-begin+1];
  // copy the string over
  for(unsigned j = 0; j < i - begin; ++j) {
    c[j] = s[begin+j];
  }
  // finish off with NULL char
  c[i-begin] = '\0';
  cmd.args.push_back(c);
}
 

int getfd(const std::string& s, bool first = false) {
  if (first && s.size() == 0) return DEFAULT;
  if (!first && s.size() == 0) return UNDEFINED;
  for(unsigned i = 0; i < s.size(); ++i) if (!isdigit(s[i])) return UNDEFINED;
  int res = std::stoi(s);
  if (first) return res;
  return (!first && res == 0)? UNDEFINED : res;
}


void handleRedir(std::vector<Command_t>& cmds, Command_t& cmd, const std::string& line,
               int& mode, unsigned& begin, unsigned& i, bool& se) {
  size_t pos     = 0;
  int fdFrom     = -1;
  int fdTo       = -1;
  fdChange_t fdd;

  std::string sub = line.substr(begin, i - begin);
  if (isConn(sub.at(sub.size() - 1)));
  size_t gtPos  = sub.find('>');
  size_t ltPos  = sub.find('<');
  size_t oQuote = sub.find('"');            // open quote
  size_t cQuote = sub.find('"', oQuote+1);  // close quote


  if (gtPos == std::string::npos && ltPos == std::string::npos) {
    i = begin;
    mode = GETWORD;
    return; // didn't find '<' or '>'; word
  }

  // index of redirect symbol
  pos = (ltPos < gtPos) ? ltPos : gtPos;

  if (oQuote < pos) {
    se = true;
    return;
  }

  // if fdFrom returns UNDEFINED, syntax error
  if (UNDEFINED == (fdFrom = getfd(sub.substr(0, pos), true))) {
    se = true;
    return;
  }

  if (line[begin+pos] == '<') { // <
    fdd.type = INPUT;
    if (line[begin+pos+1] == '&') { // <&fd
      if (UNDEFINED == (fdTo = getfd(sub.substr(pos+2)))) {
        se = true;
        return;
      }
      fdd.orig   = fdFrom;
      fdd.moveTo = fdTo;
    } else if (line[begin+pos+1] == '<' && line[begin+pos+2] == '<') { // <<<"text"
      if (oQuote == std::string::npos || cQuote == std::string::npos) {
        se = true;
        return;
      }
      fdd.orig   = fdFrom;
      fdd.moveTo = FROMSTR;
      fdd.s      = sub.substr(oQuote + 1, cQuote - oQuote - 1);
    } else { // <file
      fdd.orig = fdFrom;
      if (sub.substr(pos+1).size() == 0) {
        se = true;
        return;
      } else if (-1 == (fdd.moveTo = open(sub.substr(pos+1).c_str(), O_RDONLY))) {
        perror(sub.substr(pos+1).c_str());
        return;
      }
    }
    cmd.fdChanges.push_back(fdd);
  } else if (line[begin+pos] == '>') { // >
    fdd.type = OUTPUT;
    if (line[begin+pos+1] == '>') { // >>
      if (line[begin+pos+2] == '&') { // >>&fd
        if (UNDEFINED == (fdTo = getfd(sub.substr(pos+3)))) {
          se = true;
          return;
        }
        fdd.orig   = fdFrom;
        fdd.moveTo = fdTo;
      } else { // >>file
        fdd.orig = fdFrom;
        if (sub.substr(pos+2).size() == 0) {
          se = true;
          return;
        } else if (-1 == (fdd.moveTo = open(sub.substr(pos+2).c_str(), O_APPEND|O_CREAT, 010600))) {
          perror(sub.substr(pos+2).c_str());
          return;
        }
      }
    } else if (line[begin+pos+1] == '&') { // >&fd
      if (UNDEFINED == (fdTo = getfd(sub.substr(pos+2)))) {
        se = true;
        return;
      }
      fdd.orig   = fdFrom;
      fdd.moveTo = fdTo;
    } else { // >file
      fdd.orig = fdFrom;
      if (sub.substr(pos+1).size() == 0) {
        se = true;
        return;
      } else if (-1 == (fdd.moveTo = open(sub.substr(pos+1).c_str(), O_WRONLY|O_CREAT, 010600))) {
        perror(sub.substr(pos+1).c_str());
        return;
      }
    }
    cmd.fdChanges.push_back(fdd);
  } else { // bad
    fprintf(stderr, "Fuck\n");
    exit(1);
  }
}


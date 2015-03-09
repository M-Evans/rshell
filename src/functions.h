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
#define FILEIN    -3
#define FILEOUT   -4
#define FILEAPP   -5
#define DEFAULT   -6

#define INPUT  0
#define OUTPUT 1

struct fdChange_t {
  int orig;
  int moveTo;
  int type;
  std::string s;
  fdChange_t() : orig(UNDEFINED), moveTo(UNDEFINED), type(UNDEFINED) {}
  fdChange_t(int o, int m, int t) : orig(o), moveTo(m), type(t) {}
};


#define DT_FDIN 0
#define DT_FDO  1
#define DT_FIN  2
#define DT_FOW  3
#define DT_FOA  4
#define DT_STR  5


struct fdData_t {
  int from;
  int t;
  std::string s;
  fdData_t() : from(-1), t(UNDEFINED) {}
  fdData_t(int from, int t, const std::string& s)
    : from(from), t(t), s(s) {}
};





// struct for holding a command and what its connector is
struct Command_t {
  int connector;
  std::vector<char*> args;
  std::vector<int> closefd;
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
      if (line[i+1] == '&') {
        i++;
        if (isConn(line[i+1])) { // next thing is connector. Syntax error
          se = true;
          break;
        } else if (isspace(line[i+1])) { // it's a space. go into TRIMSPACE mode
          mode = TRIMSPACE;
        } else if (isRedir(line[i+1])) {
          begin = i + 1;
          mode = GETREDIR;
        } else {
          begin = i + 1;
          mode = GETWORD;
        }
        cmd.connector = AND; // the command has an "and" connector
        cmds.push_back(cmd); // add the command to the vector of commands that need to be executed
        // prep cmd for next use
        cmd.args.clear(); // remove command and arguments for the next bit of parsing
        cmd.fdChanges.clear();
        cmd.closefd.clear();
        cmd.connector = NONE;
      } else { // if the next thing isn't another &, syntax error
        se = true;
      }
      break;
    case '|':
      if (line[i+1] == '|') { // ||
        i++;
        if (isConn(line[i+1])) { // next thing is connector. Syntax error
          se = true;
          break;
        } else if (isspace(line[i+1])) { // it's a space. go into TRIMSPACE mode
          mode = TRIMSPACE;
        } else if (isRedir(line[i+1])) {
          begin = i + 1;
          mode = GETREDIR;
        } else { // it's neither a connector nor a space: must be a word
          begin = i + 1;
          mode = GETWORD;
        }
        cmd.connector = OR; // the command has an "or" connector
        cmds.push_back(cmd); // add the command to the vector of commands that need to be executed
        // prep cmd for next use
        cmd.args.clear(); // remove command and arguments for the next bit of parsing
        cmd.fdChanges.clear();
        cmd.closefd.clear();
        cmd.connector = NONE;
      } else { // |
        if (isConn(line[i+1])) {
          se = true;
          break;
        } else if (isspace(line[i+1])) {
          mode = TRIMSPACE;
        } else if (isRedir(line[i+1])) {
          begin = i + 1;
          mode = GETREDIR;
        } else {
          begin = i + 1;
          mode = GETWORD;
        }
        cmd.connector = PIPE;
        cmds.push_back(cmd);
        cmd.args.clear();
        cmd.fdChanges.clear();
        cmd.closefd.clear();
        cmd.connector = NONE;
      }
      break;
    case ';':
      if (line[i+1] == '&' || line[i+1] == '|') {
        // unfortunately, the next thing is a continuation connector. This is a syntax error.
        se = true;
        break;
      } else if (isspace(line[i+1])) { // it's a space. go into TRIMSPACE mode
        mode = TRIMSPACE;
      } else if (line[i+1] == ';') {
        mode = HANDLESEMI;
      } else if (isRedir(line[i+1])) {
        begin = i + 1;
        mode = GETREDIR;
      } else { // it's neither a connector nor a space: must be a word
        begin = i + 1;
        mode = GETWORD;
      }

      if (!cmd.args.empty()) { // if there are commands in the command vector
        cmd.connector = SEMI; // the current command has an semicolon connector
        cmds.push_back(cmd); // add the command to the vector of commands that need to be executed
        // prep cmd for next use
        cmd.args.clear(); // remove command and arguments for the next bit of parsing
        cmd.fdChanges.clear();
        cmd.closefd.clear();
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
    prompt.append(cbuff);

    cbuff = new char[4096];
    if (gethostname(cbuff, (unsigned)4096) == -1) {
      perror("gethostname failed");
    } else {
      prompt.append(1, '@');
      prompt.append(cbuff);
    }
    delete[] cbuff;
  }
  prompt.append(" > ");
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
  // if (first) return res;
  // return (!first && res == 0)? UNDEFINED : res;
  return res;
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
    if (fdFrom == DEFAULT) fdFrom = 0;
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
      } else {
        fdd.moveTo = FILEIN;
        fdd.s      = sub.substr(pos+1);
      }
      cmd.closefd.push_back(fdd.moveTo);
    }
    cmd.fdChanges.push_back(fdd);
  } else if (line[begin+pos] == '>') { // >
    if (fdFrom == DEFAULT) fdFrom = 1;
    fdd.type = OUTPUT;
    if (line[begin+pos+1] == '>') { // >>
      if (line[begin+pos+2] == '&') { // >>&fd
        se = true;
        return;
      } // >>file :
      fdd.orig = fdFrom;
      if (sub.substr(pos+2).size() == 0) {
        se = true;
        return;
      } else {
        fdd.moveTo = FILEAPP;
        fdd.s      = sub.substr(pos+2);
      }
      cmd.closefd.push_back(fdd.moveTo);
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
      } else {
        fdd.moveTo = FILEOUT;
        fdd.s      = sub.substr(pos+1);
      }
      cmd.closefd.push_back(fdd.moveTo);
    }
    cmd.fdChanges.push_back(fdd);
  } else { // bad
    fprintf(stderr, "OHNO\n");
    exit(1);
  }
}



void debugMap(const std::map<int, fdData_t>& fds) {
  auto it = fds.begin();
  while (it != fds.end()) {
    if (it->second.t == DT_FDIN) {
      printf("%d becomes %d\n", it->first, it->second.from);
    } else if (it->second.t == DT_FDO) {
      printf("%d becomes %d\n", it->first, it->second.from);
    } else if (it->second.t == DT_FIN) {
      printf("%d becomes \"%s\"\n", it->first, it->second.s.c_str());
    } else if (it->second.t == DT_FOW) {
      printf("\"%s\" becomes %d (overwrite)\n", it->second.s.c_str(), it->second.from);
    } else if (it->second.t == DT_FOA) {
      printf("\"%s\" becomes %d (append)\n", it->second.s.c_str(), it->second.from);
    } else if (it->second.t == DT_STR) {
      printf("%d becomes \"%s\"\n", it->first, it->second.s.c_str());
    } else {
      fprintf(stderr, "Unknown type...");
    }

    /*
    debug((*it).first);
    debug((*it).second.from);
    debug((*it).second.t);
    debug((*it).second.s);
    */

    // auto r = fds.find((*it).second.from);
 
    // debug((*r).first);
    // debug((*r).second.from);
    // debug((*r).second.t);
    // debug((*r).second.s);
 
    ++it;
  }
}



void deltaFD(const std::map<int, fdData_t>& fds) {
}



char* strToCstr(const std::string& s) {
  char* ret = new char[s.size() + 1];
  for(unsigned i = 0; i < s.size(); ++i) {
    ret[i] = s[i];
  }
  ret[s.size()] = '\0';
  return ret;
}



void fillPaths(std::vector<char*>& v) {
  std::string path(getenv("PATH"));
  if (path.size() == 0) return;
  size_t colon = 0;
  size_t  next = 0;

  while (std::string::npos != (next = path.find(':', colon))) {
    if (colon != next) {
      v.push_back(strToCstr(path.substr(colon, next - colon)));
    }
    colon = next + 1; // skip actual colon
  }
  // handle last path unless path was terminated with ':'
  if (path.size() - colon > 0) v.push_back(strToCstr(path.substr(colon, -1)));
}






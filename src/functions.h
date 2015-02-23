#include <string>
#include <vector>
#include <cctype>
#include <stdlib.h>



#define GETWORD       0
#define TRIMSPACE     1
#define HANDLESEMI    2
#define GETREDIR      3

#define NONE 0
#define AND  1
#define OR   2
#define SEMI 3

#define debug(A) std::cout << #A << ": " << A << std::endl


struct fdChange_t {
  int orig;
  int moveTo;
  fdChange_t() : orig(-1), moveTo(-1) {}
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
    case '>':
    case '<':
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
          if (line[i + 1] == '&' || line[i + 1] == '|') {
            // unfortunately, the next thing is a connector. This is a syntax error.
            se = true;
            break;
          } else if (isspace(line[i + 1])) { // it's a space. go into TRIMSPACE mode
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
          if (line[i + 1] == '&' || line[i + 1] == '|') {
            // unfortunately, the next thing is a connector. This is a syntax error.
            se = true;
            break;
          } else if (isspace(line[i + 1])) { // it's a space. go into TRIMSPACE mode
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
      if (line[i + 1] == '&' || line[i + 1] == '|') {
        // unfortunately, the next thing is a continuation connector. This is a syntax error.
        se = true;
        break;
      } else if (isspace(line[i + 1])) { // it's a space. go into TRIMSPACE mode
        mode = TRIMSPACE;
      } else if (line[i + 1] == ';') {
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
  line += "; ";
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
  char* c = new char[i - begin + 1];
  // copy the string over
  for(unsigned j = 0; j < i - begin; ++j) {
    c[j] = s[begin + j];
  }
  // finish off with NULL char
  c[i - begin] = '\0';
  cmd.args.push_back(c);
}
 

bool isFd(const std::string& s) {
  if (s.size() == 0) return false;
  for(unsigned i = 0; i < s.size(); ++i) if (!isdigit(s[i])) return false;
  return true;
}
bool isFile(const std::string& s) {
  return !isFd(s);
}


void handleRedir(std::vector<Command_t>& cmds, Command_t& cmd, const std::string& line,
               int& mode, unsigned& begin, unsigned& i, bool& se) {
}




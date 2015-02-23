#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <iostream>

#include "functions.h"


int main(int argc, char** argv) {
  std::string prompt;
  getPrompt(prompt);


  bool quit = false;

  // print welcome message and prompt
  printf("Welcome to rshell!\n");

  while (!quit) {
    // holds a single command and its arguments
    Command_t cmd;
    // holds multiple commands
    std::vector<Command_t> cmds;
    // hold a raw line of input
    std::string line;

    // print prompt and get a line of text (done in condition)
    printf("%s", prompt.c_str());
    preProcessLine(line);


    int mode = GETWORD;
    unsigned begin = 0;

    // prepare cmd
    cmd.connector = NONE;

    // syntax error flag
    bool se = false;

    // starts with a connector? I don't think so
    if (line.size() > 0 && isConn(line[0]) && line[0] != ';') {
      se = true;
    } else if (line.size() > 0 && (isRedir(line[0]) || isdigit(line[0]))) {
      mode = GETREDIR;
    }

    // handle the input
    for(unsigned i = 0; i < line.size() && !se; ++i) {
      bool con   = isConn(line[i]);
      // bool dig   = isdigit(line[i]);
      bool redir = isRedir(line[i]);
      bool space = isspace(line[i]);


      // if we're getting a word and there's a whitespace or connector here
      if (mode == GETWORD && (space || con || redir)) {
        // only happens for blank lines:
        if (i == 0 && con) break;
        if (redir) {
          mode = GETREDIR;
          continue;
        }
        // chunk the last term and throw it into the vector
        addArg(cmd, line, begin, i);

        if (space) {
          mode = TRIMSPACE;
        } else { // if (con) {
          handleCon(cmds, cmd, line, mode, begin, i, se);
        }
      } else if (mode == TRIMSPACE && !space) {
        if (con && cmd.args.empty() && line[i] != ';') {
          se = true;
        } else if (con) {
          handleCon(cmds, cmd, line, mode, begin, i, se);
        } else if (redir) {
          begin = i;
          mode = GETREDIR;
        } else {
          begin = i;
          mode = GETWORD;
        }
      } else if (mode == HANDLESEMI && line[i] != ';') {
        if (isConn(line[i])) {
          se = true;
        } else if (isspace(line[i])) {
          mode = TRIMSPACE;
        } else { // it's a word
          begin = i;
          mode = GETWORD;
        }
      } else if (mode == GETREDIR && line[i] == '"') {
        mode = GETSTRING;
      } else if (mode == GETREDIR && space) {
        handleRedir(cmds, cmd, line, mode, begin, i, se);
        mode = TRIMSPACE;
      } else if (mode == GETSTRING && line[i] == '"') {
        mode = ENDQUOTE;
      } else if (mode == ENDQUOTE) {
        if (space) {
          handleRedir(cmds, cmd, line, mode, begin, i, se);
          mode = TRIMSPACE;
        } else {
          se = true;
          continue;
        }
      }
    }

    // if the last command has a continuation connector, syntax error
    if (cmds.size() > 0 && (cmds[cmds.size()-1].connector == AND
                        ||  cmds[cmds.size()-1].connector == OR
                        ||  cmds[cmds.size()-1].connector == PIPE)) {
      se = true;
    }

    if (mode == GETSTRING) se = true;

    // if there was a syntax error
    if (se) {
      printf("Syntax error\n");
      continue;
    }

    // now to execute all the commands
    for(unsigned i = 0; i < cmds.size(); ++i) {
      int exitStatus = 0;
      char* arg = cmds[i].args[0];
      if (strcmp(arg, "exit") == 0) {
        quit = true;
        break;
      }
      char** argv = new char*[cmds[i].args.size()+1];
      for(unsigned j = 0; j < cmds[i].args.size(); ++j) {
        argv[j] = cmds[i].args[j];
      }
      argv[cmds[i].args.size()] = 0;

      // arg and argv are now prepared
      pid_t pid = fork();
      if (pid == -1) {
        perror("fork");
        exit(1);
      } else if (pid == 0) { // child process
        if (execvp(arg, argv) == -1) {
          // if there's a return value, there was a problem
          // -1 indicates a problem, specifically
          perror(arg);
          delete[] argv;
          exit(1);
        }
      } else { // parent process
        if (waitpid(pid, &exitStatus, 0) == -1) {
          perror("waitpid");
          exit(1);
        }
      } if (!exitStatus) { // all is good (0)
        while (i < cmds.size() && cmds[i].connector == OR) {
          ++i;
        }
      } else { // last command failed
        while (i < cmds.size() && cmds[i].connector == AND) {
          ++i;
        }
      }
    }

    // deallocate allocated memory
    for(unsigned i = 0; i < cmds.size(); ++i) {
      for(unsigned j = 0; j < cmds[i].args.size(); ++j) {
        delete[] cmds[i].args[j];
      }
    }
  }
  printf("Goodbye!\n");
  return 0;
}




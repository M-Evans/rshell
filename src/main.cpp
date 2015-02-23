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
        } else if (con) {
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


    /*
    for(unsigned i = 0; i < cmds.size(); ++i) {
      for(unsigned j = 0; j < cmds[i].args.size(); ++j) {
        printf("cmd %d arg %d: \"%s\"\n", i, j, cmds[i].args[j]);
      }
      if (cmds[i].connector == NONE) printf("connector: NONE\n");
      else if (cmds[i].connector == AND) printf("connector: AND\n");
      else if (cmds[i].connector == OR) printf("connector: OR\n");
      else if (cmds[i].connector == SEMI) printf("connector: SEMI\n");
      else if (cmds[i].connector == PIPE) printf("connector: PIPE\n");
      printf("file descriptor modifications:\n");
      for(unsigned j = 0; j < cmds[i].fdChanges.size(); ++j) {
        if (cmds[i].fdChanges[j].type == INPUT) {
          printf("change fd %d to ",
          (cmds[i].fdChanges[j].orig == DEFAULT) ? 0 :
           cmds[i].fdChanges[j].orig);

          if (cmds[i].fdChanges[j].moveTo == UNDEFINED)
            printf("UNDEFINED\n");
          else if (cmds[i].fdChanges[j].moveTo == FROMSTR)
            printf("FROMSTR (\"%s\")\n", cmds[i].fdChanges[j].s.c_str());
          else
            printf("Some other fd (%d)\n", cmds[i].fdChanges[j].moveTo);
        }

        if (cmds[i].fdChanges[j].type == OUTPUT) {
          printf("change fd %d to ",
          (cmds[i].fdChanges[j].orig == DEFAULT) ? 1 :
           cmds[i].fdChanges[j].orig);

          if (cmds[i].fdChanges[j].moveTo == UNDEFINED)
            printf("UNDEFINED\n");
          else
            printf("Some other fd (%d)\n", cmds[i].fdChanges[j].moveTo);
        }
      }
    }
    // *///

    // now to execute all the commands
    int pa[2];
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

      if (cmds[i].connector == PIPE) {
        // 1. make pipe
        if (-1 == pipe(pa)) {
          perror("pipe");
          exit(1);
        }
        // 2. this program gets output and next program gets input
        // this program:
        fdChange_t outfdC(1, pa[1], OUTPUT);
        fdChange_t infdC(0, pa[0], INPUT);
        cmds[i].fdChanges.push_back(outfdC);
        cmds[i].closefd.push_back(pa[1]);
        // next program:
        cmds[i+1].fdChanges.push_back(infdC);
        cmds[i+1].closefd.push_back(pa[0]);
      }

      // arg and argv are now prepared
      pid_t pid = fork();
      if (-1 == pid) {
        perror("fork");
        exit(1); // fatal error
      } else if (0 == pid) { // child process
        for(int j = cmds[i].fdChanges.size() - 1; j >= 0; ++j) {
          debug(j);
          debug(cmds[i].fdChanges[j].type);
          debug(INPUT);
          debug(cmds[i].fdChanges[j].orig);
          debug(cmds[i].fdChanges[j].moveTo);
          if (INPUT == cmds[i].fdChanges[j].type) {
            debug(cmds[i].fdChanges[j].type);
            if (-1 == close(cmds[i].fdChanges[j].orig)) {
              perror("close input orig");
              exit(1);
            }
            if (cmds[i].fdChanges[j].moveTo != FROMSTR) {
              if (-1 == dup2(cmds[i].fdChanges[j].moveTo, cmds[i].fdChanges[j].orig)) {
                perror("dup2 input moveTo orig");
                exit(1);
              }
            } else { // FROMSTR
              // create a pipe into the current program
              int pair[2];
              if (-1 == pipe(pair)) {
                perror("pipe pair");
                exit(1);
              }
              // clear original and send output there
              if (-1 == dup2(pair[1], cmds[i].fdChanges[j].orig)) {
                perror("dup2 input pair[1] orig");
                exit(1);
              }
              // print to the input
              dprintf(pair[0], "%s", cmds[i].fdChanges[j].s.c_str());
              if (-1 == close(pair[0])) {
                perror("close pair[0]");
              }
            }
          } else { // if (OUTPUT == cmds[i].fdChanges[j].type)
            if (-1 == close(cmds[i].fdChanges[j].orig)) {
              perror("close output moveTo");
              exit(1);
            }
            if (-1 == dup2(cmds[i].fdChanges[j].moveTo, cmds[i].fdChanges[j].orig)) {
              perror("dup2 output orig moveTo");
              exit(1);
            }
          }
        }
        if (-1 == execvp(arg, argv)) {
          // if there's a return value (-1), there was a problem
          perror(arg);
          delete[] argv;
          exit(1);
        }
      } else { // parent process
        // close current fd's
        for(unsigned j = 0; j < cmds[i].closefd.size(); ++j) {
          if (-1 == close(cmds[i].closefd[j])) {
            perror("closing in parent");
            exit(1);
          }
        }
        // only wait for non-pipes
        if (cmds[i].connector != PIPE && -1 == waitpid(pid, &exitStatus, 0)) {
          perror("waitpid");
          exit(1);
        }
      }
      if (!exitStatus) { // all is good (0)
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




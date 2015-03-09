#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <map>
#include <utility>
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


    /* code for debugging
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
    // */

    // now to execute all the commands
    // pipe
    int pa[2];
    std::vector<char*> paths;
    // getPaths
    fillPaths(paths);

    // prepare argv so I can delete them later
    std::vector<char**> argvs;
    for(unsigned i = 0; i < cmds.size(); ++i) {
      argvs.push_back(new char*[cmds[i].args.size()+1]);
    }

    for(unsigned cmdi = 0; cmdi < cmds.size(); ++cmdi) {
      size_t argsSize = cmds[cmdi].args.size();
      int exitStatus = 0;

      char* arg = cmds[cmdi].args[0];
      if (strcmp(arg, "exit") == 0) {
        quit = true;
        break;
      }
      char** subargv = argvs[cmdi];
      for(unsigned j = 0; j < argsSize; ++j) {
        subargv[j] = cmds[cmdi].args[j];
      }
      subargv[argsSize] = 0;

      if (cmds[cmdi].connector == PIPE) {
        // 1. make pipe
        if (-1 == pipe(pa)) {
          perror("pipe");
          exit(1);
        }
        // 2. this program gets output and next program gets input
        // this program:
        fdChange_t outfdC(1, pa[1], OUTPUT);
        fdChange_t infdC(0, pa[0], INPUT);

        cmds[cmdi].fdChanges.push_back(outfdC);
        cmds[cmdi].closefd.push_back(pa[1]);
        // next program:
        cmds[cmdi+1].fdChanges.push_back(infdC);
        cmds[cmdi+1].closefd.push_back(pa[0]);

      }

      std::map<int, fdData_t> fds;
      for(unsigned j = 0; j < cmds[cmdi].fdChanges.size(); ++j) {
        int o           = cmds[cmdi].fdChanges[j].orig;
        int mt          = cmds[cmdi].fdChanges[j].moveTo;
        int t           = cmds[cmdi].fdChanges[j].type;
        std::string str = cmds[cmdi].fdChanges[j].s;

        if (t == INPUT) {
          if (mt == FILEIN) {
            fds.insert(std::pair<int, fdData_t>(o, fdData_t(-1, DT_FIN, str)));
            // printf("Adding %d pointing to the file %s in input mode\n", o, str.c_str());
          } else if (mt == FROMSTR) {
            fds.insert(std::pair<int, fdData_t>(o, fdData_t(-1, DT_STR, str)));
            // printf("Adding %d pointing to the string %s in input mode\n", o, str.c_str());
          } else {
            fds.insert(std::pair<int, fdData_t>(o, fdData_t(mt, DT_FDIN, str)));
            // printf("Adding %d pointing to the fd %d in input mode\n", mt, o);
          }
        } else if (t == OUTPUT) {
          if (mt == FILEOUT) {
            fds.insert(std::pair<int, fdData_t>(-1, fdData_t(o, DT_FOW, str)));
            // printf("Adding %d pointing to the file %s in output write mode\n", o, str.c_str());
          } else if (mt == FILEAPP) {
            // if (fds
            fds.insert(std::pair<int, fdData_t>(-1, fdData_t(o, DT_FOA, str)));
            // printf("Adding %d pointing to the file %s in output append mode\n", o, str.c_str());
          } else {
            fds.insert(std::pair<int, fdData_t>(mt, fdData_t(o, DT_FDO, str)));
            // printf("Adding %d pointing to the fd %d in output mode\n", o, mt);
          }
        }
      }
      debugMap(fds);




      // arg and argv are now prepared
      pid_t pid = fork();
      if (-1 == pid) {
        perror("fork");
        exit(1); // fatal error
      } else if (0 == pid) { // child process
        deltaFD(fds);
        for(unsigned i = 0; i < paths.size(); ++i) {
          char* executable = new char[strlen(paths[i]) + strlen(arg) + 2];
          strcpy(executable, paths[i]);
          strcat(executable, "/");
          strcat(executable, arg);
          struct stat statRes;
          if (-1 != stat(executable, &statRes)) {
            if (-1 == execv(executable, subargv)) {
              // if there's a return value (-1), there was a problem
              debug("executing");
              perror(executable);
              delete[] subargv;
              delete[] executable;
              exit(1);
            }
          }
          errno = 0;
          delete[] executable;
        }
        fprintf(stderr, "%s: command not found\n", arg);
        delete[] subargv;
        exit(1);
      } else { // parent process
        // close current fd's
        /*
        for(unsigned j = 0; j < cmds[cmdi].closefd.size(); ++j) {
          if (-1 == close(cmds[cmdi].closefd[j])) {
            perror("closing in parent");
            exit(1);
          }
        } */
        // only wait for non-pipes
        if (cmds[cmdi].connector != PIPE && -1 == waitpid(pid, &exitStatus, 0)) {
          perror("waitpid");
          exit(1);
        }
      }
      if (!exitStatus) { // all is good (0)
        while (cmdi < cmds.size() && cmds[cmdi].connector == OR) {
          ++cmdi;
        }
      } else { // last command failed
        while (cmdi < cmds.size() && cmds[cmdi].connector == AND) {
          ++cmdi;
        }
      }
    }

    // deallocate allocated memory
    for(unsigned i = 0; i < cmds.size(); ++i) {
      for(unsigned j = 0; j < cmds[i].args.size(); ++j) {
        delete[] cmds[i].args[j];
      }
    }
    for(unsigned i = 0; i < paths.size(); ++i) {
      delete[] paths[i];
    }
    for(unsigned i = 0; i < argvs.size(); ++i) {
      delete[] argvs[i];
    }
  }
  printf("Goodbye!\n");
  return 0;
}




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


int main(int argc, char** argv)
{
  // print welcome message and prompt
  printf("Welcome to rshell!\n");

  std::string prompt;

  char* cbuff = getlogin();
  if (cbuff == NULL)
  {
    perror("getlogin");
  }
  else
  {
    prompt += cbuff;
  }

  cbuff = new char[4096];
  if (gethostname(cbuff, (unsigned)4096) == -1)
  {
    perror("gethostname");
  }
  else
  {
    prompt += '@';
    prompt += cbuff;
  }

  prompt += " > ";

  bool ext = false;

  while (!ext)
  {
    // holds a single command and its arguments
    Command cmd;
    // holds multiple commands
    std::vector<Command> cmds;
    // hold a raw line of input
    std::string line;

    // print prompt and get a line of text (done in condition)
    printf("%s", prompt.c_str());
    getline(std::cin, line);

    // look for comments
    if (line.find("#") != std::string::npos)
    {
      // remove them if necessary (they're useless)
      line = line.substr(0, line.find("#"));
    }

    // remove leading whitespace
    while (line.size() > 0 && (line[0] == ' ' || line[0] == '\t' || line[0] == ';'))
    {
      line = line.substr(1, line.size() - 1);
    }

    // adding this makes parsing easier
    line += "; ";

    if (std::cin.fail())
    {
      printf("\nGoodbye!\n");
      exit(0);
    }

    int mode = GETWORD;
    unsigned begin = 0;

    // prepare cmd
    cmd.connector = NONE;

    // syntax error flag
    bool se = false;

    // starts with a connector? I don't think so
    if (line.size() > 0 && isConn(line[0]) && line[0] != ';')
    {
      se = true;
    }

    // handle the input
    for(unsigned i = 0; i < line.size() && !se; ++i)
    {
      bool con   = isConn(line[i]);
      bool space = isspace(line[i]);

      // if we're getting a word and there's a whitespace or connector here
      if (mode == GETWORD && (space || con))
      {
        // chunk the last term and throw it into the vector
        char* c = stocstr(line.substr(begin, i - begin));
        if (strlen(c) > 0)
        {
          cmd.args.push_back(c);
        }
        else
        {
          // only happens when nothing is entered. breaks so nothing more happens
          break;
        }

        if (space)
        {
          mode = TRIMSPACE;
        }
        else // it's a connector
        {
          handleCon(cmds, cmd, line, mode, begin, i, se);
        }
      }
      else if (mode == TRIMSPACE && (!space || con))
      {
        if (con && cmd.args.empty())
        {

// (cmds[cmds.size() - 1].connector == AND || cmds[cmds.size() - 1].connector == OR))

          // if (line[i] != ';' || cmds[cmds.size() - 1].connector !=
          // {
            // why this test? two reasons:
            //   1) it 
          if (line[i] != ';')
            se = true;
          // }
        }
        else if (con)
        {
          handleCon(cmds, cmd, line, mode, begin, i, se);
        }
        else
        {
          mode = GETWORD;
          begin = i;
        }
      }
      else if (mode == HANDLESEMI && line[i] != ';')
      {
        if (isConn(line[i]))
        {
          se = true;
        }
        else if (isspace(line[i]))
        {
          mode = TRIMSPACE;
        }
        else // it's a word
        {
          mode = GETWORD;
          begin = i;
        }
      }
    }

    // if the last command has a continuation connector, syntax error
    if (cmds.size() > 0 && (cmds[cmds.size() - 1].connector == AND
                        ||  cmds[cmds.size() - 1].connector == OR))
    {
      se = true;
    }

    // if there was a syntax error
    if (se)
    {
      printf("Syntax error\n");
      continue;
    }

    // now to execute all the commands
    for(unsigned i = 0; i < cmds.size(); ++i)
    {
      int exitStatus = 0;
      char* arg = cmds[i].args[0];
      if (strcmp(arg, "exit") == 0)
      {
        ext = true;
        break;
      }
      char** argv = new char*[cmds[i].args.size() + 1];
      for(unsigned j = 0; j < cmds[i].args.size(); ++j)
      {
        argv[j] = cmds[i].args[j];
      }
      argv[cmds[i].args.size()] = 0;

      // arg and argv are now prepared
      pid_t pid = fork();
      if (pid == -1)
      {
        perror("fork");
        exit(1);
      }
      else if (pid == 0) // child process
      {
        if (execvp(arg, argv) == -1)
        {
          // if there's a return value, there was a problem
          // -1 indicates a problem, specifically
          perror("execvp");
          exit(1);
        }
      }
      else // parent process
      {
        if (waitpid(pid, &exitStatus, 0) == -1)
        {
          perror("waitpid");
          exit(1);
        }
      }
      if (!exitStatus) // all is good (0)
      {
        while (i < cmds.size() && cmds[i].connector == OR)
        {
          ++i;
        }
      }
      else // last command failed
      {
        while (i < cmds.size() && cmds[i].connector == AND)
        {
          ++i;
        }
      }
    }

    // deallocate allocated memory
    for(unsigned i = 0; i < cmds.size(); ++i)
    {
      for(unsigned j = 0; j < cmds[i].args.size(); ++j)
      {
        delete[] cmds[i].args[j];
      }
    }
  }
  printf("Goodbye!\n");
  return 0;
}




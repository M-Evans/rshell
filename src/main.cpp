#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>

#include "functions.h"


int main(int argc, char** argv)
{
  // hold a raw line of input
  std::string line;

  // print welcome message and prompt
  printf("Welcome to rshell!\n");

  while (true)
  {
    // holds a single command and its arguments
    Command cmd;
    // holds multiple commands
    std::vector<Command> cmds;

    // print prompt and get a line of text (done in condition)
    printf("$ ");
    getline(std::cin, line);

    // look for comments
    if (line.find("#") != std::string::npos)
    {
      // remove them if necessary (they're useless)
      line = line.substr(0, line.find("#"));
    }

    // remove leading whitespace
    while (line.size() > 0 && (line[0] == ' ' || line[0] == ';'))
    {
      line = line.substr(1, line.size() - 1);
    }

    // adding a space to the end makes parsing easier
    line += ' ';

    if (std::cin.fail())
    {
      printf("\nGoodbye!\n");
      exit(0);
    }

    int mode = GETWORD;
    unsigned begin = 0;

    // temporary: show pre-processed input to know what's being dealt with
    printf("You entered: \"%s\"\n", line.c_str());

    // prepare cmd
    cmd.connector = NONE;

    // syntax error flag
    bool se = false;

    if (line.size() > 0 && isConn(line[0]))
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
          printf("Oh no! Recieved a word with no length\n");
          break;
        }

        if (space)
        {
          mode = TRIMSPACE;
        }
        else // it's a connector
        {
          handleCon(cmds, cmd, line, mode, i, se);
        }
      }
      else if (mode == TRIMSPACE && (!space || con))
      {
        if (con && cmd.args.empty())
        {
          se = true;
        }
        else if (con)
        {
          handleCon(cmds, cmd, line, mode, i, se);
        }
        else
        {
          mode = GETWORD;
          begin = i;
        }
      }
      
      if (i + 1 == line.size() && !cmd.args.empty()) // parsing the last character
      {
        // if it has a continuation connector, and we're finished with parsing, syntax error
        if (cmd.connector == AND || cmd.connector == OR)
        {
          se = true;
        }
        else
        {
          cmds.push_back(cmd);
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
      printf("Syntax error detected\n");
      continue;
    }

    for(unsigned i = 0; i < cmds.size(); ++i)
    {
      printf("Command %u:\n", i);
      for(unsigned j = 0; j < cmds[i].args.size(); ++j)
      {
        printf("\t\"%s\"\n", cmds[i].args[j]);
      }
      switch(cmds[i].connector)
      {
        case AND:
          printf("\t&&\n");
          break;
        case OR:
          printf("\t||\n");
          break;
        case SEMI:
          printf("\t;\n");
          break;
        case NONE:
          printf("\tNo connector\n");
          break;
        default:
          printf("\tERROR: no valid connector specified\n");
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
  return 0;
}




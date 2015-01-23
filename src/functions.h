#include <cstdlib>
#include <string>
#include <vector>
#include <cctype>



#define GETWORD   0
#define TRIMSPACE 1

#define NONE 0
#define AND  1
#define OR   2
#define SEMI 3

#define debug(A) std::cout << #A << ": " << A << std::endl

// struct for holding a command and what its connector is
struct Command
{
    std::vector<char*> args;
    int connector;
};


// copies a string into a cstring, while being careful of
// memory management. Do NOT write past the end of the cstring
char* stocstr(const std::string& s)
{
    // allocate memory for the cstring (+1 for the NULL char)
    char* c = new char[s.size() + 1];

    // copy the string over
    for(unsigned i = 0; i < s.size(); ++i)
    {
        c[i] = s[i];
    }

    // finish off with NULL char
    c[s.size()] = '\0';

    return c;
}


bool isConn(char c)
{
    switch(c)
    {
        case ';':
        case '&':
        case '|':
        case '<':
        case '>':
            return true;
            break;
        default:
            return false;
    }
}



void handleCon(std::vector<Command>& cmds, Command cmd, const std::string& line,
       int& mode, unsigned& i, bool& se)
{
  switch(line[i])
  {
    case '&':
      // make sure we don't access past what we're supposed to
      if (i + 1 < line.size() && line[i+1] == '&')
      {
        i++;
        // check if there's more to parse after this command
        if (i + 1 < line.size())
        {
          // there is
          if (isConn(line[i + 1]))
          {
            // unfortunately, the next thing is a connector. This is a syntax error.
            se = true;
            break;
          }
          else if (isspace(line[i + 1])) // it's a space. go into TRIMSPACE mode
          {
            mode = TRIMSPACE;
          }
          else // it's neither a connector nor a space: must be a word
          {
            mode = GETWORD;
          }
        }
        cmd.connector = AND; // the command has an "and" connector
        cmds.push_back(cmd); // add the command to the vector of commands that need to be executed
        // prep cmd for next use
        cmd.args.clear(); // remove command and arguments for the next bit of parsing
        cmd.connector = NONE;
      }
      else // if the next thing isn't another &, syntax error
      {
        se = true;
      }
      break;
    case '|':
      // make sure we don't access past what we're supposed to
      if (i + 1 < line.size() && line[i+1] == '|')
      {
        i++;
        // check if there's more to parse after this command
        if (i + 1 < line.size())
        {
          // there is
          if (isConn(line[i + 1]))
          {
            // unfortunately, the next thing is a connector. This is a syntax error.
            se = true;
            break;
          }
          else if (isspace(line[i + 1])) // it's a space. go into TRIMSPACE mode
          {
            mode = TRIMSPACE;
          }
          else // it's neither a connector nor a space: must be a word
          {
            mode = GETWORD;
          }
        }
        cmd.connector = OR; // the command has an "or" connector
        cmds.push_back(cmd); // add the command to the vector of commands that need to be executed
        // prep cmd for next use
        cmd.args.clear(); // remove command and arguments for the next bit of parsing
        cmd.connector = NONE;
      }
      else // if the next thing isn't another |, syntax error (for now. :P)
      {
        se = true;
      }
      break;
    case ';':
      // check if there's more to parse after this command
      if (i + 1 < line.size())
      {
        // there is
        if (isConn(line[i + 1]))
        {
          // unfortunately, the next thing is a connector. This is a syntax error.
          se = true;
          break;
        }
        else if (isspace(line[i + 1])) // it's a space. go into TRIMSPACE mode
        {
          mode = TRIMSPACE;
        }
        else // it's neither a connector nor a space: must be a word
        {
          mode = GETWORD;
        }
      }
      cmd.connector = SEMI; // the command has an "and" connector
      cmds.push_back(cmd); // add the command to the vector of commands that need to be executed
      // prep cmd for next use
      cmd.args.clear(); // remove command and arguments for the next bit of parsing
      cmd.connector = NONE;
      break;
    default:
      printf("There's a connector, but I don't know what it is. help.\n");
      se = true;
  }
}




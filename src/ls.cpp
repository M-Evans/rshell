#include <stdio.h>
#include <string.h>

#include <vector>
#include <string>


int main(int argc, char** argv)
{
  bool a = false, l = false, R = false;
  std::vector<char*> arglist;
  std::string output;

  for(int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      // it's a flag
      unsigned slen = strlen(argv[i]);
      for(unsigned j = 1; j < slen; ++j) {
        switch (argv[i][j]) {
          case 'a':
            a = true;
            break;
          case 'l':
            l = true;
            break;
          case 'R':
            R = true;
            break;
          default:
            output += "Error: '";
            output += argv[i][j];
            output += "' is not a valid flag option\n";
        }
      }
    } else {
      // it's not a flag: it's something to list
      arglist.push_back(argv[i]);
    }
  }

  output += "Statistics:\na is ";
  if (!a)
    output += "not ";
  output += "set.\nl is ";
  if (!l)
    output += "not ";
  output += "set.\nR is ";
  if (!R)
    output += "not ";
  output += "set.\n\n";

  output += "Stuff to list:\n{";
  for(unsigned i = 0; i < arglist.size(); ++i) {
    output += arglist[i];
    if (i + 1 < arglist.size())
      output += ", ";
  }
  output += "}\n";

  printf("%s", output.c_str());

  return 0;
}




#include <string.h>

bool compareFilenamesInefficient(const char* a, const char* b)
{
  char* ac = new char[strlen(a) + 1];
  char* bc = new char[strlen(b) + 1];
  strcpy(ac, a);
  strcpy(bc, b);
  unsigned alen = strlen(a);
  unsigned blen = strlen(b);
  for(unsigned i = 0; i < alen && i < blen; ++i) {
    ac[i] = tolower(ac[i]);
    bc[i] = tolower(bc[i]);
  }
  bool ret = strcmp(ac, bc) < 0;
  delete[] ac;
  delete[] bc;
  return ret;
}
























  /*
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
  output += "}";
  */



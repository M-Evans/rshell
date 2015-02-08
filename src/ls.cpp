#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <vector>
#include <string>
#include <algorithm>

#include "ls.h"

int main(int argc, char** argv)
{
  bool a = false, l = false, R = false;
  std::vector<char*> arglist;

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
            // unknown option. Print an error message and exit
            fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], argv[i][j]);
            exit(1);
        }
      }
    } else {
      // it's not a flag: it's something to list
      arglist.push_back(argv[i]);
    }
  }

  // we're working on the current directory if nothing else
  if (arglist.empty()) {
    arglist.push_back((char*)".");
  }

  for(unsigned i = 0; i < arglist.size(); ++i) {
    std::vector<char*> files;
    // dir pointer
    DIR* dp = opendir(arglist[i]);
    if (dp == NULL) {
      perror("opendir");
      exit(1);
    }

    // dir entry pointer
    struct dirent* de;
    while((de = readdir(dp))) {
      if (a || de->d_name[0] != '.') {
        char* f = new char[PATH_MAX];
        strcpy(f, arglist[i]);
        strcat(f, "/");
        strcat(f, de->d_name);
        files.push_back(f);
      }
    }
    if (errno) {
      perror("readdir");
      exit(1);
    }

    std::sort(files.begin(), files.end(), compareFilenamesInefficient);


    if (R) { // recursive
    } else { // not recursive
      if (arglist.size() > 1) {
        printf("%s:\n", arglist[i]);
      }
      printFiles(files, l);
      if (!l) { printf("\n"); }
    }


    for(unsigned j = 0; j < files.size(); ++j) {
      delete[] files[j];
    }

    if (closedir(dp) == -1) {
      perror("closedir");
      exit(1);
    }
  }

  return 0;
}




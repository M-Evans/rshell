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
      if (slen == 1) {
        // it's just '-'. try to print it as a file
        arglist.push_back(argv[i]);
        continue;
      }
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

  // get some information about what's being printed
  argAndTypes args;
  for(unsigned i = 0; i < arglist.size(); ++i) {
    struct stat fs;
    if (stat(arglist[i], &fs) == -1) {
      perror("stat");
      errno = 0; // reset the errno. Otherwise, problems...
      // don't quit because it's a recoverable error.
      // if there's nothing else to print, it'll quit anyway
    } else {
      if (S_ISDIR(fs.st_mode)) {
        args.v.push_back(arglist[i]);
        // args.t.push_back(TYPE_DIR);
      } else {
        args.v.insert(args.v.begin(), arglist[i]);
        // args.t.insert(args.t.begin(), TYPE_FILE);
        args.numFiles++;
      }
    }
  }

  // there were files passed in. they always get printed before everything else
  if (args.numFiles > 0) {
    // alphabetically sort JUST the files (they're already sorted by type)
    std::sort(args.v.begin(), args.v.begin() + args.numFiles, compareFilenamesInefficient);
    printFiles(args.v.begin(), args.v.begin() + args.numFiles, a, l, false);
    if (!l)  printf("\n");
  }

  std::sort(args.v.begin() + args.numFiles, args.v.end(), compareFilenamesInefficient);

  // iterate over all the dirs
  for(unsigned i = args.numFiles; i < args.v.size(); ++i) {
    lsRec(args.v[i], a, l, R, 0, arglist.size());
  }
  return 0;
}




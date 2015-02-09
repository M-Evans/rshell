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
    printFiles(args.v.begin(), args.v.begin() + args.numFiles, l);
    if (!l)  printf("\n");
  }

  // iterate over all the dirs
  for(unsigned i = args.numFiles; i < args.v.size(); ++i) {
    std::vector<char*> files;
    // dir pointer
    DIR* dp = opendir(args.v[i]);
    if (dp == NULL) {
      perror("opendir");
      exit(1);
    }

    // dir entry pointer
    struct dirent* de;
    while((de = readdir(dp))) {
      if (a || de->d_name[0] != '.') {
        // buggy: 
        char* f = new char[PATH_MAX];
        strcpy(f, args.v[i]);
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
      if (i == 0)
        printf("%s:\n", args.v[i]);
      else
        printf("\n%s:\n", args.v[i]);



    } else { // not recursive
      if (arglist.size() > 1) {
        if (i == 0)
          printf("%s:\n", args.v[i]);
        else
          printf("\n%s:\n", args.v[i]);
      }
      printFiles(files.begin(), files.end(), l);
      if (!l) printf("\n");
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




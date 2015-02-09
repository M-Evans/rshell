#include <string.h>
#include <libgen.h> // dirname and basename
#include <unistd.h>
#include <math.h>
#include <pwd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <vector>
#include <iostream>



#define TYPE_UNKNOWN -1
#define TYPE_FILE    0
#define TYPE_DIR     1

struct fPerm {
  char* p;
  int type;
  fPerm() {
    p = NULL;
    type = TYPE_UNKNOWN;
  }
};

struct fandTypes {
  std::vector<fPerm> f;
  unsigned numFiles;
  fandTypes() {  // constructor
    numFiles = 0;
  }
};



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

void printFilePrepl() {} // TODO: calculate and pass back
// the widths and coloration info

void printFilePrep() {} // TODO: calculate and pass back
// the widths and coloration info. This version only gets
// the spacing for the non -l output. (no stat widths)




void printFiles(const std::vector<char*> v, bool l) {
  for(unsigned i = 0; i < v.size(); ++i) {
    if (l) {
      // get the file stats
      struct stat fs;
      if (stat(v[i], &fs) == -1) {
        perror("stat");
        exit(1);
      }

      // file type
      if (S_ISREG(fs.st_mode)) { printf("-"); }
      else if (S_ISDIR(fs.st_mode)) { printf("d"); }
      else if (S_ISCHR(fs.st_mode)) { printf("c"); }
      else if (S_ISBLK(fs.st_mode)) { printf("b"); }
      else if (S_ISFIFO(fs.st_mode)) { printf("p"); }
      else if (S_ISLNK(fs.st_mode)) { printf("l"); }
      else if (S_ISSOCK(fs.st_mode)) { printf("s"); }
      else { printf("?"); }

      // permissions
      printf("%c%c%c%c%c%c%c%c%c",
              (fs.st_mode & S_IRUSR) ? 'r' : '-',
              (fs.st_mode & S_IWUSR) ? 'w' : '-',
              (fs.st_mode & S_IXUSR) ? 'x' : '-',
              (fs.st_mode & S_IRGRP) ? 'r' : '-',
              (fs.st_mode & S_IWGRP) ? 'w' : '-',
              (fs.st_mode & S_IXGRP) ? 'x' : '-',
              (fs.st_mode & S_IROTH) ? 'r' : '-',
              (fs.st_mode & S_IWOTH) ? 'w' : '-',
              (fs.st_mode & S_IXOTH) ? 'x' : '-');

      // inode count (number of hard links)
      printf(" %2u", (unsigned)fs.st_nlink);

      // owner
      struct passwd* name;
      if ((name = getpwuid(fs.st_uid)) == NULL)
      {
        // no passwd struct returned
        // if errno is set, perror it
        if (errno) {
          perror("getpwuid");
          exit(1);
        }
        // else, nothing was returned but there was no error
        // give it something to print
        name->pw_name = (char*)"noName";
      }
      printf(" %8s", name->pw_name);

      // group
      if ((name = getpwuid(fs.st_gid)) == NULL)
      {
        // no passwd struct returned
        // if errno is set, perror it
        if (errno) {
          perror("getpwuid");
          exit(1);
        }
        // else, nothing was returned but there was no error
        // give it something to print
        name->pw_name = (char*)"noName";
      }
      printf(" %8s", name->pw_name);

      // size
      printf(" %6u", (unsigned)fs.st_size);
      
      // date
      // broken down time info gets held in the 
      // tm struct
      struct tm fileMtime;
      // use fs.st_mtime to create fileMtime
      localtime_r(&fs.st_mtime, &fileMtime);
      // 20 chars for the resulting string. Should NEVER be more
      char timestr[20];
      // get formatted time string
      // %b: short month    %d: 00->31 day
      // %H: 00->23 hour    %M: 00->59 minute
      // fileMtime is accessed, timestr gets modified
      strftime((char*)&timestr, 20, " %b %d %H:%M", &fileMtime);
      // actually print the time
      printf("%s", timestr);
      // print the filename (just the name)
      // TODO: color the files
      printf(" %s\n", basename(v[i]));
    } else {
      printf("%s  ", basename(v[i]));
    }
  }
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



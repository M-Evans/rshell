#include <string.h>
#include <libgen.h> // dirname and basename
#include <unistd.h>
#include <math.h>
#include <pwd.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <vector>
#include <iostream>

#define MAX(A, B) (((A) > (B)) ? (A) : (B))


struct argAndTypes {
  std::vector<char*> v;
  unsigned numFiles;
  argAndTypes() {  // constructor
    numFiles = 0;
  }
};



bool compareFilenamesInefficient(const char* a,
                                 const char* b)
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

void printFilePrepl(std::vector<char*>::iterator ib,
                    const std::vector<char*>::iterator& ie,
                    int* iw, /* inode */
                    int* ow, /* owner */
                    int* gw, /* group */
                    int* sw, /* size  */
                    int* tot /* total block count */) {
  *iw = *ow = *gw = *sw = *tot = 0;

  while (ib != ie) {
    struct stat fs;
    if (stat(*ib, &fs) == -1) {
      perror("stat");
      exit(1);
    }

    *tot += fs.st_blocks / 2;

    *iw = MAX(1 + (int)log10(fs.st_nlink), *iw);

    *sw = MAX(1 + (int)log10(fs.st_size), *sw);

    // owner
    struct passwd* name;
    if ((name = getpwuid(fs.st_uid)) == NULL)
    {
      if (errno) {
        perror("getpwuid");
        exit(1);
      }
      name->pw_name = (char*)"noName";
    }

    *ow = MAX((int)strlen(name->pw_name), *ow);

    // group
    if ((name = getpwuid(fs.st_gid)) == NULL)
    {
      if (errno) {
        perror("getpwuid");
        exit(1);
      }
      name->pw_name = (char*)"noName";
    }

    *gw = MAX((int)strlen(name->pw_name), *gw);
    
    ib++;
  }
}


void printFilePrep() {} // TODO: calculate and pass back
// the widths and coloration info. This version only gets
// the spacing for the non -l output. (no stat widths)




//void printFiles(const std::vector<char*> v, bool l) {
void printFiles(std::vector<char*>::iterator ib,
                const std::vector<char*>::iterator& ie,
                bool l) {
  if (ib == ie) return;

  int iw, ow, gw, sw, blocks;
  if (l) {
    printFilePrepl(ib, ie, &iw, &ow, &gw, &sw, &blocks);
    printf("total %d\n", blocks);
  }
  else {
    // not l
  }

  while (ib != ie) {
    if (l) {
      // get the file stats
      struct stat fs;
      if (stat(*ib, &fs) == -1) {
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
      printf(" %*u", iw, (unsigned)fs.st_nlink);

      
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
      printf(" %*s", ow, name->pw_name);

      
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
      printf(" %*s", gw, name->pw_name);


      
      // size
      printf(" %*u", sw, (unsigned)fs.st_size);
      
      
      
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
      printf(" %s\n", basename(*ib));
    } else {
      printf("%s  ", basename(*ib));
    }

    // increment iterator
    ib++;
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



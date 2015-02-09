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

#define MY_MAX(A, B) (((A) > (B)) ? (A) : (B))


void printFiles(std::vector<char*>::iterator, const std::vector<char*>::iterator&,
                bool, bool, bool);



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
      perror(*ib);
      errno = 0;
      ib++;
      continue;
    }

    *tot += fs.st_blocks / 2;

    *iw = MY_MAX(1 + (int)log10(fs.st_nlink), *iw);

    *sw = MY_MAX(1 + (int)log10(fs.st_size), *sw);

    // owner
    struct passwd* name;
    if ((name = getpwuid(fs.st_uid)) == NULL)
    {
      if (errno) {
        perror("getpwuid");
        exit(1);
      }
    }

    if (name)  *ow = MY_MAX((int)strlen(name->pw_name), *ow);

    // group
    if ((name = getpwuid(fs.st_gid)) == NULL)
    {
      if (errno) {
        perror("getpwuid");
        exit(1);
      }
    }

    if (name)  *gw = MY_MAX((int)strlen(name->pw_name), *gw);
    
    ib++;
  }
}



void printFilePrep() {} // TODO: calculate and pass back
// the widths and coloration info. This version only gets
// the spacing for the non -l output. (no stat widths)










void lsRec(char* d, bool a, bool l, bool R, int sz = 1, int call = 1) {
  std::vector<char*> files;
  // dir pointer
  DIR* dp = opendir(d);
  if (dp == NULL) {
    perror("opendir");
    exit(1);
  }

  // dir entry pointer
  struct dirent* de;
  while((de = readdir(dp))) {
    if (a || de->d_name[0] != '.') {
      // Potentially overflows (no such thing as a max path):
      char* f = new char[strlen(d) + strlen(de->d_name) + 2];
      strcpy(f, d);
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


  if (R || sz > 1) {
    if (call == 0) {
      printf("%s:\n", d);
    }
    else {
      printf("\n%s:\n", d);
    }
  }
  //if (R)  printf("\n");
  printFiles(files.begin(), files.end(), a, l, R);


  for(unsigned j = 0; j < files.size(); ++j) {
    delete[] files[j];
  }

  if (closedir(dp) == -1) {
    perror("closedir");
    exit(1);
  }
}









void printFiles(std::vector<char*>::iterator ib,
                const std::vector<char*>::iterator& ie,
                bool a,
                bool l,
                bool R) {
  if (ib == ie) return;

  int iw, ow, gw, sw, blocks;
  if (l) {
    printFilePrepl(ib, ie, &iw, &ow, &gw, &sw, &blocks);
    printf("total %d\n", blocks);
  }
  else {
    // do the not l one
  }

  std::vector<char*> recurse;

  while (ib != ie) {
    struct stat fs;
    if (stat(*ib, &fs) == -1) {
      perror(*ib);
      errno = 0;
      ib++;
      continue;
    }

    if (l) {
      // get the file stats
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
      struct passwd* oname;
      struct passwd* gname;
      if ((oname = getpwuid(fs.st_uid)) == NULL)
      {
        // no passwd struct returned
        // if errno is set, perror it
        if (errno) {
          perror("getpwuid");
          exit(1);
        }
        // else, nothing was returned but there was no error
        // give it something to print
      }

      char* jic = NULL; // just in case
      if (oname) {
        printf(" %*s", ow, oname->pw_name);
        jic = new char [ow + 1];
        strcpy(jic, oname->pw_name);
      }


      
      // group
      if ((gname = getpwuid(fs.st_gid)) == NULL)
      {
        // no passwd struct returned
        // if errno is set, perror it
        if (errno) {
          perror("getpwuid");
          exit(1);
        }
        // else, nothing was returned but there was no error
        // give it something to print
      }
      if (gname)     printf(" %*s", gw, gname->pw_name);
      else if (jic)  printf(" %*s", ow, jic);

      
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
      printf("%s ", timestr);
      
      
      
      // print the filename (just the name)
      char* s = basename(*ib);

      // handle color
      if (S_ISDIR(fs.st_mode)) {
        if (R)  recurse.push_back(*ib);
        if (s[0] == '.')  printf("%c[1;34;47;38;5;27m", 0x1B);
        else              printf("%c[38;5;27m", 0x1B);
      }
      else if ((fs.st_mode & S_IXUSR) | (fs.st_mode & S_IXGRP) | (fs.st_mode & S_IXOTH)) {
        if (s[0] == '.')  printf("%c[1;34;47;38;5;34m", 0x1B);
        else              printf("%c[38;5;34m", 0x1B);
      }
      printf("%s%c[0m\n", s, 0x1B);


    } else { // no -l

      char* s = basename(*ib);
      // handle color
      if (S_ISDIR(fs.st_mode)) {
        if (R)  recurse.push_back(*ib);
        if (s[0] == '.')  printf("%c[1;34;47;38;5;27m", 0x1B);
        else              printf("%c[38;5;27m", 0x1B);
      }
      else if ((fs.st_mode & S_IXUSR) | (fs.st_mode & S_IXGRP) | (fs.st_mode & S_IXOTH)) {
        if (s[0] == '.')  printf("%c[1;34;47;38;5;34m", 0x1B);
        else              printf("%c[38;5;34m", 0x1B);
      }
      printf("%s%c[0m  ", s, 0x1B);

    }

    // increment iterator
    ib++;
  }

  if (!l)  printf("\n");

  // optionally recurse
  for(unsigned i = 0; R && i < recurse.size(); ++i) {
    if (strcmp(basename(recurse[i]), ".") != 0 && strcmp(basename(recurse[i]), "..") != 0)
      lsRec(recurse[i], a, l, R, recurse.size());
  }
}





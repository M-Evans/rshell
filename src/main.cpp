#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>

int main(int argc, char** argv)
{
    // hold a raw line of input
    std::string line;

    // print welcome message and prompt
    printf("Welcome to rshell!\n$ ");
    getline(std::cin, line);

    do
    {
        // holds a single command and its arguments
        std::vector<char*> cmd;
        // holds multiple commands
        std::vector<std::vector<char*> > cmds;

        // look for comments
        if (line.find("#") != std::string::npos) {
            // remove them if necessary
            line = line.substr(0, line.find("#"));
        }

        // add a space so that every space-separated entity has
        // at least one space directly to the right of it
        // (makes parsing easier)
        line += ' ';

        // if exit was entered properly
        if (line.substr(0,5) == "exit ")
        {
            // say goodbye, and quit
            printf("Goodbye!\n");
            exit(0);
        }

        // handle the input
        printf("You entered \"%s\"\n", line.c_str());

        // todo: split 
        
        
        // print prompt and get a line of text
        printf("$ ");
    } while (getline(std::cin, line));
    return 0;
}

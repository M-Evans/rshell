#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>

#include "functions.h"


int main(int argc, char** argv)
{
    // hold a raw line of input
    std::string line;

    // print welcome message and prompt
    printf("Welcome to rshell!\n");

    while (true)
    {
        // holds a single command and its arguments
        std::vector<char*> cmd;
        // holds multiple commands
        std::vector<std::vector<char*> > cmds;

        // print prompt and get a line of text (done in condition)
        printf("$ ");
        getline(std::cin, line); // handles the EOF being passed in

        // look for comments
        if (line.find("#") != std::string::npos)
        {
            // remove them if necessary (they're useless)
            line = line.substr(0, line.find("#"));
        }

        // add a space so that every space-separated entity has
        // at least one space directly to the right of it
        // (makes parsing easier)
        line += ' ';

        // if exit was entered properly
        if (line.substr(0,5) == "exit " || std::cin.fail())
        {
            if (std::cin.fail())
            {
                printf("\n");
            }
            // say goodbye, and quit
            printf("Goodbye!\n");
            exit(0);
        }

        // handle the input
        // todo: split each group of commands into commands and arguments
        printf("You entered \"%s\"\n", line.c_str());

    }
    return 0;
}

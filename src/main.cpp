#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>

int main(int argc, char** argv) {
    std::string line;                // hold a raw line of input
    std::vector<char*> cmd;          // holds a single command and its arguments
    std::vector<std::vector<char*> > cmds; // holds multiple commands

    printf("\nWelcome to rshell!\n");

    while (1) {
        printf("$ ");
        getline(std::cin, line);
        if (line.substr(0, 4) == "exit") {
            printf("Goodbye!\n");
            exit(0);
        }
        printf("You entered \"%s\"\n", line.c_str());
    }
    return 0;
}

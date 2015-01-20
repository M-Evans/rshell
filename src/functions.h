#include <cstdlib>
#include <string>
#include <vector>


#define GETWORD   0
#define TRIMSPACE 1

#define NONE 0
#define AND  1
#define OR   2
#define SEMI 3

// struct for holding a command and what its connector is
struct Command
{
    std::vector<char*> cmdList;
    int connector = NONE;
};




// copies a string into a cstring, while being careful of
// memory management. Do NOT write past the end of the cstring
void stocstr(const std::string& s, char* c)
{
    // allocate memory for the cstring (+1 for the NULL char)
    c = new char[s.size() + 1];

    // copy the string over
    for(unsigned i = 0; i < s.size(); ++i)
    {
        c[i] = s[i];
    }

    // finish off with NULL char
    c[s.size()] = '\0';
}




#include <iostream>
#include <string>
#include <dirent.h>
#include "JackCompiler.h"

char delimiter = '/';      // Path delimiter of different systems: should be '/' on linux while '\\' on windows.

int main(int argc, char* argv[]) {
    string input(argv[1]);

    if (input.size() > 5 && input.substr(input.size()-5, 5) == ".jack") {
        // Single jack file input
        Compiler compiler(input);
        compiler.compile();
    }
    else {
        // Input is a directory
        if (input[input.size()-1] != delimiter) input += delimiter;
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(input.c_str())) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                string f_name = string(ent->d_name);
                if (f_name.size() > 5 && f_name.substr(f_name.size()-5, 5) == ".jack") {
                    Compiler compiler(input + f_name);
                    compiler.compile();
                }
            }
            closedir(dir);
        }
        else {
            cout << "Could not open: " << input << "\nDirectory doesn't exist.\n";
            return 0;
        }
    }
    return 0;
}

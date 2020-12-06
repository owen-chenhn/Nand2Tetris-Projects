#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char** argv) {

    // Obtain input file path and output file path. Input file must end with '.in'.
    string in_path(argv[1]);
    string out_path(in_path, 0, in_path.length()-2);
    out_path += "out";

    // open the input file and the output file.
    ifstream in_file(in_path.c_str(), ios::in);
    ofstream out_file(out_path.c_str(), ios::out);

    // process the file
    string line;
    int ptr, count, flag = 0;        // ptr: pointer to each char; count: count the number of chars written to output file; flag: indicate whether in /*...*/ segment.
    while (!in_file.eof()) {
        getline(in_file, line);
        if (line.length() == 0)     // filter the empty line.
            continue;
        count = 0;

        // iterate through
        for (ptr = 0; ptr < line.length(); ptr++) {
            if (flag) {
                // in the /*...*/ segment
                if (line[ptr] == '*' && ptr < line.length()-1 && line[ptr+1] == '/') {
                    flag = 0;
                    ptr++;
                }
            }
            else {
                if (line[ptr] == ' ' || line[ptr] == '\t')
                    continue;
                else if (line[ptr] == '/' && ptr < line.length()-1 && line[ptr+1] == '/')
                    break;
                else if (line[ptr] == '/' && ptr < line.length()-1 && line[ptr+1] == '*') {
                    flag = 1;
                    ptr++;
                }
                else {
                    count++;
                    out_file << line[ptr];
                }
            }
        }
        if (!flag && count > 0)
            out_file << '\n';
        line.clear();
    }

    in_file.close();
    out_file.close();
    return 0;
}

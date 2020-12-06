#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <list>
#include <bitset>
#include <ctype.h>

using namespace std;


// Initialization: constructs necessary look up tables.
// Symbol table:
unordered_map<string, int> symbolTable = {
    {"SP", 0}, 
    {"LCL", 1}, 
    {"ARG", 2}, 
    {"THIS", 3}, 
    {"THAT", 4}, 
    {"R0", 0}, 
    {"R1", 1}, 
    {"R2", 2}, 
    {"R3", 3}, 
    {"R4", 4}, 
    {"R5", 5}, 
    {"R6", 6}, 
    {"R7", 7}, 
    {"R8", 8}, 
    {"R9", 9}, 
    {"R10", 10}, 
    {"R11", 11}, 
    {"R12", 12}, 
    {"R13", 13}, 
    {"R14", 14}, 
    {"R15", 15},
    {"SCREEN", 16384}, 
    {"KBD", 24576}
};

// Dest table:
unordered_map<string, string> destTable = {
    {"", "000"},
    {"A", "100"},
    {"D", "010"},
    {"M", "001"},
    {"AD", "110"},
    {"DA", "110"},
    {"DM", "011"},
    {"MD", "011"},
    {"AM", "101"},
    {"MA", "101"},
    {"AMD", "111"},
    {"ADM", "111"},
    {"DAM", "111"},
    {"DMA", "111"},
    {"MAD", "111"},
    {"MDA", "111"}
};

// Jump table:
unordered_map<string, string> jmpTable = {
    {"", "000"},
    {"JGT", "001"},
    {"JEQ", "010"},
    {"JGE", "011"},
    {"JLT", "100"},
    {"JNE", "101"},
    {"JLE", "110"},
    {"JMP", "111"}
};

// Comp table:
unordered_map<string, string> compTable = {
    {"0", "0101010"},
    {"1", "0111111"},
    {"-1", "0111010"},
    {"D", "0001100"},
    {"A", "0110000"},
    {"M", "1110000"},
    {"!D", "0001101"},
    {"!A", "0110001"},
    {"!M", "1110001"},
    {"-D", "0001111"},
    {"-A", "0110011"},
    {"-M", "1110011"},
    {"D+1", "0011111"},
    {"1+D", "0011111"},
    {"A+1", "0110111"},
    {"1+A", "0110111"},
    {"M+1", "1110111"},
    {"1+M", "1110111"},
    {"D-1", "0001110"},
    {"A-1", "0110010"},
    {"M-1", "1110010"},
    {"D+A", "0000010"},
    {"A+D", "0000010"},
    {"D+M", "1000010"},
    {"M+D", "1000010"},
    {"D-A", "0010011"},
    {"D-M", "1010011"},
    {"A-D", "0000111"},
    {"M-D", "1000111"},
    {"D&A", "0000000"},
    {"A&D", "0000000"},
    {"D&M", "1000000"},
    {"M&D", "1000000"},
    {"D|A", "0010101"},
    {"A|D", "0010101"},
    {"D|M", "1010101"},
    {"M|D", "1010101"}
};


void text_strip(list<string> &, string);       // Read in the asm file and strip all the space and comments.
void remove_label(list<string> &);               // Remove program label and update symbolTable. 
void decode_instr(list<string> &, string);       // Decode the asm instructions and write to output .hdl file. 


int main(int argc, char** argv) {
    // Obtain input file path and output file path. Input file must end with '.asm'.
    string in_path(argv[1]);
    string out_path(in_path, 0, in_path.length()-3);
    out_path += "hack";
    cout << "Start assembling file: " << in_path << endl;

    // Use a list to store the asm program:
    list<string> asm_text;
    text_strip(asm_text, in_path);       //Read in the asm file and strip all the space and comments. 
    
    // Remove the program label and update symbolTable:
    remove_label(asm_text);

    // Decode the asm instructions and write to output .hdl file:
    decode_instr(asm_text, out_path);

    return 0;
}


void text_strip(list<string> &asm_text, string in_path) {
    // Read in .asm file.
    ifstream in_file(in_path.c_str(), ios::in);

    string in_line, out_line("");
    unsigned ptr, flag = 0;

    while (!in_file.eof()) {
        getline(in_file, in_line);
        // iterate through the line:
        for (ptr = 0; ptr < in_line.length(); ptr++) {
            if (flag) {
                // in the /*...*/ segment
                if (in_line[ptr] == '*' && ptr < in_line.length()-1 && in_line[ptr+1] == '/') {
                    flag = 0;
                    ptr++;
                }
            }
            else {
                if (isspace(in_line[ptr]))
                    continue;
                else if (in_line[ptr] == '/' && ptr < in_line.length()-1 && in_line[ptr+1] == '/')
                    break;
                else if (in_line[ptr] == '/' && ptr < in_line.length()-1 && in_line[ptr+1] == '*') {
                    flag = 1;
                    ptr++;
                }
                else {
                    out_line += in_line[ptr];
                }
            }
        }
        if (out_line.length() > 0)
            asm_text.push_back(out_line);
        in_line.clear();
        out_line.clear();
    }

    in_file.close();
    
    /* For debugging: print the stripped program texts. */
    cout << "\n=========== After stripping ============\n";
    list<string>::iterator iter;
    for (iter = asm_text.begin(); iter != asm_text.end(); iter++) {
        cout << *iter << endl;
    }
}

void remove_label(list<string> &asm_text) {
    int line_num = 0;
    list<string>::iterator iter = asm_text.begin();
    while (iter != asm_text.end()) {
        if ((*iter)[0] == '(') {
            symbolTable[(*iter).substr(1, (*iter).length()-2)] = line_num;
            iter = asm_text.erase(iter);
        }
        else {
            line_num++;
            iter++;
        }
    }

    /* For debugging: print the stripped program texts. */
    cout << "\n=========== After label removing ============\n";
    for (iter = asm_text.begin(); iter != asm_text.end(); iter++) {
        cout << *iter << endl;
    }
}

void decode_instr(list<string> &asm_text, string out_path) {
    ofstream out_file(out_path.c_str(), ios::out);
    cout << "Writing machine code to path: " << out_path << endl;

    int var_num = 16;     // Available variable number
    list<string>::iterator iter;
    string code;
    for (iter = asm_text.begin(); iter != asm_text.end(); iter++) {
        if ((*iter)[0] == '@') {    // A-instruction
            if (isdigit((*iter)[1])) {
                // Convert the integer substring to binary code. 
                code = bitset<16>(stoi((*iter).substr(1))).to_string();
            }
            else {
                // Look up the label. 
                string label = (*iter).substr(1);
                if (symbolTable.count(label) == 0) {
                    // label is not in symbol table
                    symbolTable[label] = var_num++;
                }
                code = bitset<16>(symbolTable[label]).to_string();
            }
        }
        else {      // C-instruction
            code = "111";
            string dest(""), comp(""), jmp("");
            unsigned start = 0, pos;

            for (pos = 0; pos < (*iter).length(); pos++) {
                if ((*iter)[pos] == '=') {
                    dest = (*iter).substr(start, pos);
                    start = pos + 1;
                }
                if ((*iter)[pos] == ';') {
                    comp = (*iter).substr(start, pos-start);
                    jmp = (*iter).substr(pos+1);
                    break;
                }
            }
            if (comp.length() == 0) {
                comp = (*iter).substr(start);
            }

            code += compTable[comp] + destTable[dest] + jmpTable[jmp];
        }

        out_file << code << endl;
        code.clear();
    }
    out_file.close();
}
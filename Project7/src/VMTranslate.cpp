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
unordered_map<string, string> segmentMap = {
    {"local", "LCL"},
    {"argument", "ARG"},
    {"this", "THIS"},
    {"that", "THAT"}
};

// Dest table:


void text_strip(list<string> &, string);       // Read in the asm file and strip all the space and comments.
void remove_label(list<string> &);               // Remove program label and update symbolTable.
void decode_instr(list<string> &, string);       // Decode the asm instructions and write to output .hdl file.

// Functions handling each type of vm instruction.
string instr_add();
string instr_sub();
string instr_neg();
string instr_and();
string instr_or();
string instr_not();
string instr_eq(int);
string instr_gt(int);
string instr_lt(int);
string instr_push(string, int);
string instr_pop(string, int);


int main(int argc, char** argv) {
    // Obtain input file path and output file path. Input file must end with '.asm'.
    string in_path(argv[1]);
    string out_path(in_path, 0, in_path.length()-2);
    out_path += "asm";
    cout << "Start translating vm file: " << in_path << endl;

    // Use a list to store the vm program:
    list<string> text;
    text_strip(text, in_path);       //Read in the vm file and strip comments.

    // Decode the asm instructions and write to output .asm file:
    decode_instr(text, out_path);

    return 0;
}


void text_strip(list<string> &in_text, string in_path) {
    // Read in vm file.
    ifstream in_file(in_path.c_str(), ios::in);

    string in_line, out_line;
    unsigned ptr, flag = 0;

    cout << "Start reading file\n";
    while (!in_file.eof()) {
        getline(in_file, in_line);
        for (ptr = 0; ptr < in_line.length(); ptr++) {
            if (flag) {
                // in the /*...*/ segment
                if (in_line[ptr] == '*' && ptr < in_line.length()-1 && in_line[ptr+1] == '/') {
                    flag = 0;
                    ptr++;
                }
            }
            else {
                if (in_line[ptr] == '/' && ptr < in_line.length()-1 && in_line[ptr+1] == '/')
                    break;
                else if (in_line[ptr] == '/' && ptr < in_line.length()-1 && in_line[ptr+1] == '*') {
                    flag = 1;
                    ptr++;
                }
                else if (in_line[ptr] != ' ' && isspace(in_line[ptr]))
                    continue;
                else {
                    out_line += in_line[ptr];
                }
            }
        }
        if (out_line.length() > 0) {
            in_text.push_back(out_line);
        }
        in_line.clear();
        out_line.clear();
    }

    in_file.close();
}

void decode_instr(list<string> &in_text, string out_path) {
    ofstream out_file(out_path.c_str(), ios::out);
    cout << "Writing asm file to path: " << out_path << endl;

    list<string>::iterator iter;
    string code;
    int false_num = 0;
    for (iter = in_text.begin(); iter != in_text.end(); iter++) {
        // Cannot be empty line.
        if (*iter == "add") {
            code = instr_add();
        }
        else if (*iter == "sub") {
            code = instr_sub();
        }
        else if (*iter == "neg") {
            code = instr_neg();
        }
        else if (*iter == "and") {
            code = instr_and();
        }
        else if (*iter == "or") {
            code = instr_or();
        }
        else if (*iter == "not") {
            code = instr_not();
        }
        else if (*iter == "eq") {
            code = instr_eq(false_num++);
        }
        else if (*iter == "gt") {
            code = instr_gt(false_num++);
        }
        else if (*iter == "lt") {
            code = instr_lt(false_num++);
        }
        else {
            // memory access
            unsigned ptr0 = 0, ptr = 1, offset;
            string command(""), segment("");
            while (ptr < (*iter).length()) {
                if ((*iter)[ptr] == ' ') {
                    if (command.length() == 0)
                        command = (*iter).substr(ptr0, ptr-ptr0);
                    else
                        segment = (*iter).substr(ptr0, ptr-ptr0);
                    ptr0 = ptr + 1;
                    ptr = ptr0 + 1;
                }
                else
                    ptr++;
            }
            offset = stoi((*iter).substr(ptr0, ptr-ptr0));
            if (command == "push") {
                code = instr_push(segment, offset);
            }
            else {
                code = instr_pop(segment, offset);
            }
        }
        out_file << code;
        code.clear();
    }
    out_file.close();
}

string instr_add() {
    return  string("@SP\n") +
            "AM=M-1\n" +
            "D=M\n" +
            "A=A-1\n" +
            "M=M+D\n";
}

string instr_sub() {
    return  string("@SP\n") +
            "AM=M-1\n" +
            "D=M\n" +
            "A=A-1\n" +
            "M=M-D\n";
}

string instr_neg() {
    return  string("@SP\n") +
            "A=M-1\n" +
            "M=-M\n";
}

string instr_and() {
    return  string("@SP\n") +
            "AM=M-1\n" +
            "D=M\n" +
            "A=A-1\n" +
            "M=D&M\n";
}

string instr_or() {
    return  string("@SP\n") +
            "AM=M-1\n" +
            "D=M\n" +
            "A=A-1\n" +
            "M=D|M\n";
}

string instr_not() {
    return  string("@SP\n") +
            "A=M-1\n" +
            "M=!M\n";
}

string instr_eq(int false_label) {
    return  string("@SP\n") +
            "AM=M-1\n" +
            "D=M\n" +
            "A=A-1\n" +
            "D=M-D\n" +
            "@FALSE" + to_string(false_label) + "\n" +
            "D;JNE\n" +
            "@SP\n" +
            "A=M-1\n" +
            "M=-1\n" +
            "@END" + to_string(false_label) + "\n" +
            "0;JMP\n" +
            "(FALSE" + to_string(false_label) + ")\n" +
            "@SP\n" +
            "A=M-1\n" +
            "M=0\n" +
            "(END" + to_string(false_label) + ")\n";
}

string instr_gt(int false_label) {
    return  string("@SP\n") +
            "AM=M-1\n" +
            "D=M\n" +
            "A=A-1\n" +
            "D=M-D\n" +
            "@FALSE" + to_string(false_label) + "\n" +
            "D;JLE\n" +
            "@SP\n" +
            "A=M-1\n" +
            "M=-1\n" +
            "@END" + to_string(false_label) + "\n" +
            "0;JMP\n" +
            "(FALSE" + to_string(false_label) + ")\n" +
            "@SP\n" +
            "A=M-1\n" +
            "M=0\n" +
            "(END" + to_string(false_label) + ")\n";
}

string instr_lt(int false_label) {
    return  string("@SP\n") +
            "AM=M-1\n" +
            "D=M\n" +
            "A=A-1\n" +
            "D=M-D\n" +
            "@FALSE" + to_string(false_label) + "\n" +
            "D;JGE\n" +
            "@SP\n" +
            "A=M-1\n" +
            "M=-1\n" +
            "@END" + to_string(false_label) + "\n" +
            "0;JMP\n" +
            "(FALSE" + to_string(false_label) + ")\n" +
            "@SP\n" +
            "A=M-1\n" +
            "M=0\n" +
            "(END" + to_string(false_label) + ")\n";
}

string instr_push(string segment, int offset) {
    string code;
    if (segment == "constant") {
        code = "@" + to_string(offset) + "\n" +
                "D=A\n" +
                "@SP\n" +
                "A=M\n" +
                "M=D\n" +
                "@SP\n" +
                "M=M+1\n";
    }
    else if (segment == "pointer") {
        if (offset == 0) {
            code = "@THIS\n";
        }
        else {
            code = "@THAT\n";
        }
        code += string("D=M\n") +
                "@SP\n" +
                "A=M\n" +
                "M=D\n" +
                "@SP\n" +
                "M=M+1\n";
    }
    else if (segment == "temp") {
        code = "@R" + to_string(offset+5) + "\n" +
                "D=M\n" +
                "@SP\n" +
                "A=M\n" +
                "M=D\n" +
                "@SP\n" +
                "M=M+1\n";
    }
    else if (segment == "static") {
        code = "@Static" + to_string(offset) + "\n" +
                "D=M\n" +
                "@SP\n" +
                "A=M\n" +
                "M=D\n" +
                "@SP\n" +
                "M=M+1\n";
    }
    else {
        // local, argument, this and that segment
        code = "@" + segmentMap[segment] + "\n" +
                "D=M\n" +
                "@" + to_string(offset) + "\n" +
                "A=D+A\n" +
                "D=M\n" +
                "@SP\n" +
                "A=M\n" +
                "M=D\n" +
                "@SP\n" +
                "M=M+1\n";
    }
    return code;
}

string instr_pop(string segment, int offset) {
    string code;
    if (segment == "pointer") {
        code = string("@SP\n") +
                "AM=M-1\n" +
                "D=M\n";
        if (offset == 0) {
            code += "@THIS\n";
        }
        else {
            code += "@THAT\n";
        }
        code += "M=D\n";
    }
    else if (segment == "temp") {
        code = string("@SP\n") +
                "AM=M-1\n" +
                "D=M\n" +
                "@R" + to_string(offset+5) + "\n" +
                "M=D\n";
    }
    else if (segment == "static") {
        code = string("@SP\n") +
                "AM=M-1\n" +
                "D=M\n" +
                "@Static" + to_string(offset) + "\n" +
                "M=D\n";
    }
    else {
        // local, argument, this and that segment
        code = "@" + segmentMap[segment] + "\n" +
                "D=M\n" +
                "@" + to_string(offset) + "\n" +
                "D=D+A\n" +
                "@R13\n" +
                "M=D\n" +
                "@SP\n" +
                "AM=M-1\n" +
                "D=M\n" +
                "@R13\n" +
                "A=M\n"
                "M=D\n";
    }
    return code;
}

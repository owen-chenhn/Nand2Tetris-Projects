#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <list>
#include <vector>
#include <ctype.h>
#include <io.h>

using namespace std;


// Symbol table:
unordered_map<string, string> segmentMap = {
    {"local", "LCL"},
    {"argument", "ARG"},
    {"this", "THIS"},
    {"that", "THAT"}
};

void bootstrap(ofstream &);                      // Write bootstrap code to output file.
void text_strip(list<string> &, string);       // Read in the asm file and strip all the space and comments.
void decode_instr(list<string> &, string, ofstream &);       // Decode the asm instructions and write to output .hdl file.

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
string instr_push(string, int, string);
string instr_pop(string, int, string);
// program flow:
string instr_label(string);
string instr_goto(string);
string instr_if_goto(string);
// subroutine call:
string instr_func_f(string, int);  // VM instruction of (function f nVars).
string instr_call_f(string, int, int, string); // VM instruction of (call f nArgs).
string instr_return();  // VM instruction of return.


int main(int argc, char** argv) {
    string input(argv[1]);
    char delimiter = '\\';      // Path delimiter of different system. Linux is '/' and windows is '\\'.
    if (input[input.length()-1] == delimiter) {
        input.resize(input.length()-1);
    }

    // Parse the directory name (as the output file name)
    int head = input.length() - 1, tail = input.length();
    while (head > 0 && input[head-1] != delimiter) {
        head--;
    }
    string out_path = input.substr(0, tail) + delimiter + input.substr(head, tail-head) + ".asm";
    cout << "Write asm file to: " + out_path + "\n";
    ofstream w_f(out_path.c_str(), ios::out);

    // Write bootstrap code. Comment this function call if the bootstrap code is not needed (for the first several tests).
    //bootstrap(w_f);

    // Search the vm files under input directory and process each vm file:
    string class_name, file_name, search_path = input + delimiter + "*.vm";
    long handle;
    struct _finddata_t fileinfo;
    list<string> text;

    if ((handle = _findfirst(search_path.c_str(), &fileinfo)) == -1) {
        cout << "Error: input directory doesn't contain vm files.\n";
        return 0;
    }
    do {
        file_name = string(fileinfo.name);
        cout << "Find vm file: " << file_name << endl;
        class_name = file_name.substr(0, file_name.length()-3);

        // strip the comments and empty lines
        text_strip(text, input+delimiter+file_name);

        // Decode the vm instructions and write to output file:
        cout << "Translate the class: " << class_name << endl;
        decode_instr(text, class_name, w_f);

        file_name.clear();
        class_name.clear();
        text.clear();
    }while (_findnext(handle, &fileinfo) == 0);

    _findclose(handle);
    w_f.close();
    cout << "Translation finished.\n";
    return 0;
}


void bootstrap(ofstream &w_f) {
    // Bootstrap code: (1)set SP = 256; (2)call Sys.init
    w_f <<  "@256\n" <<
            "D=A\n" <<
            "@SP\n" <<
            "M=D\n" <<
            instr_call_f("Sys.init", 0, 0, "Boot");
}

void text_strip(list<string> &in_text, string in_path) {
    // Read in vm file.
    cout << "Read in file: " << in_path <<endl;
    ifstream in_file(in_path.c_str(), ios::in);

    string in_line, out_line;
    unsigned ptr, flag = 0;

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

void decode_instr(list<string> &in_text, string class_name, ofstream &w_f) {
    list<string>::iterator iter;
    string code;
    // Label the unique jump location of each boolean operation and the unique return address of each function call.
    // Note that func_call_num starts from 1 because in bootstrapping the function call of Sys.init have used number 0.
    unsigned false_num = 0, func_call_num = 1;
    unsigned head, tail;
    vector<string> commands;
    for (iter = in_text.begin(); iter != in_text.end(); iter++) {
        // split the command by ' ' and store in the vector commands
        head = 0, tail = 0;
        while (tail < (*iter).length()) {
            if ((*iter)[tail] == ' ') {
                commands.push_back((*iter).substr(head, tail - head));
                head = tail + 1;
                tail = head + 1;
            }
            else
                tail++;
        }
        if (head < (*iter).length()) {
            commands.push_back((*iter).substr(head, tail - head));
        }

        if (commands[0] == "add") {
            code = instr_add();
        }
        else if (commands[0] == "sub") {
            code = instr_sub();
        }
        else if (commands[0] == "neg") {
            code = instr_neg();
        }
        else if (commands[0] == "and") {
            code = instr_and();
        }
        else if (commands[0] == "or") {
            code = instr_or();
        }
        else if (commands[0] == "not") {
            code = instr_not();
        }
        else if (commands[0] == "eq") {
            code = instr_eq(false_num++);
        }
        else if (commands[0] == "gt") {
            code = instr_gt(false_num++);
        }
        else if (commands[0] == "lt") {
            code = instr_lt(false_num++);
        }
        else if (commands[0] == "push") {
            code = instr_push(commands[1], stoi(commands[2]), class_name);
        }
        else if (commands[0] == "pop") {
            code = instr_pop(commands[1], stoi(commands[2]), class_name);
        }
        else if (commands[0] == "label") {
            code = instr_label(commands[1]);
        }
        else if (commands[0] == "goto") {
            code = instr_goto(commands[1]);
        }
        else if (commands[0] == "if-goto") {
            code = instr_if_goto(commands[1]);
        }
        else if (commands[0] == "function") {
            code = instr_func_f(commands[1], stoi(commands[2]));
        }
        else if (commands[0] == "call") {
            code = instr_call_f(commands[1], stoi(commands[2]), func_call_num++, class_name);
        }
        else if (commands[0] == "return") {
            code = instr_return();
        }

        w_f << code;
        code.clear();
        commands.clear();
    }
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

string instr_push(string segment, int offset, string class_name) {
    string code;
    if (segment == "constant") {
        code = "@" + to_string(offset) + "\n" +
                "D=A\n" +
                "@SP\n" +
                "AM=M+1\n" +
                "A=A-1\n" +
                "M=D\n";
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
                "AM=M+1\n" +
                "A=A-1\n" +
                "M=D\n";
    }
    else if (segment == "temp") {
        code = "@R" + to_string(offset+5) + "\n" +
                "D=M\n" +
                "@SP\n" +
                "AM=M+1\n" +
                "A=A-1\n" +
                "M=D\n";
    }
    else if (segment == "static") {
        code = "@" + class_name + ".Static" + to_string(offset) + "\n" +
                "D=M\n" +
                "@SP\n" +
                "AM=M+1\n" +
                "A=A-1\n" +
                "M=D\n";
    }
    else {
        // local, argument, this and that segment
        code = "@" + segmentMap[segment] + "\n" +
                "D=M\n" +
                "@" + to_string(offset) + "\n" +
                "A=D+A\n" +
                "D=M\n" +
                "@SP\n" +
                "AM=M+1\n" +
                "A=A-1\n" +
                "M=D\n";
    }
    return code;
}

string instr_pop(string segment, int offset, string class_name) {
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
                "@" + class_name + ".Static" + to_string(offset) + "\n" +
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

string instr_label(string label) {
    return "(" + label + ")\n";
}

string instr_goto(string label) {
    return "@" + label + "\n" +
            "0;JMP\n";
}

string instr_if_goto(string label) {
    return string("@SP\n") +
            "AM=M-1\n" +
            "D=M\n" +
            "@" + label + "\n"
            "D;JNE\n";
}

string instr_func_f(string func_name, int nVars) {
    // func_name: name of the function
    // nVars: number of local variables
    string code = "(" + func_name + ")\n";
    for (; nVars > 0; nVars--) {
        code += instr_push("constant", 0, "");
    }
    return code;
}

string instr_call_f(string func_name, int nArgs, int func_call_num, string class_name) {
    // func_name: name of the function
    // nArgs: number of arguments
    // func_call_num: number used to label the return address
    return "@" + class_name + ".Call" + to_string(func_call_num) + "\n" +   // push return address
            "D=A\n" +
            "@SP\n" +
            "AM=M+1\n" +
            "A=A-1\n" +
            "M=D\n" +

            "@LCL\n" +      // push LCL
            "D=M\n" +
            "@SP\n" +
            "AM=M+1\n" +
            "A=A-1\n" +
            "M=D\n" +

            "@ARG\n" +      // push ARG
            "D=M\n" +
            "@SP\n" +
            "AM=M+1\n" +
            "A=A-1\n" +
            "M=D\n" +

            "@THIS\n" +      // push THIS
            "D=M\n" +
            "@SP\n" +
            "AM=M+1\n" +
            "A=A-1\n" +
            "M=D\n" +

            "@THAT\n" +      // push THAT
            "D=M\n" +
            "@SP\n" +
            "AM=M+1\n" +
            "A=A-1\n" +
            "M=D\n" +

            "@SP\n" +        // ARG = SP - nArgs - 5
            "D=M\n" +
            "@" + to_string(nArgs) + "\n" +
            "D=D-A\n" +
            "@5\n" +
            "D=D-A\n" +
            "@ARG\n" +
            "M=D\n" +

            "@SP\n" +      // LCL = SP
            "D=M\n" +
            "@LCL\n" +
            "M=D\n" +

            instr_goto(func_name) +
            "(" + class_name + ".Call" +  to_string(func_call_num) + ")\n";
}

string instr_return() {
    return string("@LCL\n") +   // set FRAME = LCL
            "D=M\n" +
            "@FRAME\n" +
            "M=D\n" +

            "@5\n" +      // set RET = *(FRAME - 5)
            "A=D-A\n" +
            "D=M\n" +
            "@RET\n" +
            "M=D\n" +

            "@SP\n" +      // set *ARG = pop()
            "AM=M-1\n" +
            "D=M\n" +
            "@ARG\n" +
            "A=M\n" +
            "M=D\n" +

            "@ARG\n" +      // set SP = ARG + 1
            "D=M+1\n" +
            "@SP\n" +
            "M=D\n" +

            "@FRAME\n" +      // THAT = *(FRAME - 1)
            "A=M-1\n" +
            "D=M\n" +
            "@THAT\n" +
            "M=D\n" +

            "@FRAME\n" +      // THIS = *(FRAME - 2)
            "D=M\n" +
            "@2\n" +
            "A=D-A\n" +
            "D=M\n" +
            "@THIS\n" +
            "M=D\n" +

            "@FRAME\n" +      // ARG = *(FRAME - 3)
            "D=M\n" +
            "@3\n" +
            "A=D-A\n" +
            "D=M\n" +
            "@ARG\n" +
            "M=D\n" +

            "@FRAME\n" +      // LCL = *(FRAME - 4)
            "D=M\n" +
            "@4\n" +
            "A=D-A\n" +
            "D=M\n" +
            "@LCL\n" +
            "M=D\n" +

            "@RET\n" +      // goto *(RET)
            "A=M\n" +
            "0;JMP\n";
}

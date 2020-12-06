#include "JackCompiler.h"
#include <iostream>
#include <ctype.h>
#include <assert.h>


unordered_set<string> Compiler::KEYWORDS = {
    "class",
    "constructor",
    "function",
    "method",
    "field",
    "static",
    "var",
    "int",
    "char",
    "boolean",
    "void",
    "true",
    "false",
    "null",
    "this",
    "let",
    "do",
    "if",
    "else",
    "while",
    "return"
};

unordered_set<char> Compiler::SYMBOLS = {
    '{',
    '}',
    '(',
    ')',
    '[',
    ']',
    '.',
    ',',
    ';',
    '+',
    '-',
    '*',
    '/',
    '&',
    '|',
    '<',
    '>',
    '=',
    '~'
};

unordered_map<string, string> Compiler::OPERANDS = {
    {"+", "add"},
    {"-", "sub"},
    {"*", "call Math.multiply 2"},
    {"/", "call Math.divide 2"},
    {"&", "and"},
    {"|", "or"},
    {"<", "lt"},
    {">", "gt"},
    {"=", "eq"}
};

unordered_map<string, string> Compiler::UNARY_OPS = {
    {"-", "neg"},
    {"~", "not"}
};


void Compiler::compile() {
    cout << "Compiling file: " << source_path << endl;
    tokenize();
    parse();
    cout << "VM commands written to: " << source_path.substr(0, source_path.size()-5)+".vm" << endl;
}


void Compiler::tokenize() {
    ifstream f(source_path, ios::in);
    string cur_token;   // Current token
    string line;        // stores each line
    unsigned i;
    // Set flages for each situations:
    bool comment_flag = false, str_flag = false, digit_flag = false, alpha_flag = false;
    while (!f.eof()) {
        getline(f, line);
        // Scan each character:
        for(i = 0; i < line.size(); i++) {
            if (comment_flag) {
                // In the /* ... */ comment segment
                if (line[i]=='*' && i<line.size()-1 && line[i+1]=='/') {
                    comment_flag = false;
                    i++;
                }
            }
            else if (str_flag) {
                // tokenizing a string constant
                if (line[i] == '"') {
                    // end the string constant
                    str_flag = false;
                    tokens.push_back(token(cur_token, "stringConstant"));
                    cur_token.clear();
                }
                else {
                    cur_token += line[i];
                }
            }
            else if (digit_flag) {
                // tokenizing a integer constant
                if (!isdigit(line[i])) {
                    digit_flag = false;
                    tokens.push_back(token(cur_token, "integerConstant"));
                    cur_token.clear();
                    i--;
                }
                else {
                    cur_token += line[i];
                }
            }
            else if (alpha_flag) {
                // tokenizing a keyword or identifier
                if (isspace(line[i]) || SYMBOLS.count(line[i])) {
                    alpha_flag = false;
                    if (KEYWORDS.count(cur_token)) {
                        // the token is a keyword
                        tokens.push_back(token(cur_token, "keyword"));
                    }
                    else {
                        // the token is an identifier
                        tokens.push_back(token(cur_token, "identifier"));
                    }
                    cur_token.clear();
                    i--;
                }
                else {
                    cur_token += line[i];
                }
            }

            else {
                // finding a new token type
                if (line[i]=='/' && i<line.size()-1 && line[i+1]=='/')
                    break;
                else if (line[i]=='/' && i<line.size()-1 && line[i+1]=='*') {
                    comment_flag = true;
                    i++;
                }
                else if (!isspace(line[i])) {
                    // meet a new character:
                    if (SYMBOLS.count(line[i])) {
                        // tokenize a symbol
                        tokens.push_back(token(line.substr(i,1), "symbol"));
                    }
                    else if (line[i] == '"') {
                        // tokenize a string constant
                        str_flag = true;
                    }
                    else if (isdigit(line[i])) {
                        // tokenize a number
                        digit_flag = true;
                        cur_token += line[i];
                    }
                    else {
                        // tokenize a keyword or identifier
                        alpha_flag = true;
                        cur_token += line[i];
                    }
                }
            }
        }
        line.clear();
    }
    f.close();
}


string Compiler::find(string var) {
    for (unordered_map<string, table_entry>& table: symbol_tables) 
    {
        if (table.count(var) == 1) {
            table_entry &e = table.at(var);
            return e.segment + " " + to_string(e.num);
        }
    }
    // The identifier cannot be found:
    return "";
}


string Compiler::find_type(string var) {
    for (unordered_map<string, table_entry>& table: symbol_tables) 
    {
        if (table.count(var) == 1) {
            table_entry &e = table.at(var);
            return e.type;
        }
    }
    // The identifier cannot be found:
    return "";
}


void Compiler::parse() {
    w_f.open(source_path.substr(0, source_path.size()-5)+".vm", ios::out);

    assert (tokens[0].value == "class");
    parseClass(0);      // method argument "idx" always points the next tokens to be parsed.

    w_f.close();
}


int Compiler::parseClass(int idx) {
    // No need to check the grammer. It is always checked before the method is called.
    assert (tokens[idx++].value == "class");

    // maintain the className, static_num, field_num, if_num and while_num
    class_name = tokens[idx++].value;
    static_num = field_num = if_num = while_num = 0; 

    // construct symbol table for the class
    symbol_tables.emplace_front();
    assert (symbol_tables.size() == 1);

    // '{'
    assert (tokens[idx++].value == "{");

    // classVarDec(s)
    while (tokens[idx].value == "static" || tokens[idx].value == "field") {
        if (tokens[idx].value == "static") { idx = parseClassVarDec(idx, static_num); }
        else { idx = parseClassVarDec(idx, field_num); }
    }

    // subroutineDec(s)
    while (tokens[idx].value == "constructor" || tokens[idx].value == "function" || tokens[idx].value == "method") {
        idx = parseSubroutineDec(idx);
    }

    // '}'
    assert (tokens[idx++].value == "}");
    assert ((unsigned) idx == tokens.size());
    return idx;
}


int Compiler::parseClassVarDec(int idx, int &num) {
    string seg, type, name; 
    unordered_map<string, table_entry> &table = symbol_tables.front();

    if (tokens[idx++].value == "static") { seg = "static"; }
    else { seg = "this"; }      // field var

    type = tokens[idx++].value;
    name = tokens[idx++].value;
    table[name] = table_entry(type, seg, num++);

    while (tokens[idx++].value == ",") {
        name = tokens[idx++].value;
        table[name] = table_entry(type, seg, num++);
    }
    return idx;
}


int Compiler::parseSubroutineDec(int idx) {
    // construct a symbol table of the current scope. 
    symbol_tables.emplace_front();
    unordered_map<string, table_entry> &table = symbol_tables.front();

    string subroutine_type = tokens[idx++].value;
    string return_type = tokens[idx++].value;
    string subroutine_name = tokens[idx++].value;

    string type, name;
    arg_num = 0, local_num = 0;     // reset arg_num and local_num for every subroutine

    if (subroutine_type == "method") {
        table[string("this")] = table_entry(class_name, "argument", arg_num++);
    }
    // parameterList
    assert (tokens[idx++].value == "(");
    if (tokens[idx].value != ")") {
        type = tokens[idx++].value;   // type
        name = tokens[idx++].value;   // varName
        table[name] = table_entry(type, "argument", arg_num++);

        while (tokens[idx].value == ",") {
            idx++;
            type = tokens[idx++].value;   // type
            name = tokens[idx++].value;   // varName
            table[name] = table_entry(type, "argument", arg_num++);
        }
    }
    assert (tokens[idx++].value == ")");

    // subroutineBody
    assert (tokens[idx++].value == "{");

    // varDec
    while (tokens[idx].value == "var") {
        idx++;
        type = tokens[idx++].value;   // type
        name = tokens[idx++].value;   // varName
        table[name] = table_entry(type, "local", local_num++);

        while (tokens[idx++].value == ",") {
            name = tokens[idx++].value;   // varName
            table[name] = table_entry(type, "local", local_num++);
        }
    }

    // write VM function definition command
    w_f << "function " << class_name << "." << subroutine_name << " " << local_num << endl;
    if (subroutine_type == "constructor") {
        w_f << "push constant " << field_num << endl;
        w_f << "call Memory.alloc 1" << endl;
        w_f << "pop pointer 0" << endl;
    }
    else if (subroutine_type == "method") {
        w_f << "push argument 0" << endl;
        w_f << "pop pointer 0" << endl;
    }

    // statements
    if (tokens[idx].value != "}")
        idx = parseStatements(idx);

    assert (tokens[idx++].value == "}");

    // destruct the current symbol table. 
    symbol_tables.pop_front();
    return idx;
}


int Compiler::parseStatements(int idx) {
    while (true) {
        if (tokens[idx].value == "let") {
            idx = parseLetStatement(idx+1);
        }
        else if (tokens[idx].value == "if") {
            idx = parseIfStatement(idx+1, this->if_num);
        }
        else if (tokens[idx].value == "while") {
            idx = parseWhileStatement(idx+1, this->while_num);
        }
        else if (tokens[idx].value == "do") {
            idx = parseDoStatement(idx+1);
        }
        else if (tokens[idx].value == "return") {
            idx = parseReturnStatement(idx+1);
        }
        else
            break;
    }
    return idx;
}


int Compiler::parseLetStatement(int idx) {
    // destination varName
    string dest = tokens[idx++].value;
    if (tokens[idx].value == "=") {
        idx = parseExpression(idx+1);
        w_f << "pop " << find(dest) << endl;
    }
    else {
        // array element assignment
        w_f << "push " << find(dest) << endl;
        assert (tokens[idx++].value == "[");
        idx = parseExpression(idx);
        assert (tokens[idx++].value == "]");
        assert (tokens[idx++].value == "=");
        w_f << "add" << endl;
        idx = parseExpression(idx);
        w_f << "pop temp 0" << endl;
        w_f << "pop pointer 1" << endl;
        w_f << "push temp 0" << endl;
        w_f << "pop that 0" << endl;
    }

    // ';'
    assert (tokens[idx++].value == ";");
    return idx;
}


int Compiler::parseIfStatement(int idx, int num) {
    this->if_num = num + 1;
    assert (tokens[idx++].value == "(");
    // expression
    idx = parseExpression(idx);
    assert (tokens[idx++].value == ")");

    w_f << "not" << endl;
    w_f << "if-goto " << "IFBrunch_" << num << endl;

    assert (tokens[idx++].value == "{");
    // statements
    if (tokens[idx].value != "}") 
        idx = parseStatements(idx); 
    assert (tokens[idx++].value == "}");

    if (tokens[idx].value == "else") {
        idx++;
        w_f << "goto " << "IFEnd_" << num << endl;
        w_f << "label " << "IFBrunch_" << num << endl;

        assert (tokens[idx++].value == "{");
        // statements
        if (tokens[idx].value != "}")
            idx = parseStatements(idx);
        assert (tokens[idx++].value == "}");
        
        w_f << "label " << "IFEnd_" << num << endl;
    }
    else {
        w_f << "label " << "IFBrunch_" << num << endl;
    }

    return idx;
}


int Compiler::parseWhileStatement(int idx, int num) {
    this->while_num = num + 1;
    w_f << "label " << "While_" << num << endl;

    assert (tokens[idx++].value == "(");
    // expression
    idx = parseExpression(idx);
    assert (tokens[idx++].value == ")");

    w_f << "not" << endl;
    w_f << "if-goto " << "WhileEnd_" << num << endl;

    assert (tokens[idx++].value == "{");
    // statements
    if (tokens[idx].value != "}")
        idx = parseStatements(idx);
    assert (tokens[idx++].value == "}");

    w_f << "goto " << "While_" << num << endl;
    w_f << "label " << "WhileEnd_" << num << endl;
    return idx;
}


int Compiler::parseDoStatement(int idx) {
    // subroutine call
    idx = parseSubroutineCall(idx);

    // clear the return value of void subroutine
    w_f << "pop temp 0" << endl;
    assert (tokens[idx++].value == ";");
    return idx;
}


int Compiler::parseReturnStatement(int idx) {
    if (tokens[idx].value == ";") {
        // void
        w_f << "push constant 0" << endl;
    }
    else if (tokens[idx].value == "this") {
        w_f << "push pointer 0" << endl;
        idx++;
    }
    else {
        // expression
        idx = parseExpression(idx);
    }
    w_f << "return" << endl;

    // ';'
    assert (tokens[idx++].value == ";");
    return idx;
}


int Compiler::parseSubroutineCall(int idx) {
    int n_args = 0;
    string subroutine_name, called_class, obj_map;

    if (tokens[idx+1].value == ".") {
        subroutine_name = tokens[idx+2].value;
        obj_map = find(tokens[idx].value);
        if (obj_map != "") {
            called_class = find_type(tokens[idx].value);
        }
        else {
            called_class = tokens[idx].value;
        }
        idx += 3;
    }
    else {
        subroutine_name = tokens[idx].value;
        obj_map = "pointer 0";
        called_class = class_name;
        idx++;
    }
    // push the calling object as argument 0, if this is a method call
    if (obj_map != "") {
        w_f << "push " << obj_map << endl;
        n_args++;
    }

    assert (tokens[idx++].value == "(");           // '('
    // expressionList
    if (tokens[idx].value != ")") {
        // expression
        idx = parseExpression(idx);
        n_args++;
        while (tokens[idx].value == ",") {
            idx = parseExpression(idx+1);
            n_args++;
        }
    }
    assert (tokens[idx++].value == ")");
    w_f << "call " << called_class << "." << subroutine_name << " " << n_args << endl;
    
    return idx;
}


int Compiler::parseExpression(int idx) {
    // term
    idx = parseTerm(idx);

    if (OPERANDS.count(tokens[idx].value)) {
        string op = OPERANDS[tokens[idx++].value];
        idx = parseTerm(idx);
        w_f << op << endl;
    }
    return idx;
}


int Compiler::parseTerm(int idx) {
    if (tokens[idx].value == "-" || tokens[idx].value == "~") {
        // unary operand
        string op = UNARY_OPS[tokens[idx++].value];
        idx = parseTerm(idx);
        w_f << op << endl;
    }
    else if (tokens[idx].value == "(") {
        idx = parseExpression(idx+1);
        assert (tokens[idx++].value == ")");
    }
    else if (tokens[idx+1].value == "[") {      // handle array
        w_f << "push " << find(tokens[idx++].value) << endl;       // arr name
        idx = parseExpression(idx+1);
        w_f << "add" << endl;
        w_f << "pop pointer 1" << endl;
        w_f << "push that 0" << endl;
        assert (tokens[idx++].value == "]");
    }
    else if (tokens[idx+1].value == "(" || tokens[idx+1].value == ".") {
        idx = parseSubroutineCall(idx);
    }
    else
        // push single term
        if (tokens[idx].type == "integerConstant") {
            w_f << "push constant " << tokens[idx++].value << endl;
        }
        else if (tokens[idx].type == "stringConstant") {
            string s = tokens[idx++].value;
            int len = s.size();
            w_f << "push constant " << len << endl;
            w_f << "call String.new 1" << endl;
            for (char c: s) {
                w_f << "push constant " << (int) c << endl;
                w_f << "call String.appendChar 2" << endl;
            }
        }
        else if (tokens[idx].value == "this") {
            w_f << "push pointer 0" << endl;
            idx++;
        }
        else if (tokens[idx].value == "false" || tokens[idx].value == "null") {
            w_f << "push constant 0" << endl;
            idx++;
        }
        else if (tokens[idx].value == "true") {
            w_f << "push constant 1" << endl;
            w_f << "neg" << endl;
            idx++;
        }
        else {
            // variable
            w_f << "push " << find(tokens[idx++].value) << endl;
        }
    return idx;
}
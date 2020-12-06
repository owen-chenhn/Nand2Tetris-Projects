#ifndef JACKCOMPILER_H
#define JACKCOMPILER_H

#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <list>

using namespace std;

// Struct to store a token. The field type is one of: "keyword", "symbol", "intergerConstant", "stringConstant" and "identifier".
struct token {
    string value;
    string type;
    token(string v, string t)
        : value(v), type(t) {}
};


// Struct of table entry for symbol tables.
struct table_entry {
    string type;        // type of the symbol, should be one of: "int", "boolean", "char".
    string segment;     // the virtual vm memory segment of this symbol, like "local", "argument", "static" or "this" (which is "field" var of an object).
    int num;            // the number of this symbol.
    table_entry(string t, string s, int n)
        : type(t), segment(s), num(n) {}
    table_entry()
        : type(""), segment(""), num(0) {}
};


class Compiler {
    public:
        // Constructor. Input is the path to each jack file.
        Compiler(string path)
            : source_path(path) {}

        void compile();     // Driver function to compile a single jack file.

    private:
        // pre-construct the keyword set and symbol set for ease of looking up
        static unordered_set<string> KEYWORDS;
        static unordered_set<char> SYMBOLS;
        static unordered_map<string, string> OPERANDS;
        static unordered_map<string, string> UNARY_OPS;

        string source_path;         // path to the source jack file
        vector<token> tokens;       // vector of tokens
        ofstream w_f;               // file handler for writing vm commands. It is used in parse()

        list<unordered_map<string, table_entry>> symbol_tables;

        string class_name;          // class name of this file
        int static_num, field_num;
        int arg_num, local_num;     // reset to 0 at the start of every subroutine
        int if_num, while_num;      // label the number of if and while statement

        void tokenize();    // Tokenize the source jack file and stores the tokenized results to the vector.
        void parse();       // Parse the token string and write translated vm commands to vm files.
        string find(string var);        // Map a var to "segment i" format using symbol tables. 
        string find_type(string var);   // Get the type of this var from symbol tables. 

        // Parse each type of rule. The index of the next token is returned by each parsing method.
        int parseClass(int idx);
        int parseClassVarDec(int idx, int &);
        int parseSubroutineDec(int idx);
        int parseStatements(int idx);
        int parseLetStatement(int idx);
        int parseIfStatement(int idx, int num);
        int parseWhileStatement(int idx, int num);
        int parseDoStatement(int idx);
        int parseReturnStatement(int idx);
        int parseSubroutineCall(int idx);
        int parseExpression(int idx);
        int parseTerm(int idx);
};


#endif  // JACKCOMPILER_H

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <ctype.h>
#include <assert.h>
#include <dirent.h>

using namespace std;


char delimiter = '/';      // Path delimiter of different systems: should be '/' on linux while '\\' on windows.

// Construct a struct to store a token. The field type is one of: "keyword", "symbol", "intergerConstant", "stringConstant" and "identifier".
// The field xml_line has the form "<type> value </type>\n".
struct token {
    string value;
    string type;
    string xml_line;
    token(string v, string t, string x)
        : value(v), type(t), xml_line(x) {}
};

// pre-construct the keyword set and symbol set for ease of looking up
unordered_set<string> keywords = {
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

unordered_set<char> symbols = {
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

unordered_map<char, string> special_symbols = {
    {'<', "&lt;"},
    {'>', "&gt;"},
    {'"', "&quot;"},
    {'&', "&amp;"}
};

unordered_set<string> operands = {
    "+",
    "-",
    "*",
    "/",
    "&",
    "|",
    "<",
    ">",
    "="
};

// The class to analyze a single jack file. It consists of tokenization and parsing.
class Analizer {
    public:
        // Constructor. Input is the path to each jack file.
        Analizer(string path)
        {
            source_path = path;
        }

        void tokenize();    // Tokenize the source jack file, given the path to the file and stores the tokenized results to the vector.
        void parse();       // Parse the token string.

    private:
        string source_path;         // path to the source jack file
        vector<token> tokens;       // vector of tokens
        ofstream w_f;               // file handler for writing to two output xml files. It is reused in tokenize() and parse(). 

        // Parse each type of rule. The index of the next token is returned by each parsing method.
        int parseClass(int idx, string indents);
        int parseClassVarDec(int idx, string indents);
        int parseSubroutineDec(int idx, string indents);
        int parseParameterList(int idx, string indents);
        int parseSubroutineBody(int idx, string indents);
        int parseVarDec(int idx, string indents);
        int parseStatements(int idx, string indents);
        int parseLetStatement(int idx, string indents);
        int parseIfStatement(int idx, string indents);
        int parseWhileStatement(int idx, string indents);
        int parseDoStatement(int idx, string indents);
        int parseReturnStatement(int idx, string indents);
        int parseSubroutineCall(int idx, string indents);
        int parseExpression(int idx, string indents);
        int parseTerm(int idx, string indents);
        int parseExpressionList(int idx, string indents);
};


int main(int argc, char* argv[]) {
    string input(argv[1]);

    if (input.size() > 5 && input.substr(input.size()-5, 5) == ".jack") {
        // Single jack file input
        Analizer analizer(input);
        analizer.tokenize();
        analizer.parse();
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
                    Analizer analizer(input + f_name);
                    analizer.tokenize();
                    analizer.parse();
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


void Analizer::tokenize() {
    ifstream f(source_path, ios::in);
    cout << "Tokenizing file: " << source_path << endl;

    w_f.open(source_path.substr(0, source_path.size()-5)+"T_Mine.xml", ios::out);
    w_f << "<tokens>\n";

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
                    w_f << "<stringConstant> " << cur_token << " </stringConstant>\n";
                    tokens.push_back(token(cur_token, "stringConstant", "<stringConstant> "+cur_token+" </stringConstant>\n"));
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
                    w_f << "<integerConstant> " << cur_token << " </integerConstant>\n";
                    tokens.push_back(token(cur_token, "integerConstant", "<integerConstant> "+cur_token+" </integerConstant>\n"));
                    cur_token.clear();
                    i--;
                }
                else {
                    cur_token += line[i];
                }
            }
            else if (alpha_flag) {
                // tokenizing a keyword or identifier
                if (isspace(line[i]) || symbols.count(line[i])) {
                    alpha_flag = false;
                    if (keywords.count(cur_token)) {
                        // the token is a keyword
                        w_f << "<keyword> " << cur_token << " </keyword>\n";
                        tokens.push_back(token(cur_token, "keyword", "<keyword> "+cur_token+" </keyword>\n"));
                    }
                    else {
                        // the token is an identifier
                        w_f << "<identifier> " << cur_token << " </identifier>\n";
                        tokens.push_back(token(cur_token, "identifier", "<identifier> "+cur_token+" </identifier>\n"));
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
                    if (symbols.count(line[i])) {
                        // tokenize a symbol
                        if (special_symbols.count(line[i])) {
                            w_f << "<symbol> " << special_symbols[line[i]] << " </symbol>\n";
                            tokens.push_back(token(line.substr(i,1), "symbol", "<symbol> "+special_symbols[line[i]]+" </symbol>\n"));
                        }
                        else {
                            w_f << "<symbol> " << line[i] << " </symbol>\n";
                            tokens.push_back(token(line.substr(i,1), "symbol", "<symbol> "+line.substr(i,1)+" </symbol>\n"));
                        }
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
    w_f << "</tokens>\n";
    w_f.close();
}

void Analizer::parse() {
    cout << "Parsing tokens ...\n";
    w_f.open(source_path.substr(0, source_path.size()-5)+"_Mine.xml", ios::out);

    assert (tokens[0].value == "class");
    w_f << "<class>\n";
    parseClass(0, "\t");      // method argument "idx" always points the next tokens to be parsed.
    w_f << "</class>\n";

    w_f.close();
}

int Analizer::parseClass(int idx, string indents) {
    // No need to check the grammer. It is always checked before the method is called.
    // write "class"
    w_f << indents << tokens[idx++].xml_line;

    // write className
    assert (tokens[idx].type == "identifier");
    w_f << indents << tokens[idx++].xml_line;

    // write '{'
    assert (tokens[idx].value == "{");
    w_f << indents << tokens[idx++].xml_line;

    // write classVarDec(s)
    while (tokens[idx].value == "static" || tokens[idx].value == "field") {
        w_f << indents << "<classVarDec>\n";
        idx = parseClassVarDec(idx, indents+"\t");
        w_f << indents << "</classVarDec>\n";
    }

    // write subroutineDec(s)
    while (tokens[idx].value == "constructor" || tokens[idx].value == "function" || tokens[idx].value == "method") {
        w_f << indents << "<subroutineDec>\n";
        idx = parseSubroutineDec(idx, indents+"\t");
        w_f << indents << "</subroutineDec>\n";
    }

    // write '}'
    assert (tokens[idx].value == "}");
    w_f << indents << tokens[idx++].xml_line;
    assert ((unsigned) idx == tokens.size());
    return idx;
}

int Analizer::parseClassVarDec(int idx, string indents) {
    w_f << indents << tokens[idx++].xml_line;

    // write type
    assert (tokens[idx].type == "keyword" || tokens[idx].type == "identifier");
    w_f << indents << tokens[idx++].xml_line;

    // write varName
    assert (tokens[idx].type == "identifier");
    w_f << indents << tokens[idx++].xml_line;
    while (tokens[idx].value == ",") {
        w_f << indents << tokens[idx++].xml_line;
        assert (tokens[idx].type == "identifier");
        w_f << indents << tokens[idx++].xml_line;
    }

    // write ';'
    assert (tokens[idx].value == ";");
    w_f << indents << tokens[idx++].xml_line;
    return idx;
}

int Analizer::parseSubroutineDec(int idx, string indents) {
    w_f << indents << tokens[idx++].xml_line;

    // write return type
    assert (tokens[idx].type == "keyword" || tokens[idx].type == "identifier");
    w_f << indents << tokens[idx++].xml_line;

    // write subroutineName
    assert (tokens[idx].type == "identifier");
    w_f << indents << tokens[idx++].xml_line;

    // '('
    assert (tokens[idx].value == "(");
    w_f << indents << tokens[idx++].xml_line;

    // parameterList
    w_f << indents << "<parameterList>\n";
    if (tokens[idx].value != ")")
        idx = parseParameterList(idx, indents+"\t");
    w_f << indents << "</parameterList>\n";

    // ')'
    assert (tokens[idx].value == ")");
    w_f << indents << tokens[idx++].xml_line;

    // write subroutineBody
    w_f << indents << "<subroutineBody>\n";
    idx = parseSubroutineBody(idx, indents+"\t");
    w_f << indents << "</subroutineBody>\n";

    return idx;
}

int Analizer::parseParameterList(int idx, string indents) {
    w_f << indents << tokens[idx++].xml_line;   // type
    w_f << indents << tokens[idx++].xml_line;   // varName
    while (tokens[idx].value == ",") {
        w_f << indents << tokens[idx++].xml_line;   // ','
        w_f << indents << tokens[idx++].xml_line;   // type
        w_f << indents << tokens[idx++].xml_line;   // varName
    }
    return idx;
}

int Analizer::parseSubroutineBody(int idx, string indents) {
    // write '{'
    assert (tokens[idx].value == "{");
    w_f << indents << tokens[idx++].xml_line;

    // varDec
    while (tokens[idx].value == "var") {
        w_f << indents << "<varDec>\n";
        idx = parseVarDec(idx, indents+"\t");
        w_f << indents << "</varDec>\n";
    }

    // statements
    w_f << indents << "<statements>\n";
    if (tokens[idx].value != "}")
        idx = parseStatements(idx, indents+"\t");
    w_f << indents << "</statements>\n";

    // write '}'
    assert (tokens[idx].value == "}");
    w_f << indents << tokens[idx++].xml_line;

    return idx;
}

int Analizer::parseVarDec(int idx, string indents) {
    w_f << indents << tokens[idx++].xml_line;   // write "var"

    w_f << indents << tokens[idx++].xml_line;   // type
    w_f << indents << tokens[idx++].xml_line;   // varName
    while (tokens[idx].value == ",") {
        w_f << indents << tokens[idx++].xml_line;   // ','
        w_f << indents << tokens[idx++].xml_line;   // varName
    }
    assert (tokens[idx].value == ";");
    w_f << indents << tokens[idx++].xml_line;   // write ';'
    return idx;
}

int Analizer::parseStatements(int idx, string indents) {
    while (true) {
        if (tokens[idx].value == "let") {
            w_f << indents << "<letStatement>\n";
            idx = parseLetStatement(idx, indents+"\t");
            w_f << indents << "</letStatement>\n";
        }
        else if (tokens[idx].value == "if") {
            w_f << indents << "<ifStatement>\n";
            idx = parseIfStatement(idx, indents+"\t");
            w_f << indents << "</ifStatement>\n";
        }
        else if (tokens[idx].value == "while") {
            w_f << indents << "<whileStatement>\n";
            idx = parseWhileStatement(idx, indents+"\t");
            w_f << indents << "</whileStatement>\n";
        }
        else if (tokens[idx].value == "do") {
            w_f << indents << "<doStatement>\n";
            idx = parseDoStatement(idx, indents+"\t");
            w_f << indents << "</doStatement>\n";
        }
        else if (tokens[idx].value == "return") {
            w_f << indents << "<returnStatement>\n";
            idx = parseReturnStatement(idx, indents+"\t");
            w_f << indents << "</returnStatement>\n";
        }
        else
            break;
    }
    return idx;
}

int Analizer::parseLetStatement(int idx, string indents) {
    w_f << indents << tokens[idx++].xml_line;   // write "let"

    // write varName
    assert (tokens[idx].type == "identifier");
    w_f << indents << tokens[idx++].xml_line;

    if (tokens[idx].value == "[") {
        w_f << indents << tokens[idx++].xml_line;   // '['
        w_f << indents << "<expression>\n";
        idx = parseExpression(idx, indents+"\t");
        w_f << indents << "</expression>\n";
        assert (tokens[idx].value == "]");          // ']'
        w_f << indents << tokens[idx++].xml_line;
    }

    // '='
    assert (tokens[idx].value == "=");
    w_f << indents << tokens[idx++].xml_line;

    // expression
    w_f << indents << "<expression>\n";
    idx = parseExpression(idx, indents+"\t");
    w_f << indents << "</expression>\n";

    // ';'
    assert (tokens[idx].value == ";");
    w_f << indents << tokens[idx++].xml_line;
    return idx;
}

int Analizer::parseIfStatement(int idx, string indents) {
    w_f << indents << tokens[idx++].xml_line;   // write "if"

    // '('
    assert (tokens[idx].value == "(");
    w_f << indents << tokens[idx++].xml_line;

    // expression
    w_f << indents << "<expression>\n";
    idx = parseExpression(idx, indents+"\t");
    w_f << indents << "</expression>\n";

    // ')'
    assert (tokens[idx].value == ")");
    w_f << indents << tokens[idx++].xml_line;

    // '{'
    assert (tokens[idx].value == "{");
    w_f << indents << tokens[idx++].xml_line;

    // statements
    w_f << indents << "<statements>\n";
    if (tokens[idx].value != "}")
        idx = parseStatements(idx, indents+"\t");
    w_f << indents << "</statements>\n";

    // '}'
    assert (tokens[idx].value == "}");
    w_f << indents << tokens[idx++].xml_line;

    if (tokens[idx].value == "else") {
        w_f << indents << tokens[idx++].xml_line;   // write "else"
        // '{'
        assert (tokens[idx].value == "{");
        w_f << indents << tokens[idx++].xml_line;

        // statements
        w_f << indents << "<statements>\n";
        if (tokens[idx].value != "}")
            idx = parseStatements(idx, indents+"\t");
        w_f << indents << "</statements>\n";

        // '}'
        assert (tokens[idx].value == "}");
        w_f << indents << tokens[idx++].xml_line;
    }

    return idx;
}

int Analizer::parseWhileStatement(int idx, string indents) {
    w_f << indents << tokens[idx++].xml_line;   // write "while"

    // '('
    assert (tokens[idx].value == "(");
    w_f << indents << tokens[idx++].xml_line;

    // expression
    w_f << indents << "<expression>\n";
    idx = parseExpression(idx, indents+"\t");
    w_f << indents << "</expression>\n";

    // ')'
    assert (tokens[idx].value == ")");
    w_f << indents << tokens[idx++].xml_line;

    // '{'
    assert (tokens[idx].value == "{");
    w_f << indents << tokens[idx++].xml_line;

    // statements
    w_f << indents << "<statements>\n";
    if (tokens[idx].value != "}")
        idx = parseStatements(idx, indents+"\t");
    w_f << indents << "</statements>\n";

    // '}'
    assert (tokens[idx].value == "}");
    w_f << indents << tokens[idx++].xml_line;
    return idx;
}

int Analizer::parseDoStatement(int idx, string indents) {
    w_f << indents << tokens[idx++].xml_line;   // write "do"

    // subroutine call
    idx = parseSubroutineCall(idx, indents);

    // ';'
    assert (tokens[idx].value == ";");
    w_f << indents << tokens[idx++].xml_line;
    return idx;
}

int Analizer::parseReturnStatement(int idx, string indents) {
    w_f << indents << tokens[idx++].xml_line;   // write "return"

    if (tokens[idx].value != ";") {
        // expression
        w_f << indents << "<expression>\n";
        idx = parseExpression(idx, indents+"\t");
        w_f << indents << "</expression>\n";
    }

    // ';'
    assert (tokens[idx].value == ";");
    w_f << indents << tokens[idx++].xml_line;
    return idx;
}

int Analizer::parseSubroutineCall(int idx, string indents) {
    assert (tokens[idx].type == "identifier");   // subroutineName or className or varName
    w_f << indents << tokens[idx++].xml_line;
    if (tokens[idx].value == ".") {
        w_f << indents << tokens[idx++].xml_line;   // '.'
        w_f << indents << tokens[idx++].xml_line;   // subroutineName
    }
    assert (tokens[idx].value == "(");           // '('
    w_f << indents << tokens[idx++].xml_line;
    // expressionList
    w_f << indents << "<expressionList>\n";
    if (tokens[idx].value != ")")
        idx = parseExpressionList(idx, indents+"\t");
    w_f << indents << "</expressionList>\n";
    assert (tokens[idx].value == ")");           // ')'
    w_f << indents << tokens[idx++].xml_line;
    return idx;
}

int Analizer::parseExpression(int idx, string indents) {
    // term
    w_f << indents << "<term>\n";
    idx = parseTerm(idx, indents+"\t");
    w_f << indents << "</term>\n";

    while (operands.count(tokens[idx].value)) {
        w_f << indents << tokens[idx++].xml_line;   // operand
        w_f << indents << "<term>\n";
        idx = parseTerm(idx, indents+"\t");
        w_f << indents << "</term>\n";
    }

    return idx;
}

int Analizer::parseTerm(int idx, string indents) {
    if (tokens[idx].value == "-" || tokens[idx].value == "~") {
        w_f << indents << tokens[idx++].xml_line;   // unary operand
        w_f << indents << "<term>\n";
        idx = parseTerm(idx, indents+"\t");
        w_f << indents << "</term>\n";
    }
    else if (tokens[idx].value == "(") {
        w_f << indents << tokens[idx++].xml_line;   // '('
        w_f << indents << "<expression>\n";
        idx = parseExpression(idx, indents+"\t");
        w_f << indents << "</expression>\n";
        assert (tokens[idx].value == ")");
        w_f << indents << tokens[idx++].xml_line;
    }
    else if (tokens[idx+1].value == "[") {
        w_f << indents << tokens[idx++].xml_line;   // varName
        w_f << indents << tokens[idx++].xml_line;   // '['
        w_f << indents << "<expression>\n";
        idx = parseExpression(idx, indents+"\t");
        w_f << indents << "</expression>\n";
        assert (tokens[idx].value == "]");
        w_f << indents << tokens[idx++].xml_line;   // ']'
    }
    else if (tokens[idx+1].value == "(" || tokens[idx+1].value == ".") {
        idx = parseSubroutineCall(idx, indents);
    }
    else
        w_f << indents << tokens[idx++].xml_line;
    return idx;
}

int Analizer::parseExpressionList(int idx, string indents) {
    // expression
    w_f << indents << "<expression>\n";
    idx = parseExpression(idx, indents+"\t");
    w_f << indents << "</expression>\n";
    while (tokens[idx].value == ",") {
        w_f << indents << tokens[idx++].xml_line;   // ','
        w_f << indents << "<expression>\n";
        idx = parseExpression(idx, indents+"\t");
        w_f << indents << "</expression>\n";
    }
    return idx;
}

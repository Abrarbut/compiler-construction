#include <iostream>
#include <fstream>
#include <cctype>
using namespace std;

string keywords[] = {
    "break","case","char","const","continue","default",
    "double","else","enum","extern","float","for",
    "goto","if","int","long","return",
    "short","static","struct","switch","void","while"
};

bool isKeyword(string word) {
    for (int i = 0; i < 23; i++) {
        if (keywords[i] == word)
            return true;
    }
    return false;
}

bool isIdentifier(string word) {
    if (!(isalpha(word[0]) || word[0] == '_'))
        return false;

    for (int i = 1; i < word.length(); i++) {
        if (!(isalnum(word[i]) || word[i] == '_'))
            return false;
    }
    return true;
}

bool isNumber(string word) {
    bool hasDot = false;

    for (int i = 0; i < word.length(); i++) {
        if (word[i] == '.') {
            if (hasDot)
                return false;
            hasDot = true;
        }
        else if (!isdigit(word[i]))
            return false;
    }
    return true;
}

bool isOperator(string s) {
    string ops[] = {"+","-","*","/","%","++","--","==","!=",
                    ">","<",">=","<="};
    for (int i = 0; i < 13; i++) {
        if (ops[i] == s)
            return true;
    }
    return false;
}

bool isPunctuator(char c) {
    char punctuators[] = {'{','}','(',')','[',']','=',';',',','.',':'};
    for (int i = 0; i < 11; i++) {
        if (punctuators[i] == c)
            return true;
    }
    return false;
}

string removeComments(string code) {
    string result = "";
    for (int i = 0; i < code.length(); i++) {

        if (code[i] == '/' && code[i+1] == '/') {
            while (i < code.length() && code[i] != '\n')
                i++;
        }

        else if (code[i] == '/' && code[i+1] == '*') {
            i += 2;
            while (i < code.length() &&
                   !(code[i] == '*' && code[i+1] == '/'))
                i++;
            i++;
        }

        else {
            result += code[i];
        }
    }
    return result;
}

int main() {

    ifstream file("input_scanner.h");
    if (!file) {
        cout << "File not found\n";
        return 0;
    }

    string code = "", line;

    while (getline(file, line))
        code += line + "\n";

    file.close();

    code = removeComments(code);

    cout << "TOKENS:\n\n";

    for (int i = 0; i < code.length(); i++) {

        if (isspace(code[i]))
            continue;

        // IDENTIFIER or KEYWORD
        if (isalpha(code[i]) || code[i] == '_') {
            string word = "";
            while (isalnum(code[i]) || code[i] == '_') {
                word += code[i];
                i++;
            }
            i--;

            if (isKeyword(word))
                cout << "Keyword : " << word << endl;
            else
                cout << "Identifier : " << word << endl;
        }

        // NUMBER
        else if (isdigit(code[i])) {
            string num = "";
            while (isdigit(code[i]) || code[i] == '.') {
                num += code[i];
                i++;
            }
            i--;

            cout << "Number : " << num << endl;
        }

        // OPERATOR (check 2-character first)
        else if (i+1 < code.length()) {
            string twoChar = "";
            twoChar += code[i];
            twoChar += code[i+1];

            if (isOperator(twoChar)) {
                cout << "Operator : " << twoChar << endl;
                i++;
            }
            else if (isOperator(string(1, code[i]))) {
                cout << "Operator : " << code[i] << endl;
            }
            else if (isPunctuator(code[i])) {
                cout << "Punctuator : " << code[i] << endl;
            }
        }
    }

    return 0;
}

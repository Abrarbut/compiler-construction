#include <iostream>
#include <fstream>
using namespace std;

string keywords[] = {
    "break","case","char","const","continue","default",
    "double","else","enum","extern","float","for",
    "goto","if","int","long","return",
    "short","static","struct","switch","void","while"
};

bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isDigit(char c) {
    return (c >= '0' && c <= '9');
}

bool isWhitespace(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}

bool isKeyword(string word) {
    for (int i = 0; i < 23; i++)
        if (keywords[i] == word)
            return true;
    return false;
}

bool isIdentifier(string word) {
    if (!(isLetter(word[0]) || word[0] == '_'))
        return false;

    for (int i = 1; i < word.length(); i++) {
        if (!(isLetter(word[i]) || isDigit(word[i]) || word[i] == '_'))
            return false;
    }
    return true;
}

bool isNumber(string word) {
    int dotCount = 0;

    for (int i = 0; i < word.length(); i++) {
        if (word[i] == '.') {
            dotCount++;
            if (dotCount > 1)
                return false;
        }
        else if (!isDigit(word[i]))
            return false;
    }
    return true;
}

bool isOperator(string s) {
    string ops[] = {"+","-","*","/","%","++","--",
                    "==","!=","<",">","<=",">="};

    for (int i = 0; i < 13; i++)
        if (ops[i] == s)
            return true;
    return false;
}

bool isPunctuator(char c) {
    char p[] = {'{','}','(',')','[',']','=',';',',','.',':'};

    for (int i = 0; i < 11; i++)
        if (p[i] == c)
            return true;
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

        if (isWhitespace(code[i]))
            continue;

        if (isLetter(code[i]) || code[i] == '_') {
            string word = "";

            while (isLetter(code[i]) || isDigit(code[i]) || code[i] == '_') {
                word += code[i];
                i++;
            }
            i--;

            if (isKeyword(word))
                cout << "Keyword : " << word << endl;
            else
                cout << "Identifier : " << word << endl;
        }

        else if (isDigit(code[i])) {
            string num = "";

            while (isDigit(code[i]) || code[i] == '.') {
                num += code[i];
                i++;
            }
            i--;

            cout << "Number : " << num << endl;
        }

        else {
            if (i+1 < code.length()) {
                string two = "";
                two += code[i];
                two += code[i+1];

                if (isOperator(two)) {
                    cout << "Operator : " << two << endl;
                    i++;
                    continue;
                }
            }

            string one = "";
            one += code[i];

            if (isOperator(one))
                cout << "Operator : " << one << endl;

            else if (isPunctuator(code[i]))
                cout << "Punctuator : " << code[i] << endl;
        }
    }

    return 0;
}

#include <iostream>
#include <fstream>
using namespace std;

struct Symbol {
    string tokenName;
    string tokenValue;
    int hashValue;
};

Symbol table[1000];
int tableIndex = 0;

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

int generateHash(string word) {
    int hash = 0;
    for (int i = 0; i < word.length(); i++)
        hash = hash + word[i];
    return hash;
}

void insert(string name, string value) {
    table[tableIndex].tokenName = name;
    table[tableIndex].tokenValue = value;
    table[tableIndex].hashValue = generateHash(value);
    tableIndex++;
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

    ifstream file("leapyear.c");
    if (!file) {
        cout << "File not found\n";
        return 0;
    }

    string code = "", line;

    while (getline(file, line))
        code += line + "\n";

    file.close();

    code = removeComments(code);

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
                insert("Keyword", word);
            else
                insert("Identifier", word);
        }

        else if (isDigit(code[i])) {
            string num = "";

            while (isDigit(code[i]) || code[i] == '.') {
                num += code[i];
                i++;
            }
            i--;

            insert("Number", num);
        }

        else {
            string sym = "";
            sym += code[i];
            insert("Symbol", sym);
        }
    }

    cout << "Token Name\tToken Value\tHash Value\n";
    cout << "-------------------------------------------\n";

    for (int i = 0; i < tableIndex; i++) {
        cout << table[i].tokenName << "\t\t"
             << table[i].tokenValue << "\t\t"
             << table[i].hashValue << endl;
    }

    return 0;
}

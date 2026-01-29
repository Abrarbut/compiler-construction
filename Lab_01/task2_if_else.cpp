#include <iostream>
#include <string>
using namespace std;

bool validateAlphabet(const string &s) {
    for(char ch : s) {
        if(ch != 'a' && ch != 'b') return false;
    }
    return true;
}

bool DFA_if_else(const string& s) {
    int state = 0;  // q0

    for(char ch : s) {
        if(state == 0) {
            if(ch == 'a') state = 1;
            else if(ch == 'b') state = 2;
        }
        else if(state == 1) {
            if(ch == 'a') state = 0;
            else if(ch == 'b') state = 3;
        }
        else if(state == 2) {
            if(ch == 'a') state = 3;
            else if(ch == 'b') state = 0;
        }
        else if(state == 3) {
            if(ch == 'a') state = 2;
            else if(ch == 'b') state = 1;
        }
    }
    return (state == 0);
}


int main() {
    string inputs[] = {"aa", "abba", "abab", "aabbaabb", ""};
    
    for(string s : inputs) {
        cout << s << " -> ";
        if(!validateAlphabet(s)) {
            cout << "Invalid alphabet\n";
        } else if(DFA_if_else(s)) {
            cout << "Accepted\n";
        } else {
            cout << "Rejected\n";
        }
    }
    return 0;
}

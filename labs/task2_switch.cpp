#include <iostream>
#include <string>
using namespace std;
bool validateAlphabet(const string &s) {
    for(char ch : s) {
        if(ch != 'a' && ch != 'b') return false;
    }
    return true;
}
bool DFA_switch(const string& s) {
    int state = 0; // q0

    for(char ch : s) {
        switch(state) {
            case 0:
                if(ch == 'a') state = 1;
                else if(ch == 'b') state = 2;
                break;

            case 1:
                if(ch == 'a') state = 0;
                else if(ch == 'b') state = 3;
                break;

            case 2:
                if(ch == 'a') state = 3;
                else if(ch == 'b') state = 0;
                break;

            case 3:
                if(ch == 'a') state = 2;
                else if(ch == 'b') state = 1;
                break;
        }
    }
    return (state == 0);
}
int main() {
    string tests[] = {"aa", "abba", "abab", "aabbaabb", ""};

    for(string s : tests) {
        cout << "Input: \"" << s << "\" -> ";

        if(!validateAlphabet(s)) {
            cout << "Invalid alphabet\n";
        }
        else if(DFA_switch(s)) {
            cout << "Accepted\n";
        }
        else {
            cout << "Rejected\n";
        }
    }
    return 0;
}

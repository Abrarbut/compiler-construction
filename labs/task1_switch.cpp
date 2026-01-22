#include <iostream>
#include <string>
using namespace std;

bool validateAlphabet(const string &s) {
    for(char ch : s) {
        if(ch != 'a' && ch != 'b' && ch != 'c') return false;
    }
    return true;
}

bool DFA_switch(const string &s) {
    int state = 0; 
    for(char ch : s) {
        switch(state) {
            case 0:
                if(ch == 'a') state = 1;
                else return false;
                break;
            case 1:
                if(ch == 'b') state = 2;
                else return false;
                break;
            case 2:
                if(ch == 'b') state = 3;
                else if(ch == 'c') state = 5;
                else return false;
                break;
            case 3:
                if(ch == 'b') state = 2;
                else if(ch == 'c') state = 5;
                else return false;
                break;
            case 5:
                return false;
        }
    }
    return (state == 5);
}

int main() {
    string inputs[] = {"abc", "abbc", "abcd", "abbbc", "abbbbc"};
    
    for(string s : inputs) {
        cout << s << " -> ";
        if(!validateAlphabet(s)) {
            cout << "Invalid alphabet\n";
        } else if(DFA_switch(s)) {
            cout << "Accepted\n";
        } else {
            cout << "Rejected\n";
        }
    }
    return 0;
}

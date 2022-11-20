#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <limits>
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char *argv[]) {
    if(argc < 4) {
        perror("Not enough arguments");
        exit(-1);
    }

    string inFile = argv[3];

    stringstream ss;
    ss << argv[1];
    int M = stoi(ss.str());

    stringstream ss1;
    ss1 << argv[2];
    int R = stoi(ss1.str());

    // inFile = "\"" + inFile + "\"";
    ifstream file(inFile);

    if(file.is_open()){
        int x;
        file >> x;
        cout << M << " " << R << " " << x << endl;
    } else {
        cout << "file not opend";
    }

    file.close();
    return 0;
}
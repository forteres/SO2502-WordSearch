#include <iostream>
#include <thread>
#include <fstream> 
#include <string> 
#include <vector>
#include <sstream>

using namespace std; 

struct output {
    string word;
    pair<int,int> initialPos;
    pair<string, string> direction;   
};
vector<vector<char>> matriz;
vector<string> words;

int main () {
    
    string filePath = "cacapalavras.txt";

    ifstream file (filePath);

    if (!file.is_open()) {
        cout << "Erro." << endl;
    }

    string readLine;
    int matrizLine, matrizColumn;

    if(getline(file,readLine)){
        istringstream iss(readLine);
        iss >> matrizLine >> matrizColumn;
    }
        
    matriz.resize(matrizLine, vector<char>(matrizColumn));

    for (int i = 0; i < matrizLine; i++) {
        
        getline(file, readLine);

        for (int j = 0; j < matrizColumn; j++) {
            matriz[i][j] = readLine[j];
        }
    }

    while(getline(file, readLine)){
        if(!readLine.empty()){
            words.push_back(readLine);
        }
    }

    file.close();

// relava, teste ok
    cout << "Primeira linha (20 primeiros elementos): ";
    for (int j = 0; j < min(20, matrizColumn); j++) {
        cout << matriz[0][j] << " ";
    }
    cout << endl;

    return 0;
}
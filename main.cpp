#include <iostream>
#include <thread>
#include <fstream> 
#include <string> 
#include <vector>

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
        cout << "Erro ao abrir o arquivo." << endl;
        return 1;
    }

    string readLine;
    int matrizLine, matrizColumn;

    if(getline(file,readLine)){
        istringstream iss(readLine);
        iss >> matrizLine >> matrizColumn;
    }
    else(
        cout << "Arquivo inválido" << endl;
        return 1;
    )
        
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

// releva, teste ok
    cout << "Lendo arquivo): \n";
    for (long long unsigned int i = 0; i < words.size(); i++) {
        cout << words[i] << endl;
    }
    cout << endl;

    return 0;
}
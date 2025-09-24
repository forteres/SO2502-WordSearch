#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>

using namespace std;

struct output {
    string word;
    pair<int,int> initialPos;  // linha/coluna
    string direction;
    vector<pair<int,int>> positions;
};

vector<vector<char>> matriz;
vector<string> words;

// Declaracao da ThreadPool para paralelismo
class ThreadPool {
public:
    void Start();
    void QueueJob(const function<void()>& job);
    void Stop();
private:
    void ThreadLoop();
    bool should_terminate = false;
    mutex queue_mutex;
    condition_variable mutex_condition;
    vector<thread> threads;
    queue<function<void()>> jobs;
};

void ThreadPool::Start() {
    const uint32_t num_threads = thread::hardware_concurrency();
    for(uint32_t i=0; i<num_threads; ++i)
        threads.emplace_back(&ThreadPool::ThreadLoop, this);
}

void ThreadPool::ThreadLoop() {
    while(true) {
        function<void()> job;
        {
            unique_lock<mutex> lock(queue_mutex);
            mutex_condition.wait(lock, [this]{ return !jobs.empty() || should_terminate; });
            if(should_terminate && jobs.empty()) return;
            job = jobs.front();
            jobs.pop();
        }
        job();
    }
}

void ThreadPool::QueueJob(const function<void()>& job) {
    {
        unique_lock<mutex> lock(queue_mutex);
        jobs.push(job);
    }
    mutex_condition.notify_one();
}

void ThreadPool::Stop() {
    {
        unique_lock<mutex> lock(queue_mutex);
        should_terminate = true;
    }
    mutex_condition.notify_all();
    for(thread &t : threads) t.join();
    threads.clear();
}

//Declaracao do Trie para pesquisa otimizada na matriz
struct TrieNode {
    bool isWord = false;
    unordered_map<char, TrieNode*> children;
};

class Trie {
public:
    Trie() { root = new TrieNode(); }
    void insert(const string& word) {
        TrieNode* node = root;
        for(char c : word) {
            c = tolower(c);
            if(node->children.find(c) == node->children.end())
                node->children[c] = new TrieNode();
            node = node->children[c];
        }
        node->isWord = true;
    }
    TrieNode* getRoot() { return root; }
private:
    TrieNode* root;
};

string getDirectionName(int dx, int dy) {
    if(dx==1 && dy==0) return "direita";
    if(dx==-1 && dy==0) return "esquerda";
    if(dx==0 && dy==1) return "baixo";
    if(dx==0 && dy==-1) return "cima";
    if(dx==1 && dy==1) return "direita/baixo";
    if(dx==1 && dy==-1) return "direita/cima";
    if(dx==-1 && dy==1) return "esquerda/baixo";
    if(dx==-1 && dy==-1) return "esquerda/cima";
    return "desconhecida";
}

void searchDirection(Trie& trie, int dx, int dy, vector<output>& results, mutex& results_mutex) {
    int rows = (int)matriz.size();
    int cols = (int)matriz[0].size();

    for(int y=0; y<rows; ++y) {
        for(int x=0; x<cols; ++x) {
            TrieNode* node = trie.getRoot();
            int nx = x, ny = y;
            string currentWord = "";
            vector<pair<int,int>> positions;

            while(nx >=0 && nx < cols && ny >=0 && ny < rows) {
                char c = tolower(matriz[ny][nx]);
                if(node->children.find(c) == node->children.end()) break;

                currentWord += c;
                node = node->children[c];
                positions.push_back({ny, nx});  // linha/coluna

                if(node->isWord) {
                    lock_guard<mutex> lock(results_mutex);
                    results.push_back(output{currentWord, {y, x}, getDirectionName(dx,dy), positions});
                }

                nx += dx;
                ny += dy;
            }
        }
    }
}

int main() {
    // Acesso e leitura do arquivo
    string filePath = "../cacapalavras.txt";
    ifstream file(filePath);
    if(!file.is_open()) { cout << "Erro ao abrir o arquivo." << endl; return 1; }

    int matrizLine, matrizColumn;
    string readLine;
    if(getline(file, readLine)) {
        istringstream iss(readLine);
        iss >> matrizLine >> matrizColumn;
    } else { cout << "Arquivo inválido" << endl; return 1; }

    matriz.resize(matrizLine, vector<char>(matrizColumn));
    for(int i=0;i<matrizLine;++i) {
        getline(file, readLine);
        for(int j=0;j<matrizColumn;++j)
            matriz[i][j] = readLine[j];
    }

    while(getline(file, readLine)) if(!readLine.empty()) words.push_back(readLine);
    file.close();

    // Processamento da Trie invocando o processamento paralelo em todas as direcoes uma vez que a primeira letra eh valida
    Trie trie;
    for(auto &w : words) trie.insert(w);

    vector<output> foundWords;
    mutex results_mutex;

    ThreadPool thread_pool;
    thread_pool.Start();

    vector<pair<int,int>> directions = {
        {1,0},{-1,0},{0,1},{0,-1},
        {1,1},{1,-1},{-1,1},{-1,-1}
    };

    for(auto [dx,dy] : directions)
        thread_pool.QueueJob([&trie,dx,dy,&foundWords,&results_mutex]{
            searchDirection(trie, dx, dy, foundWords, results_mutex);
        });

    thread_pool.Stop();

    // Atualiza matriz output com letras maiusculas
    for(auto &o : foundWords)
        for(auto [row,col] : o.positions)
            matriz[row][col] = toupper(matriz[row][col]);

    ofstream outFile("../saida.txt");
    if(!outFile.is_open()) { cout << "Erro ao criar saída" << endl; return 1; }

    for(auto &line : matriz) {
        for(char c : line) outFile << c;
        outFile << "\n";
    }
    outFile << "\n";

    // Cria a relacao de quais palavras foram encontradas e suas respectivas posicoes
    for(auto &w : words) {
        vector<output> occurrences;
        for(auto &o : foundWords)
            if(o.word.size() == w.size() &&
               equal(w.begin(), w.end(), o.word.begin(),
                     [](char a,char b){ return tolower(a)==tolower(b); }))
                occurrences.push_back(o);

        if(occurrences.empty())
            outFile << w << ": não encontrada\n";
        else
            for(auto &o : occurrences)
                outFile << o.word
                        << " (" << o.initialPos.first + 1 << "," << o.initialPos.second + 1 << "):"
                        << o.direction << "\n";
    }
    outFile.close();
    return 0;
}

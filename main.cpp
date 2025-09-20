#include <iostream>
#include <thread>
#include <fstream> 
#include <string> 
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

using namespace std; 

struct output {
    string word;
    pair<int,int> initialPos;
    pair<string, string> direction;   
};

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

    void ThreadPool::Start() {
        const uint32_t num_threads = thread::hardware_concurrency();
        for (uint32_t ii = 0; ii < num_threads; ++ii) {
            threads.emplace_back(thread(&ThreadPool::ThreadLoop,this));
        }
    }

    void ThreadPool::ThreadLoop() {
        while (true) {
            function<void()> job;
            {
                unique_lock<mutex> lock(queue_mutex);
                mutex_condition.wait(lock, [this] {
                    return !jobs.empty() || should_terminate;
                });
                if (should_terminate) {
                    return;
                }
                job = jobs.front();
                jobs.pop();
            }
            job();
        }
    }

    void ThreadPool::QueueJob(const function<void()>& job) { //thread_pool->QueueJob([] { /* ... */ });
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
        for (thread& active_thread : threads) {
            active_thread.join();
        }
        threads.clear();
    }
};

struct TrieNode {
    bool isWord = false;  // marca se o caminho forma uma palavra completa
    unordered_map<char, TrieNode*> children;
};

class Trie {
public:
    Trie() { root = new TrieNode(); }
    
    void insert(const string& word) {
        TrieNode* node = root;
        for(char c : word) {
            if(node->children.find(c) == node->children.end()) {
                node->children[c] = new TrieNode();
            }
            node = node->children[c];
        }
        node->isWord = true;
    }

    bool search(const string& word) {
        TrieNode* node = root;
        for(char c : word) {
            if(node->children.find(c) == node->children.end())
                return false;
            node = node->children[c];
        }
        return node->isWord;
    }

    TrieNode* getRoot() { return root; }

private:
    TrieNode* root;
};

void readWord(int8_t right,int8_t up, int posX, int posY){
    string result;
    int x = posX;
    int y = posY;
    
    while(x >= 0 && x < matriz[0].size() && y >= 0 && y < matriz.size()) {
        result += matriz[y][x];
        x += right;
        y -= up; // y diminui para cima, aumenta para baixo
    }

    output resultNode;
    resultNode.word = result;
    resultNode.initialPos.first = posX;
    resultNode.initialPos.second = posY;
    resultNode.direction.first = "random";
    resultNode.direction.second = "random2";

    results.push_back(resultNode);
}

void searchDirection(Trie& trie, int dx, int dy, vector<output>& results, mutex& results_mutex) {
    for(int y = 0; y < matriz.size(); y++) {
        for(int x = 0; x < matriz[0].size(); x++) {
            char firstChar = matriz[y][x];
            if(!trie.hasStartingChar(firstChar))
                continue;  // ignora se não existe palavra começando com essa letra

            string word = "";
            int nx = x, ny = y;
            TrieNode* node = trie.getRoot();
            while(nx >= 0 && nx < matriz[0].size() && ny >= 0 && ny < matriz.size()) {
                char c = matriz[ny][nx];
                if(node->children.find(c) == node->children.end())
                    break;
                word += c;
                node = node->children[c];
                if(node->isWord) {
                    lock_guard<mutex> lock(results_mutex);
                    results.push_back({word, {x,y}, {"dx="+to_string(dx),"dy="+to_string(dy)}});
                }
                nx += dx;
                ny += dy;
            }
        }
    }
}

    vector<output> results;
    vector<vector<char>> matriz;
    vector<string> words;

int main () {

//#READ FILE
    
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

//#START PROGRAM

    Trie trie;
    for(const auto& w : words)
        trie.insert(w);

    vector<output> foundWords;
    mutex results_mutex;

    ThreadPool thread_pool;
    thread_pool.Start();

    // 8 direções
    vector<pair<int,int>> directions = {
        {1,0}, {-1,0}, {0,1}, {0,-1},    // direita, esquerda, baixo, cima
        {1,1}, {1,-1}, {-1,1}, {-1,-1}   // diagonais
    };

    for(auto [dx, dy] : directions) {
        // captura referências corretamente
        thread_pool.QueueJob([&trie, dx, dy, &foundWords, &results_mutex]() {
            searchDirection(trie, dx, dy, foundWords, results_mutex);
        });
    }

    thread_pool.Stop();  // espera todas as threads terminarem

    // Imprime resultados
    for(auto& o : foundWords) {
        cout << "Palavra: " << o.word 
             << " Inicio: (" << o.initialPos.first << "," << o.initialPos.second << ")"
             << " Direcao: (" << o.direction[0] << "," << o.direction[1] << ")\n";
    }


    
    return 0;
}
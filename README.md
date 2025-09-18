# SO 2025/2 - Word Search

## Enunciado: 
"Você deve procurar uma lista de palavras em um
diagrama de letras e indicar se cada palavra foi encontrada ou não. Se a
palavra foi encontrada, você deve indicar a posição inicial e o sentido
(todos os sentidos são válidos). Observe o diagrama abaixo e procure as
palavras threads, arquivos, sinais, pipe, processos e mutex.
Lembre-se de procurar as palavras em todas as direções
possíveis.
O arquivo de entrada deverá conter o tamanho do diagrama, o diagrama
e uma lista de palavras. O arquivo de saída deverá conter o diagrama
com as palavras encontradas em letras maiúsculas e a lista das palavras
encontradas com a posição e direção indicadas."

## Ideia:
* Abrir um arquivo;
* Ler o arquivo particionando em três partes:
    * Primeira parte define tamanho da matriz (não precisa manter guardado após definir a matriz?);
    * Segunda parte é a matriz que será lida conforme o tamanho definido na primeira parte (guarda como matriz [][]);
    * Terceira parte é o restante  que armazenará as palavras a serem procuradas (guarda como lista, vector ou trie se der tempo);
        * Trie parece ser dificil, porém é *aparentemente* super eficiente e evita alguns problemas na busca quando há palavras em comum;
* Tratativa de erro básica na leitura do arquivo (deve estar conforme o padrão);
* Criar "pool" de threads conforme máximo de threads disponíveis (ou inferior);
* Criar "fila" de tarefas que as threads consumirão;
* Criar "index" com a primeira letra de cada palavra;
    * Desnecessário caso utilizado a abordagem com trie;
* Criar struct que irá armazenar a saida com: (palavra, pos inicial e direção);
* Ler matriz de forma sequencial comparando if(letra atual is in index);
* Quando if retornar true criar uma tarefa para cada direção e adicionar a tarefa a fila, armazenando as informações relevantes para struct;
    * A tarefa irá comparar as demais letras de cada direção uma a uma e:
        * Em caso de sucesso da tarefa, preencher a struct e adicionar a struct a uma lista de output;
        * Em caso de fracasso, reciclar a memória consumida e abortar;
* Demais melhorias simples de usabilidade e robustez;
* "Código bem comentado e explicativo";
* "Estrutura modular e organizada (funções/procedimentos)";

* Opcional:
    * Interface simples;
    * Relatório curto explicando as decisões;
    * "Otimizações na busca (algoritmos eficientes)" - Trie?
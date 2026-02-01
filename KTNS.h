#ifndef KTNS_H
#define KTNS_H

#include <iostream>
#include <vector>
#include <numeric>
#include <limits>

using namespace std;

long KTNS(const vector<int> processos, bool debug = false) {

	extern vector<vector<int>> matrixFerramentas;
    extern vector<unsigned> executionTime;
    extern vector<unsigned> toolLife;
	extern unsigned t;
	extern unsigned c;

    vector<int> carregadas(t, 0);
    vector<vector<int>> prioridades(t, vector<int>(processos.size()));
    vector<vector<int>> magazine(t, vector<int>(processos.size()));
    vector<unsigned> remainingLife = toolLife;

    int u = 0; // ferramentas no magazine
    long trocas = 0;

    if (processos.size() == 0) {
        cout << "Vetor processos vazio." << endl;
        return 0;
    }

    if (debug) cout << "----------- Iniciando a função KTNS -------------" << endl;
    if (debug) cout << "Iniciando processos[0]" << endl;


    for (unsigned int j = 0; j < t; j++) {
        // preenche o magazine com as ferramentas para a primeira tarefa
        carregadas[j] = matrixFerramentas[j][processos[0]];
        if (matrixFerramentas[j][processos[0]] == 1) {
            ++u;
            // já decrementa o tempo de execução da tarefa 0 das ferramentas carregadas
            remainingLife[j] -= executionTime[processos[0]];
        }
        // preenche o vetor magazine
        for (unsigned i = 0; i < processos.size(); i++) {
            magazine[j][i] = matrixFerramentas[j][processos[i]];
        }
    }
    // Preenche a matriz de prioridades
    for (unsigned i = 0; i < t; ++i) {
        for (unsigned j = 0; j < processos.size(); ++j) {
            if (magazine[i][j] == 1)
                prioridades[i][j] = 0;
            else {
                int proxima = 0;
                bool usa = false;
                for (unsigned k = j + 1; k < processos.size(); ++k) {
                    ++proxima;
                    if (magazine[i][k] == 1) {
                        usa = true;
                        break;
                    }
                }
                if (usa)
                    prioridades[i][j] = proxima;
                else
                    prioridades[i][j] = -1;
            }
        }
    }

    for (unsigned i = 1; i < processos.size(); ++i) { // i = tarefa
        for (unsigned j = 0; j < t; ++j) { // j = ferramenta
            // troca preditiva por desgaste
            if (magazine[j][i] == 1 && carregadas[j] == 1) {
                if (remainingLife[j] < executionTime[processos[i]]) {
                    trocas++;
                    remainingLife[j] = toolLife[j];
                }
            }
        }
        for (int j = 0; j < t; ++j) {
            if ((magazine[j][i] == 1) && (carregadas[j] == 0)) {
                carregadas[j] = 1;
                ++u;
                if (remainingLife[j] < executionTime[processos[i]])
                    remainingLife[j] = toolLife[j];
            }

        }

        while (u > c) {
            int remove = -1;
            bool removed = false;

            for (unsigned j = 0; j < t; ++j) {
                if (carregadas[j] == 1 && magazine[j][i] != 1 && prioridades[j][i] == -1) {
                    remove = j;
                    removed = true;
                    break;
                }
            }

            if (!removed) {
                int min = numeric_limits<int>::max();
                int index;

                for (unsigned j = 0; j < t; ++j) {
                    if (carregadas[j] == 1 && magazine[j][i] != 1 && prioridades[j][i] > 0) {
                        // próxima vez que a ferramenta j será usada
                        int nextIndex = i + prioridades[j][i];

                        int current = remainingLife[j] - executionTime[processos[nextIndex]];

                        if (current < min) {
                            min = current;
                            index = j;
                        }
                    }
                }

                // se a ferramenta index não tiver tempo suficiente para realizar a sua próxima tarefa, remove ela de uma vez
                if (min < 0) {
                    remove = index;
                }
                else {
                    min = 0;
                    for (unsigned j = 0; j < t; ++j) {
                        if (magazine[j][i] != 1) {
                            if ((prioridades[j][i] > min) && carregadas[j] == 1) {
                                min = prioridades[j][i];
                                remove = j;
                            }
                        }
                    }
                }
            }


            carregadas[remove] = 0;
            --u;
            trocas++;
        }

        for (unsigned j = 0; j < t; ++j) {
            if (magazine[j][i] == 1 && carregadas[j] == 1) {
                remainingLife[j] -= executionTime[processos[i]];
            }
        }
    }
    return trocas + c;

}

#endif

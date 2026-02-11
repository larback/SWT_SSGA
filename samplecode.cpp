#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>

#include "SampleDecoder.h"
#include "SSGA.h"
#include "KTNS.h"

using namespace std;

std::vector<std::vector<int>> matrixFerramentas;
unsigned n = 0;
unsigned t = 0;
unsigned c;
vector<unsigned> executionTime;
vector<unsigned> toolLife;

// ---------------- Parâmetros do iRace ----------------
unsigned POP_SIZE = 15; // p
const unsigned MAX_VOID = 10000; // iterações sem melhoria
const unsigned CHILDREN_PER_ITER = 2; // filhos por iteração (2 filhos do mesmo crossover)
const unsigned TOURNAMENT_K = 2; // torneio
const double MUT_GENE_PROB = 0.03; // 0,1% por gene (swap)
const double INTENS_PROB = 0.30; // taxa de intensificação (busca local)
bool TWO_SWAP = true; // swap2
bool INSERTION = true; // insertion
bool TWO_OPT = true; // opt2

unsigned generation = 0;

unsigned nVoid = 0;
std::vector<int>solution;
std::ofstream fileSolution;

int main(int argc, char* argv[]) {

	using namespace std::chrono;
	matrixFerramentas.clear();
	cin >> n;
	cin >> t;
	cin >> c;

	auto populacao = n * POP_SIZE;
	unsigned chromosome_size = n;

	toolLife.resize(t);
	executionTime.resize(n);

	for (int i = 0; i < t; ++i)
        cin >> toolLife[i];

    for (int i = 0; i < n; ++i)
        cin >> executionTime[i];

	for (unsigned j = 0; j<t; j++){
		std::vector<int> tmpF;
		for (unsigned i = 0; i<n; ++i){
			int tooli = 0;
			cin >> tooli;
			tmpF.push_back(tooli);
		}
		matrixFerramentas.push_back(tmpF);
	}

	double best = 9e18;
	double initialBest = -1;
	// gero o arquivo de saída para garantir que o runner encontre em uma possível nova execução
	std::ofstream fileLock(argv[1]);
	if (!fileLock.is_open()){
		std::cerr << "Erro ao gerar o arquivo de saída previamente." << std::endl;
		return 1;
	}
	fileLock << "";
	fileLock.close();
	
	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	unsigned RNG_SEED = static_cast<unsigned>(
		std::chrono::high_resolution_clock::now().time_since_epoch().count()
	);
	
	SampleDecoder decoder;
	SSGA ssga(chromosome_size, populacao, TOURNAMENT_K, CHILDREN_PER_ITER, MUT_GENE_PROB, INTENS_PROB, TWO_SWAP, INSERTION, TWO_OPT, 7200.0, RNG_SEED, decoder);

	long teste_geracao = 0;
	high_resolution_clock::time_point t2;
	duration<double> time_span;
	cout << "Iniciando processo evolutivo do SSGA...\n";

	initialBest = ssga.bestFitness();
	best = initialBest;

	generation = 0;
	do {
		++generation;
		double before = ssga.bestFitness();
		ssga.iterateOnce();
		double after = ssga.bestFitness();

		if (after < best) {
			nVoid = 0;
			teste_geracao = generation;
			best = after;
		} else
			++nVoid;

		(void)before;

		t2 = high_resolution_clock::now();
		time_span = duration_cast<duration<double>>(t2 - t1);

		cout << "Iteração: " << generation << " | Melhor: " << best << " | nVoid: " << nVoid << endl;
	} while (nVoid < MAX_VOID && time_span.count() < 7200.0);

	const char* stopReason = "UNKNOWN";
	if (nVoid >= MAX_VOID) stopReason = "MAX_VOID";
	else if (time_span.count() >= 7200.0) stopReason = "TIME_LIMIT";

	t2 = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
 
	vector<double> ch = ssga.bestChromosome();

	std::vector < std::pair < double, unsigned > > ranking(ch.size());
	for(unsigned i = 0; i < ch.size(); ++i){
		ranking[i]=std::pair<double,unsigned>(ch[i],i);
	}
	
	std::sort(ranking.begin(), ranking.end());

	solution.clear();
	for(std::vector<std::pair<double, unsigned>>::const_iterator i = ranking.begin(); i!=ranking.end(); ++i){
		solution.push_back(i->second);
	}


	cout << "Makespan: " << best << endl;
	cout << "Solução:\n";
	for (int i=0;i<solution.size();++i){
		cout << solution[i] << " ";
	}
	cout << endl;

	double makespan = 00;

	fileSolution.open(argv[1]);

	fileSolution << initialBest << " " << best << " " << time_span.count() << " " << generation << std::endl;

	fileSolution << "\n==== RESULTADO DA EXECUÇÃO ====\n";
	fileSolution << "Makespan inicial: " << initialBest << "\n";
	fileSolution << "Melhor makespan: " << best << "\n";
	fileSolution << "Tempo de execução (s): " << time_span.count() << "\n";
	fileSolution << "Total de iterações: " << generation << "\n";
	fileSolution << "Razão de parada: " << stopReason << "\n";
	fileSolution << "Melhor iteração: " << teste_geracao << "\n";
	fileSolution << "Iterações sem melhoria: " << nVoid << "\n";
	fileSolution << "\n==== Estatísticas SSGA ====\n";
	fileSolution << "Filhos gerados: " << ssga.stats().childrenGenerated << "\n";
	fileSolution << "Rejeitados por pior absoluto (childFit > worst): " << ssga.stats().childrenRejectedByMean << "\n";
	fileSolution << "Sem candidatos piores para substituir (defensivo): " << ssga.stats().childrenNoWorseCandidates << "\n";
	fileSolution << "Inseridos na população: " << ssga.stats().childrenInserted << "\n";

	// Busca local
	fileSolution << "\n==== Busca Local (médias) ====\n";
	{
		auto s = ssga.stats();
		double s2_avg_us = (s.s2_n == 0) ? 0.0 : (static_cast<double>(s.s2_us) / static_cast<double>(s.s2_n));
		double in_avg_us = (s.in_n == 0) ? 0.0 : (static_cast<double>(s.in_us) / static_cast<double>(s.in_n));
		double o2_avg_us = (s.o2_n == 0) ? 0.0 : (static_cast<double>(s.o2_us) / static_cast<double>(s.o2_n));
		fileSolution << "swap2: n=" << s.s2_n << " avg_us=" << s2_avg_us << " melhoras=" << s.s2_b << " ganho_total=" << s.s2_g << "\n";
		fileSolution << "insertion: n=" << s.in_n << " avg_us=" << in_avg_us << " melhoras=" << s.in_b << " ganho_total=" << s.in_g << "\n";
		fileSolution << "opt2: n=" << s.o2_n << " avg_us=" << o2_avg_us << " melhoras=" << s.o2_b << " ganho_total=" << s.o2_g << "\n";
		long long best_g = std::max(std::max(s.s2_g, s.in_g), s.o2_g);
		int ties = 0;
		if (s.s2_g == best_g) ++ties;
		if (s.in_g == best_g) ++ties;
		if (s.o2_g == best_g) ++ties;
		const char* win = "EMPATE";
		if (ties == 1) {
			if (s.s2_g == best_g) win = "swap2";
			else if (s.in_g == best_g) win = "insertion";
			else win = "opt2";
		}
		fileSolution << "Melhor (por ganho_total): " << win << "\n";
	}

    fileSolution << "\n==== Parametros do Algoritmo ====\n";
	  fileSolution << "Algoritmo: SSGA (steady-state)\n";
	  fileSolution << "Tamanho da população: " << populacao << "\n";
	  fileSolution << "Filhos por iteração: " << CHILDREN_PER_ITER << "\n";
	  fileSolution << "Torneio (k): " << TOURNAMENT_K << "\n";
	  fileSolution << "Prob. mutação por posição (swap): " << MUT_GENE_PROB << "\n";
	  fileSolution << "Máximo sem melhoria (MAX_VOID): " << MAX_VOID << "\n";
	  fileSolution << "Razão de parada: " << stopReason << "\n";

  	fileSolution << "\n==== Parâmetros da instância ====\n";
  	fileSolution << "Tarefas: " << n << "\n";
  	fileSolution << "Ferramentas: " << t << "\n";
  	fileSolution << "Capacidade Magazine: " << c << "\n";

	fileSolution << "\n==== Solução ====\n";
    fileSolution << "Sequência de tarefas: ";
	for (int i=0;i<solution.size();++i)
		fileSolution << solution[i] << " ";
	fileSolution << "\n";

	fileSolution.close();
	return 0;
}

#include <dirent.h>
#include <cstdlib>
#include <string>
#include <fstream>

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

static bool get_num(const std::string& s, const std::string& key, double& out) {
	size_t p = s.find(key);
	if (p == std::string::npos) return false;
	p += key.size();
	size_t e = s.find_first_of(" \t\r\n", p);
	std::string v = s.substr(p, (e == std::string::npos) ? std::string::npos : (e - p));
	try {
		out = std::stod(v);
		return true;
	} catch (...) {
		return false;
	}
}

static bool get_tail(const std::string& s, const std::string& key, std::string& out) {
	size_t p = s.find(key);
	if (p == std::string::npos) return false;
	p += key.size();
	while (p < s.size() && (s[p] == ' ' || s[p] == '\t')) ++p;
	out = s.substr(p);
	while (!out.empty() && (out.back() == '\r' || out.back() == '\n' || out.back() == ' ' || out.back() == '\t')) out.pop_back();
	return true;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
        std::cerr << "Diretório com as instâncias não foi informado." << std::endl;
        exit (1);
    }
	std::string nomeDir = argv[1];
	int repeticoes = std::stoi(argv[2]);

	if (!nomeDir.empty() && nomeDir.back() != '/')
		nomeDir.push_back('/');
	DIR *dir = 0;
    struct dirent *entrada = 0;
    unsigned char isFile = 0x8;

    dir = opendir (nomeDir.c_str());

    if (dir == 0) {
        std::cerr << "Nao foi possível abrir diretorio com as instâncias." << std::endl;
        exit (1);
    }

	std::string solucoesPath = nomeDir;
	if (!solucoesPath.empty() && solucoesPath.back() != '/')
		solucoesPath.push_back('/');
	solucoesPath += "solucoes";

	struct stat st;
	if (stat(solucoesPath.c_str(), &st) == -1) {
		if (mkdir(solucoesPath.c_str(), 0755) != 0) {
			std::cerr << "Failed to create directory '" << solucoesPath << "': " << strerror(errno) << std::endl;
		} else {
			std::cout << "Created directory: " << solucoesPath << std::endl;
		}
	}
	while ((entrada = readdir (dir))){
		if (entrada->d_type == isFile){
			for (int i=1;i<=repeticoes;++i) {
				std::stringstream convert;
				std::cout << i << " Execucao: " << nomeDir+entrada->d_name << std::endl;
				convert << i;
				std::string outFile = nomeDir + "solucoes/GA_SOLUCAO_"+entrada->d_name + "_" + convert.str();
				struct stat st_out;
				if (stat(outFile.c_str(), &st_out) == 0){
					std::cout << "This running is done. Skipping to next run." << std::endl;
					continue;
				}
				std::string cmd = "./samplecode " + outFile + " <" + nomeDir + entrada->d_name;
				std::cout << "Running: " << cmd << std::endl;
				int s = system(cmd.c_str());
				std::cout << "Return code: " << s << " for output file: " << outFile << std::endl;
				
				if (stat(outFile.c_str(), &st_out) == 0){
					std::cout << "Output created: " << outFile << " (size=" << st_out.st_size << ")" << std::endl;
				} else {
					std::cout << "Output NOT found: " << outFile << " after running samplecode" << std::endl;
				}
			}
		}
	}
    closedir (dir);

	std::cout << "Generating summary in: " << solucoesPath << std::endl;
	DIR *dir2 = opendir(solucoesPath.c_str());
	if (dir2 == nullptr) {
		std::cerr << "Could not open solucoes directory for summary: " << strerror(errno) << std::endl;
	} else {
		std::ifstream file;
		std::ofstream fileR;
		std::string resumoPath = solucoesPath + "/GA_RESUMO.txt";
		fileR.open(resumoPath.c_str());
		if (!fileR.is_open()){
			std::cerr << "Failed to open resumo file for writing: " << resumoPath << std::endl;
		} else {
			// Cabe\u00e7alho
			fileR << "Arquivo\tMakespan_Inicial\tMelhor_Makespan\tTempo_Execucao(s)\tIteracoes\tLS_best\tswap2_avg_us\tins_avg_us\topt2_avg_us" << std::endl;
			
			struct dirent *ent2;
			while ((ent2 = readdir(dir2)) != nullptr){
				if (ent2->d_type == isFile){
					std::string nomeArq = ent2->d_name;
					if (nomeArq == "GA_RESUMO.txt") continue;
					std::string filePath = solucoesPath + "/" + nomeArq;
					file.open(filePath.c_str(), std::ifstream::in);
					if (!file.is_open()) continue;
					
					// L\u00ea a primeira linha (resumo parsev\u00e1vel)
					std::string line;
					std::string inicial_str, melhor_str, tempo_str, iters_str;
					if (!std::getline(file, line)) {
						file.close();
						continue;
					}
					std::istringstream iss(line);
					if (iss >> inicial_str >> melhor_str >> tempo_str >> iters_str) {
						// compat: linha antiga tinha um 5º token (evals) que agora ignoramos
						std::string ignored_evals;
						(void)(iss >> ignored_evals);

						double s2_us = -1.0;
						double in_us = -1.0;
						double o2_us = -1.0;
						std::string win = "NA";

						while (std::getline(file, line)) {
							if (line.find("swap2:") != std::string::npos) {
								get_num(line, "avg_us=", s2_us);
							} else if (line.find("insertion:") != std::string::npos) {
								get_num(line, "avg_us=", in_us);
							} else if (line.find("opt2:") != std::string::npos) {
								get_num(line, "avg_us=", o2_us);
							} else if (line.find("Melhor (por ganho_total):") != std::string::npos) {
								get_tail(line, "Melhor (por ganho_total):", win);
							}
						}

						int win_id = 0; // 0=NA/EMPATE, 1=swap2, 2=insertion, 3=opt2
						if (win == "swap2") win_id = 1;
						else if (win == "insertion") win_id = 2;
						else if (win == "opt2") win_id = 3;

						fileR << nomeArq
							  << "\t" << inicial_str
							  << "\t" << melhor_str
							  << "\t" << tempo_str
							  << "\t" << iters_str
							  << "\t" << win_id
							  << "\t" << s2_us
							  << "\t" << in_us
							  << "\t" << o2_us
							  << std::endl;
					}
					file.close();
				}
			}
			fileR.close();
			std::cout << "Summary written to: " << resumoPath << std::endl;
		}
		closedir(dir2);
	}
	closedir(dir);
	return 0;
}

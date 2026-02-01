#ifndef SAMPLEDECODER_H
#define SAMPLEDECODER_H

#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include "KTNS.h"
#define maxsize 4501
extern unsigned n;
extern std::vector<int>solution;

class SampleDecoder {
public:
	SampleDecoder()  { }
	~SampleDecoder() { }

	double decode(std::vector< double >& chromosome) const{
		std::vector<int>jobs;
		std::vector < std::pair < double, unsigned > > ranking(chromosome.size());

		for(unsigned i = 0; i < chromosome.size(); ++i){
			ranking[i]=std::pair<double,unsigned>(chromosome[i],i);
		}
		
		std::sort(ranking.begin(), ranking.end());
		jobs.clear();
		
		for(std::vector<std::pair<double, unsigned>>::const_iterator i = ranking.begin(); i!=ranking.end(); ++i){
			jobs.push_back(i->second);
		}
		
		long trocas = KTNS(jobs);
		return trocas;
	}

private:
	std::vector<unsigned> tProcessamento;
};

#endif

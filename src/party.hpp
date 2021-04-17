#ifndef __PARTY_HPP__
#define __PARTY_HPP__

#include "dataset.hpp"
#include <abycore/aby/abyparty.h>
#include <string>

class Party
{
private:
	Dataset datatset;
	size_t nr_interset;
	vector<KV_type> shr_dataset;

	struct /* ABYParty Parameters */
	{
		e_role role;
		std::string address;
		uint16_t port;
		seclvl seclevel;
		uint32_t bitlen;
		uint32_t nthreads;
		e_mt_gen_alg mt_alg;
	};

	int func1(KV_type & element, CSocket * tsocket);
	int func2(CSocket * tsocket);
	// share * BuildCompareCircuit(BooleanCircuit * bcirc, share * srv_i, share * srv_j, share * cli_i, share * cli_j);

protected:
	void Prune();
	void Merge();
	void Sort();

public:
	Party() {}
	~Party() {}

	void Run();

};

#endif
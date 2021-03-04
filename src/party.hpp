#ifndef __PARTY_HPP__
#define __PARTY_HPP__

// ABY Libaraies
#include <abycore/aby/abyparty.h>
#include <abycore/circuit/share.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/sharing/sharing.h>

#include "config.h"
#include "dataset.hpp"

class Party
{
private:
	std::vector<share*> BuildMergeAndSortCircuit(share** srv_set, share** cli_set, share** shr_rnd_srv_set, uint32_t neles, uint32_t bitlen, BooleanCircuit* circ);
	std::vector<uint32_t> PutVectorBitonicSortGate(share** srv_set, share** cli_set, uint32_t neles, uint32_t bitlen, BooleanCircuit* circ);
	std::vector<uint32_t> PutVectorCondSwapGate(uint32_t a, uint32_t b, uint32_t s, BooleanCircuit* circ);
	data_t xor_nonces(data_t x);
	template<class T>
	share* PutINGate(BooleanCircuit* circ, uint32_t bitlen, e_role role, T data);
	uint64_t generate_R();

	// share vector
	std::vector<data_t> shr_data_set;
	std::vector<int> shr_gap;
	std::vector<double> shr_mass; 

protected:
	// Original data set
	DataSet data_set;

	// ABY Party parameters
	e_role role;
	std::string address;
	uint16_t port;
	seclvl seclevel;
	uint32_t bitlen;
	uint32_t nthreads;
	e_mt_gen_alg mt_alg;

	void ReadDataSet();

	void Prune();
	void MergeAndShare();
	void SelectionProbability();
	size_t TopOneSelection();
	void TopKSelection();
	uint64_t RandomDraw(uint64_t M);

public:
	Party() {}
	~Party() {}

	void Init(e_role role, std::string address, uint16_t port, seclvl seclevel, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg);
	void Run();
};

#endif
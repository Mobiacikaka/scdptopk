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

    // Arithmetic Share
	std::vector<data_t> shr_dataset;
	std::vector<int> shr_gap;
	std::vector<double> shr_mass;

	// nonces list of size k
	size_t m_k;
	std::vector<data_t> nonces1;
	std::vector<data_t> nonces2;

	// auxiliary functions
	static inline size_t generate_s() {
		return kS;
	}
	std::vector<uint32_t> PutVectorBitonicSortGate(share** srv_set, share** cli_set, uint32_t neles, uint32_t bitlen, BooleanCircuit* circ);
	std::vector<uint32_t> PutVectorCondSwapGate(uint32_t a, uint32_t b, uint32_t s, BooleanCircuit* circ);
	std::vector<share*>   BuildMergeAndSortCircuit(share** srv_set, share** cli_set, share** r_srv_set, uint32_t neles, uint32_t bitlen, BooleanCircuit* bcirc);
	virtual data_t xor_nonces(data_t nonces) = 0;

	// Algorithm One
	virtual void Prune() = 0;
	// Algorithm Two
	virtual void MergeAndShare() = 0;
	// Algorithm Three
	virtual void SelectionProbability() = 0;
	// Algorithm Four
	// virtual void MedianSelection() = 0;
	void MedianSelection();
	virtual uint64_t generate_R() = 0;
	template<class T>
		share* PutINGate(BooleanCircuit* circ, uint32_t bitlen, e_role role, T data);
	// Algorithm Seven
	uint64_t RandomDraw(uint64_t M, std::vector<data_t>& nonces);

public:
	Party();
	virtual ~Party() = 0;

	void SetParameters(e_role role, std::string address, uint16_t port, seclvl seclevel, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg);
	void Run();

};

#endif
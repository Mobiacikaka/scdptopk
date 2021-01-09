//utilities
#include <ENCRYPTO_utils/crypto/crypto.h>
#include <ENCRYPTO_utils/parse_options.h>
#include <ENCRYPTO_utils/socket.h>
#include <ENCRYPTO_utils/connection.h>
//ABY Party class
#include <abycore/sharing/sharing.h>
#include <abycore/circuit/arithmeticcircuits.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/aby/abyparty.h>
//system
#include <cassert>

#include "client.hpp"

Client::Client() 
{

}

Client::~Client() {
}

size_t Client::generate_k() {
    std::unique_ptr<CSocket> tsocket;
    size_t srv_size(0);
	size_t cli_size(this->data_set.GetSizeofDataSet());

    tsocket = Connect(this->address, this->port);
    if(!tsocket) {
		std::cerr << "Listen failed!" << std::endl;
		std::exit(1);
    }

    tsocket->Send   (static_cast<void*>(&cli_size), sizeof(size_t));
    tsocket->Receive(static_cast<void*>(&srv_size), sizeof(size_t));
    tsocket->Close();

    return (srv_size + cli_size) / 2;
}

uint32_t Client::comp_median() {
    const data_t median = data_set.GetMedian();

    ABYParty* party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg);
    std::vector<Sharing*>& sharings = party->GetSharings();

    BooleanCircuit* circ = (BooleanCircuit*) sharings[S_YAO]->GetCircuitBuildRoutine();

    share *shr_srv_median, *shr_cli_median, *shr_out;

    shr_srv_median = circ->PutDummyINGate(bitlen);
    shr_cli_median = circ->PutINGate(median, bitlen, role);

    shr_out = circ->PutGTGate(shr_srv_median, shr_cli_median);
    shr_out = circ->PutOUTGate(shr_out, ALL);

    party->ExecCircuit();

    uint32_t o(shr_out->get_clear_value<uint32_t>());

    delete party;

    return o;
}

uint64_t Client::generate_R() {
    std::unique_ptr<CSocket> tsocket;
    double mass_srv(0);
	double mass_cli(this->shr_mass[this->shr_mass.size()-1]);

    tsocket = Connect(this->address, this->port);
    if(!tsocket) {
		std::cerr << "Listen failed!" << std::endl;
		std::exit(1);
    }

    tsocket->Send   (static_cast<void*>(&mass_cli), sizeof(double));
    tsocket->Receive(static_cast<void*>(&mass_srv), sizeof(double));
    tsocket->Close();

    return static_cast<uint64_t>(mass_srv+mass_cli);
}

data_t Client::xor_nonces(data_t nonces_cli) {
    ABYParty* party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg, 4000000);
    std::vector<Sharing*>& sharings = party->GetSharings();

    BooleanCircuit* circ = (BooleanCircuit*) sharings[S_BOOL]->GetCircuitBuildRoutine();

    share *shr_srv, *shr_cli, *shr_xor, *shr_out;

    shr_srv = circ->PutDummyINGate(bitlen);
    shr_cli = circ->PutINGate(nonces_cli, bitlen, role);

    shr_xor = circ->PutXORGate(shr_srv, shr_cli);
    shr_out = circ->PutOUTGate(shr_xor, ALL);

    party->ExecCircuit();

    uint32_t o(shr_out->get_clear_value<data_t>());

    delete party, shr_srv, shr_cli, shr_xor, shr_out;

    return o;
}

void Client::Prune() {
    const data_t padding = kB;
    const size_t s = this->generate_s();
    const size_t k = this->generate_k();
	bool comp(false);

    assert(padding == kA || padding == kB);

    this->m_k = k;

    data_set.Pad(k, padding);

    for (size_t i = 0; i < s; i ++) {
        // std::cout << "Pruning Steps " << i+1 << std::endl;

		comp = comp_median();
        assert(comp == 1 || comp == 0);

        if(comp == true) data_set.KeepLowerHalf();
        else data_set.KeepUpperHalf();
    }
}

void Client::MergeAndShare() {
    assert(data_set.IsSorted() == true);

	ABYParty* party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg, 4000000);
	std::vector<Sharing*>& sharings = party->GetSharings();

	BooleanCircuit* bcirc = (BooleanCircuit*) sharings[S_BOOL]->GetCircuitBuildRoutine();

    size_t neles = data_set.GetSizeofDataSet();
    size_t shrsize = 2 * neles;
    share** shr_srv_set = (share**) malloc(sizeof(share*) * neles);
    share** shr_cli_set = (share**) malloc(sizeof(share*) * neles);
    share** shr_rnd_srv_set = (share**) malloc(sizeof(share*) * shrsize);
    share** shr_out = (share**) malloc(sizeof(share*) * shrsize);

    for (size_t i = 0; i < neles; i ++) {
        shr_srv_set[i] = bcirc->PutDummySIMDINGate(bitlen, 1);
        shr_cli_set[i] = bcirc->PutSIMDINGate(bitlen, data_set[neles-1-i], 1, role);
    }

    for (size_t i = 0; i < shrsize; i ++) {
        shr_rnd_srv_set[i] = bcirc->PutDummySIMDINGate(bitlen, 1);
    }

	std::vector<share*> out = BuildMergeAndSortCircuit(shr_srv_set, shr_cli_set, shr_rnd_srv_set, neles, bitlen, bcirc);

    for (size_t i = 0; i < shrsize; i ++) {
        shr_out[i] = bcirc->PutOUTGate(out[i], CLIENT);
    }

    party->ExecCircuit();

    shr_dataset.resize(shrsize);
    for(size_t i = 0; i < shrsize; i ++) {
        shr_dataset[i] = shr_out[i]->get_clear_value<data_t>();
    }

    // delete operation
    delete party;
    free(shr_srv_set);
    free(shr_cli_set);
    free(shr_rnd_srv_set);
    for(size_t i = 0; i < shrsize; i ++) delete shr_out[i];
    free(shr_out);
}

void Client::SelectionProbability()
{
    shr_dataset.insert(shr_dataset.begin(), kA);
    shr_dataset.insert(shr_dataset.end(), kB);

    size_t length(shr_dataset.size());
    shr_gap.resize(length);
    shr_mass.resize(length);

    // compute gaps(share) first
    size_t mpos(length/2); // median position
    for(size_t i = 0; i < mpos-1; i ++) 
        shr_gap[i] = static_cast<int>(shr_dataset[i+1] - shr_dataset[i]);
    shr_gap[mpos-1] = 1;
    for(size_t i = mpos; i < length; i ++)
        shr_gap[i] = static_cast<int>(shr_dataset[i] - shr_dataset[i-1]);

    // compute other utility
    int utility;
    double weight, shift;
    for(size_t i = 0; i < length; i ++) {
        utility = i < mpos ? i - mpos + 1 : mpos - i;
        weight = exp(kEPSILON * utility);
        shift = i > 0 ? shr_mass[i-1] : 0;
        shr_mass[i] = shift + weight * shr_gap[i];
    }

    nonces1.resize(this->m_k);
    for(size_t i = 0; i < nonces1.size(); i ++)
        nonces1[i] = random_range(0, kB-kA);
    nonces2.resize(this->m_k);
    for(size_t i = 0; i < nonces2.size(); i ++)
        nonces2[i] = random_range(0, kB-kA);

    shr_dataset = std::vector<uint32_t>(shr_dataset.begin()+1, shr_dataset.end()-1);
    shr_gap     = std::vector<int32_t >(shr_gap.begin()+1,     shr_gap.end()-1);
    shr_mass    = std::vector<double  >(shr_mass.begin()+1,    shr_mass.end()-1);
}

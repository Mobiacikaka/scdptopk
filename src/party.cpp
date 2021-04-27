#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "party.hpp"

#include <cassert>
#include <random>
#include <cmath>
#include <iostream>
#include <ENCRYPTO_utils/socket.h>
#include <ENCRYPTO_utils/connection.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/sharing/sharing.h>
#include <cryptopp/md5.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <boost/math/distributions/laplace.hpp>

using namespace std;

void Party::set_param(e_role role, std::string address, uint16_t port, seclvl seclevel, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg)
{
	this->role = role;
	this->address = address;
	this->port = port;
	this->seclevel = seclevel;
	this->bitlen = bitlen;
	this->nthreads = nthreads;
	this->mt_alg = mt_alg;
}

void Party::Run()
{
	datatset.ReadDataset();
	datatset.SortDataset();

	this->Prune();
	datatset.print("Prune.out");

}

const size_t k = 50;
const size_t prune_times = 5;

void Party::Prune()
{
	size_t i;
	struct bloom * blm;
	unique_ptr<CSocket> tsocket;

	if(role == SERVER)
	{
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed!" << endl;
			exit(1);
		}

		for(i = 0; i < prune_times; i ++)
		{
			blm = datatset.BloomPack(k * pow(2, i));
			tsocket->Send	((void *)blm, sizeof(struct bloom));
			tsocket->Receive((void *)(&nr_interset), sizeof(size_t));

			if(nr_interset * 1.0 / k >= 0.9) break;
		}

		tsocket->Close();
	}
	else if(role == CLIENT)
	{
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed!" << endl;
			exit(1);
		}

		for(i = 0; i < prune_times; i ++)
		{
			tsocket->Receive((void *)blm, sizeof(struct bloom));
			nr_interset = datatset.BloomCheck(blm, k * pow(2, i));
			tsocket->Send	((void *)(&nr_interset), sizeof(size_t));

			if(nr_interset * 1.0 / k >= 0.9) break;
		}

		tsocket->Close();
	}
	else
	{
		cerr << "Wrong e_role!" << endl;
		exit(0);
	}

	datatset.Prune(k * pow(2, i));
}

int Party::MakeShareSrv(KV_type & element, CSocket * tsocket)
{
	using namespace CryptoPP;
	
	string digest;
	int shr_rnd(0);
	Weak1::MD5 hash;

	hash.Update((const byte *)element.ID.c_str(), element.ID.size());
	digest.resize(hash.DigestSize());
	hash.Final((byte *)&digest[0]);

	string encoded;
	StringSink tmpssnk(encoded);
	HexEncoder tmphe(&tmpssnk);
	StringSource tmpssrc(digest, true, &tmphe);

	tsocket->Send((void *)encoded.c_str(), encoded.size());
	tsocket->Receive((void *)&shr_rnd, sizeof(shr_rnd));

	return shr_rnd + element.count;
}

int Party::MakeShareCli(CSocket * tsocket)
{
	using namespace CryptoPP;

	// Part 1: decode the message send from server
	string encoded;
	string decoded;

	encoded.resize(64);
	tsocket->Receive((void *)&encoded[0], encoded.size());

	HexDecoder decoder;
	decoder.Put((byte *)encoded.data(), encoded.size());
	decoder.MessageEnd();
	
	word64 size = decoder.MaxRetrievable();
	if(size && size <= SIZE_MAX)
	{
		decoded.resize(size);
		decoder.Get((byte *)&decoded[0], decoded.size());
	}
	
	// Part 2: check the local dataset and find the same one
	size_t i;
	for(i = 0; i < datatset.size(); i ++)
	{
		string ID = datatset[i].ID;
		string digest;
		Weak1::MD5 hash;
		
		hash.Update((const byte *)&ID[0], ID.size());
		digest.resize(hash.DigestSize());
		hash.Final((byte *)&digest[0]);
		
		if(digest == decoded) break;
	}
	
	int shr_rnd = rand();
	tsocket->Send((void *)&shr_rnd, sizeof(shr_rnd));

	if(i < datatset.size())
	{
		shr_rnd += datatset[i].count;
		datatset.erase(i);
	}

	return shr_rnd;
}

void Party::Merge()
{
	unique_ptr<CSocket> tsocket;

	if(role == SERVER)
	{
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed!" << endl;
			exit(1);
		}

		for(size_t i = 0; i < datatset.size(); i ++)
		{
			KV_type tmp_kv(datatset[i].ID, MakeShareSrv(datatset[i], tsocket.get()));
			shr_dataset.push_back(tmp_kv);
		}

		for(size_t i = 0; i < datatset.size() - nr_interset; i ++) {
			int rnd;
			tsocket->Receive((void *)&rnd, sizeof(rnd));
			KV_type tmp_kv("", rnd);
			shr_dataset.push_back(tmp_kv);
		}
	}
	else {
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed!" << endl;
			exit(1);
		}

		for(size_t i = 0; i < datatset.size(); i ++)
		{
			KV_type tmp_kv("", MakeShareCli(tsocket.get()));
			shr_dataset.push_back(tmp_kv);
		}

		for(size_t i = 0; i < datatset.size(); i ++)
		{
			int rnd = rand();
			tsocket->Send((void *)&rnd, sizeof(rnd));
			KV_type tmp_kv(datatset[i].ID, rnd-datatset[i].count);
			shr_dataset.push_back(tmp_kv);
		}
	}

	tsocket->Close();
}

void Party::Sort()
{
	for(size_t i = 1; i < shr_dataset.size(); i ++)
	{
		size_t j = i-1;
		while(j >= 0)
		{
			bool flag(false);

			{
				ABYParty * party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg, 4000000);
				vector<Sharing*> sharings = party->GetSharings();
				BooleanCircuit * bcirc = (BooleanCircuit *) sharings[S_YAO]->GetCircuitBuildRoutine();

				share *srv_i, *cli_i, *srv_j, *cli_j;
				if(role == SERVER)
				{
					srv_i = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[i].count), bitlen, role);
					srv_j = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[j].count), bitlen, role);
					cli_i = bcirc->PutDummyINGate(bitlen);
					cli_j = bcirc->PutDummyINGate(bitlen);
				}
				else
				{
					srv_i = bcirc->PutDummyINGate(bitlen);
					srv_j = bcirc->PutDummyINGate(bitlen);
					cli_i = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[i].count), bitlen, role);
					cli_j = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[j].count), bitlen, role);
				}

				share *cmb_i, *cmb_j, *shr_cmp, *shr_out;
				cmb_i = bcirc->PutSUBGate(srv_i, cli_i);
				cmb_j = bcirc->PutSUBGate(srv_j, cli_j);
				shr_cmp = bcirc->PutGTGate(cmb_i, cmb_j);
				shr_out = bcirc->PutOUTGate(shr_cmp, ALL);

				party->ExecCircuit();

				uint32_t output = shr_out->get_clear_value<uint32_t>();
				flag = output;

				delete party;
				delete srv_i, cli_i, srv_j, cli_j;
				delete cmb_i, cmb_j, shr_cmp, shr_out;
			}

			if(flag == true)
			{
				KV_type tmp = shr_dataset[i];
				shr_dataset[i] = shr_dataset[j];
				shr_dataset[j] = tmp;
			}
		}
	}
}

double Party::get_delta_q(double delta, size_t kbar, double c)
{
	double delta_q = delta;
	double delta_max;

	delta_max = kbar * ( ( 2*pow(delta_q, c) + delta_q - c*(pow(delta_q, c) + 2*delta_q) )/(4 - 4*c) );
	while(delta_max > delta)
	{
		delta_q = delta_q * 0.99;
		delta_max = kbar * ( ( 2*pow(delta_q, c) + delta_q - c*(pow(delta_q, c) + 2*delta_q) )/(4 - 4*c) );
	}

	return delta_q;
}

double Party::get_T(double delta_q, double eps1, double eps2)
{
	unique_ptr<CSocket> tsocket;

	double T;
	if(role == SERVER) {
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed" << endl;
			exit(1);
		}

		T = log( 1/delta_q ) / (eps2 / 2) + boost::math::laplace(0, 1/eps1).location();
		tsocket->Send((void *)&T, sizeof(T));
		tsocket->Close();
	}
	else {
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed" << endl;
			exit(1);
		}

		tsocket->Receive((void *)&T, sizeof(T));
		tsocket->Close();
	}

	return T;
}

double Party::get_qi(size_t i, double eps2)
{
	ABYParty * party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg, 4000000);
	vector<Sharing*> sharings = party->GetSharings();
	BooleanCircuit * bcirc = (BooleanCircuit *) sharings[S_YAO]->GetCircuitBuildRoutine();

	share *srv_i, *cli_i, *srv_j, *cli_j;
	if(role == SERVER)
	{
		srv_i = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[i].count), bitlen, role);
		srv_j = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[i+1].count), bitlen, role);
		cli_i = bcirc->PutDummyINGate(bitlen);
		cli_j = bcirc->PutDummyINGate(bitlen);
	}
	else
	{
		srv_i = bcirc->PutDummyINGate(bitlen);
		srv_j = bcirc->PutDummyINGate(bitlen);
		cli_i = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[i].count), bitlen, role);
		cli_j = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[i+1].count), bitlen, role);
	}

	share *cmb_i, *cmb_j, *cmb_dif, *shr_out;
	cmb_i = bcirc->PutSUBGate(srv_i, cli_i);
	cmb_j = bcirc->PutSUBGate(srv_j, cli_j);
	cmb_dif = bcirc->PutSUBGate(cmb_i, cmb_j);
	shr_out = bcirc->PutOUTGate(cmb_dif, ALL);

	party->ExecCircuit();

	int output = static_cast<int32_t>(shr_out->get_clear_value<uint32_t>());

	delete party;
	delete srv_i, srv_j, cli_i, cli_j;
	delete cmb_i, cmb_j, cmb_dif, shr_out;

	double qi = output - 1;
	double qi_n;
	if(role == SERVER)
	{
		unique_ptr<CSocket> tsocket;
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed!" << endl;
			exit(1);
		}

		qi_n = qi + boost::math::laplace(0, 1/(eps2/2)).location();
		tsocket->Send((void*)&qi_n, sizeof(qi_n));
		tsocket->Close();
	}
	else
	{
		unique_ptr<CSocket> tsocket;
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed" << endl;
			exit(1);
		}

		tsocket->Receive((void*)&qi_n, sizeof(qi_n));
		tsocket->Close();
	}
	
	return qi_n;
}

double Party::generate_R(double mass)
{
	double mass2;

	unique_ptr<CSocket> tsocket;
	if(role == SERVER) {
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed" << endl;
			exit(1);
		}

		tsocket->Send((void*)&mass, sizeof(mass));
		tsocket->Receive((void *)&mass2, sizeof(mass2));
	}
	else {
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed" << endl;
			exit(1);
		}

		tsocket->Receive((void *)&mass2, sizeof(mass2));
		tsocket->Send((void *)&mass, sizeof(mass));
	}

	tsocket->Close();
	return mass + mass2;
}

uint64_t Party::RandomDraw(double mass)
{
	double R = generate_R(mass);
	uint64_t M = (uint64_t)R;
	uint64_t mask = 0;

	for(size_t i = 63; i >=0; i --) {
		if(M >> i) {
			mask = (1 << ++i) - 1;
			break;
		}
	}

	unique_ptr<CSocket> tsocket;
	uint64_t xrnd;
	if(role == SERVER) {
		tsocket = Listen(address, role);
		if(!tsocket) {
			cerr << "Listen Failed" << endl;
			exit(1);
		}

		while(true) {
			uint64_t rnd1 = rand();
			uint64_t rnd2 = rand();
			xrnd = rnd1 ^ rnd2;
			xrnd &= mask;
			
			if(xrnd < M) break;
		}
		
		tsocket->Send((void *)&xrnd, sizeof(xrnd));
	}
	else {
		tsocket = Connect(address, role);
		if(!tsocket) {
			cerr << "Connect Failed" << endl;
			exit(1);
		}

		tsocket->Receive((void *)&xrnd, sizeof(xrnd));
	}
	tsocket->Close();

	return xrnd;
}

/*
size_t Party::Top1Selection()
{
	uint64_t r = RandomDraw();

	ABYParty * party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg, 4000000);
	vector<Sharing*> & sharings = party->GetSharings();
	BooleanCircuit * bcirc = (BooleanCircuit*)sharings[S_BOOL]->GetCircuitBuildRoutine();

	for(size_t i = 0; i < shr_dataset.size(); )
}
*/

template<class T>
void Party::erase(vector<T> v, size_t i)
{
	size_t len = v.size();
	v.erase(v.begin() + i);
	if(i >= len/2)
		v.erase(v.end() - i);
	else 
		v.erase(v.end() - i - 1);
}

vector<size_t> Party::random_draw_output(double eps_em)
{
	// Selection Probability Calculate
	vector<int> shr_dataset;
	vector<int> gap;
	vector<double> mass;
	vector<size_t> nr;

	shr_dataset.resize(this->shr_dataset.size() * 2);
	size_t length(shr_dataset.size());
	for(size_t i = 0; i < this->shr_dataset.size(); i ++)
		shr_dataset[i] = this->shr_dataset[i].count;
	for(size_t i = length; i < length * 2; i ++)
		shr_dataset[i] = shr_dataset[2*length - i - 1];

	gap.resize(length);
	mass.resize(length);

	for(size_t i = 0; i < length / 2; i ++)
		nr[i] = nr[length - i - 1] = i;

	size_t middle(length / 2);
	for(size_t i = 0; i < length; i ++)
	{
		if(i == 0)
			gap[i] = static_cast<int>(shr_dataset[i] - 0);
		else if(i < middle)
			gap[i] = static_cast<int>(shr_dataset[i] - shr_dataset[i-1]);
		else if(i < length - 1)
			gap[i] = static_cast<int>(shr_dataset[i] - shr_dataset[i+1]);
		else
			gap[i] = static_cast<int>(shr_dataset[i]);

		int utility = i < middle ? i - middle + 1 : middle - i;
		double weight = exp(eps_em * utility);
		double shift = i > 0 ? mass[i - 1] : 0;
		mass[i] = shift + weight * gap[i];
	}

	// Top K Selection
	vector<size_t> output;
	uint64_t r = RandomDraw(mass[length - 1]);
	for(size_t i = 0; i < k; i ++)
	{
		int j = -1;
		for(size_t i = 0; i < shr_dataset.size(); i ++)
		{
			ABYParty * party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg, 4000000);
			vector<Sharing*> & sharings = party->GetSharings();
			BooleanCircuit * bcirc = (BooleanCircuit*)sharings[S_BOOL]->GetCircuitBuildRoutine();

			share *di_srv, *di_cli, *shr_e;
			if(role == SERVER) {
				di_srv = bcirc->PutINGate((uint32_t)shr_dataset[i], bitlen, role);
				di_cli = bcirc->PutDummyINGate(bitlen);
			}
			else {
				di_srv = bcirc->PutDummyINGate(bitlen);
				di_cli = bcirc->PutINGate((uint32_t)shr_dataset[i], bitlen, role);
			}
			shr_e = bcirc->PutSUBGate(di_srv, di_cli);

			share *gapi_srv, *gapi_cli, *shr_gap;
			if(role == SERVER) {
				gapi_srv = bcirc->PutINGate((uint32_t)gap[i], bitlen, role);
				gapi_cli = bcirc->PutDummyINGate(bitlen);
			}
			else {
				gapi_srv = bcirc->PutDummyINGate(bitlen);
				gapi_cli = bcirc->PutINGate((uint32_t)gap[i], bitlen, role);
			}
			shr_gap = bcirc->PutSUBGate(gapi_srv, gapi_cli);

			share *massi_srv, *massi_cli, *shr_mass;
			if(role == SERVER) {
				massi_srv = bcirc->PutINGate((uint64_t)mass[i], 64, role);
				massi_cli = bcirc->PutDummyINGate(64);
			}
			else {
				massi_srv = bcirc->PutDummyINGate(64);
				massi_cli = bcirc->PutINGate((uint64_t)mass[i], 64, role);
			}
			shr_mass = bcirc->PutSUBGate(massi_srv, massi_cli);

			share * shr_r;
			if(role == SERVER)
				shr_r = bcirc->PutINGate(r, 64, role);
			else
				shr_r = bcirc->PutDummyINGate(64);

			share * shr_cmp = bcirc->PutGTGate(shr_r, shr_mass);

			share *out_cmp = bcirc->PutOUTGate(shr_cmp, ALL);

			party->ExecCircuit();

			uint32_t cmp = out_cmp->get_clear_value<uint32_t>();

			delete party;
			delete di_srv, di_cli, shr_e;
			delete gapi_srv, gapi_cli, shr_gap;
			delete massi_srv, massi_cli, shr_mass;
			delete shr_r, shr_cmp, out_cmp;

			if(cmp == true) {
				output.push_back(nr[i]);
				erase(nr, i);
				erase(shr_dataset, i);
				erase(gap, i);
				erase(mass, i);
			}
		}
	}

	return output;
}

vector<size_t> Party::Selection(const size_t k, const size_t kbar, const double epsilon, const double p1, const double eps_em, const double delta)
{
	double eps1, eps2;
	double c;
	double delta_q;
	double T; // threshold

	eps1 = p1 * epsilon;
	eps2 = epsilon - eps1;
	c = 2 * eps1 / eps2 ;
	delta_q = get_delta_q(delta, kbar, c);
	T = get_T(delta_q, eps1, eps2);

	vector<size_t> output; // set of indices
	for(size_t i = kbar; i > 0; i --)
	{
		double qi_n = get_qi(i, eps2); // noisy qi
		
		if(qi_n > T)
		{
			if(i > k)
			{
				shr_dataset.erase(shr_dataset.begin()+i, shr_dataset.end());
				return random_draw_output(eps_em);
			}
			else
			{
				output.resize(i);
				for(size_t j = 0; j < i; j ++) output[j] = j;
				return output;
			}
		}
	}

	return output;
}

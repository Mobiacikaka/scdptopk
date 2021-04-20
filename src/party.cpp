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

void Party::Run()
{
	this->Prune();
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
	tsocket->Send((void *)shr_rnd, sizeof(shr_rnd));

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

vector<size_t> Party::random_draw_output(double eps_em)
{
	const int nrolls = shr_dataset.size() * 10;

	default_random_engine generator;
	vector<uint32_t> pos;
	vector<uint32_t> count;

	count.resize(shr_dataset.size());
	for(size_t i = 0; i < shr_dataset.size(); i ++) 
		count[i] = static_cast<uint32_t>(shr_dataset[i].count);

	discrete_distribution<size_t> distribution(count.begin(), count.end());

	for(size_t i = 0; i < nrolls; i ++) {
		size_t number = distribution(generator);
		pos[number] ++;
	}

	// sort
	vector<size_t> loc;
	for(size_t i = 0; i < shr_dataset.size(); i ++)
	{
		loc[i] = i;
	}

	for(size_t i = 1; i < pos.size(); i ++)
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
					srv_i = bcirc->PutINGate(pos[i], bitlen, role);
					srv_j = bcirc->PutINGate(pos[j], bitlen, role);
					cli_i = bcirc->PutDummyINGate(bitlen);
					cli_j = bcirc->PutDummyINGate(bitlen);
				}
				else
				{
					srv_i = bcirc->PutDummyINGate(bitlen);
					srv_j = bcirc->PutDummyINGate(bitlen);
					cli_i = bcirc->PutINGate(pos[i], bitlen, role);
					cli_j = bcirc->PutINGate(pos[j], bitlen, role);
				}

				share *cmb_i, *cmb_j, *shr_cmp, *shr_out;
				cmb_i = bcirc->PutADDGate(srv_i, cli_i);
				cmb_j = bcirc->PutADDGate(srv_j, cli_j);
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
				uint32_t tmp = pos[i];
				pos[i] = pos[j];
				pos[j] = tmp;

				size_t tmploc = loc[i];
				loc[i] = loc[j];
				loc[j] = loc[i];
			}
		}
	}

	// random draw top k
	loc.erase(loc.begin() + k, loc.end());
	return loc;
}

vector<size_t> Party::Selection(const size_t k, const size_t kbar, const double epsilon, const double p1, const double eps_em, const double delta)
{
	size_t k, kbar;
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

#include "party.hpp"

#include <cassert>
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

int Party::func1(KV_type & element, CSocket * tsocket)
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

int Party::func2(CSocket * tsocket)
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
			KV_type tmp_kv(datatset[i].ID, func1(datatset[i], tsocket.get()));
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
			KV_type tmp_kv("", func2(tsocket.get()));
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
				KV_type tmp = shr_dataset[i];
				shr_dataset[i] = shr_dataset[j];
				shr_dataset[j] = tmp;
			}
		}
	}
}

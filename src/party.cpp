#include "party.hpp"

#include <cmath>
#include <iostream>
#include <ENCRYPTO_utils/socket.h>
#include <ENCRYPTO_utils/connection.h>

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
	if(role == SERVER)
	{
		unique_ptr<CSocket> tsocket;
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed!" << endl;
			exit(1);
		}

		for(i = 0; i < prune_times; i ++)
		{
			size_t nr_interset(0);
			struct bloom * blm;

			blm = datatset.BloomPack(k * pow(2, i));
			tsocket->Send	((void *)blm, sizeof(struct bloom));
			tsocket->Receive((void *)(&nr_interset), sizeof(size_t));

			if(nr_interset * 1.0 / k >= 0.9) break;
		}

		tsocket->Close();
	}
	else if(role == CLIENT)
	{
		unique_ptr<CSocket> tsocket;
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed!" << endl;
			exit(1);
		}

		for(i = 0; i < prune_times; i ++)
		{
			size_t nr_interset(0);
			struct bloom * blm;

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
}
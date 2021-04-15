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

	e_role role;
	std::string address;
	uint16_t port;

	int func1(KV_type & element, CSocket * tsocket);
	int func2(CSocket * tsocket);

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
#ifndef __PARTY_HPP__
#define __PARTY_HPP__

#include "dataset.hpp"
#include <abycore/aby/abyparty.h>
#include <string>

class Party
{
private:
	Dataset datatset;

	e_role role;
	std::string address;
	uint16_t port;

protected:
	void Prune();
	void Merge();

public:
	Party() {}
	~Party() {}

	void Run();

};

#endif
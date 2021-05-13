#ifndef __PARTY_HPP__
#define __PARTY_HPP__

#include "dataset.hpp"
#include <abycore/aby/abyparty.h>
#include <string>
#include <vector>

class Party
{
private:
	Dataset dataset;
	size_t nr_interset;
	std::vector<KV_type> shr_dataset;
	void print_dataset(std::string filename);

	struct /* ABYParty Parameters */
	{
		e_role role;
		uint16_t port;
		seclvl seclevel;
		uint32_t bitlen;
		uint32_t nthreads;
		e_mt_gen_alg mt_alg;
	};
	std::string address;

	size_t k;
	size_t kbar;
	double eps;
	double p1;
	double eps_em;
	double delta;

	int MakeShareSrv(KV_type & element, CSocket * tsocket);
	int MakeShareCli(CSocket * tsocket);

	bool compare(KV_type & kv1, KV_type & kv2);
	bool compare(KV_type & kv1, KV_type & kv2, int);
	size_t partition(size_t left, size_t right);

	double get_delta_q(double delta, size_t kbar, double c);
	double get_T(double delta_q, double eps1, double eps2);
	double get_qi(size_t i, double eps2);
	double gen_laplace(double location, double scale);

	template<class T>
	void erase(std::vector<T> v, size_t i);
	double generate_R(double mass);
	uint64_t RandomDraw(double mass);
	std::vector<size_t> random_draw_output(double eps_em);
	void RandomSelection();

protected:
	void Prune();
	void Merge();
	void Sort();
	// std::vector<size_t> Selection( const size_t k, const size_t kbar, const double epsilon, const double p1, const double eps_em, const double delta);
	void Selection();

public:
	Party() {}
	~Party() {}

	void set_param(e_role role, std::string address, 
		uint16_t port, seclvl seclevel, uint32_t bitlen, 
		uint32_t nthreads, e_mt_gen_alg mt_alg,
		size_t k, size_t kbar, double eps, double p1,
		double eps_em);
	void Run();

};

#endif
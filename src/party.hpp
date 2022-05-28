#ifndef __PARTY_HPP__
#define __PARTY_HPP__

#include "dataset.hpp"
#include <abycore/aby/abyparty.h>
#include <string>
#include <vector>
#include <cryptopp/integer.h>

struct Key {
	struct Pub {
		uint64_t n, y;
	} pub;
	struct Priv {
		uint64_t p, q;
	} priv;
};

class Party
{
private:
	Dataset dataset;
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

	size_t prune_size;
	std::vector<std::string> md5set;
	void makeMD5set();
	int MakeShareSrv(size_t & index, CSocket * tsocket);
	int MakeShareCli(CSocket * tsocket);

	bool compare(KV_type & kv1, KV_type & kv2);
	bool compare(KV_type & kv1, KV_type & kv2, int);

	double get_delta(size_t nr_users);
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

	const int M = 1000;
	struct Key read_key();
	uint64_t encrypt_bit(uint64_t bit, struct Key &key);
	uint64_t decrypt_bit(uint64_t bit, struct Key &key);
	int64_t jacobi(uint64_t bitc, uint64_t p);
	uint64_t power(uint64_t x, uint64_t y, uint64_t p);
	uint64_t get_sizeof_interset_server(std::unique_ptr<CSocket> &, struct Key &, size_t);
	uint64_t get_sizeof_interset_client(std::unique_ptr<CSocket> &, struct Key &, size_t);

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

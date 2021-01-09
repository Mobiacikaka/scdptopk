#include <ENCRYPTO_utils/crypto/crypto.h>
#include <ENCRYPTO_utils/parse_options.h>
#include <abycore/aby/abyparty.h>

#include "party.hpp"
#include "server.hpp"
#include "client.hpp"

int32_t ReadTestOptions(
	int32_t* 		argcp, 
	char*** 		argvp, 
	e_role* 		role,
	uint32_t* 		bitlen, 
	uint32_t* 		nvals, 
	uint32_t* 		secparam, 
	std::string* 	address,
	uint16_t* 		port, 
	int32_t* 		test_op
) 
{
	uint32_t int_role = 0, int_port = 0;

	parsing_ctx options[] =
	{ 
		{ (void*) &int_role, 	T_NUM, "r", "Role: 0/1", 												true , false }, 
		{ (void*) nvals, 		T_NUM, "n", "Number of parallel operation elements", 					false, false }, 
		{ (void*) bitlen, 		T_NUM, "b", "Bit-length, default 32", 									false, false }, 
		{ (void*) secparam, 	T_NUM, "s", "Symmetric Security Bits, default: 128", 					false, false }, 
		{ (void*) address, 		T_STR, "a", "IP-address, default: localhost", 							false, false }, 
		{ (void*) &int_port, 	T_NUM, "p", "Port, default: 7766", 										false, false }, 
		{ (void*) test_op, 		T_NUM, "t", "Single test (leave out for all operations), default: off",	false, false } 
	};

	if (!parse_options(argcp, argvp, options, sizeof(options) / sizeof(parsing_ctx))) {
		print_usage(*argvp[0], options, sizeof(options) / sizeof(parsing_ctx));
		std::cout << "Exiting" << std::endl;
		exit(0);
	}

	assert(int_role < 2);
	*role = (e_role) int_role;

	if (int_port != 0) {
		assert(int_port < 1 << (sizeof(uint16_t) * 8));
		*port = (uint16_t) int_port;
	}

	return 1;
}

int main(int argc, char** argv) {
   
    e_role role;
    uint32_t bitlen = 32, nvals = 31, secparam = 128, nthreads = 1;
    uint16_t port = 7766;
    std::string address = "127.0.0.1";
    int32_t test_op = -1;
    e_mt_gen_alg mt_alg = MT_OT;

    ReadTestOptions(&argc, &argv, &role, &bitlen, &nvals, &secparam, &address, &port, &test_op);

    seclvl seclevel = get_sec_lvl(secparam);

	Party *party;
	if (role == SERVER) party = new Server();
	else party = new Client();

	party->SetParameters(role, address, port, seclevel, bitlen, nthreads, mt_alg);
	party->Run();

	return 0;
}

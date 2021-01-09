#ifndef __SERVER_HPP_
#define __SERVER_HPP_

#include "party.hpp"

class Server : public Party {
private:

protected:
    // auxiliary functions
    size_t generate_k();
    uint64_t generate_R();
    uint32_t comp_median();
    data_t xor_nonces(data_t nonces_srv);

    // main functions
    // One
    void Prune();
    // Two
    void MergeAndShare();
    // Three
    void SelectionProbability();
    // Four
	void MedianSelection();
    // Seven
    // uint64_t RandomDraw(uint64_t M, std::vector<data_t>& nonces);

public:
    Server();
    ~Server();

    void Run();
};

#endif
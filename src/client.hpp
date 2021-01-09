#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include "party.hpp"
class Client : public Party {
private:

protected:
    // auxiliary functions
    size_t generate_k();
    uint64_t generate_R();
    uint32_t comp_median();
    data_t xor_nonces(data_t nonces_cli);

    // main functions
    void Prune();
    void MergeAndShare();
    void SelectionProbability();
    void MedianSelection();

public:
    Client();
    ~Client();

    void Run();
};

#endif
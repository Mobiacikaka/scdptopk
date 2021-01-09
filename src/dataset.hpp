#ifndef __DATASET_HPP__
#define __DATASET_HPP__

#include <vector>

#include "config.h"

class DataSet
{
    friend class Party;
    friend class Server;
    friend class Client;

private:
    void GenerateRandomDataSet(size_t n);

    void SortDataSet();

    std::vector<data_t> data_set;

    bool sorted;

protected:
    // This pad version is not optimized
    void Pad(size_t k, data_t p);

    // get the median element of the data set
    data_t GetMedian() const;

    // retain only the upper half
    void KeepUpperHalf();
    
    // retain only the lower half
    void KeepLowerHalf();

    // Get the size of data set
    size_t GetSizeofDataSet() const;

    // 
    bool IsSorted() const;

    // Pack the PutSIMDINGate in the DataSet class
    data_t operator[](size_t index);

    //! Following function only for test
    void PrintAllElement();

    DataSet();
    ~DataSet();

    // Use Random Function to generate random int list
    void Init();
};

#endif
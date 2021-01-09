#include <iostream>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <random>
#include <memory>

#include "config.h"
#include "dataset.hpp"

DataSet::DataSet()
{
}

DataSet::~DataSet()
{
}

void DataSet::PrintAllElement()
{
    PrintElements(this->data_set);
}

void DataSet::GenerateRandomDataSet(size_t n)
{
    assert(kA < kB);
    this->data_set.resize(n);
    for(size_t i = 0; i < n; i ++)
        data_set[i] = random_range(kA, kB);
}

void DataSet::Pad(size_t k, data_t p)
{
    // std::cout << "Entering Pad" << std::endl;

	assert(p == kA || p == kB);

    // 1. Sort D_p and retain only the k smallest values
	if(this->sorted == false) this->SortDataSet();

    // 2. Pad D_p with +\infinity until |D_p|=k
	if(k < this->data_set.size())
		this->data_set.erase(this->data_set.begin()+k, this->data_set.end());
	else
		this->data_set.insert(this->data_set.end(), k-this->data_set.size(), kB);

    // 3. Pad D_p with \hat{p} until |D_p|=2^{\log{2}{(k)}}
	k = pow( 2, std::ceil( std::log2( k ) ) );
    size_t padsize = k - this->data_set.size();
	if (p == kB)
	{
		for(size_t i = 0; i < padsize; i ++)
			this->data_set.insert(this->data_set.end(),   kB);
	}
	else
	{
		for(size_t i = 0; i < padsize; i ++)
			this->data_set.insert(this->data_set.begin(), kA);
	}

    // 4. return D_p
   // std::cout << "Exit Pad" << std::endl;
}

void DataSet::Init()
{
    std::srand(time(NULL));
    size_t n = random_range(kRANDOM_LIST_MIN_LENGTH, kRANDOM_LIST_MAX_LENGTH);
    this->GenerateRandomDataSet(n);
    this->SortDataSet();
}

data_t DataSet::GetMedian() const
{
    assert(sorted == true);
    std::vector<data_t>::const_iterator middleiterator = 
        this->data_set.cbegin() + this->data_set.size()/2;
    return *middleiterator;
}


size_t DataSet::GetSizeofDataSet() const
{
    return this->data_set.size();
}

void DataSet::KeepUpperHalf()
{
    std::vector<data_t>::iterator middleiterator = 
        this->data_set.begin() + this->data_set.size()/2;
    this->data_set.erase(middleiterator, this->data_set.end());
}

void DataSet::KeepLowerHalf()
{
    std::vector<data_t>::iterator middleiterator = 
        this->data_set.begin() + this->data_set.size()/2;
    this->data_set.erase(this->data_set.begin(), middleiterator);
}

void DataSet::SortDataSet()
{
    std::sort(this->data_set.begin(), this->data_set.end());
    this->sorted = true;
}

bool DataSet::IsSorted() const
{
    return sorted;
}

data_t DataSet::operator[](size_t index)
{
    return data_set[index];
}

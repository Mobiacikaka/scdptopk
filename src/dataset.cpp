#include <iostream>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <random>
#include <memory>

#include "config.h"
#include "dataset.hpp"

void DataSet::GenerateRandomDataSet(size_t n)
{
	assert(kA < kB);
	this->data_set.resize(n);
	for (size_t i = 0; i < n; i++)
		data_set[i] = random_range(kA, kB);
}

void DataSet::Init()
{
	std::srand(time(NULL));
	size_t n = random_range(kRANDOM_LIST_MIN_LENGTH, kRANDOM_LIST_MAX_LENGTH);
	this->GenerateRandomDataSet(n);
	this->SortDataSet();
}

void DataSet::SortDataSet()
{
	std::sort(this->data_set.begin(), this->data_set.end());
	this->sorted = true;
}

void DataSet::KeepTopK(size_t k)
{
	assert(this->sorted == true);
	auto kit = data_set.end() - k;
	data_set.erase(data_set.begin(), kit);
}
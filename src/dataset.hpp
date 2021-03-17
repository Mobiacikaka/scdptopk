#ifndef __DATASET_HPP__
#define __DATASET_HPP__

#include <vector>

#include "config.h"
#include "node.hpp"

template <typename T>
static inline void PrintElements(std::vector<T> &v)
{
	for (auto it = v.begin(); it < v.end(); it++)
		std::cout << *it << std::endl;
}

class DataSet
{
	friend class Party;

private:
	void GenerateRandomDataSet();

	void ReadDataSet(std::string filename);
	void SortDataSet();

	// std::vector<data_t> data_set;
	std::vector<node> data_set;

	bool sorted;

protected:
	// Get Top-K of dataset
	void KeepTopK(size_t k);

	// Get the size of data set
	size_t GetSizeofDataSet() const { return data_set.size(); }

	bool IsSorted() const { return sorted; }

	// Pack the PutSIMDINGate in the DataSet class
	data_t operator[](size_t index) { return data_set[index].payment; }

	//! Following function only for test
	void PrintAllElement() { PrintElements(data_set); }

	DataSet() {}
	~DataSet() {}

	void Init(std::string filename);
};

#endif

#ifndef __DATASET_HPP__
#define __DATASET_HPP__

#include <vector>
#include <string>
#include <bloom.h>

typedef struct KV_type
{
	std::string ID;
	int count;

	KV_type(std::string ID, int count)
	{
		this->ID = ID;
		this->count = count;
	}

	~KV_type() {}

	bool operator<(struct KV_type& kv2)
	{
		return this->count < kv2.count;
	}

} KV_type;

class Dataset
{
private:
	std::vector<KV_type> content;

public:
	Dataset() {}
	~Dataset() {}

	void ReadDataset();
	void SortDataset();
	struct bloom * BloomPack(int k);
};

#endif
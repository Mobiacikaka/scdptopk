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

	KV_type(const struct KV_type & t)
	{
		this->ID = t.ID;
		this->count = t.count;
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
	void Prune(size_t s);

	struct bloom * BloomPack(size_t k);
	size_t BloomCheck(struct bloom * blm, size_t k);
	size_t size() { return content.size(); }
	KV_type & operator[](size_t i) { return content[i]; }
	void erase(size_t i) { content.erase(content.begin()+i); }
	void print(std::string filename);
};

#endif
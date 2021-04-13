#include "dataset.hpp"

#include <algorithm>
#include <fstream>
using namespace std;

void Dataset::ReadDataset()
{
	ifstream file("dataset.txt");

	string str;
	int cnt;
	while(file >> str)
	{
		file >> cnt;
		KV_type tmp_kv(str, cnt);
		this->content.push_back(tmp_kv);
	}

	file.close();
}

void Dataset::SortDataset()
{
	sort(content.begin(), content.end());
}

/**
 * expand size if needed?
*/
#define K_ENTRIES	1000000
#define K_ERRORS	0.01

struct bloom * Dataset::BloomPack(size_t k)
{
	struct bloom * blm = new struct bloom;

	bloom_init(blm, K_ENTRIES, K_ERRORS);

	for(size_t i = 0; i < k; i ++)
	{
		bloom_add(blm, (const void *)content[i].ID.c_str(), content[i].ID.size());
	}
	
	return blm;
}

size_t Dataset::BloomCheck(struct bloom * blm, size_t k)
{
	size_t count(0);

	for(size_t i = 0; i < k; i ++)
	{
		if(bloom_check(blm, (const void *)content[i].ID.c_str(), content[i].ID.size()))
			count ++;
	}

	return count;
}

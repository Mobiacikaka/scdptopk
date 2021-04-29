#include "dataset.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
using namespace std;

void Dataset::ReadDataset()
{
	ifstream file("dataset.txt");

	if(file.is_open() == false) {
		cerr << "dataset.txt open error!" << endl;
		exit(1);
	} 

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
	sort(content.rbegin(), content.rend());
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

void Dataset::Prune(size_t s)
{
	content.erase(content.begin()+s, content.end());
}

void Dataset::print(std::string filename)
{
	if(filename.empty()) {
		for(size_t i = 0; i < this->content.size(); i ++) {
			cout << content[i].ID << "\t" << content[i].count << endl;
		}
		return;
	}

	ofstream file(filename);
	if(!file.is_open()) {
		cerr << filename << " open error!" << endl;
		exit(1);
	}

	for(size_t i = 0; i < this->content.size(); i ++)
	{
		file << content[i].ID << "\t" << content[i].count << endl;
	}

	file.close();
}

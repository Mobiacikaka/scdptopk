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

#include "node.hpp"

bool operator<(node &n1, node &n2)
{
	return n1.payment < n2.payment;
}

std::istream &operator>>(std::istream &input, node &n)
{
	input >> n.ID;
	input >> n.payment;
	return input;
}

std::ostream &operator<<(std::ostream &output, node &n)
{
	output << n.ID << "\t" << n.payment;
	return output;
}


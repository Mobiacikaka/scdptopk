#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <iostream>
#include <string>

#include "config.h"

typedef struct node
{
	std::string ID;
	data_t payment;

	node() {}
	~node() {}

	data_t operator=(data_t payment)
	{
		this->payment = payment;
		return payment;
	}

	node(const node &x)
	{
		this->ID = x.ID;
		this->payment = x.payment;
	}

} node;

bool operator<(node &n1, node &n2);
std::istream &operator>>(std::istream &input, node &n);
std::ostream &operator<<(std::ostream &output, node &n);

#endif
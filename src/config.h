/**
 * \file
 * \author
 * \copyright
 * \brief
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <cstddef>
#include <limits>
#include <cstdint>

#define concat(a, b) a ## b

// Probability Selection Parameter
const double kEPSILON = 1;
// Top k
const size_t kK = 4;

#define random_range(min, max) \
	(min) + (rand() % static_cast<uint32_t>((max) - (min) + 1))

// Range of the Universe
const size_t kRANDOM_LIST_MAX_LENGTH = 101;
const size_t kRANDOM_LIST_MIN_LENGTH = 80;

typedef uint32_t data_t;

const uint64_t mask = 0b1111111111;
const data_t kA = 0;
const data_t kB = mask;

#define DEBUG_INFO \
	std::cout << "DEBUG INFO " << __LINE__ << std::endl;

template <typename T>
static inline void PrintElements(std::vector<T> &v)
{
	for (auto it = v.begin(); it < v.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;
}

#define free_share_list(sharelist, length)	\
	{										 \
		for (size_t i = 0; i < (length); i++) \
		{									 \
			delete (sharelist)[i];			\
		}									 \
		free((sharelist));					\
	}

#endif

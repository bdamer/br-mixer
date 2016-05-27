#pragma once

#include <stdint.h>
#include <istream>
#include <vector>
#include <cassert>

class TreFile
{
private:
	int32_t num_entries;
	int32_t length; // length of the dat file
	std::vector<std::string> strings;

public:
	TreFile(int32_t length) : length(length) { }
	~TreFile(void) { }

	int32_t count(void) const { return num_entries; }
	std::string get_string(int idx) const { assert(idx < num_entries); return strings[idx]; }

	friend std::istream& operator>>(std::istream& is, TreFile& file);
};

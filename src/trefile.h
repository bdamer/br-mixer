#pragma once

#include <stdint.h>
#include <istream>
#include <vector>

class TreFile
{
private:
	std::size_t numEntries;
	int32_t length; // length of the dat file
	std::vector<std::string> strings;

public:
	TreFile(int32_t length) : length(length) { }
	~TreFile(void) { }

	std::size_t size(void) const { return numEntries; }
	std::string getString(int idx) const { return strings[idx]; }

	friend std::istream& operator>>(std::istream& is, TreFile& file);
};

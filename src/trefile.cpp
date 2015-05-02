#include "trefile.h"

#include <iostream>

TreFile::TreFile(std::ifstream& in, int32_t length) : in(in), length(length)
{
	in.read((char*)&numEntries, sizeof(int32_t));
	std::cout << "TRE file contains " << numEntries << " entries." << std::endl;

	// ordering of offsets? so far these always appear to be 0, 1, 2, ... count
	indexes.resize(numEntries);
	in.read((char*)&indexes[0], numEntries * sizeof(int32_t));

	// offsets starting after <count>
	std::vector<int32_t> offsets;
	offsets.resize(numEntries);
	in.read((char*)&offsets[0], numEntries * sizeof(int32_t));
}

TreFile::~TreFile(void)
{
}

std::string TreFile::loadStringEntry(int i)
{
	std::vector<char> buffer;
	// offsets do not include <count>
	int start = offsets[i] + 4; 
	int stop = (i + 1 < numEntries) ? offsets[i + 1] + 4 : length; 
	buffer.resize(stop - start);
	in.seekg(numEntries + start);
	in.read(&buffer[0], buffer.size());
	std::string str(buffer.begin(), buffer.end());
	return str;
}
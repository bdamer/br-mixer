#include "trefile.h"
#include <iostream>

std::istream& operator>>(std::istream& is, TreFile& file)
{
	int32_t tmp;
	is.read((char*)&tmp, sizeof(int32_t));
	file.numEntries = (size_t)tmp;
	std::cout << "TRE file contains " << file.numEntries << " entries." << std::endl;

	// ordering of offsets? so far these always appear to be 0, 1, 2, ... count
	std::vector<int32_t> indexes(file.numEntries);
	is.read((char*)&indexes[0], file.numEntries * sizeof(int32_t));

	// offsets starting after <count>
	std::vector<int32_t> offsets;
	offsets.resize(file.numEntries);
	is.read((char*)&offsets[0], file.numEntries * sizeof(int32_t));

	for (std::size_t i = 0; i < file.numEntries; i++)
	{
		std::vector<char> buffer;
		// offsets do not include <count>
		int start = offsets[i] + 4;
		int stop = (i + 1 < file.numEntries) ? offsets[i + 1] + 4 : file.length;
		buffer.resize(stop - start);
		is.seekg(file.numEntries + start);
		is.read(&buffer[0], buffer.size());
		file.strings.push_back(std::string(buffer.begin(), buffer.end()));
	}

	return is;
}

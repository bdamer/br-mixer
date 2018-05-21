#include "trefile.h"
#include <iostream>

// TODO: review, not working for CLUES.TRE
std::istream& operator>>(std::istream& is, TreFile& file)
{
	is.read((char*)&file.num_entries, sizeof(int32_t));

	// ordering of offsets? so far these always appear to be 0, 1, 2, ... count
	std::vector<int32_t> indexes(file.num_entries);
	is.read((char*)&indexes[0], file.num_entries * sizeof(int32_t));

	// offsets starting after <count>
	std::vector<int32_t> offsets;
	offsets.resize(file.num_entries);
	is.read((char*)&offsets[0], file.num_entries * sizeof(int32_t));

	for (auto i = 0; i < file.num_entries; i++)
	{
		std::vector<char> buffer;
		// offsets do not include <count>
		int start = offsets[i] + 4;
		int stop = (i + 1 < file.num_entries) ? offsets[i + 1] + 4 : file.length;
		buffer.resize(stop - start);
		is.seekg(file.num_entries + start);
		is.read(&buffer[0], buffer.size());
		file.strings.push_back(std::string(buffer.begin(), buffer.end()));
	}

	return is;
}

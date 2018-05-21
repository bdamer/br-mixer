#include "datfile.h"
#include <iostream>

std::istream& operator>>(std::istream& is, DatFile& file)
{
	is.read((char*)&file.page_size, sizeof(int32_t));
	is.read((char*)&file.page_count, sizeof(int32_t));
	is.read((char*)&file.palette_count, sizeof(int32_t));

	std::cout << "DAT file contains " << file.page_size << " pages." << std::endl;

	std::cout << "DAT file contains " << file.palette_count << " color palettes." << std::endl;
	int skip = file.palette_count * 256 * 3; // each palette contains 256 RGB entries
	is.seekg(skip, std::ios_base::cur);

	is.read((char*)&file.animation_count, sizeof(int32_t));
	std::cout << "DAT file contains " << file.animation_count << " animations." << std::endl;

	skip = file.animation_count * 8 * 4; // each animation consists of 8 * 4 byte attributes

	return is;
}

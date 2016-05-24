#include "datfile.h"
#include <iostream>

std::istream& operator>>(std::istream& is, DatFile& file)
{
	is.read((char*)&file.pageSize, sizeof(int32_t));
	is.read((char*)&file.pageCount, sizeof(int32_t));
	is.read((char*)&file.paletteCount, sizeof(int32_t));

	std::cout << "DAT file contains " << file.pageSize << " pages." << std::endl;

	std::cout << "DAT file contains " << file.paletteCount << " color palettes." << std::endl;
	int skip = file.paletteCount * 256 * 3; // each palette contains 256 RGB entries
	is.seekg(skip, std::ios_base::cur);

	is.read((char*)&file.animationCount, sizeof(int32_t));
	std::cout << "DAT file contains " << file.animationCount << " animations." << std::endl;

	skip = file.animationCount * 8 * 4; // each animation consists of 8 * 4 byte attributes

	return is;
}

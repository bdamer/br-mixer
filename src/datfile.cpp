#include "datfile.h"

#include <iostream>

DatFile::DatFile(std::ifstream& in, int32_t length) : in(in), length(length)
{
	in.read((char*)&pageSize, sizeof(int32_t));
	in.read((char*)&pageCount, sizeof(int32_t));
	in.read((char*)&paletteCount, sizeof(int32_t));

	std::cout << "DAT file contains " << pageSize << " pages." << std::endl;

	std::cout << "DAT file contains " << paletteCount << " color palettes." << std::endl;
	int skip = paletteCount * 256 * 3; // each palette contains 256 RGB entries
	in.seekg(skip, std::ios_base::cur);

	in.read((char*)&animationCount, sizeof(int32_t));
	std::cout << "DAT file contains " << animationCount << " animations." << std::endl;

	skip = animationCount * 8 * 4; // each animation consists of 8 * 4 byte attributes
}

DatFile::~DatFile(void)
{
}

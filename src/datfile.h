#pragma once

#include <stdint.h>
#include <fstream>
#include <vector>

class DatFile
{
private:
	std::ifstream& in;
	int32_t pageSize;
	int32_t pageCount;
	int32_t paletteCount;
	int32_t animationCount;

	int32_t length; // length of the dat file

public:
	DatFile(std::ifstream& in, int32_t length);
	~DatFile(void);

};
#pragma once

#include <stdint.h>
#include <istream>

class DatFile
{
private:
	int32_t pageSize;
	int32_t pageCount;
	int32_t paletteCount;
	int32_t animationCount;
	int32_t length; // length of the dat file

public:
	DatFile(int32_t length) : length(length) { }
	~DatFile(void) { }

	friend std::istream& operator>>(std::istream& is, DatFile& file);
};

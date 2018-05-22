#pragma once

#include <stdint.h>
#include <istream>

class DatFile
{
private:
	int32_t page_size;
	int32_t page_count;
	int32_t palette_count;
	int32_t animation_count;
	int32_t length; // length of the dat file

public:
	DatFile(int32_t length) : length(length) { }
	~DatFile(void) { }

	friend std::istream& operator>>(std::istream& is, DatFile& file);
};

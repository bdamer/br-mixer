#pragma once

#include <istream>
#include <stdint.h>
#include <string>
#include <png++/png.hpp>

#pragma pack(push, 1)
struct ShpHeader
{
	int32_t width;
	int32_t height;
	int32_t length;
};
#pragma pack(pop)

class ShpFile
{
private:
	int32_t num_images;
	std::vector<png::image<png::rgb_pixel>> images;

public:
	ShpFile(void) { }
	~ShpFile(void) { }

	// Saves the image at the specified index from this SHP to a file.
	void save_as_png(int idx, const std::string& filename);
	int32_t count() const { return num_images; }

	friend std::istream& operator>>(std::istream& is, ShpFile& file);
};

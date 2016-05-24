#include "shpfile.h"
#include <iostream>
#include <vector>
#include <png++/png.hpp>

void ShpFile::saveAsPng(int idx, const std::string& filename)
{
	std::cout << "Saving image " << idx << " to " << filename << std::endl;
	images[idx].write(filename);
}

std::istream& operator>>(std::istream& is, ShpFile& file)
{
	// Blade Runner SHP is different from previous Westwood SHP files. 
	// Each file starts with the number of images:
	is.read((char*)&file.numImages, sizeof(int32_t));
	std::cout << "SHP contains " << file.numImages << " images." << std::endl;

	// Each image has a header followed by the raw, high-color pixel data.
	for (int i = 0; i < file.numImages; i++)
	{
		ShpHeader info;
		is.read((char*)&info, sizeof(ShpHeader));

		png::image<png::rgb_pixel> img(info.width, info.height);
		int16_t color;
		for (int y = 0; y < info.height; y++)
		{
			for (int x = 0; x < info.width; x++)
			{
				is.read((char*)&color, sizeof(int16_t));
				int8_t r = color >> 7 & 0xff;
				int8_t g = color >> 2 & 0xff;
				int8_t b = color << 3 & 0xff;
				img.set_pixel(x, y, png::rgb_pixel(r, g, b));
			}
		}

		file.images.push_back(img);
	}

	return is;
}

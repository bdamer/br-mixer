#include "shpfile.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <png++/png.hpp>

ShpFile::ShpFile(std::ifstream& in)
{
	// Blade Runner SHP is different from previous Westwood SHP files. 
	// Each file starts with the number of images:
	in.read((char*)&numImages, sizeof(int32_t));
	std::cout << "SHP contains " << numImages << " images." << std::endl;

	// Each image has a header followed by the raw, high-color pixel data.
	for (int i = 0; i < numImages; i++)
	{
		ShpHeader info;
		in.read((char*)&info, sizeof(ShpHeader));

		png::image<png::rgb_pixel> img(info.width, info.height);
		int16_t color;
		for (int y = 0; y < info.height; y++)
		{
			for (int x = 0; x < info.width; x++)
			{
				in.read((char*)&color, sizeof(int16_t));
				int8_t r = color >> 7 & 0xff;
				int8_t g = color >> 2 & 0xff;
				int8_t b = color << 3 & 0xff;
				img.set_pixel(x, y, png::rgb_pixel(r, g, b));
			}
		}

		images.push_back(img);
	}
}

ShpFile::~ShpFile(void)
{
}

void ShpFile::saveAsPng(int idx, const std::string& filename)
{
	std::cout << "Saving image " << idx << " to " << filename << std::endl;
	images[idx].write(filename);
}


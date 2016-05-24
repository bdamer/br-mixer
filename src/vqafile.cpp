#include "vqafile.h"

#include <iostream>
#include <vector>
#include <png++/png.hpp>
#include <stdexcept>

uint32_t b2l(uint8_t* b)
{
	return ((int)b[0] << 24) + ((int)b[1] << 16) + ((int)b[2] << 8) + b[3];
}

void VqaFile::readHeader(std::istream& is)
{
	uint32_t id;
	uint32_t size;
	is.read((char*)&id, sizeof(uint32_t));
	if (id != FORM)
	{
		throw std::runtime_error("Invalid FORM header.");
	}

	// size of chunk (big-endian)
	is.read((char*)&size, sizeof(uint32_t));

	is.read((char*)&id, sizeof(uint32_t));
	if (id != WVQA)
	{
		throw std::runtime_error("Invalid WVQA header.");
	}

	is.read((char*)&id, sizeof(uint32_t));
	if (id != VQHD)
	{
		throw std::runtime_error("Invalid VQHD header.");
	}

	// size of chunk (big-endian)
	is.read((char*)&size, sizeof(uint32_t));

	is.read((char*)&header, sizeof(VqhdChunk));
}

void VqaFile::readCodebook(std::istream& is)
{
	// screen block data -> array of blockW * blockH
	// read size
	uint8_t tmp[4];
	//uint32_t size;
	is.read((char*)tmp, sizeof(uint32_t));
	//size = b2l(tmp);

	uint8_t format;
	is.read((char*)&format, sizeof(uint8_t));
	if (format == 0x0)
	{
		readFormat80(is);
	}
	else
	{
		throw std::runtime_error("Not implemented.");
	}
}

void VqaFile::readFormat80(std::istream& is)
{
	// target buffer
	std::vector<uint8_t> dst;
	uint8_t cmd, count8, pos8;
	uint16_t count16, pos16;

	// TODO: the following reads too much data... find out why
	while (1)
	{
		is.read((char*)&cmd, sizeof(uint8_t));
		
		// Method 2: 0cccpppp pppppppp
		// Copy <count> bytes from dest at current position - <pos> to current position.
		if (~cmd & 0x80)
		{
			count8 = (cmd >> 4) + 3;
			is.read((char*)&pos8, sizeof(uint8_t));
			pos8 = (((cmd & 0xf) << 8) + pos8);

			// copy <count> from <pos> to <dst>
			dst.resize(dst.size() + count8);
			auto from = dst.end() - pos8 - count8;
			std::copy(from, from + count8, dst.end() - count8);
			continue;
		}

		// get bits 0-5
		count8 = cmd & 0x3f;

		// Method 1: 10cccccc
		// Copy <count> bytes from src to dest.
		if (~cmd & 0x40)
		{
			if (!count8)
				break; // end of image

			// copy <cout> bytes from <in> to <dst>
			auto cur = dst.size();
			dst.resize(cur + count8);
			is.read((char*)&dst[cur], count8 * sizeof(uint8_t)); 
			continue;
		} 

		// Method 3: 11cccccc pppppppp
		// Copy <count> bytes from dest at current position - <pos> to current position
		if (count8 < 0x3e)
		{
			count8 += 3;
			is.read((char*)&pos16, sizeof(uint16_t));
			// copy <count> from <pos> to <dst>
			dst.resize(dst.size() + count8);
			auto from = dst.end() - pos16 - count8;
			std::copy(from, from + count8, dst.end() - count8);
			continue;
		}

		// Method 4: 11111110 ccccxx
		// Fill <count> bytes with color <xx>
		if (count8 == 0x3e)
		{
			is.read((char*)&count16, sizeof(uint16_t));
			uint8_t color;
			is.read((char*)&color, sizeof(uint8_t));
			while (count16-- > 0)
			{
				dst.push_back(color);
			}
			continue;
		}

		// Method 5: 11111111 ccccpppp
		// Copy <count> bytes from dest at cur - <pos> to dest.
		is.read((char*)&count16, sizeof(uint16_t));
		is.read((char*)&pos16, sizeof(uint16_t));
		// copy <count> from <pos> to <dst>
		dst.resize(dst.size() + count16);
		auto from = dst.end() - pos16 - count16;
		std::copy(from, from + count16, dst.end() - count16);
	}
}

std::istream& operator>>(std::istream& is, VqaFile& file)
{
	file.readHeader(is);
	std::cout << "Found VQA with " << file.header.numFrames << " frames of size "
		<< file.header.width << " by " << file.header.height << std::endl;
	return is; // giving up on this format for now...

	while (true)
	{
		uint32_t id;
		uint32_t size;
		uint8_t tmp[4];
		is.read((char*)&id, sizeof(uint32_t));

		std::cout << std::hex << id << std::endl;

		switch (id)
		{
		case VqaFile::LINF:
			// followed by 4 bytes
			is.seekg(4, std::ios_base::cur);
			break;
		case VqaFile::LINH:
			// followed by 10 bytes
			is.seekg(10, std::ios_base::cur);
			break;
		case VqaFile::LIND:
			// followed by 8 bytes
			is.seekg(8, std::ios_base::cur);
			break;
		case VqaFile::CINF:
			// followed by 4 bytes
			is.seekg(4, std::ios_base::cur);
			break;
		case VqaFile::CINH:
			// followed by 12 bytes
			is.seekg(12, std::ios_base::cur);
			break;
		case VqaFile::CIND:
			// followed by 10 bytes
			is.seekg(10, std::ios_base::cur);
			break;
		case VqaFile::FINF: // Frame Information - offset into each frame
			is.seekg(4, std::ios_base::cur); // not clear what this is...
			is.seekg(file.header.numFrames * sizeof(uint32_t), std::ios_base::cur); // offsets need to multipled by 2
			break;
		case VqaFile::SN2J:
			is.read((char*)tmp, sizeof(uint32_t));
			size = b2l(tmp);
			is.seekg(size, std::ios_base::cur);
			break;
		case VqaFile::SND2:
			is.read((char*)tmp, sizeof(uint32_t));
			size = b2l(tmp);
			// TODO: why is there an extra byte here?
			is.seekg(size + 1, std::ios_base::cur);
			break;

			// Vector Quantized Frame
		case VqaFile::VQFR:
			is.read((char*)tmp, sizeof(uint32_t));
			size = b2l(tmp);
			// followed by chunks
			break;

			// Code book, full, compressed 
		case VqaFile::CBFZ:
			file.readCodebook(is);
			break;

		default:
			break;
		}
	}

	return is;
}

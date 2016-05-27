#include "vqafile.h"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>

#include <png++/png.hpp>

uint32_t b2l(uint8_t* b)
{
	return ((int)b[0] << 24) + ((int)b[1] << 16) + ((int)b[2] << 8) + b[3];
}

void VqaFile::process_chunk_header(std::istream& is, ChunkHeader& ch)
{
	is.read(reinterpret_cast<char*>(&ch.id), sizeof(uint32_t));
	uint8_t tmp[4];
	is.read(reinterpret_cast<char*>(tmp), sizeof(uint8_t) * 4);
	ch.size = b2l(tmp);
}

void VqaFile::read_header(std::istream& is)
{
	ChunkHeader ch;
	process_chunk_header(is, ch);
	if (ch.id != FORM)
	{
		throw std::runtime_error("Invalid FORM header.");
	}

	uint32_t type;
	is.read(reinterpret_cast<char*>(&type), sizeof(uint32_t));
	if (type != WVQA)
	{
		throw std::runtime_error("Invalid WVQA header.");
	}

	process_chunk_header(is, ch);
	if (ch.id != VQHD)
	{
		throw std::runtime_error("Invalid VQHD header.");
	}
	is.read((char*)&header, sizeof(VqhdChunk));
}

void VqaFile::read_codebook(std::istream& is)
{
	// screen block data -> array of blockW * blockH

	uint8_t format;
	is.read((char*)&format, sizeof(uint8_t));
	if (format == 0x0)
	{
		read_format_80(is);
	}
	else
	{
		throw std::runtime_error("Not implemented.");
	}
}

void VqaFile::read_format_80(std::istream& is)
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
	file.read_header(is);
	std::cout << "Found VQA with " << file.header.num_frames << " frames of size "
		<< file.header.width << " by " << file.header.height << std::endl;
	return is; // giving up on this format for now...

	ChunkHeader ch;
	uint16_t loop_count;
	uint32_t loop_flags;
	uint16_t clip_count;
	uint32_t skip;
	while (true)
	{
		file.process_chunk_header(is, ch);

		switch (ch.id)
		{
		case VqaFile::LINF:
			std::cout << "LINF (" << ch.size << ")" << std::endl;
			break;
		case VqaFile::LINH:
			std::cout << "LINH (" << ch.size << ")" << std::endl;
			is.read(reinterpret_cast<char*>(&loop_count), sizeof(uint16_t));
			is.read(reinterpret_cast<char*>(&loop_flags), sizeof(uint32_t));
			std::cout << "Loop count: " << loop_count << " flags: " << loop_flags << std::endl;
			break;
		case VqaFile::LIND:
			std::cout << "LIND (" << ch.size << ")" << std::endl;
			for (auto i = 0; i < loop_count; i++)
			{
				uint16_t begin, end;
				is.read(reinterpret_cast<char*>(&begin), sizeof(uint16_t));
				is.read(reinterpret_cast<char*>(&end), sizeof(uint16_t));
				std::cout << "Loop " << i << " from " << begin << " to " << end << std::endl;
			}
			break;
		case VqaFile::CINF:
			std::cout << "CINF (" << ch.size << ")" << std::endl;
			break;
		case VqaFile::CINH:
			std::cout << "CINH (" << ch.size << ")" << std::endl;
			is.read(reinterpret_cast<char*>(&clip_count), sizeof(uint16_t));
			std::cout << "Clip count: " << clip_count << std::endl;
			is.seekg(6, std::ios_base::cur);
			break;
		case VqaFile::CIND:
			std::cout << "CIND (" << ch.size << ")" << std::endl;
			for (auto i = 0; i < clip_count; i++)
			{
				// TODO: document
				uint16_t tmp16;
				uint32_t tmp32;
				is.read(reinterpret_cast<char*>(&tmp16), sizeof(uint16_t));
				is.read(reinterpret_cast<char*>(&tmp32), sizeof(uint32_t));
			}
			break;
		case VqaFile::FINF: // Frame Information - offset into each frame
			std::cout << "FINF (" << ch.size << ")" << std::endl;
			// alternatively, could skip ch.size
			is.seekg(file.header.num_frames * sizeof(uint32_t), std::ios_base::cur); // offsets need to multipled by 2
			break;
		case VqaFile::SN2J:
			std::cout << "SN2J (" << ch.size << ")" << std::endl;
			is.seekg(ch.size, std::ios_base::cur);
			break;
		case VqaFile::SND2:
			std::cout << "SND2 (" << ch.size << ")" << std::endl;
			// skip size rounded up
			skip = (ch.size + 1) & ~1u;
			is.seekg(skip, std::ios_base::cur);
			break;

		// Vector Quantized Frame
		case VqaFile::VQFR:
			std::cout << "VQFR (" << ch.size << ")" << std::endl;
			// followed by chunks
			break;

		// Code book, full, compressed 
		case VqaFile::CBFZ:
			std::cout << "CBFZ (" << ch.size << ")" << std::endl;
			file.read_codebook(is);
			break;

		default:
			std::cout << "Unknown: 0x" << std::hex << ch.id << std::dec << std::endl;
			break;
		}
	}

	return is;
}

#include "mixfile.h"
#include "shpfile.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <array>
#include <stdexcept>

std::map<uint32_t,std::string> MixFile::KNOWN_IDS;

MixFile::MixFile(const std::string& filename)
{
	std::cout << "Loading MIX file: " << filename << std::endl;
	in.open(filename, std::ios::in | std::ios::binary);
	if (!in)
	{
		std::cerr << "Could not open file." << std::endl;
		throw std::runtime_error("Error opening file.");
	}
	load();
}

MixFile::~MixFile(void)
{
	if (in)
	{
		in.close();
	}
}

void MixFile::loadKnownIds(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file)
	{
		std::cerr << "Could not open file." << std::endl;
		throw std::runtime_error("Error opening file.");
	}

	for (std::string line; std::getline(file, line); )
	{
		int idx = line.find('#');
		if (idx == 0)
			continue;
		idx = line.find('=');
		if (idx > -1)
		{
			auto id = std::stoul(line.substr(0, idx), 0, 16);
			auto name = line.substr(idx + 1);
			KNOWN_IDS[id] = name;
		}
	}
}

void MixFile::load(void)
{
	in.read((char*)&header, sizeof(header));
	std::cout << "Found " << header.fileCount << " entries." << std::endl;
	entries.resize(header.fileCount);
	in.read((char*)&entries[0], header.fileCount * sizeof(MixEntry));
	// compute base offset once
	dataOffset = sizeof(header) + header.fileCount * sizeof(MixEntry);
}

int MixFile::moveToFile(int idx)
{
	int offset = dataOffset + entries[idx].offset;
	in.seekg(offset);
	return offset;
}

void MixFile::listFiles(void)
{
	char tmp[4];
	std::cout << "Entry\tID      \tName    \tOffset  \tSize    \tType" << std::endl;
	for (int i = 0; i < header.fileCount; i++)
	{
		// Attempt to detect type
		moveToFile(i);
		in.read(tmp, 4);
		auto t = detectFileType(tmp);

		std::string name = "UNKNOWN";
		if (KNOWN_IDS.count(entries[i].id) > 0)
		{
			name = KNOWN_IDS[entries[i].id];
		}

		std::cout << std::setw(5) << i << "\t" 
			<< std::hex << std::setw(8) << entries[i].id << "\t"
			<< std::setw(8) << name << "\t"
			<< std::dec << std::setw(8) << (dataOffset + entries[i].offset) << "\t"
			<< std::dec << std::setw(8) << entries[i].size << "\t"
			<< std::setw(4) << t << std::endl;
	}
}

void MixFile::loadByName(const std::string& name)
{
	// Computes a simple file hash. The hash is case-insensitive and works for 
	// ASCII characters only.

	// pad size to multiple of 4
	int size = (name.length() + 3) & ~0x03;
	std::vector<unsigned char> buffer(size);
	std::transform(name.begin(), name.end(), buffer.begin(), toupper);

	// process characters in groups of 4 bytes
	uint32_t res = 0;
	for (int i = 0; i < size; i += 4)
	{
		res = _rotl(res, 1); // rotate left by 1
		res += (buffer[i+3] << 24) + (buffer[i+2] << 16) + (buffer[i+1] << 8) + buffer[i]; // accumulate
	}

	loadById(res);
}

void MixFile::loadById(uint32_t id)
{
	// TODO: IDs are stored in increasing order -> perform binary search
	for (auto i = 0; i < (int)entries.size(); i++)
	{
		if (entries[i].id == id)
		{
			loadByIndex(i);
			return;
		}
	}
	std::cerr << "Could not find entry for id: 0x" << std::hex << id << std::dec << std::endl;
}

FileType MixFile::detectFileType(char* data) const
{
	if (data[0] == 'F' && data[1] == 'O' && data[2] == 'R' && data[3] == 'M')
	{
		return FileType::VQA;
	}
	else if (data[0] == 'S' && data[1] == 'e' && data[2] == 't' && data[3] == '0')
	{
		return FileType::SET;
	}
	else if (data[0] == 0x49)
	{
		return FileType::DAT;
	}
	else
	{
		return FileType::UNKNOWN;
	}
}

void MixFile::loadByIndex(int idx)
{
	int offset = moveToFile(idx);

	// Autodetect file type
	char tmp[4];
	in.read(tmp, 4);
	auto t = detectFileType(tmp);

	switch (t)
	{
	case VQA:
		std::cout << "Loading VQA file." << std::endl;
		break;

	case SET:
		{
			std::cout << "Loading Set file." << std::endl;
			SetHeader sh;
			in.read((char*)&sh, sizeof(sh));
			std::vector<SetItem> items;
			items.resize(sh.count);
			in.read((char*)&items[0], sh.count * sizeof(SetItem));		
			// then, read count2
			// count2 * 60 (I think)
			// not clear what comes after that...
		}
		break;

	case DAT:
		{
			std::cout << "Loading Data file." << std::endl;
			DatHeader rh;
			in.read((char*)&rh, sizeof(rh));

			// header is 72 bytes
			int pos = 72;	
			// 4-character 0-terminated strings
			pos += rh.roomCount * 5; 		
			// 8-character strings 0-terminated strings
			pos += (rh.animCount1 + rh.animCount2 + rh.movieCount) * 9;
			in.seekg(offset + pos);
		}
		break;

	case STR: // TRE?
		{
			std::cout << "Loading String Resource file." << std::endl;
			int32_t count = tmp[0] | (tmp[1]<<8) | (tmp[2]<<16) | (tmp[3]<<24);

			// ordering of offsets? so far these always appear to be 0, 1, 2, ... count
			std::vector<int32_t> indexes;
			indexes.resize(count);
			in.read((char*)&indexes[0], count * sizeof(int32_t));

			// offsets starting after <count>
			std::vector<int32_t> offsets;
			offsets.resize(count);
			in.read((char*)&offsets[0], count * sizeof(int32_t));

			std::vector<char> buffer;
			for (int i = 0; i < count; i++)
			{
				// offsets do not include <count>
				int start = offsets[i] + 4; 
				int stop = (i + 1 < count) ? offsets[i + 1] + 4 : entries[idx].size; 
				buffer.resize(stop - start);
				in.seekg(offset + start);
				in.read(&buffer[0], buffer.size());
				std::string str(buffer.begin(), buffer.end());
				std::cout << "String " << i << " = " << str << std::endl;
			}
		}
		break;

	case UNKNOWN:
	default:
		{
			char choice;
			std::cout << "Unknown file format: " << std::hex 
				<< (int)tmp[0] << " "
				<< (int)tmp[1] << " "
				<< (int)tmp[2] << " "
				<< (int)tmp[3] << std::dec << std::endl;
			std::cout << "Press [s] to attempt to load file as SHP, or press any other key to return...";
			std::cin >> choice;

			if (choice == 's')
			{
				in.seekg(offset);
				ShpFile shp(in);
			}
		}		
		break;
	}
}

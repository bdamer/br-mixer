#include "mixfile.h"
#include "shpfile.h"
#include "vqafile.h"
#include "datfile.h"
#include "trefile.h"

#include <iostream>
#include <sstream>
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
	std::cout << "Entry\tID      \tName    \tOffset  \tSize    \tType" << std::endl;
	for (int i = 0; i < header.fileCount; i++)
	{
		// Attempt to detect type
		moveToFile(i);
		auto t = detectFileType();

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

// Computes a simple file hash. The hash is case-insensitive and works for 
// ASCII characters only.
uint32_t MixFile::computeHash(const std::string& s) const
{
	// pad size to multiple of 4
	int size = (s.length() + 3) & ~0x03;
	std::vector<unsigned char> buffer(size);
	std::transform(s.begin(), s.end(), buffer.begin(), toupper);

	// process characters in groups of 4 bytes
	uint32_t res = 0;
	for (int i = 0; i < size; i += 4)
	{
		res = _rotl(res, 1); // rotate left by 1
		res += (buffer[i+3] << 24) + (buffer[i+2] << 16) + (buffer[i+1] << 8) + buffer[i]; // accumulate
	}
	return res;
}

void MixFile::loadByName(const std::string& name)
{
	loadById(computeHash(name));
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

FileType MixFile::detectFileType(void)
{
	uint32_t tmp;
	in.read((char*)&tmp, sizeof(uint32_t));

	if ((tmp & VQA_ID) == VQA_ID)
	{
		return FileType::VQA;
	}	
	else if ((tmp & SET_ID) == SET_ID)		
	{
		return FileType::SET;
	}
	else if ((tmp & GAMEINFO_ID) == GAMEINFO_ID)
	{
		return FileType::GAMEINFO;
	}
	else if ((tmp & DAT_ID) == DAT_ID)
	{
		return FileType::DAT;
	}
	else
	{
//		std::cout << "Unknown file format: " << std::hex << tmp << std::dec << std::endl;
		return FileType::UNKNOWN;
	}
}

void MixFile::loadByIndex(int idx)
{
	int offset = moveToFile(idx);
	// Autodetect file type
	switch (detectFileType())
	{
	case VQA:
		{
			std::cout << "Loading VQA file." << std::endl;
			in.seekg(offset);
			VqaFile vqa(in);
		}
		break;

	case SET:
		{
			std::cout << "Loading Set file." << std::endl;
			SetHeader sh;
			in.read((char*)&sh, sizeof(sh));
			std::vector<SetItem> items;
			items.resize(sh.count);
			in.read((char*)&items[0], sh.count * sizeof(SetItem));
			for (auto i : items)
			{
				std::cout << i.name << std::endl;
			}

			// then, read count2
			// count2 * 60 (I think)
			// not clear what comes after that...
		}
		break;

	case GAMEINFO:
		{
			std::cout << "Loading GAMEINFO file." << std::endl;
			in.seekg(offset);
			GameInfo info;
			in.read((char*)&info, sizeof(info));

			std::ostringstream ss;

			std::cout << std::endl << "Listing sets: " << std::hex << std::endl;
			char name[9];
			for (int i = 0; i < info.setCount; i++)
			{
				ss.str("");
				in.read(name, 5 * sizeof(char));
				ss << name << "-MIN.SET";
				std::cout << computeHash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::endl << "Listing audio files: " << std::endl;
			for (int i = 0; i < info.audCount; i++)
			{
				ss.str("");
				in.read(name, 9 * sizeof(char));
				ss << name << ".AUD";
				std::cout << computeHash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::endl << "Listing music files: " << std::endl;
			for (int i = 0; i < info.musCount; i++)
			{
				ss.str("");
				in.read(name, 9 * sizeof(char));
				ss << name << ".AUD";
				std::cout << computeHash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::endl << "Listing movies: " << std::endl;
			for (int i = 0; i < info.vqaCount; i++)
			{
				ss.str("");
				in.read(name, 9 * sizeof(char));
				ss << name << ".VQA";
				std::cout << computeHash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::dec;
		}
		break;

	case DAT:
		{
			// Doesn't work for GAMEINFO.DAT or INDEX.DAT
			std::cout << "Loading DAT file." << std::endl;
			DatFile dat(in, entries[idx].size);
		}
		break;

	case TRE:
		{
			std::cout << "Loading TRE file." << std::endl;
			in.seekg(offset);
			TreFile dat(in, entries[idx].size);
		}
		break;

	case UNKNOWN:
	default:
		{
			char choice;
			std::cout << "Press [s] to attempt to load file as SHP, or press any other key to return...";
			std::cin >> choice;

			if (choice == 's')
			{
				in.seekg(offset);
				ShpFile shp(in);
				std::cout << "Save to disk? (y/n)";
				std::cin >> choice;
				if (choice == 'y')
				{
					for (int i = 0; i < shp.count(); i++)
					{
						std::ostringstream ss;
						ss << "C:\\" << "sprite-" << idx << "." << i << ".png";
						shp.saveAsPng(i, ss.str());
					}
				}
			}
		}		
		break;
	}
}

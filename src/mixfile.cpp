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
	fs = std::fstream(filename, std::fstream::in | std::fstream::binary);
	if (!fs)
	{
		throw std::runtime_error("Error opening file: " + filename);
	}
	fs >> *this;
}

void MixFile::loadKnownIds(const std::string& filename)
{
	std::cout << "Loading known file IDs from: " << filename << std::endl;
	std::ifstream file(filename);
	if (!file)
	{
		throw std::runtime_error("Error opening file: " + filename);
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

void MixFile::loadFilenames(const std::string & filename)
{
	std::cout << "Loading known filenames from: " << filename << std::endl;
	std::ifstream file(filename);
	if (!file)
	{
		throw std::runtime_error("Error opening file: " + filename);
	}

	for (std::string line; std::getline(file, line); )
	{
		if (line.size() == 0)
			continue;
		auto id = MixFile::computeHash(line);
		KNOWN_IDS[id] = line;
	}
}

int MixFile::moveToFile(const MixEntry& entry)
{
	int offset = dataOffset + entry.offset;
	fs.seekg(offset);
	return offset;
}

void MixFile::listFiles(void)
{
	std::cout << "ID      \tName    \tOffset  \tSize    \tType" << std::endl;
	int i = 0;
	for (auto it : entries)
	{
		// Attempt to detect type
		moveToFile(it.second);
		auto t = detectFileType(it.second);

		std::string name = "UNKNOWN";
		if (KNOWN_IDS.count(it.first) > 0)
		{
			name = KNOWN_IDS[it.first];
		}

		std::cout << std::hex << std::setw(8) << it.second.id << "\t"
			<< std::setw(8) << name << "\t"
			<< std::dec << std::setw(8) << (dataOffset + it.second.offset) << "\t"
			<< std::dec << std::setw(8) << it.second.size << "\t"
			<< std::setw(4) << t << std::endl;
	}
}

// Computes a simple file hash. The hash is case-insensitive and works for 
// ASCII characters only.
uint32_t MixFile::computeHash(const std::string& s)
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
	if (entries.count(id) == 0)
	{
		std::cerr << "Could not find entry for id: 0x" << std::hex << id << std::dec << std::endl;
	}
	else
	{
		loadEntry(entries.at(id));
	}
}

FileType MixFile::detectFileType(const MixEntry& entry)
{
	uint32_t tmp;
	fs.read((char*)&tmp, sizeof(uint32_t));

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
		if (KNOWN_IDS.count(entry.id) > 0)
		{
			auto ext = fileExtension(KNOWN_IDS[entry.id]);
			if (ext == "DAT")
			{
				return FileType::DAT;
			}
			else if (ext == "SET")
			{
				return FileType::SET;
			}
			else if (ext == "SHP")
			{
				return FileType::SHP;
			}
			else if (ext == "TRE")
			{
				return FileType::TRE;
			}
			else if (ext == "VQA")
			{
				return FileType::VQA;
			}
		}
		return FileType::UNKNOWN;
	}
}

void MixFile::loadEntry(const MixEntry& entry)
{
	int offset = moveToFile(entry);
	// Autodetect file type
	switch (detectFileType(entry))
	{
	case VQA:
		{
			std::cout << "Loading VQA file." << std::endl;
			fs.seekg(offset);
			VqaFile vqa;
			fs >> vqa;
		}
		break;

	case SET:
		{
			std::cout << "Loading Set file." << std::endl;
			SetHeader sh;
			fs.read((char*)&sh, sizeof(sh));
			std::vector<SetItem> items;
			items.resize(sh.count);
			fs.read((char*)&items[0], sh.count * sizeof(SetItem));
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
			fs.seekg(offset);
			GameInfo info;
			fs.read((char*)&info, sizeof(info));

			std::ostringstream ss;

			std::cout << std::endl << "Listing sets: " << std::hex << std::endl;
			char name[9];
			for (int i = 0; i < info.setCount; i++)
			{
				ss.str("");
				fs.read(name, 5 * sizeof(char));
				ss << name << "-MIN.SET";
				std::cout << computeHash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::endl << "Listing audio files: " << std::endl;
			for (int i = 0; i < info.audCount; i++)
			{
				ss.str("");
				fs.read(name, 9 * sizeof(char));
				ss << name << ".AUD";
				std::cout << computeHash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::endl << "Listing music files: " << std::endl;
			for (int i = 0; i < info.musCount; i++)
			{
				ss.str("");
				fs.read(name, 9 * sizeof(char));
				ss << name << ".AUD";
				std::cout << computeHash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::endl << "Listing movies: " << std::endl;
			for (int i = 0; i < info.vqaCount; i++)
			{
				ss.str("");
				fs.read(name, 9 * sizeof(char));
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
			DatFile dat(entry.size);
			fs >> dat;
		}
		break;

	case TRE:
		{
			std::cout << "Loading TRE file." << std::endl;
			fs.seekg(offset);
			TreFile tre(entry.size);
			fs >> tre;
			for (std::size_t i = 0; i < tre.size(); i++)
			{
				std::cout << tre.getString(i) << std::endl;
			}
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
				fs.seekg(offset);
				ShpFile shp;
				fs >> shp;
				std::cout << "Save to disk? (y/n)";
				std::cin >> choice;
				if (choice == 'y')
				{
					for (int i = 0; i < shp.count(); i++)
					{
						std::ostringstream ss;
						ss << "C:\\" << "sprite-" << entry.id << "." << i << ".png";
						shp.saveAsPng(i, ss.str());
					}
				}
			}
		}		
		break;
	}
}

std::istream& operator>>(std::istream& is, MixFile& utf)
{
	is.read((char*)&utf.header, sizeof(MixHeader));
	std::cout << "Found " << utf.header.fileCount << " entries." << std::endl;
	MixEntry entry;
	for (auto i = 0; i < utf.header.fileCount; i++)
	{
		is.read((char*)&entry, sizeof(MixEntry));
		utf.entries[entry.id] = entry;
	}
	// compute base offset once
	utf.dataOffset = sizeof(MixHeader) + utf.header.fileCount * sizeof(MixEntry);
	return is;
}

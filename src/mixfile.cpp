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

MixFile::MixFile() {
	_isTLK = false;
}

void MixFile::load(const std::string& filename)
{
	fs.open(filename, std::fstream::in | std::fstream::binary);

	if (filename.find(".tlk") != std::string::npos || filename.find(".TLK") != std::string::npos)
		_isTLK = true;
	else
		_isTLK = false;

	std::cout << "Loading MIX file: " << filename << (_isTLK ? " which is TLK" : "") << std::endl;
	if (!fs)
	{
		throw std::runtime_error("Error opening file: " + filename);
	}
	fs >> *this;
}

bool MixFile::load_known_ids(const std::string& filename)
{
	std::cout << "Loading known file IDs from: " << filename << std::endl;
	std::ifstream file(filename);
	if (!file)
	{
		return false;
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

	return true;
}

bool MixFile::load_filenames(const std::string & filename)
{
	std::cout << "Loading known filenames from: " << filename << std::endl;
	std::ifstream file(filename);
	if (!file)
	{
		return false;
	}

	for (std::string line; std::getline(file, line); )
	{
		if (line.size() == 0)
			continue;
		auto id = compute_hash(line);
		KNOWN_IDS[id] = line;
	}
	return true;
}

int MixFile::move_to_file(const MixEntry& entry)
{
	int offset = data_offset + entry.offset;
	fs.seekg(offset);
	return offset;
}

void MixFile::list_files(void)
{
	std::cout << "ID      \tName    \tOffset  \tSize    \tType" << std::endl;
	for (auto it : entries)
	{
		// Attempt to detect type
		move_to_file(it.second);
		auto t = detect_file_types(it.second);

		std::string name = "UNKNOWN";
		if (KNOWN_IDS.count(it.first) > 0)
		{
			name = KNOWN_IDS[it.first];
		}

		std::cout << std::hex << std::setw(8) << it.second.id << "\t"
			<< std::setw(8) << name << "\t"
			<< std::dec << std::setw(8) << (data_offset + it.second.offset) << "\t"
			<< std::dec << std::setw(8) << it.second.size << "\t"
			<< std::setw(4) << t << std::endl;
	}
}

void MixFile::extract_all_files(void)
{
	for (auto it : entries)
	{
		extract_entry(it.second);
	}
}

// Computes a simple file hash. The hash is case-insensitive and works for
// ASCII characters only.
uint32_t MixFile::compute_hash(const std::string& s)
{
	if (_isTLK) {
		const char *buffer = s.c_str();

		int actor_id  =   10 * (buffer[0] - '0') +
					   (buffer[1] - '0');

		int speech_id = 1000 * (buffer[3] - '0') +
						 100 * (buffer[4] - '0') +
						  10 * (buffer[5] - '0') +
							   (buffer[6] - '0');

		return 10000 * actor_id + speech_id;

	}
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

void MixFile::load_file(const std::string& name)
{
	load_file(compute_hash(name));
}

void MixFile::load_file(uint32_t id)
{
	if (entries.count(id) == 0)
	{
		std::cerr << "Could not find entry for id: 0x" << std::hex << id << std::dec << std::endl;
	}
	else
	{
		load_entry(entries.at(id));
	}
}

FileType MixFile::detect_file_types(const MixEntry& entry)
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
			auto ext = get_file_extension(KNOWN_IDS[entry.id]);
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

		if (_isTLK)
			return FileType::AUD;

		return FileType::UNKNOWN;
	}
}

void MixFile::load_entry(const MixEntry& entry)
{
	int offset = move_to_file(entry);
	// Autodetect file type
	switch (detect_file_types(entry))
	{
	case VQA:
		{
			std::cout << "Loading VQA file..." << std::endl;
			fs.seekg(offset);
			VqaFile vqa;
			fs >> vqa;
		}
		break;

	case SET:
		{
			std::cout << "Loading SET file..." << std::endl;
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
			std::cout << "Loading GAMEINFO file..." << std::endl;
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
				std::cout << compute_hash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::endl << "Listing audio files: " << std::endl;
			for (int i = 0; i < info.audCount; i++)
			{
				ss.str("");
				fs.read(name, 9 * sizeof(char));
				ss << name << ".AUD";
				std::cout << compute_hash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::endl << "Listing music files: " << std::endl;
			for (int i = 0; i < info.musCount; i++)
			{
				ss.str("");
				fs.read(name, 9 * sizeof(char));
				ss << name << ".AUD";
				std::cout << compute_hash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::endl << "Listing movies: " << std::endl;
			for (int i = 0; i < info.vqaCount; i++)
			{
				ss.str("");
				fs.read(name, 9 * sizeof(char));
				ss << name << ".VQA";
				std::cout << compute_hash(ss.str()) << "=" << ss.str() << std::endl;
			}

			std::cout << std::dec;
		}
		break;

	case DAT:
		{
			// Doesn't work for GAMEINFO.DAT or INDEX.DAT
			std::cout << "Loading DAT file..." << std::endl;
			DatFile dat(entry.size);
			fs >> dat;
		}
		break;

	case TRE:
		{
			std::cout << "Loading TRE file..." << std::endl;
			fs.seekg(offset);
			TreFile tre(entry.size);
			fs >> tre;
			std::cout << "Found " << tre.count() << " entries." << std::endl;
			for (auto i = 0; i < tre.count(); i++)
			{
				std::cout << tre.get_string(i) << std::endl;
			}
		}
		break;

	case SHP:
		{
			std::cout << "Loading SHP file..." << std::endl;
			fs.seekg(offset);
			ShpFile shp;
			fs >> shp;
			std::cout << "Found " << shp.count() << " images." << std::endl;

			char c;
			do
			{
				std::cout << "Save images to disk? (y/n) ";
				std::cin >> c;
			}
			while (c != 'Y' && c != 'y' && c != 'N' && c != 'n');
			if (c == 'Y' || c == 'y')
			{
				std::string basename;
				if (KNOWN_IDS.count(entry.id) > 0)
				{
					basename = KNOWN_IDS.at(entry.id);
				}
				else
				{
					basename = std::to_string(entry.id);
				}

				for (int i = 0; i < shp.count(); i++)
				{
					std::ostringstream ss;
					ss << "../data/" << basename << "-" << i << ".png";
					shp.save_as_png(i, ss.str());
				}
			}
		}
		break;

	case UNKNOWN:
	default:
		{
			char c;
			do
			{
				std::cout << "Unable to determine file type. Would you like to extract the file to disk? (y/n) ";
				std::cin >> c;
			}
			while (c != 'Y' && c != 'y' && c != 'N' && c != 'n');
			if (c == 'Y' || c == 'y')
			{
				extract_file(entry.id);
			}
		}
		break;
	}
}

void MixFile::extract_file(const std::string& name)
{
	auto id = compute_hash(name);
	extract_file(id);
}

void MixFile::extract_file(uint32_t id)
{
	if (entries.count(id) == 0)
	{
		std::cerr << "Could not find entry for id: 0x" << std::hex << id << std::dec << std::endl;
	}
	else
	{
		extract_entry(entries.at(id));
	}
}

void MixFile::extract_entry(const MixEntry& entry)
{
	std::string target = "../data/";
	if (KNOWN_IDS.count(entry.id) > 0)
	{
		target += KNOWN_IDS.at(entry.id);
	}
	else
	{
		target += std::to_string(entry.id);
	}

	std::cout << "Extracting file to: " << target << std::endl;
	std::fstream os(target, std::fstream::out | std::fstream::binary);
	if (!os)
	{
		throw std::runtime_error("Could not open file: " + target);
	}

	std::vector<char> buffer(entry.size);
	move_to_file(entry);
	fs.read(buffer.data(), entry.size);
	os.write(buffer.data(), entry.size);
}

std::istream& operator>>(std::istream& is, MixFile& utf)
{
	is.read((char*)&utf.header, sizeof(MixHeader));
	std::cout << "Found " << utf.header.file_count << " entries." << std::endl;
	MixEntry entry;
	for (auto i = 0; i < utf.header.file_count; i++)
	{
		is.read((char*)&entry, sizeof(MixEntry));
		utf.entries[entry.id] = entry;

		if (utf.isTLK()) {
			int actor_id, speech_id;

			actor_id = entry.id / 10000;
			speech_id = entry.id - actor_id * 10000;

			char buf[50];
			snprintf(buf, 50, "%02d-%04d.AUD", actor_id, speech_id);
			MixFile::KNOWN_IDS[entry.id] = buf;
		}
	}
	// compute base offset once
	utf.data_offset = sizeof(MixHeader) + utf.header.file_count * sizeof(MixEntry);
	return is;
}

#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <stdint.h>
#include <map>

#ifndef _MSC_VER
#define _rotl(x,r) ((x << r) | (x >> (32 - r)))
#endif

#pragma pack(push, 1)
struct MixHeader
{
	int16_t file_count;		// number of file entries
	int32_t size;			// byte length of data segment
};

struct MixEntry
{
	uint32_t id;			// id file name hash
	int32_t offset;			// byte offset in data segment
	int32_t size;			// byte length of data entry
};

struct GameInfo
{
	int32_t offset;			// Start of data + 1
	int32_t zero;			// 00 00 00 00		=	0
	int32_t unknown1;		// 20 03 00 00		=	800
	int32_t unknown2;		// 20 01 00 00		=	288
	int32_t unknown3;		// 64 00 00 00		=	100

	int32_t setCount;		// number of sets

	int32_t unknown4;		// 4E 00 00 00		=	78
	int32_t unknown5;		// 68 00 00 00		=	104
	int32_t unknown6;		// 45 00 00 00		=	69
	int32_t unknown7;		// C8 00 00 00		=	200
	int32_t unknown8;		// 58 02 00 00		=	600

	int32_t audCount;		// number of audio files
	int32_t musCount;		// number of music files
	int32_t vqaCount;		// number of movies

	int32_t unknown12;		// 09 00 00 00		=	9
	int32_t unknown13;		// 0C 00 00 00		=	12
	int32_t unknown14;		// 25 00 00 00		=	37
	int32_t unknown15;		// 4D 00 00 00		=	77
};

struct UnknownHeader1
{
	int32_t count1;			// count of 20-byte segments following the header
};

struct SetHeader
{
	int32_t sixty;			// 0x3c = 60
	int32_t count;			// not sure yet
};

struct SetItem
{
	char name[20];			// item name
	char unknown[30];		// flags?
};
#pragma pack(pop)

enum FileType
{
	UNKNOWN,
	VQA,
	SHP,
	SET,
	DAT,
	GAMEINFO,
	TRE,
	TLK
};

class MixFile
{
private:
	static std::map<uint32_t,std::string> KNOWN_IDS;
	static const uint32_t VQA_ID = 0x4d524f46;
	static const uint32_t SET_ID = 0x30746553;
	static const uint32_t GAMEINFO_ID = 0x00000049;
	static const uint32_t DAT_ID = 0x3457b6f6; // timestamp 10/29/1997 @ 10:21pm (UTC)

	std::fstream fs;
	MixHeader header;
	std::map<uint32_t, MixEntry> entries;
	// Offset of entries in MIX file
	int data_offset;

	bool _isTLK;

	// Loads a MixEntry.
	void load_entry(const MixEntry& entry);
	// Extracts a MixEntry.
	void extract_entry(const MixEntry& entry);
	// Positions stream pointer at start of file and returns the offset.
	int move_to_file(const MixEntry& entry);
	// Attempts to detect file type based by reading data from file.
	FileType detect_file_types(const MixEntry& entry);
	// Computes file id hash.
	static uint32_t compute_hash(const std::string& s);

public:
	// Loads a list of id => filename from a file.
	static bool load_known_ids(const std::string& filename);
	// Loads and indexes a list of filenames from a file.
	static bool load_filenames(const std::string& filename);

	MixFile(const std::string& filename);
	~MixFile(void) { }

	void list_files(void);
	void load_file(const std::string& name);
	void load_file(uint32_t id);
	void extract_file(const std::string& name);
	void extract_file(uint32_t id);

	bool isTLK() { return _isTLK; }

	// Loads header from mix file.
	friend std::istream& operator>>(std::istream& is, MixFile& utf);
};

inline std::string get_file_extension(const std::string& filename)
{
	return filename.substr(filename.rfind('.') + 1);
}

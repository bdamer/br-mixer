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
	int16_t fileCount;		// number of file entries
	int32_t size;			// byte length of data segment
};

struct MixEntry
{
	uint32_t id;	// id file name hash
	int32_t offset;			// byte offset in data segment
	int32_t size;			// byte length of data entry
};

// Vector Quantized Animation
struct VqaFile
{
	char form[4];			// F O R M
	int32_t unknown1;
	char wvqa[4];			// W V Q A signature
};

struct VqaHeader
{
	char vqhd[4];			// V Q H D chunk
	int16_t version;
	int16_t flags;
	int16_t numFrames;
	int16_t width;
	int16_t height;
	// ...
};

struct DatHeader 
{
	//49 00 00 00		=	73
	int32_t zero;			// 00 00 00 00		=	0
	int32_t unknown1;		// 20 03 00 00		=	800
	int32_t unknown2;		// 20 01 00 00		=	288
	int32_t unknown3;		// 64 00 00 00		=	100
	int32_t roomCount;		
	int32_t unknown4;		// 4E 00 00 00		=	78
	int32_t unknown5;		// 68 00 00 00		=	104
	int32_t unknown6;		// 45 00 00 00		=	69
	int32_t unknown7;		// C8 00 00 00		=	200
	int32_t unknown8;		// 58 02 00 00		=	600
	// Not clear yet what each of these represent - but the total
	// makes up for the string entries in this file
	int32_t animCount1;		
	int32_t animCount2;		
	int32_t movieCount;		// number of movie titles

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
	SET,
	DAT,
	STR,
	SHP
};

class MixFile
{
private:
	static std::map<uint32_t,std::string> KNOWN_IDS;	

	std::ifstream in;
	MixHeader header;
	std::vector<MixEntry> entries;
	// Offset of entries in MIX file
	int dataOffset;

	// Loads header from mix file.
	void load();
	// Positions stream pointer at start of file and returns the offset.
	int moveToFile(int idx);
	// Attempts to detect file type based by reading data from file.
	FileType detectFileType(char* data) const;

public:
	static void loadKnownIds(const std::string& filename);

	MixFile(const std::string& filename);
	~MixFile(void);

	void listFiles(void);
	void loadByName(const std::string& name);
	void loadByIndex(int idx);
	void loadById(uint32_t id);
};
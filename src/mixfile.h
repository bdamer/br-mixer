#pragma once

#include <string>
#include <fstream>
#include <vector>

#pragma pack(push, 1)
struct MixHeader
{
	__int16 fileCount;		// number of file entries
	__int32 size;			// byte length of data segment
};

struct MixEntry
{
	unsigned __int32 id;	// id file name hash
	__int32 offset;			// byte offset in data segment
	__int32 size;			// byte length of data entry
};

// Vector Quantized Animation
struct VqaFile
{
	char form[4];			// F O R M
	__int32 unknown1;
	char wvqa[4];			// W V Q A signature
};

struct VqaHeader
{
	char vqhd[4];			// V Q H D chunk
	__int16 version;
	__int16 flags;
	__int16 numFrames;
	__int16 width;
	__int16 height;
	// ...
};

struct ShpHeader
{
	__int16 imageCount;		// number of images
	__int16 unknown1;
	__int16 unknown2;
	__int16 width;			// width of images
	__int16 height;			// height of images
	__int32 unknown3;		
};

struct ShpInfo
{
	__int8 offset[3];		// offset in file
	__int8 format;			// image format (0x80, 0x40, etc)
	__int16 refOffset;		// offset of reference image for format 20 / 40
	__int16 refFormat;		// format of the reference image
};

struct DatHeader 
{
	//49 00 00 00		=	73
	__int32 zero;			// 00 00 00 00		=	0
	__int32 unknown1;		// 20 03 00 00		=	800
	__int32 unknown2;		// 20 01 00 00		=	288
	__int32 unknown3;		// 64 00 00 00		=	100
	__int32 roomCount;		
	__int32 unknown4;		// 4E 00 00 00		=	78
	__int32 unknown5;		// 68 00 00 00		=	104
	__int32 unknown6;		// 45 00 00 00		=	69
	__int32 unknown7;		// C8 00 00 00		=	200
	__int32 unknown8;		// 58 02 00 00		=	600
	// Not clear yet what each of these represent - but the total
	// makes up for the string entries in this file
	__int32 animCount1;		
	__int32 animCount2;		
	__int32 movieCount;		// number of movie titles

	__int32 unknown12;		// 09 00 00 00		=	9
	__int32 unknown13;		// 0C 00 00 00		=	12
	__int32 unknown14;		// 25 00 00 00		=	37
	__int32 unknown15;		// 4D 00 00 00		=	77
};

struct UnknownHeader1
{
	__int32 count1;			// count of 20-byte segments following the header
};

struct SetHeader
{
	__int32 sixty;			// 0x3c = 60
	__int32 count;			// not sure yet
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
	STR
};

class MixFile
{
private:
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
	MixFile(const std::string& filename);
	~MixFile(void);

	void listFiles(void);
	void loadByName(const std::string& name);
	void loadByIndex(int idx);
	void loadById(unsigned int id);
};
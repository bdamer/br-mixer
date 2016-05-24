#pragma once

#include <istream>
#include <stdint.h>

#pragma pack(push, 1)
struct VqhdChunk
{
	uint16_t version;			// VQA format version
	uint16_t flags;				// Flags
	uint16_t numFrames;			// Number of frames in this movie
	uint16_t width;				// Width in pixels
	uint16_t height;			// Height in pixels
	uint8_t blockW;				// Width of each image block in pixels
	uint8_t blockH;				// Height of each image block in pixels
	uint8_t frameRate;			// Frame rate of VQA
	
	// ...
	uint8_t   CBParts;       /* How many images use the same lookup table  */
	uint16_t  Colors;        /* Maximum number of colors used in VQA       */
	uint16_t  MaxBlocks;     /* Maximum number of image blocks             */
	int32_t   Unknown1;      /* Always 0 ???                               */
	uint16_t  Unknown2;      /* Some kind of size ???                      */
	uint16_t  Freq;          /* Sound sampling frequency                   */
	uint8_t   Channels;      /* Number of sound channels                   */
	uint8_t   Bits;          /* Sound resolution                           */
	int32_t   Unknown3;      /* Always 0 ???                               */
	uint16_t  Unknown4;      /* 0 in old VQAs, 4 in HiColor ones ???       */
	int32_t   MaxCBFZSize;   /* 0 in old VQAs, max. CBFZ size in HiColor   */
	int32_t   Unknown5;      /* Always 0 ???                               */
};
#pragma pack(pop)

// Vector Quantized Animation
class VqaFile
{
private:
	static const uint32_t CBFZ = 0x5a464243;
	static const uint32_t CINF = 0x464e4943;
	static const uint32_t CINH = 0x484e4943;
	static const uint32_t CIND = 0x444e4943;
	static const uint32_t FINF = 0x464e4946;
	static const uint32_t FORM = 0x4d524f46;
	static const uint32_t LINF = 0x464e494c;
	static const uint32_t LINH = 0x484e494c;
	static const uint32_t LIND = 0x444e494c;
	static const uint32_t SN2J = 0x4a324e53;
	static const uint32_t SND2 = 0x32444e53;
	static const uint32_t VQFR = 0x52465156;	
	static const uint32_t VQHD = 0x44485156;
	static const uint32_t WVQA = 0x41515657;

	VqhdChunk header;

	void readHeader(std::istream& is);
	bool readNextBodyChunk(std::istream& is);
	void readCodebook(std::istream& is);
	void readFormat80(std::istream& is);

public:
	VqaFile(void) { };
	~VqaFile(void) { };

	friend std::istream& operator >> (std::istream& is, VqaFile& file);
};

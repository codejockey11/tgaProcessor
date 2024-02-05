/*
www.gamers.org/dEngine/quake3/TGA.txt
unix4lyfe.org/targa/
www.fileformat.info/format/tga/egff.htm#TGA-DMYID.2

The original TGA format (v1.0) is structured as follows:
Header containing information on the image data and palette
Optional image identification field
Optional color map
Bitmap data

The new TGA format (v2.0) contains all of the structures included in the original TGA format and also appends several data structures onto the end of the original TGA format. The following structures may follow the bitmap data:
Optional developer directory, which may contain a variable number of tags pointing to pieces of information stored in the TGA file
Optional developer area
Optional extension area, which contains information typically found in the header of a bitmap file
Optional color-correction table
Optional postage-stamp image
Optional scan-line table
Footer, which points to the developer and extension areas and identifies the TGA file as a new TGA format file

ImageType	Image Data Type					Colormap	Encoding
0			No image data included in file	No			No
1			Colormapped image data			Yes			No
2			Truecolor image data			No			No
3			Monochrome image data			No			No
9			Colormapped image data			Yes			Yes
10			Truecolor image data			No			Yes
11			Monochrome image data			No			Yes

Palette Entry Sizes
Truevision Bits Per Attribute		Bits		Color Formats
Display Adapter		Colormap Entry	Per Pixel	Supported
Targa M8			24				0			Pseudo
Targa 16			15				1			True
Targa 24			24				0			True
Targa 32			32				8			True
ICB					0				0			True
VDA					16				0			Pseudo
VDA/D				16				0			Pseudo
Vista				24 or 32		0 or 8		True, Pseudo, Direct
*/

#pragma once

#define TGAIMAGE_API __declspec(dllexport)

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

class TgaHeader
{
public:

	BYTE	idLength;        /* 00h  Size of Image ID field */
	BYTE	colorMapType;    /* 01h  Color map type */
	BYTE	imageType;       /* 02h  Image type code */
	WORD	cMapStart;       /* 03h  Color map origin */
	WORD	cMapLength;      /* 05h  Color map length */
	BYTE	cMapDepth;       /* 07h  Depth of color map entries */
	WORD	xOffset;         /* 08h  X origin of image */
	WORD	yOffset;         /* 0Ah  Y origin of image */
	WORD	width;           /* 0Ch  Width of image */
	WORD	height;          /* 0Eh  Height of image */
	BYTE	pixelDepth;      /* 10h  Image pixel size */

	// bit 0-3 contain the number of attribute bits per pixel; bits are found only for 16 and 32 bit
	// bit 4 left or right
	// bit 5 top or bottom
	// bit 6-7 are unused
	BYTE	imageDescriptor;

	TgaHeader()
	{
		memset(this, 0x00, sizeof(TgaHeader));
	}
};

class TgaFooter
{
public:

	DWORD	extensionOffset;	/* Extension Area Offset */
	DWORD	developerOffset;	/* Developer Directory Offset */

	CHAR	signature[18];		/* TGA Signature */

	TgaFooter()
	{
		memset(this, 0x00, sizeof(TgaFooter));
	}

	void ReadValues(FILE* f)
	{
		fread_s(&extensionOffset, sizeof(DWORD), sizeof(DWORD), 1, f);
		fread_s(&developerOffset, sizeof(DWORD), sizeof(DWORD), 1, f);
		fread_s(&signature, sizeof(CHAR) * 18, sizeof(CHAR), 18, f);
	}
};

class TgaTag
{
public:

	WORD	tagNumber;	/* ID Number of the tag */
	DWORD	offset;		/* Offset location of the tag data */
	DWORD	size;		/* Size of the tag data in bytes */

	TgaTag()
	{
		memset(this, 0x00, sizeof(TgaTag));
	}
};

class TgaExtension
{
public:

	WORD  Size;                   /* Extension Size */
	CHAR  AuthorName[41];         /* Author Name */
	CHAR  AuthorComment[324];     /* Author Comment */
	WORD  StampMonth;             /* Date/Time Stamp: Month */
	WORD  StampDay;               /* Date/Time Stamp: Day */
	WORD  StampYear;              /* Date/Time Stamp: Year */
	WORD  StampHour;              /* Date/Time Stamp: Hour */
	WORD  StampMinute;            /* Date/Time Stamp: Minute */
	WORD  StampSecond;            /* Date/Time Stamp: Second */
	CHAR  JobName[41];            /* Job Name/ID */
	WORD  JobHour;                /* Job Time: Hours */
	WORD  JobMinute;              /* Job Time: Minutes */
	WORD  JobSecond;              /* Job Time: Seconds */
	CHAR  SoftwareId[41];         /* Software ID */
	WORD  VersionNumber;          /* Software Version Number */
	BYTE  VersionLetter;          /* Software Version Letter */
	DWORD KeyColor;               /* Key Color */
	WORD  PixelNumerator;         /* Pixel Aspect Ratio */
	WORD  PixelDenominator;       /* Pixel Aspect Ratio */
	WORD  GammaNumerator;         /* Gamma Value */
	WORD  GammaDenominator;       /* Gamma Value */
	DWORD ColorOffset;            /* Color Correction Offset */
	DWORD StampOffset;            /* Postage Stamp Offset */
	DWORD ScanOffset;             /* Scan-Line Table Offset */
	BYTE  AttributesType;         /* Attributes Types */

	TgaExtension()
	{
		memset(this, 0x00, sizeof(TgaExtension));
	}
};

class TGAColorCorrectionTable
{
public:

	SHORT Alpha;

	SHORT Red;
	SHORT Green;
	SHORT Blue;

	TGAColorCorrectionTable()
	{
		memset(this, 0x00, sizeof(TGAColorCorrectionTable));
	}
};

class BGRA
{
public:

	BYTE Blue;
	BYTE Green;
	BYTE Red;

	BYTE Alpha;

	friend bool operator == (const BGRA &p1, const BGRA &p2)
	{
		return ((p1.Blue == p2.Blue) && (p1.Green == p2.Green) && (p1.Red == p2.Red) && (p1.Alpha == p2.Alpha));
	}

	friend bool operator != (const BGRA &p1, const BGRA &p2)
	{
		return !(p1 == p2);
	}

	BGRA()
	{
		memset(this, 0x00, sizeof(BGRA));
	}
};

class ARGB
{
public:

	BYTE Alpha;

	BYTE Red;
	BYTE Green;
	BYTE Blue;

	friend bool operator == (const ARGB &p1, const ARGB &p2)
	{
		return ((p1.Alpha == p2.Alpha) && (p1.Red == p2.Red) && (p1.Green == p2.Green) && (p1.Blue == p2.Blue));
	}

	friend bool operator != (const ARGB &p1, const ARGB &p2)
	{
		return !(p1 == p2);
	}

	ARGB()
	{
		memset(this, 0x00, sizeof(ARGB));
	}
};

class BGR
{
public:

	BYTE Blue;
	BYTE Green;
	BYTE Red;

	friend bool operator == (const BGR &p1, const BGR &p2)
	{
		return ((p1.Blue == p2.Blue) && (p1.Green == p2.Green) && (p1.Red == p2.Red));
	}

	friend bool operator != (const BGR &p1, const BGR &p2)
	{
		return !(p1 == p2);
	}

	BGR()
	{
		memset(this, 0x00, sizeof(BGR));
	}
};

#undef RGB

class RGB
{
public:

	BYTE Red;
	BYTE Green;
	BYTE Blue;

	friend bool operator == (const RGB &p1, const RGB &p2)
	{
		return ((p1.Red == p2.Red) && (p1.Green == p2.Green) && (p1.Blue == p2.Blue));
	}

	friend bool operator != (const RGB &p1, const RGB &p2)
	{
		return !(p1 == p2);
	}

	RGB()
	{
		memset(this, 0x00, sizeof(RGB));
	}
};

/*
compression packet

byte 1
bit 7			1 for compressed byte(s)
bit 6 - 0 		count 0 - 127 and add 1

8 bit
byte 2			value 0 - 255

32 bit
byte 2,3,4,5	value 0 - 255
*/
class RLEPacket
{
public:

	BYTE count;

	RLEPacket()
	{
		memset(this, 0x00, sizeof(RLEPacket));
	}

	void ReadCount(FILE* f)
	{
		fread_s(&count, sizeof(BYTE), sizeof(BYTE), 1, f);
	}

	bool IsEncoded()
	{
		if (count & 128)
		{
			return true;
		}

		return false;
	}

	BYTE GetCount()
	{
		// remove the RLE bit
		BYTE c = count << 1;

		c = c >> 1;

		// add 1 for RLE packets
		c += 1;

		return c;
	}
};

class RLEPacket8 : public RLEPacket
{
public:

	BYTE value;

	RLEPacket8()
	{
		memset(this, 0x00, sizeof(RLEPacket8));
	}

	void ReadValue(FILE* f)
	{
		fread_s(&value, sizeof(BYTE), sizeof(BYTE), 1, f);
	}
};

class RLEPacket24 : public RLEPacket
{
public:

	BGR value;

	RLEPacket24()
	{
		memset(this, 0x00, sizeof(RLEPacket24));
	}

	void ReadValue(FILE* f)
	{
		fread_s(&value, sizeof(BGR), sizeof(BGR), 1, f);
	}
};

class RLEPacket32 : public RLEPacket
{
public:

	BGRA value;

	RLEPacket32()
	{
		memset(this, 0x00, sizeof(RLEPacket32));
	}

	void ReadValue(FILE* f)
	{
		fread_s(&value, sizeof(BGRA), sizeof(BGRA), 1, f);
	}
};

class TgaImage
{
public:

	TgaHeader*		header;
	TgaFooter*		footer;

	TgaExtension*	extension;

	BGR				colorTable[256];

	BYTE			imageDescription[256];

	int				size8;
	int				size24;
	int				size32;

	BYTE*			pixels8;
	BYTE*			pixels24;
	BYTE*			pixels32;

	TGAIMAGE_API TgaImage();
	TGAIMAGE_API ~TgaImage();

	TGAIMAGE_API TgaImage(char* f);

	bool TGAIMAGE_API Load();

	void TGAIMAGE_API DumpHeader();

	void TGAIMAGE_API DumpFooter();

	void TGAIMAGE_API DumpColorMap();

	void TGAIMAGE_API DumpRawPixels8();
	void TGAIMAGE_API DumpRawPixels24();
	void TGAIMAGE_API DumpRawPixels32();

private:

	FILE*		file;

	size_t		err;

	bool		isInitialized;

	char		buffer[1024];

	static const int	MAX_BUFFER_LEN = 1024;

	void LoadHeader();

	void LoadImageDescription();

	void LoadColorTable();

	void LoadPixelData();

	void Load8BitUncompressed();
	void Load24BitUncompressed();
	void Load32BitUncompressed();

	void Load8BitCompressed();
	void Load24BitCompressed();
	void Load32BitCompressed();

	void LoadExtensions();
};
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#include "../myLibrary/TgaImage.h"

char		filename[32];

TgaImage*	image;

int main()
{
	memset(filename, 0x00, 32);
	
	strcat_s(filename, 32, "8bitc.tga");
	//strcat_s(filename, 32, "8bitu.tga");

	//strcat_s(filename, 32, "24bitc.tga");
	//strcat_s(filename, 32, "24bitu.tga");
	
	//strcat_s(filename, 32, "32bitc.tga");
	//strcat_s(filename, 32, "32bitu.tga");

	image = new TgaImage(filename);

	image->Load();

	image->DumpHeader();

	image->DumpFooter();

	image->DumpColorMap();

	image->DumpRawPixels8();

	image->DumpRawPixels24();

	image->DumpRawPixels32();

	FILE* f;

	fopen_s(&f, "test.tga", "wb");

	fwrite(&image->header->idLength, sizeof(BYTE), 1, f);
	fwrite(&image->header->colorMapType, sizeof(BYTE), 1, f);

	BYTE id = 1;
	fwrite(&id, sizeof(BYTE), 1, f);

	fwrite(&image->header->cMapStart, sizeof(WORD), 1, f);
	fwrite(&image->header->cMapLength, sizeof(WORD), 1, f);
	fwrite(&image->header->cMapDepth, sizeof(BYTE), 1, f);
	fwrite(&image->header->xOffset, sizeof(WORD), 1, f);
	fwrite(&image->header->yOffset, sizeof(WORD), 1, f);
	fwrite(&image->header->width, sizeof(WORD), 1, f);
	fwrite(&image->header->height, sizeof(WORD), 1, f);
	fwrite(&image->header->pixelDepth, sizeof(BYTE), 1, f);
	fwrite(&image->header->imageDescriptor, sizeof(BYTE), 1, f);

	fwrite(&image->colorTable, sizeof(RGB), image->header->cMapLength, f);

	fwrite(image->pixels8, sizeof(BYTE), image->header->width * image->header->height, f);

	fclose(f);

	delete image;
	
	return 0;
}

#include "TgaImage.h"

TgaImage::TgaImage()
{
	memset(this, 0x00, sizeof(TgaImage));
}

TgaImage::~TgaImage()
{
	delete header;

	delete footer;

	delete extension;

	if (pixels8)
	{
		free(pixels8);
	}

	if (pixels24)
	{
		free(pixels24);
	}

	if (pixels32)
	{
		free(pixels32);
	}

	if (file)
	{
		fclose(file);
	}

}

TgaImage::TgaImage(char* f)
{
	memset(this, 0x00, sizeof(TgaImage));

	header = new TgaHeader();

	footer = new TgaFooter();

	extension = new TgaExtension();

	err = fopen_s(&file, f, "rb");

	if (err != 0)
	{
		memset(buffer, 0x00, MAX_BUFFER_LEN);
		strerror_s(buffer, err);

		printf("Error opening %s:%d %s", f, err, buffer);
		return;
	}

	isInitialized = true;
}

bool TgaImage::Load()
{
	if (!isInitialized)
	{
		return false;
	}

	LoadHeader();

	LoadImageDescription();

	LoadColorTable();

	LoadPixelData();

	LoadExtensions();

	fclose(file);

	return true;
}

void TgaImage::DumpHeader()
{
	if (!isInitialized)
	{
		return;
	}

	FILE* out;

	size_t err = fopen_s(&out, "header.txt", "w");

	memset(buffer, 0x00, MAX_BUFFER_LEN);
	sprintf_s(buffer, MAX_BUFFER_LEN, "%05d", header->width);

	fwrite(buffer, sizeof(CHAR), strlen(buffer), out);

	memset(buffer, 0x00, MAX_BUFFER_LEN);
	sprintf_s(buffer, MAX_BUFFER_LEN, "%05d", header->height);

	fwrite(buffer, sizeof(CHAR), strlen(buffer), out);

	fclose(out);
}

void TgaImage::DumpFooter()
{
	if (!isInitialized)
	{
		return;
	}

	FILE* out;

	size_t err = fopen_s(&out, "footer.txt", "w");

	fwrite(footer->signature, sizeof(CHAR), strlen(footer->signature), out);

	fclose(out);
}

void TgaImage::DumpColorMap()
{
	if (!isInitialized)
	{
		return;
	}

	FILE* out;

	size_t err = fopen_s(&out, "colorMap.txt", "wb");

	int i = sizeof(BGR);

	fwrite(colorTable, i, header->cMapLength, out);

	fclose(out);
}

void TgaImage::DumpRawPixels8()
{
	if (!isInitialized)
	{
		return;
	}

	FILE* out;

	size_t err = fopen_s(&out, "pixels8.txt", "wb");

	if (pixels8 > 0)
	{
		fwrite(pixels8, sizeof(BYTE), size8, out);
	}

	fclose(out);
}

void TgaImage::DumpRawPixels24()
{
	if (!isInitialized)
	{
		return;
	}

	FILE* out;

	size_t err = fopen_s(&out, "pixels24.txt", "wb");

	if (pixels24 > 0)
	{
		fwrite(pixels24, sizeof(RGB), size24, out);
	}

	fclose(out);
}

void TgaImage::DumpRawPixels32()
{
	if (!isInitialized)
	{
		return;
	}

	FILE* out;

	size_t err = fopen_s(&out, "pixels32.txt", "wb");

	if (pixels32 > 0)
	{
		fwrite(pixels32, sizeof(ARGB), size32, out);
	}

	fclose(out);
}

void TgaImage::LoadHeader()
{
	err = fread_s(&header->idLength, sizeof(BYTE), sizeof(BYTE), 1, file);
	err = fread_s(&header->colorMapType, sizeof(BYTE), sizeof(BYTE), 1, file);
	err = fread_s(&header->imageType, sizeof(BYTE), sizeof(BYTE), 1, file);
	err = fread_s(&header->cMapStart, sizeof(WORD), sizeof(WORD), 1, file);
	err = fread_s(&header->cMapLength, sizeof(WORD), sizeof(WORD), 1, file);
	err = fread_s(&header->cMapDepth, sizeof(BYTE), sizeof(BYTE), 1, file);
	err = fread_s(&header->xOffset, sizeof(WORD), sizeof(WORD), 1, file);
	err = fread_s(&header->yOffset, sizeof(WORD), sizeof(WORD), 1, file);
	err = fread_s(&header->width, sizeof(WORD), sizeof(WORD), 1, file);
	err = fread_s(&header->height, sizeof(WORD), sizeof(WORD), 1, file);
	err = fread_s(&header->pixelDepth, sizeof(BYTE), sizeof(BYTE), 1, file);
	err = fread_s(&header->imageDescriptor, sizeof(BYTE), sizeof(BYTE), 1, file);

	size8 = header->width * header->height;
	size24 = header->width * header->height * sizeof(RGB);
	size32 = header->width * header->height * sizeof(ARGB);
}

void TgaImage::LoadImageDescription()
{
	// max length is 256
	if (header->idLength > 0)
	{
		err = fread_s(&imageDescription, sizeof(BYTE) * 256, sizeof(BYTE), header->idLength, file);
	}
}

void TgaImage::LoadColorTable()
{
	if (header->cMapLength > 0)
	{
		err = fread_s(&colorTable, sizeof(BGR) * 256, sizeof(BGR), header->cMapLength, file);
	}
}

void TgaImage::LoadPixelData()
{
	switch (header->imageType)
	{
		// Colormapped image data
	case 1:
	{
		if (header->pixelDepth == 8)
		{
			Load8BitUncompressed();
		}

		break;
	}

	// Truecolor image data
	case 2:
	{
		if (header->pixelDepth == 24)
		{
			Load24BitUncompressed();
		}

		if (header->pixelDepth == 32)
		{
			Load32BitUncompressed();
		}

		break;
	}

	// Colormap with RLE Compression
	case 9:
	{
		if (header->pixelDepth == 8)
		{
			Load8BitCompressed();
		}

		if (header->pixelDepth == 24)
		{
			Load24BitCompressed();
		}

		if (header->pixelDepth == 32)
		{
			Load32BitCompressed();
		}

		break;
	}

	default:
	{
		printf("Invalid image type: %d", header->imageType);

		break;
	}
	}
}

void TgaImage::Load8BitUncompressed()
{
	pixels8 = (BYTE*)malloc(size8);

	pixels24 = (BYTE*)malloc(size24);

	pixels32 = (BYTE*)malloc(size32);

	BYTE* pIterator8 = pixels8;

	BYTE* pIterator24 = pixels24;

	BYTE* pIterator32 = pixels32;

	BYTE value;

	int pcount = 0;

	while (size32 > pcount)
	{
		fread_s(&value, sizeof(BYTE), sizeof(BYTE), 1, file);

		BGR bgr = colorTable[value];

		*pIterator8 = value;
		pIterator8 += 1;


		*pIterator24 = bgr.Red;
		pIterator24 += 1;

		*pIterator24 = bgr.Green;
		pIterator24 += 1;

		*pIterator24 = bgr.Blue;
		pIterator24 += 1;


		*pIterator32 = 255;
		pIterator32 += 1;

		*pIterator32 = bgr.Red;
		pIterator32 += 1;

		*pIterator32 = bgr.Green;
		pIterator32 += 1;

		*pIterator32 = bgr.Blue;
		pIterator32 += 1;

		pcount += 4;
	}

}

void TgaImage::Load24BitUncompressed()
{
	pixels24 = (BYTE*)malloc(size24);

	pixels32 = (BYTE*)malloc(size32);

	BYTE* pIterator24 = pixels24;

	BYTE* pIterator32 = pixels32;

	BGR bgr;

	int pcount = 0;

	while (size32 > pcount)
	{
		fread_s(&bgr, sizeof(BGR), sizeof(BGR), 1, file);

		*pIterator24 = bgr.Red;
		pIterator24 += 1;

		*pIterator24 = bgr.Green;
		pIterator24 += 1;

		*pIterator24 = bgr.Blue;
		pIterator24 += 1;


		*pIterator32 = 255;
		pIterator32 += 1;

		*pIterator32 = bgr.Red;
		pIterator32 += 1;

		*pIterator32 = bgr.Green;
		pIterator32 += 1;

		*pIterator32 = bgr.Blue;
		pIterator32 += 1;

		pcount += 4;
	}

}

void TgaImage::Load32BitUncompressed()
{
	pixels24 = (BYTE*)malloc(size24);

	pixels32 = (BYTE*)malloc(size32);

	BYTE* pIterator24 = pixels24;

	BYTE* pIterator32 = pixels32;

	BGRA bgra;

	int pcount = 0;

	while (size32 > pcount)
	{
		fread_s(&bgra, sizeof(BGRA), sizeof(BGRA), 1, file);

		*pIterator24 = bgra.Red;
		pIterator24 += 1;

		*pIterator24 = bgra.Green;
		pIterator24 += 1;

		*pIterator24 = bgra.Blue;
		pIterator24 += 1;


		*pIterator32 = bgra.Alpha;
		pIterator32 += 1;

		*pIterator32 = bgra.Red;
		pIterator32 += 1;

		*pIterator32 = bgra.Green;
		pIterator32 += 1;

		*pIterator32 = bgra.Blue;
		pIterator32 += 1;

		pcount += 4;
	}

}

void TgaImage::Load8BitCompressed()
{
	pixels8 = (BYTE*)malloc(size8);

	pixels24 = (BYTE*)malloc(size24);

	pixels32 = (BYTE*)malloc(size32);

	BYTE* pIterator8 = pixels8;

	BYTE* pIterator24 = pixels24;

	BYTE* pIterator32 = pixels32;

	int pcount = 0;

	RLEPacket8* p8 = new RLEPacket8();

	while (size32 > pcount)
	{
		p8->ReadCount(file);

		int count = p8->GetCount();

		// encoded or raw
		if (p8->IsEncoded())
		{
			p8->ReadValue(file);

			BGR bgr = colorTable[p8->value];

			for (int c = 0; c < count; c++)
			{
				*pIterator8 = p8->value;
				pIterator8 += 1;


				*pIterator24 = bgr.Red;
				pIterator24 += 1;

				*pIterator24 = bgr.Green;
				pIterator24 += 1;

				*pIterator24 = bgr.Blue;
				pIterator24 += 1;


				*pIterator32 = 255;
				pIterator32 += 1;

				*pIterator32 = bgr.Red;
				pIterator32 += 1;

				*pIterator32 = bgr.Green;
				pIterator32 += 1;

				*pIterator32 = bgr.Blue;
				pIterator32 += 1;

				pcount += 4;
			}
		}
		else
		{
			for (int c = 0; c < count; c++)
			{
				fread_s(&p8->value, sizeof(BYTE), sizeof(BYTE), 1, file);

				BGR bgr = colorTable[p8->value];

				*pIterator8 = p8->value;
				pIterator8 += 1;


				*pIterator24 = bgr.Red;
				pIterator24 += 1;

				*pIterator24 = bgr.Green;
				pIterator24 += 1;

				*pIterator24 = bgr.Blue;
				pIterator24 += 1;


				*pIterator32 = 255;
				pIterator32 += 1;

				*pIterator32 = bgr.Red;
				pIterator32 += 1;

				*pIterator32 = bgr.Green;
				pIterator32 += 1;

				*pIterator32 = bgr.Blue;
				pIterator32 += 1;

				pcount += 4;
			}
		}
	}

	delete p8;
}

void TgaImage::Load24BitCompressed()
{
	pixels24 = (BYTE*)malloc(size24);

	pixels32 = (BYTE*)malloc(size32);

	BYTE* pIterator24 = pixels24;

	BYTE* pIterator32 = pixels32;

	int pcount = 0;

	RLEPacket24* p24 = new RLEPacket24();

	while (size32 > pcount)
	{
		p24->ReadCount(file);

		int count = p24->GetCount();

		// encoded or raw
		if (p24->IsEncoded())
		{
			p24->ReadValue(file);

			for (int c = 0; c < count; c++)
			{
				*pIterator24 = p24->value.Red;
				pIterator24 += 1;

				*pIterator24 = p24->value.Green;
				pIterator24 += 1;

				*pIterator24 = p24->value.Blue;
				pIterator24 += 1;


				*pIterator32 = 255;
				pIterator32 += 1;

				*pIterator32 = p24->value.Red;
				pIterator32 += 1;

				*pIterator32 = p24->value.Green;
				pIterator32 += 1;

				*pIterator32 = p24->value.Blue;
				pIterator32 += 1;

				pcount += 4;
			}
		}
		else
		{
			for (int c = 0; c < count; c++)
			{
				p24->ReadValue(file);

				*pIterator24 = p24->value.Red;
				pIterator24 += 1;

				*pIterator24 = p24->value.Green;
				pIterator24 += 1;

				*pIterator24 = p24->value.Blue;
				pIterator24 += 1;


				*pIterator32 = 255;
				pIterator32 += 1;

				*pIterator32 = p24->value.Red;
				pIterator32 += 1;

				*pIterator32 = p24->value.Green;
				pIterator32 += 1;

				*pIterator32 = p24->value.Blue;
				pIterator32 += 1;

				pcount += 4;
			}
		}
	}

}

void TgaImage::Load32BitCompressed()
{
	pixels24 = (BYTE*)malloc(size24);

	pixels32 = (BYTE*)malloc(size32);

	BYTE* pIterator24 = pixels24;

	BYTE* pIterator32 = pixels32;

	int pcount = 0;

	RLEPacket32* p32 = new RLEPacket32();

	while (size32 > pcount)
	{
		p32->ReadCount(file);

		int count = p32->GetCount();

		// encoded or raw
		if (p32->IsEncoded())
		{
			p32->ReadValue(file);

			for (int c = 0; c < count; c++)
			{
				*pIterator24 = p32->value.Red;
				pIterator24 += 1;

				*pIterator24 = p32->value.Green;
				pIterator24 += 1;

				*pIterator24 = p32->value.Blue;
				pIterator24 += 1;


				*pIterator32 = p32->value.Alpha;
				pIterator32 += 1;

				*pIterator32 = p32->value.Red;
				pIterator32 += 1;

				*pIterator32 = p32->value.Green;
				pIterator32 += 1;

				*pIterator32 = p32->value.Blue;
				pIterator32 += 1;

				pcount += 4;
			}
		}
		else
		{
			for (int c = 0; c < count; c++)
			{
				p32->ReadValue(file);

				*pIterator24 = p32->value.Red;
				pIterator24 += 1;

				*pIterator24 = p32->value.Green;
				pIterator24 += 1;

				*pIterator24 = p32->value.Blue;
				pIterator24 += 1;


				*pIterator32 = p32->value.Alpha;
				pIterator32 += 1;

				*pIterator32 = p32->value.Red;
				pIterator32 += 1;

				*pIterator32 = p32->value.Green;
				pIterator32 += 1;

				*pIterator32 = p32->value.Blue;
				pIterator32 += 1;

				pcount += 4;
			}
		}
	}

}

void TgaImage::LoadExtensions()
{
	if (!feof(file))
	{
		footer->ReadValues(file);
	}

	if (footer->extensionOffset > 0)
	{

	}

	if (footer->developerOffset > 0)
	{

	}
}

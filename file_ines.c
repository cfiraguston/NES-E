/* bases on https://wiki.nesdev.com/w/index.php/INES */
#include "file_ines.h"

FILE* file_ines_open(char* filename)
{
	FILE* fd = fopen(filename, "rb");

	return fd;
}

bool file_ines_read_header(FILE* fd, uint8_t* data)
{
	if (0 != fseek(fd, 0, SEEK_SET))
	{
		return false;
	}

	if (1 != fread(data, sizeof(st_File_INES_Header), 1, fd))
	{
		return false;
	}

	return true;
}

bool file_ines_read_PRG_ROM(FILE* fd, uint8_t Size_of_PRG_ROM, uint8_t* data)
{
	if (Size_of_PRG_ROM != fread(data, sizeof(uint8_t) * 16 * 1024, Size_of_PRG_ROM, fd))
	{
		return false;
	}

	return true;
}

bool file_ines_read_CHR_ROM(FILE* fd, uint8_t Size_of_CHR_ROM, uint8_t* data)
{
	if (Size_of_CHR_ROM != fread(data, sizeof(uint8_t) * 16 * 1024, Size_of_CHR_ROM, fd))
	{
		return false;
	}

	return true;
}
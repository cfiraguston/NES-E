#ifndef __FILE_INES_H__
#define __FILE_INES_H__

#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* Flags 6
76543210
||||||||
|||||||+- Mirroring: 0: horizontal (vertical arrangement) (CIRAM A10 = PPU A11)
|||||||              1: vertical (horizontal arrangement) (CIRAM A10 = PPU A10)
||||||+-- 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
|||||+--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
||||+---- 1: Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM
++++----- Lower nybble of mapper number
*/
typedef struct _tag_st_Flags6
{
	uint8_t Mirroring			: 1;
	uint8_t PersistantMemory	: 1;
	uint8_t Trainer				: 1;
	uint8_t MirroringControl	: 1;
	uint8_t MapperNumber		: 4;
} st_Flags6;

typedef struct _tag_st_File_INES_Header
{
	uint8_t Type[4];			/* Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file) */
	uint8_t Size_of_PRG_ROM;	/* In 16 KB units */
	uint8_t Size_of_CHR_ROM;	/* In 8 KB units (Value 0 means the board uses CHR RAM) */
	st_Flags6 Flags6;			/* Mapper, mirroring, battery, trainer */
	uint8_t Flags7;				/* Mapper, VS/Playchoice, NES 2.0 */
	uint8_t Flags8;				/* PRG-RAM size (rarely used extension) */
	uint8_t Flags9;				/* TV system (rarely used extension) */
	uint8_t Flags10;			/* TV system, PRG-RAM presence (unofficial, rarely used extension) */
	uint8_t Padding[5];			/* Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15) */
} st_File_INES_Header;


FILE* file_ines_open(char* filename);
bool file_ines_read_header(FILE* fd, uint8_t* data);
bool file_ines_read_PRG_ROM(FILE* fd, uint8_t Size_of_PRG_ROM, uint8_t* data);
bool file_ines_read_CHR_ROM(FILE* fd, uint8_t Size_of_CHR_ROM, uint8_t* data);

#endif	/* __FILE_INES_H__ */
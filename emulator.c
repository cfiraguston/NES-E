#include "emulator.h"
#include "cpu6502.h"
#include "memory.h"
#include "ppu.h"

#include "file_ines.h"

#include <stdlib.h>
#include <string.h>

bool emulatorInit()
{
	Memory* RAM;
	
	RAM = (Memory*)malloc(sizeof(Memory) * SIZE_OF_RAM);
	memset(RAM, 0x00, SIZE_OF_RAM);
	
	/*FILE* fd = file_ines_open("nestest.nes");*/
	FILE* fd = file_ines_open("Donkey_Kong.nes");
	if (NULL == fd)
	{
		return false;
	}

	st_File_INES_Header FileHeader;
	file_ines_read_header(fd, (uint8_t*)&FileHeader);
	uint8_t* PRG_ROM_Data = (uint8_t*)malloc(1024 * 64);
	file_ines_read_PRG_ROM(fd, FileHeader.Size_of_PRG_ROM, PRG_ROM_Data);
	uint8_t* CHR_ROM_Data = (uint8_t*)malloc(1024 * 64);
	file_ines_read_CHR_ROM(fd, FileHeader.Size_of_CHR_ROM, CHR_ROM_Data);

	/* initialize CPU memory map */
	st_MemoryMap *CPU_MemoryMap = MemoryMapInit(2000);
	MemoryMapAdd(CPU_MemoryMap, 0x0000, 0x0800, (uint8_t*)&(RAM[0]), MemoryRegionRead, MemoryRegionWrite);		/* 2KB of work RAM */
	MemoryMapAdd(CPU_MemoryMap, 0x0800, 0x0800, (uint8_t*)&(RAM[0]), MemoryRegionRead, MemoryRegionWrite);		/* Mirror of $000-$7FF */
	MemoryMapAdd(CPU_MemoryMap, 0x1000, 0x0800, (uint8_t*)&(RAM[0]), MemoryRegionRead, MemoryRegionWrite);		/* Mirror of $000-$7FF */
	MemoryMapAdd(CPU_MemoryMap, 0x1800, 0x0800, (uint8_t*)&(RAM[0]), MemoryRegionRead, MemoryRegionWrite);		/* Mirror of $000-$7FF */
	for (uint16_t address = 0x2000; address < 0x4000; address += 0x0008)
	{
		MemoryMapAdd(CPU_MemoryMap, address, 0x0008, (uint8_t*)(Get_PPU_Registers()), ppuRegisterRead, ppuRegisterWrite);	/* PPU registers are mirrored $2008-3FFF */
	}
	/* TODO: COMPLETE MAPPING:
	$4000	$20	Registers (Mostly APU)
	$4020	$1FDF	Cartridge Expansion ROM
	$6000	$2000	SRAM
	*/
	Memory* TempMemory;
	TempMemory = (Memory*)malloc(sizeof(Memory) * 0x4000);
	memset(TempMemory, 0x00, SIZE_OF_RAM);
	MemoryMapAdd(CPU_MemoryMap, 0x4000, 0x4000, (uint8_t*)&(TempMemory[0]), MemoryRegionRead, MemoryRegionWrite);		/* TEMP MEMORY ALLOCATION TO HAVE A COMPLETE MAP */
	MemoryMapAdd(CPU_MemoryMap, 0x8000, 0x4000, (uint8_t*)&(PRG_ROM_Data[0]), MemoryRegionRead, MemoryRegionWrite);		/* PRG-ROM */
	MemoryMapAdd(CPU_MemoryMap, 0xC000, 0x4000, (uint8_t*)&(PRG_ROM_Data[0]), MemoryRegionRead, MemoryRegionWrite);		/* PRG-ROM */
	
	/* initialize PPU memory map */
	/*	Address range	Size	Description
		$0000-$0FFF		$1000	Pattern table 0
		$1000-$1FFF		$1000	Pattern table 1
		$2000-$23FF		$0400	Nametable 0
		$2400-$27FF		$0400	Nametable 1
		$2800-$2BFF		$0400	Nametable 2
		$2C00-$2FFF		$0400	Nametable 3
		$3000-$3EFF		$0F00	Mirrors of $2000-$2EFF
		$3F00-$3F1F		$0020	Palette RAM indexes
		$3F20-$3FFF		$00E0	Mirrors of $3F00-$3F1F
	*/
	st_MemoryMap* PPU_MemoryMap = MemoryMapInit(100);
	MemoryMapAdd(PPU_MemoryMap, 0x0000, 0x2000, (uint8_t*)&(CHR_ROM_Data[0]), MemoryRegionRead, MemoryRegionWrite);		/* Pattern table - CHR ROM */
	uint8_t* Nametable1 = (uint8_t*)malloc(1024);
	uint8_t* Nametable2 = (uint8_t*)malloc(1024);
	/* hardware on the cartridge controls address bit 10 of CIRAM to map one nametable on top of another. */
	if (FileHeader.Flags6.Mirroring == 1)	/* vertical mirroring (horizontal arrangement) */
	{
		MemoryMapAdd(PPU_MemoryMap, 0x2000, 0x0400, (uint8_t*)&(Nametable1[0]), MemoryRegionRead, MemoryRegionWrite);
		MemoryMapAdd(PPU_MemoryMap, 0x2400, 0x0400, (uint8_t*)&(Nametable2[0]), MemoryRegionRead, MemoryRegionWrite);
		MemoryMapAdd(PPU_MemoryMap, 0x2800, 0x0400, (uint8_t*)&(Nametable1[0]), MemoryRegionRead, MemoryRegionWrite);
		MemoryMapAdd(PPU_MemoryMap, 0x2C00, 0x0400, (uint8_t*)&(Nametable2[0]), MemoryRegionRead, MemoryRegionWrite);
	}
	else									/* horizontal mirroring (vertical arrangement) */
	{
		MemoryMapAdd(PPU_MemoryMap, 0x2000, 0x0400, (uint8_t*)&(Nametable1[0]), MemoryRegionRead, MemoryRegionWrite);
		MemoryMapAdd(PPU_MemoryMap, 0x2400, 0x0400, (uint8_t*)&(Nametable1[0]), MemoryRegionRead, MemoryRegionWrite);
		MemoryMapAdd(PPU_MemoryMap, 0x2800, 0x0400, (uint8_t*)&(Nametable2[0]), MemoryRegionRead, MemoryRegionWrite);
		MemoryMapAdd(PPU_MemoryMap, 0x2C00, 0x0400, (uint8_t*)&(Nametable2[0]), MemoryRegionRead, MemoryRegionWrite);
	}
	MemoryMapAdd(PPU_MemoryMap, 0x3000, 0x0400, MemoryMapGet(PPU_MemoryMap, 0x2000), MemoryRegionRead, MemoryRegionWrite);
	MemoryMapAdd(PPU_MemoryMap, 0x3400, 0x0400, MemoryMapGet(PPU_MemoryMap, 0x2400), MemoryRegionRead, MemoryRegionWrite);
	MemoryMapAdd(PPU_MemoryMap, 0x3800, 0x0400, MemoryMapGet(PPU_MemoryMap, 0x2800), MemoryRegionRead, MemoryRegionWrite);
	MemoryMapAdd(PPU_MemoryMap, 0x3C00, 0x0300, MemoryMapGet(PPU_MemoryMap, 0x2C00), MemoryRegionRead, MemoryRegionWrite);
	uint8_t* Palette = (uint8_t*)malloc(32);
	for (uint16_t paletteIdx = 0u; paletteIdx < 8u; paletteIdx++)
	{
		MemoryMapAdd(PPU_MemoryMap, 0x3F00 + (0x20 * paletteIdx), 0x0020, (uint8_t*)&(Palette[0]), ppuPaletteRead, ppuPaletteWrite);
	}

	cpu6502Init(CPU_MemoryMap);
	ppuInit(PPU_MemoryMap);

	return true;
}

bool emulatorRun()
{
	uint32_t cycle = 0u;

	while (1)
	{
		if (false == ppuRunCycle(cycle))
		{
			break;
		}
		if (false == cpu6502RunCycle(cycle))
		{
			break;
		}

		cycle++;
	}

	return true;
}

bool emulatorDestroy()
{
	cpu6502Destroy();
	return true;
}

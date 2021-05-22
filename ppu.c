/* Based on: http://wiki.nesdev.com/w/index.php/PPU */
/* Based on: https://bugzmanov.github.io/nes_ebook/chapter_6_1.html */

#include "ppu.h"

#include "SDL.h"

st_RGB_Color Palette_2C02[] = {
	/*0x00*/	{84,	84,		84},
	/*0x01*/	{0,		30,		116},
	/*0x02*/	{8,		16,		144},
	/*0x03*/	{48,	0,		136},
	/*0x04*/	{68,	0,		100},
	/*0x0%*/	{92,	0,		48},
	/*0x06*/	{84,	4,		0},
	/*0x0&*/	{60,	24,		0},
	/*0x08*/	{32,	42,		0},
	/*0x09*/	{8,		58,		0},
	/*0x0A*/	{0,		64,		0},
	/*0x0B*/	{0,		60,		0},
	/*0x0C*/	{0,		50,		60},
	/*0x0D*/	{0,		0,		0},
	/*0X0E*/	{0,		0,		0},
	/*0X0F*/	{0,		0,		0},
	/*0x10*/	{152,	150,	152},
	/*0x11*/	{8,		76,		196},
	/*0x12*/	{48,	50,		236},
	/*0x13*/	{92,	30,		228},
	/*0x14*/	{136,	20,		176},
	/*0x15*/	{160,	20,		100},
	/*0x16*/	{152,	34,		32},
	/*0x17*/	{120,	60,		0},
	/*0x18*/	{84,	90,		0},
	/*0x19*/	{40,	114,	0},
	/*0x1A*/	{8,		124,	0},
	/*0x1B*/	{0,		118,	40},
	/*0x1C*/	{0,		102,	120},
	/*0x1D*/	{0,		0,		0},
	/*0x1E*/	{0,		0,		0},
	/*0x1F*/	{0,		0,		0},
	/*0x20*/	{236,	238,	236},
	/*0x21*/	{76,	154,	236},
	/*0x22*/	{120,	124,	236},
	/*0x23*/	{176,	98,		236},
	/*0x24*/	{228,	84,		236},
	/*0x25*/	{236,	88,		180},
	/*0x26*/	{236,	106,	100},
	/*0x27*/	{212,	136,	32},
	/*0x28*/	{160,	170,	0},
	/*0x29*/	{116,	196,	0},
	/*0x2A*/	{76,	208,	32},
	/*0x2B*/	{56,	204,	108},
	/*0x2C*/	{56,	180,	204},
	/*0x2D*/	{60,	60,		60},
	/*0x2E*/	{0,		0,		0},
	/*0x2F*/	{0,		0,		0},
	/*0x30*/	{236,	238,	236},
	/*0x31*/	{168,	204,	236},
	/*0x32*/	{188,	188,	236},
	/*0x33*/	{212,	178,	236},
	/*0x34*/	{236,	174,	236},
	/*0x35*/	{236,	174,	212},
	/*0x36*/	{236,	180,	176},
	/*0x37*/	{228,	196,	144},
	/*0x38*/	{204,	210,	120},
	/*0x39*/	{180,	222,	120},
	/*0x3A*/	{168,	226,	144},
	/*0x3B*/	{152,	226,	180},
	/*0x3C*/	{160,	214,	228},
	/*0x3D*/	{160,	162,	160},
	/*0x3E*/	{0,		0,		0},
	/*0x3F*/	{0,		0,		0}
};

static st_RGB_Color* pCurrentColorPalette = Palette_2C02;

static st_PPU_Registers PPU_Registers;
static st_MemoryMap* MemoryMap = NULL;

st_PPU_Registers* Get_PPU_Registers()
{
	return &PPU_Registers;
}

uint8_t ppuAddressRead(uint16_t address)
{
	return MemoryMapRead(MemoryMap, address);
}

void ppuAddressWrite(uint16_t address, uint8_t value)
{
	MemoryMapWrite(MemoryMap, address, value);
}

static uint16_t PPU_Address_Latch = 0u;

uint8_t ppuRegisterRead(uint8_t* region, uint16_t address)
{
	uint8_t read_data = 0;
	switch (address)
	{
	case 0x0000:	/* 0x2000 - PPUCTRL */
		break;
	case 0x0001:	/* 0x2001 - PPUMASK */
		break;
	case 0x0002:	/* 0x2002 - PPUSTATUS */
		/* Reading the status register will clear bit 7 mentioned above and also the address latch used by PPUSCROLL and PPUADDR. */
		read_data = ppuAddressRead(address);
		Get_PPU_Registers()->PPUSTATUS.V = 0;	/* Vertical blank is cleared after reading $2002. */
		PPU_Address_Latch = 0u;
		break;
	case 0x0003:	/* 0x2003 - OAMADDR */
		break;
	case 0x0004:	/* 0x2004 - OAMDATA */
		break;
	case 0x0005:	/* 0x2005 - PPUSCROLL */
		break;
	case 0x0006:	/* 0x2006 - PPUADDR */
		break;
	case 0x0007:	/* 0x2007 - PPUDATA */
		read_data = ppuAddressRead(PPU_Address_Latch);	/* read to absolute address */
		printf("PPUDATA Read = %04X=%02X\n", PPU_Address_Latch, read_data);
		PPU_Address_Latch += (((Get_PPU_Registers()->PPUCTRL.I) * 31) + 1); /* I = 0: add 1, going across; 1: add 32, going down */
		break;
	default:
		printf("Error address %04X in ppuRegisterRead\n", address);
	}

	return read_data;
}

void ppuRegisterWrite(uint8_t* region, uint16_t address, uint8_t value)
{
	static uint8_t PPU_Address_High = 1;
	switch (address)	/* addresses are offset from the begininng of region */
	{
	case 0x0000:	/* 0x2000 - PPUCTRL */
		break;
	case 0x0001:	/* 0x2001 - PPUMASK */
		break;
	case 0x0002:	/* 0x2002 - PPUSTATUS */
		break;
	case 0x0003:	/* 0x2003 - OAMADDR */
		break;
	case 0x0004:	/* 0x2004 - OAMDATA */
		break;
	case 0x0005:	/* 0x2005 - PPUSCROLL */
		break;
	case 0x0006:	/* 0x2006 - PPUADDR */
		PPU_Address_Latch |= value << ((8 * PPU_Address_High));
		if (0 == PPU_Address_High)	/* Valid addresses are $0000-$3FFF; higher addresses will be mirrored down. 
									    Calculate this when reading the low address into the latch. */
		{
			PPU_Address_Latch %= 0x3FFF;
			printf("PPUADDR = %04X\n", PPU_Address_Latch);
		}
		PPU_Address_High = 1 - PPU_Address_High;
		break;
	case 0x0007:	/* 0x2007 - PPUDATA */
		printf("PPUDATA Write = %04X=%02X\n", PPU_Address_Latch, value);
		ppuAddressWrite(PPU_Address_Latch, value);	/* write to absolute address */
		PPU_Address_Latch += (((Get_PPU_Registers()->PPUCTRL.I) * 31) + 1); /* I = 0: add 1, going across; 1: add 32, going down */
		break;
	default:
		printf("Error address %04X in ppuRegisterWrite\n", address);
	}
}

uint8_t ppuPaletteRead(uint8_t* region, uint16_t address)
{
	return (region[address]);
}

void ppuPaletteWrite(uint8_t* region, uint16_t address, uint8_t value)
{
	region[address] = value;

	if (0x0000 == address)	/* Address 0x3F00 is offset to beginning of region - 0x0000.
							   Universal background color, mirrored to all palettes */
	{
		for (uint16_t mirrorIdx = 1u; mirrorIdx < 8u; mirrorIdx++)
		{
			region[4 * mirrorIdx] = value;
		}
	}
}


static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Surface* screenshot = NULL;

void ppuPutPixel(uint16_t col, uint16_t row, uint8_t c, SDL_Renderer* renderer)
{
	switch (c)
	{
	case 0:
	case 1:
	case 2:
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
		break;
	case 3:
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
		break;
	}

	/* Output one NES pixel to screen */
	for (uint16_t i = 0; i < NES_PIXEL_SIZE; i++)
	{
		for (uint16_t j = 0; j < NES_PIXEL_SIZE; j++)
		{
			SDL_RenderDrawPoint(renderer, (col * NES_PIXEL_SIZE) + j, (row * NES_PIXEL_SIZE) + i);
		}
	}
}

bool ppuDrawPatern(uint16_t col, uint16_t row, uint8_t patternID, SDL_Renderer* renderer)
{
	PPU_PatternAddressing PatternAddressing;
	PatternAddressing.T = 0;
	PatternAddressing.P = 0;
	PatternAddressing.C = col / 8;
	PatternAddressing.R = row / 8;
	PatternAddressing.H = Get_PPU_Registers()->PPUCTRL.B;
	PatternAddressing.zero = 0;
	PatternAddressing.spare = 0;
	
	uint8_t LowPlane = 0;
	uint8_t HighPlane = 0;

	for (uint8_t rowIdx = 0; rowIdx < 8; rowIdx++)
	{
		PatternAddressing.T = rowIdx;
		PatternAddressing.P = 0;
		LowPlane = ppuAddressRead(PatternAddressing.value);
		/*printf("%04X=%02X ", PatternAddressing.value, LowPlane);*/
		PatternAddressing.P = 1;
		HighPlane = ppuAddressRead(PatternAddressing.value);
		/*printf("%04X=%02X\n", PatternAddressing.value, HighPlane);*/
		for (uint8_t colIdx = 0; colIdx < 8; colIdx++)
		{
			uint8_t color =  
				(LowPlane >> (7 - colIdx)) & 0x01 |
				((HighPlane >> (7 - colIdx)) & 0x01) << 1;
			ppuPutPixel(col + colIdx, row + rowIdx, color, renderer);
		}
	}

	return true;
}

bool ppuInit(st_MemoryMap* map)
{
	MemoryMap = map;

	/*SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer((256 * NES_PIXEL_SIZE), (128 * NES_PIXEL_SIZE), 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);*/

	return true;
}

/* Each nametable has 30 rows of 32 tiles each. */
static uint8_t ppuGetNameTable(uint8_t row, uint8_t tile)
{
	/* !!!!!TODO: GET DATA FROM RELEVANT NAMETABLE: 0x2000, 0x2400, 0x2800, 0x2C00 */
	return ppuAddressRead(0x2000 + ((row * 32) + tile));
}

static uint8_t ppuGetAttributeTable(uint8_t row, uint8_t tile)
{
	uint8_t attribute = 0;
	/* !!!!!TODO: GET DATA FROM RELEVANT ATTRIBUTETABLE: 0x23C0, 0x27C0, 0x2BC0, 0x2FC0 */
	attribute =  ppuAddressRead(0x23C0 + (((row / 4) * 32) + (tile / 4)));

	/* 
	7654 3210
	|||| ||++- Color bits 3-2 for top left quadrant of this byte
	|||| ++--- Color bits 3-2 for top right quadrant of this byte
	||++------ Color bits 3-2 for bottom left quadrant of this byte
	++-------- Color bits 3-2 for bottom right quadrant of this byte */
	attribute >>= (((row % 4) >= 2) * 4) + (((tile % 4) >= 2) * 2);

	return attribute;
}

static uint8_t ppuGetPatternTableTile(uint8_t pattern, uint8_t yOffset, uint8_t bitPlane)
{
	PPU_PatternAddressing PatternAddressing;
	PatternAddressing.T = yOffset;
	PatternAddressing.P = bitPlane;
	PatternAddressing.C = pattern % 16;
	PatternAddressing.R = pattern / 16;
	PatternAddressing.H = Get_PPU_Registers()->PPUCTRL.B;
	PatternAddressing.zero = 0;
	PatternAddressing.spare = 0;
	return ppuAddressRead(PatternAddressing.value);
}

static uint8_t latch_pallete = 0;

static bool ppuHandleVisibleScanlines(uint32_t cycle, uint16_t scanline)
{
	static uint8_t nametable = 0;
	static uint8_t attribute = 0;

	if (0 == cycle)
	{
		/* This is an idle cycle. */
	}
	else if ((1 <= cycle) && (cycle <= 256))
	{
		if (((cycle - 1) / 8) == 0)			/* Get NT byte*/
		{
			nametable = ppuGetNameTable((scanline / 8), ((cycle - 1) / 8));
		}
		else if (((cycle - 3) / 8) == 0)	/* Get AT byte*/
		{
			latch_pallete = ppuGetAttributeTable((scanline / 8), ((cycle - 1) / 8));
		}
		else if (((cycle - 5) / 8) == 0)	/* Get Low BG byte*/
		{
			ppuGetPatternTableTile(nametable, (scanline % 8), 0);
		}
		else if (((cycle - 7) / 8) == 0)	/* Get High BG byte*/
		{
			ppuGetPatternTableTile(nametable, (scanline % 8), 1);
		}
	}
	else if ((257 <= cycle) && (cycle <= 320))
	{

	}
	else if ((321 <= cycle) && (cycle <= 336))
	{

	}
	else if ((337 <= cycle) && (cycle <= 340))
	{

	}

	return true;
}

static bool ppuHandlePostRenderScanlines(uint32_t cycle, uint16_t scanline)
{
	/* The PPU just idles during this scanline. */
	return true;
}

static bool ppuHandleVerticalBlankingScanlines(uint32_t cycle, uint16_t scanline)
{
	/* The VBlank flag of the PPU is set at tick 1 (the second tick) of scanline 241. */
	if (241 == scanline)
	{
		if (1 == cycle)
		{
			Get_PPU_Registers()->PPUSTATUS.V = 1;
		}
	}

	return true;
}

bool ppuRunCycle(uint32_t cycle)
{
	uint32_t ppuCycle = (cycle % 341);
	static uint16_t currentScanline = 261u;

	if ((0 <= currentScanline) && (currentScanline <= 239))
	{
		ppuHandleVisibleScanlines(ppuCycle, currentScanline);
	}
	else if (240 == currentScanline)
	{
		ppuHandlePostRenderScanlines(ppuCycle, currentScanline);
	}
	else if ((241 <= currentScanline) && (currentScanline <= 260))
	{
		ppuHandleVerticalBlankingScanlines(ppuCycle, currentScanline);
	}
	else if (261 == currentScanline)
	{

	}

	currentScanline = (currentScanline + 1) % 262;

	/*uint8_t patternID = 0;
	for (uint16_t row = 0; row < 128; row += 8)
	{
		for (uint16_t col = 0; col < 128; col += 8)
		{
			ppuDrawPatern(col, row, patternID, renderer);
			patternID++;
		}
	}
	Get_PPU_Registers()->PPUCTRL.B = 1;
	for (uint16_t row = 0; row < 128; row += 8)
	{
		for (uint16_t col = 128; col < 256; col += 8)
		{
			ppuDrawPatern(col, row, patternID, renderer);
			patternID++;
		}
	}*/

	/*SDL_RenderPresent(renderer);

	screenshot = SDL_CreateRGBSurface(
		0,
		(256 * NES_PIXEL_SIZE),
		(128 * NES_PIXEL_SIZE),
		32,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0x00000000);

	SDL_RenderReadPixels(renderer, NULL, SDL_GetWindowPixelFormat(window), screenshot->pixels, screenshot->pitch);*/

	return true;
}

bool ppuDestroy()
{
	/*SDL_FreeSurface(screenshot);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();*/

	return true;
}

#ifndef __PPU_H__
#define __PPU_H__

#include <stdint.h>
#include "memory.h"

/* PPUCTRL register
7  bit  0
---- ----
VPHB SINN
|||| ||||
|||| ||++- Base nametable address
|||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
|||| |+--- VRAM address increment per CPU read/write of PPUDATA
|||| |     (0: add 1, going across; 1: add 32, going down)
|||| +---- Sprite pattern table address for 8x8 sprites
||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
|||+------ Background pattern table address (0: $0000; 1: $1000)
||+------- Sprite size (0: 8x8 pixels; 1: 8x16 pixels)
|+-------- PPU master/slave select
|          (0: read backdrop from EXT pins; 1: output color on EXT pins)
+--------- Generate an NMI at the start of the
           vertical blanking interval (0: off; 1: on)
*/
typedef struct _tag_st_PPUCTRL
{
    uint8_t N   : 2;    /* Base nametable address */
    uint8_t I   : 1;    /* VRAM address increment per CPU read/write of PPUDATA */
    uint8_t S   : 1;    /* Sprite pattern table address for 8x8 sprites */
    uint8_t B   : 1;    /* Background pattern table address */
    uint8_t H   : 1;    /* Sprite size */
    uint8_t P   : 1;    /* PPU master/slave select */
    uint8_t V   : 1;    /* Generate an NMI at the start of the vertical blanking interval */
} st_PPUCTRL;

/* PPUMASK register
7  bit  0
---- ----
BGRs bMmG
|||| ||||
|||| |||+- Greyscale (0: normal color, 1: produce a greyscale display)
|||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide
|||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide
|||| +---- 1: Show background
|||+------ 1: Show sprites
||+------- Emphasize red (green on PAL/Dendy)
|+-------- Emphasize green (red on PAL/Dendy)
+--------- Emphasize blue
*/
typedef struct _tag_st_PPUMASK
{
    uint8_t Gr  : 1;    /* Greyscale (0: normal color, 1: produce a greyscale display) */
    uint8_t m   : 1;    /* 1: Show background in leftmost 8 pixels of screen, 0: Hide */
    uint8_t M   : 1;    /* 1: Show sprites in leftmost 8 pixels of screen, 0: Hide */
    uint8_t b   : 1;    /* 1: Show background */
    uint8_t s   : 1;    /* 1: Show sprites */
    uint8_t R   : 1;    /* Emphasize red (green on PAL/Dendy) */
    uint8_t G   : 1;    /* Emphasize green (red on PAL/Dendy) */
    uint8_t B   : 1;    /* Emphasize blue */
} st_PPUMASK;

/* PPUSTATUS register
7  bit  0
---- ----
VSO. ....
|||| ||||
|||+-++++- Least significant bits previously written into a PPU register
|||        (due to register not being updated for this address)
||+------- Sprite overflow. The intent was for this flag to be set
||         whenever more than eight sprites appear on a scanline, but a
||         hardware bug causes the actual behavior to be more complicated
||         and generate false positives as well as false negatives; see
||         PPU sprite evaluation. This flag is set during sprite
||         evaluation and cleared at dot 1 (the second dot) of the
||         pre-render line.
|+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps
|          a nonzero background pixel; cleared at dot 1 of the pre-render
|          line.  Used for raster timing.
+--------- Vertical blank has started (0: not in vblank; 1: in vblank).
           Set at dot 1 of line 241 (the line *after* the post-render
           line); cleared after reading $2002 and at dot 1 of the
           pre-render line.
*/
typedef struct _tag_st_PPUSTATUS
{
    uint8_t lsb_prev    : 5;    /* Least significant bits previously written into a PPU register */
    uint8_t O           : 1;    /* Sprite overflow */
    uint8_t S           : 1;    /* Sprite 0 Hit */
    uint8_t V           : 1;    /* Vertical blank has started */
} st_PPUSTATUS;

typedef struct _tag_st_PPU_Registers
{
    st_PPUCTRL      PPUCTRL;
    st_PPUMASK      PPUMASK;
    st_PPUSTATUS    PPUSTATUS;
    uint8_t         OAMADDR;
    uint8_t         OAMDATA;
    uint8_t         PPUSCROLL;
    uint8_t         PPUADDR;
    uint8_t         PPUDATA;
} st_PPU_Registers;

typedef struct _tag_st_RGB_Color
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
} st_RGB_Color;

/*
DCBA98 76543210
---------------
0HRRRR CCCCPTTT
|||||| |||||+++- T: Fine Y offset, the row number within a tile
|||||| ||||+---- P: Bit plane (0: "lower"; 1: "upper")
|||||| ++++----- C: Tile column
||++++---------- R: Tile row
|+-------------- H: Half of sprite table (0: "left"; 1: "right")
+--------------- 0: Pattern table is at $0000-$1FFF
*/
typedef union _tag_st_PPU_PatternAddressing
{
    uint16_t value;
    struct
    {
        uint8_t T : 3;          /* Fine Y offset, the row number within a tile */
        uint8_t P : 1;          /* Bit plane (0: "lower"; 1: "upper") */
        uint8_t C : 4;          /* Tile column */
        uint8_t R : 4;          /* Tile row */
        uint8_t H : 1;          /* Half of sprite table (0: "left"; 1: "right") */
        uint8_t zero : 1;       /* Pattern table is at $0000-$1FFF */
        uint8_t spare : 2;      /* Unused */
    };
} PPU_PatternAddressing;

#define NES_PIXEL_SIZE  3       /* Number of pixels to draw per NES actual pixel */

st_PPU_Registers* Get_PPU_Registers();
uint8_t ppuAddressRead(uint16_t address);
void ppuAddressWrite(uint16_t address, uint8_t value);
uint8_t ppuRegisterRead(uint8_t* region, uint16_t address);
void ppuRegisterWrite(uint8_t* region, uint16_t address, uint8_t value);
uint8_t ppuPaletteRead(uint8_t* region, uint16_t address);
void ppuPaletteWrite(uint8_t* region, uint16_t address, uint8_t value);
bool ppuInit(st_MemoryMap* map);
bool ppuRunCycle(uint32_t cycle);
bool ppuDestroy();

#endif /* __PPU_H__ */

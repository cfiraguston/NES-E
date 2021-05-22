#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t Memory;

typedef uint8_t(*func_memory_read)(uint8_t* region, uint16_t);
typedef void(*func_memory_write)(uint8_t* region, uint16_t, uint8_t);

typedef struct _tag_st_MemoryRegion
{
	uint16_t offest;
	uint16_t size;
	uint8_t* region;
	func_memory_read func_read;
	func_memory_write func_write;
} st_MemoryRegion;

typedef struct _tag_st_MemoryMap
{
	st_MemoryRegion* Region;
	uint16_t num_of_regions;
	uint16_t num_of_allocated;
} st_MemoryMap;

st_MemoryMap* MemoryMapInit(uint16_t num_of_regions);
bool MemoryMapAdd(st_MemoryMap* MemoryMap, uint16_t offset, uint16_t size, uint8_t* region, func_memory_read func_read, func_memory_write func_write);
uint8_t* MemoryMapGet(st_MemoryMap* MemoryMap, uint16_t offset);
uint8_t MemoryMapRead(st_MemoryMap* MemoryMap, uint16_t address);
void MemoryMapWrite(st_MemoryMap* MemoryMap, uint16_t address, uint8_t value);
uint8_t MemoryRegionRead(uint8_t* region, uint16_t address);
void MemoryRegionWrite(uint8_t* region, uint16_t address, uint8_t value);
bool MemoryMapDestroy(st_MemoryMap* MemoryMap);

#endif	/* __MEMORY_H__ */

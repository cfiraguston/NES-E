#include "memory.h"
#include <stdlib.h>

st_MemoryMap* MemoryMapInit(uint16_t num_of_regions)
{
	st_MemoryMap* map = (st_MemoryMap*)malloc(sizeof(st_MemoryMap) * 1);
	map->Region = (st_MemoryRegion*)malloc(sizeof(st_MemoryRegion) * num_of_regions);
	map->num_of_allocated = 0;
	map->num_of_regions = num_of_regions;
	return map;
}

bool MemoryMapAdd(st_MemoryMap* MemoryMap, uint16_t offset, uint16_t size, uint8_t* region, func_memory_read func_read, func_memory_write func_write)
{
	if (MemoryMap->num_of_regions <= MemoryMap->num_of_allocated)
	{
		return false;
	}

	MemoryMap->Region[MemoryMap->num_of_allocated].offest = offset;
	MemoryMap->Region[MemoryMap->num_of_allocated].size = size;
	MemoryMap->Region[MemoryMap->num_of_allocated].region = region;
	MemoryMap->Region[MemoryMap->num_of_allocated].func_read = func_read;
	MemoryMap->Region[MemoryMap->num_of_allocated].func_write = func_write;
	MemoryMap->num_of_allocated++;

	return true;
}

uint8_t* MemoryMapGet(st_MemoryMap* MemoryMap, uint16_t offset)
{
	for (size_t idx = (MemoryMap->num_of_allocated - 1); idx >= 0; idx--)
	{
		if (MemoryMap->Region[idx].offest == offset)
		{
			return (MemoryMap->Region[idx].region);
		}
	}

	return NULL;
}

uint8_t MemoryMapRead(st_MemoryMap* MemoryMap, uint16_t address)
{
	for (size_t idx = (MemoryMap->num_of_allocated - 1); idx >= 0; idx--)
	{
		if (MemoryMap->Region[idx].offest <= address)
		{
			/*printf("MemoryMapRead: [%04X]\n", address);*/
			return MemoryMap->Region[idx].func_read(
				MemoryMap->Region[idx].region,
				address - (MemoryMap->Region[idx].offest));
		}
	}

	return (0xFF);
}

void MemoryMapWrite(st_MemoryMap* MemoryMap, uint16_t address, uint8_t value)
{
	for (size_t idx = (MemoryMap->num_of_allocated - 1); idx >= 0; idx--)
	{
		if (MemoryMap->Region[idx].offest <= address)
		{
			/*if (address > 0x2000)
				printf("MemoryMapWrite: [%04X]:%02X\n", address, value);*/
			MemoryMap->Region[idx].func_write(
				MemoryMap->Region[idx].region,
				(address - (MemoryMap->Region[idx].offest)), value);
			break;
		}
	}
}

uint8_t MemoryRegionRead(uint8_t* region, uint16_t address)
{
	return (region[address]);
}

void MemoryRegionWrite(uint8_t* region, uint16_t address, uint8_t value)
{
	region[address] = value;
}

bool MemoryMapDestroy(st_MemoryMap* MemoryMap)
{
	free(MemoryMap->Region);
	free(MemoryMap);
	return true;
}

#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include <stdbool.h>

#define SIZE_OF_RAM		2048	/* 2KB internal RAM */

bool emulatorInit();
bool emulatorRun();
bool emulatorDestroy();

#endif	/* __EMULATOR_H__ */
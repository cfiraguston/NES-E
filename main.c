#include "emulator.h"

int main(int argc, char* argv[])
{
	emulatorInit();

	emulatorRun();

	emulatorDestroy();

	return 0;
}
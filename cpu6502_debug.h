#ifndef __CPU6502_DEBUG_H__
#define __CPU6502_DEBUG_H__

#define INSTRUCTION_NAME_LENGTH		7

typedef char*(*func_debug_print)(char*, char*, func_addressing_type);

typedef struct _tag_st_InstructionDebug
{
	char name[INSTRUCTION_NAME_LENGTH];
	func_debug_print print;
} st_InstructionDebug;


char* PrintInstructionJump(char* str, char* name, func_addressing_type func_addressing);
char* PrintInstructionUndocumented(char* str, char* name, func_addressing_type func_addressing);
char* PrintInstruction(char* str, char* name, func_addressing_type func_addressing);
void cpu6502_DebugPrint(st_CPU6502_Instruction* instruction);

#endif /* __CPU6502_DEBUG_H__ */

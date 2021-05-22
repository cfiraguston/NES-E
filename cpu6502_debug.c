
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "cpu6502.h"
#include "cpu6502_debug.h"

static st_InstructionDebug CPU6502_Instruction_Debug[INSTRUCTION_TOTAL] = {
	/* INSTRUCTION_ADC */		{"ADC",			&PrintInstruction},
	/* INSTRUCTION_AND */		{"AND",			&PrintInstruction},
	/* INSTRUCTION_ASL */		{"ASL",			&PrintInstruction},
	/* INSTRUCTION_BCC */		{"BCC",			&PrintInstruction},
	/* INSTRUCTION_BCS */		{"BCS",			&PrintInstruction},
	/* INSTRUCTION_BEQ */		{"BEQ",			&PrintInstruction},
	/* INSTRUCTION_BIT */		{"BIT",			&PrintInstruction},
	/* INSTRUCTION_BMI */		{"BMI",			&PrintInstruction},
	/* INSTRUCTION_BNE */		{"BNE",			&PrintInstruction},
	/* INSTRUCTION_BPL */		{"BPL",			&PrintInstruction},
	/* INSTRUCTION_BRK */		{"BRK",			&PrintInstruction},
	/* INSTRUCTION_BVC */		{"BVC",			&PrintInstruction},
	/* INSTRUCTION_BVS */		{"BVS",			&PrintInstruction},
	/* INSTRUCTION_CLC */		{"CLC",			&PrintInstruction},
	/* INSTRUCTION_CLD */		{"CLD",			&PrintInstruction},
	/* INSTRUCTION_CLI */		{"CLI",			&PrintInstruction},
	/* INSTRUCTION_CLV */		{"CLV",			&PrintInstruction},
	/* INSTRUCTION_CMP */		{"CMP",			&PrintInstruction},
	/* INSTRUCTION_CPX */		{"CPX",			&PrintInstruction},
	/* INSTRUCTION_CPY */		{"CPY",			&PrintInstruction},
	/* INSTRUCTION_DEC */		{"DEC",			&PrintInstruction},
	/* INSTRUCTION_DEX */		{"DEX",			&PrintInstruction},
	/* INSTRUCTION_DEY */		{"DEY",			&PrintInstruction},
	/* INSTRUCTION_EOR */		{"EOR",			&PrintInstruction},
	/* INSTRUCTION_INC */		{"INC",			&PrintInstruction},
	/* INSTRUCTION_INX */		{"INX",			&PrintInstruction},
	/* INSTRUCTION_INY */		{"INY",			&PrintInstruction},
	/* INSTRUCTION_JMP */		{"JMP",			&PrintInstructionJump},
	/* INSTRUCTION_JSR */		{"JSR",			&PrintInstructionJump},
	/* INSTRUCTION_LDA */		{"LDA",			&PrintInstruction},
	/* INSTRUCTION_LDX */		{"LDX",			&PrintInstruction},
	/* INSTRUCTION_LDY */		{"LDY",			&PrintInstruction},
	/* INSTRUCTION_LSR */		{"LSR",			&PrintInstruction},
	/* INSTRUCTION_NOP */		{"NOP",			&PrintInstruction},
	/* INSTRUCTION_ORA */		{"ORA",			&PrintInstruction},
	/* INSTRUCTION_PHA */		{"PHA",			&PrintInstruction},
	/* INSTRUCTION_PHP */		{"PHP",			&PrintInstruction},
	/* INSTRUCTION_PLA */		{"PLA",			&PrintInstruction},
	/* INSTRUCTION_PLP */		{"PLP",			&PrintInstruction},
	/* INSTRUCTION_ROL */		{"ROL",			&PrintInstruction},
	/* INSTRUCTION_ROR */		{"ROR",			&PrintInstruction},
	/* INSTRUCTION_RTI */		{"RTI",			&PrintInstruction},
	/* INSTRUCTION_RTS */		{"RTS",			&PrintInstruction},
	/* INSTRUCTION_SBC */		{"SBC",			&PrintInstruction},
	/* INSTRUCTION_SEC */		{"SEC",			&PrintInstruction},
	/* INSTRUCTION_SED */		{"SED",			&PrintInstruction},
	/* INSTRUCTION_SEI */		{"SEI",			&PrintInstruction},
	/* INSTRUCTION_STA */		{"STA",			&PrintInstruction},
	/* INSTRUCTION_STX */		{"STX",			&PrintInstruction},
	/* INSTRUCTION_STY */		{"STY",			&PrintInstruction},
	/* INSTRUCTION_TAX */		{"TAX",			&PrintInstruction},
	/* INSTRUCTION_TAY */		{"TAY",			&PrintInstruction},
	/* INSTRUCTION_TSX */		{"TSX",			&PrintInstruction},
	/* INSTRUCTION_TXA */		{"TXA",			&PrintInstruction},
	/* INSTRUCTION_TXS */		{"TXS",			&PrintInstruction},
	/* INSTRUCTION_TYA */		{"TYA",			&PrintInstruction},
	/* INSTRUCTION_AAX */		{"SAX",			&PrintInstructionUndocumented},
	/* INSTRUCTION_DCP */		{"DCP",			&PrintInstructionUndocumented},
	/* INSTRUCTION_DOP */		{"NOP",			&PrintInstructionUndocumented},
	/* INSTRUCTION_ISC */		{"ISB",			&PrintInstructionUndocumented},
	/* INSTRUCTION_LAX */		{"LAX",			&PrintInstructionUndocumented},
	/* INSTRUCTION_NOP2*/		{"NOP",			&PrintInstructionUndocumented},
	/* INSTRUCTION_RLA */		{"RLA",			&PrintInstructionUndocumented},
	/* INSTRUCTION_RRA */		{"RRA",			&PrintInstructionUndocumented},
	/* INSTRUCTION_SBC2*/		{"SBC",			&PrintInstructionUndocumented},
	/* INSTRUCTION_SLO */		{"SLO",			&PrintInstructionUndocumented},
	/* INSTRUCTION_SRE */		{"SRE",			&PrintInstructionUndocumented},
	/* INSTRUCTION_TOP */		{"NOP",			&PrintInstructionUndocumented},
	/* INSTRUCTION_UNKNOWN */	{"UNKNOWN",		&PrintInstruction},
};

static char* PrintAddressingAccumulator(char* str, char* name, func_addressing_type func_addressing)
{
	sprintf(str, "A");
	return str;
}

static char* PrintAddressingAbsolute(char* str, char* name, func_addressing_type func_addressing)
{
	sprintf(str, "$%04X = %02X", 
		AddressingOperand16(func_addressing), 
		AddressingOperand8Value(func_addressing));
	return str;
}

static char* PrintAddressingAbsoluteX(char* str, char* name, func_addressing_type func_addressing)
{
	uint8_t ui8LowAddress = cpu6502AddressRead(GetRegisterPC().value + 1);
	uint8_t ui8HighAddress = cpu6502AddressRead(GetRegisterPC().value + 2);
	sprintf(str, "$%04X,X @ %04X = %02X",
		(uint16_t)(ui8LowAddress | (ui8HighAddress << 8)),
		AddressingOperand16(func_addressing),
		AddressingOperand8Value(func_addressing));
	return str;
}

static char* PrintAddressingAbsoluteY(char* str, char* name, func_addressing_type func_addressing)
{
	uint8_t ui8LowAddress = cpu6502AddressRead(GetRegisterPC().value + 1);
	uint8_t ui8HighAddress = cpu6502AddressRead(GetRegisterPC().value + 2);
	sprintf(str, "$%04X,Y @ %04X = %02X",
		(uint16_t)(ui8LowAddress | (ui8HighAddress << 8)),
		AddressingOperand16(func_addressing),
		AddressingOperand8Value(func_addressing));
	return str;
}

static char* PrintAddressingImmediate(char* str, char* name, func_addressing_type func_addressing)
{
	sprintf(str, "#$%02X", AddressingOperand16(func_addressing));
	return str;
}

static char* PrintAddressingImplied(char* str, char* name, func_addressing_type func_addressing)
{
	sprintf(str, "");
	return str;
}

static char* PrintAddressingIndirect(char* str, char* name, func_addressing_type func_addressing)
{
	sprintf(str, "ind");
	return str;
}

static char* PrintAddressingIndirectX(char* str, char* name, func_addressing_type func_addressing)
{
	sprintf(str, "($%02X,X) @ %02X = %04X = %02X", 
		cpu6502AddressRead(GetRegisterPC().value + 1),
		((int8_t)cpu6502AddressRead(GetRegisterPC().value + 1) + (int8_t)GetRegisterX()) & 0xFF,
		AddressingOperand8Address(func_addressing),
		cpu6502AddressRead(AddressingOperand8Address(func_addressing)));
	return str;
}

static char* PrintAddressingIndirectY(char* str, char* name, func_addressing_type func_addressing)
{
	uint16_t operand = cpu6502AddressRead(GetRegisterPC().value + 1);
	uint8_t ui8LowAddress = cpu6502AddressRead(operand & 0xFF);
	uint8_t ui8HighAddress = cpu6502AddressRead((operand + 1) & 0xFF);
	sprintf(str, "($%02X),Y = %04X @ %04X = %02X",
		operand,
		(uint16_t)(ui8LowAddress | (ui8HighAddress << 8)),
		AddressingOperand16(func_addressing),
		AddressingOperand8Value(func_addressing));
	return str;
}

static char* PrintAddressingRelative(char* str, char* name, func_addressing_type func_addressing)
{
	sprintf(str, "$%04X", AddressingOperand16(func_addressing) + 2);
	return str;
}

static char* PrintAddressingZeropage(char* str, char* name, func_addressing_type func_addressing)
{
	sprintf(str, "$%02X = %02X", AddressingOperand8Address(func_addressing), AddressingOperand8Value(func_addressing));
	return str;
}

static char* PrintAddressingZeropageX(char* str, char* name, func_addressing_type func_addressing)
{
	sprintf(str, "$%02X,X @ %02X = %02X",
		cpu6502AddressRead(GetRegisterPC().value + 1),
		AddressingOperand8Address(func_addressing),
		AddressingOperand8Value(func_addressing));
	return str;
}

static char* PrintAddressingZeropageY(char* str, char* name, func_addressing_type func_addressing)
{
	sprintf(str, "$%02X,Y @ %02X = %02X",
		cpu6502AddressRead(GetRegisterPC().value + 1),
		AddressingOperand8Address(func_addressing),
		AddressingOperand8Value(func_addressing));
	return str;
}

typedef struct _tag_st_PrintAddressing
{
	func_addressing_type func_addressing;
	func_debug_print print;
} st_PrintAddressing;

st_PrintAddressing PrintAddressingMap[] = {
	{&AddressingAccumulator,		&PrintAddressingAccumulator},
	{&AddressingAbsolute,			&PrintAddressingAbsolute},
	{&AddressingAbsoluteX,			&PrintAddressingAbsoluteX},
	{&AddressingAbsoluteY,			&PrintAddressingAbsoluteY},
	{&AddressingImmediate,			&PrintAddressingImmediate},
	{&AddressingImplied,			&PrintAddressingImplied},
	{&AddressingIndirect,			&PrintAddressingIndirect},
	{&AddressingIndirectX,			&PrintAddressingIndirectX},
	{&AddressingIndirectY,			&PrintAddressingIndirectY},
	{&AddressingRelative,			&PrintAddressingRelative},
	{&AddressingZeropage,			&PrintAddressingZeropage},
	{&AddressingZeropageX,			&PrintAddressingZeropageX},
	{&AddressingZeropageY,			&PrintAddressingZeropageY}
};

static char* PrintAddressing(char* str, func_addressing_type func_addressing)
{
	for (size_t idx = 0; idx < 13; idx++)
	{
		if (func_addressing == PrintAddressingMap[idx].func_addressing)
		{
			PrintAddressingMap[idx].print(str, NULL, func_addressing);
			return str;
		}
	}
	return NULL;
}

char* PrintInstructionJump(char* str, char* name, func_addressing_type func_addressing)
{
	char addressing[20] = { '\0' };
	if (func_addressing == &AddressingAbsolute)
	{
		sprintf(str, " %s $%04X", name, AddressingOperand16(func_addressing));
	}
	else if (func_addressing == &AddressingIndirect)
	{
		uint8_t ui8LowAddress = cpu6502AddressRead(GetRegisterPC().value + 1);
		uint8_t ui8HighAddress = cpu6502AddressRead(GetRegisterPC().value + 2);
		sprintf(str, " %s ($%04X) = %04X", 
			name, 
			(uint16_t)(ui8LowAddress | (ui8HighAddress << 8)),
			AddressingOperand16(func_addressing));
	}

	return str;
}

char* PrintInstructionUndocumented(char* str, char* name, func_addressing_type func_addressing)
{
	char addressing[50] = { '\0' };
	PrintAddressing(addressing, func_addressing);
	sprintf(str, "*%s %s", name, addressing);
	return str;
}

char* PrintInstruction(char* str, char* name, func_addressing_type func_addressing)
{
	char addressing[50] = { '\0' };
	PrintAddressing(addressing, func_addressing);
	sprintf(str, " %s %s", name, addressing);
	return str;
}

void cpu6502_DebugPrint(st_CPU6502_Instruction* instruction)
{
	static bool first = false;
	static FILE* fd = NULL;
	char line[100] = { '\0' };
	uint32_t line_idx = 0;
	uint8_t idx = 0;

	if (first == false)
	{
		fd = fopen("CPURESULTS.TXT", "w");
		first = true;
	}

	line_idx += sprintf(&(line[line_idx]), "%04X  ", GetRegisterPC().value);

	for (idx = 0; idx < instruction->bytes; idx++)
	{
		line_idx += sprintf(&(line[line_idx]), "%02X ", cpu6502AddressRead(GetRegisterPC().value + idx));
	}

	for (; idx < 3; idx++)
	{
		line_idx += sprintf(&(line[line_idx]), "   ");
	}

	/*line_idx += sprintf(&(line[line_idx]), " ");*/

	line_idx += sprintf(&(line[line_idx]), "%s ", 
		CPU6502_Instruction_Debug[instruction->instr].print(
			&(line[line_idx]),
			CPU6502_Instruction_Debug[instruction->instr].name,
			instruction->func_addressing)
		);

	while (line_idx < 48)
	{
		line_idx += sprintf(&(line[line_idx]), " ");
	}

	/*line_idx += sprintf(&(line[line_idx]), "\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X [0x01FF]=%02X [0x01FE]=%02X",
		AC, X, Y, SR.value, SP, AddressRead(0x01FF), AddressRead(0x01FE));*/
		/* CURRENTLY SP IS NOT PRINTED */
	line_idx += sprintf(&(line[line_idx]), "A:%02X X:%02X Y:%02X P:%02X SP:%02X",
		GetRegisterAC(), GetRegisterX(), GetRegisterY(), GetRegisterSR().value, GetRegisterSP());

	/* complete print assigment of values: ST*, BIT, LDA, LDX*/

	line_idx += sprintf(&(line[line_idx]), "\n");

	fprintf(fd, line);
	fflush(fd);

	printf("%s", line);
}


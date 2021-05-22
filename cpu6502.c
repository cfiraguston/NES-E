/* Based on: https://www.masswerk.at/6502/6502_instruction_set.html */
/* Based on: http://www.6502.org/tutorials/6502opcodes.html */
/* Based on: http://nesdev.com/undocumented_opcodes.txt */
/* Based on: http://ist.uwaterloo.ca/~schepers/MJK/ascii/65xx_ill.txt */

/* TODO:
1. Extra cycle when page boundries acheived
2. Undocumented opcodes
*/

#include <stdio.h>
#include "cpu6502.h"
#include "cpu6502_debug.h"
#include "memory.h"

static Reg16 PC;		/* Program counter register (16-bits) */
static Reg8 AC;			/* Accumulator register (8-bits) */
static Reg8 X;			/* X register (8-bits) */
static Reg8 Y;			/* Y register (8-bits) */
static RegStatus SR;	/* status register [NV-BDIZC] (8-bits) */
static Reg8 SP;			/* Stack pointer register (8-bit) */

static st_CPU6502_Instruction CPU6502_Instruction_Set[256] = {
	/*					 Opcode func		Addressing func			bytes	cycles	Instruction				*/
	/*					----------------------------------------------------------------------------------- */
	/* 0x00	- BRK */	{&cpu6502_BRK,		&AddressingImplied,		1,		7,		INSTRUCTION_BRK},
	/* 0x01	- ORA */	{&cpu6502_ORA,		&AddressingIndirectX,	2,		6,		INSTRUCTION_ORA},
	/* 0x02		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x03 -*SLO */	{&cpu6502_SLO,		&AddressingIndirectX,	2,		8,		INSTRUCTION_SLO},
	/* 0x04 -*DOP */	{&cpu6502_DOP,		&AddressingZeropage,	2,		3,		INSTRUCTION_DOP},
	/* 0x05	- ORA */	{&cpu6502_ORA,		&AddressingZeropage,	2,		3,		INSTRUCTION_ORA},
	/* 0x06	- ASL */	{&cpu6502_ASL,		&AddressingZeropage,	2,		5,		INSTRUCTION_ASL},
	/* 0x07 -*SLO */	{&cpu6502_SLO,		&AddressingZeropage,	2,		5,		INSTRUCTION_SLO},
	/* 0x08	- PHP */	{&cpu6502_PHP,		&AddressingImplied,		1,		3,		INSTRUCTION_PHP},
	/* 0x09	- ORA */	{&cpu6502_ORA,		&AddressingImmediate,	2,		2,		INSTRUCTION_ORA},
	/* 0x0A	- ASL */	{&cpu6502_ASL,		&AddressingAccumulator,	1,		2,		INSTRUCTION_ASL},
	/* 0x0B		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x0C -*TOP */	{&cpu6502_TOP,		&AddressingAbsolute,	3,		4,		INSTRUCTION_TOP},
	/* 0x0D	- ORA */	{&cpu6502_ORA,		&AddressingAbsolute,	3,		4,		INSTRUCTION_ORA},
	/* 0x0E	- ASL */	{&cpu6502_ASL,		&AddressingAbsolute,	3,		6,		INSTRUCTION_ASL},
	/* 0x0F -*SLO */	{&cpu6502_SLO,		&AddressingAbsolute,	3,		6,		INSTRUCTION_SLO},
	/* 0x10	- BPL */	{&cpu6502_BPL,		&AddressingRelative,	2,		2,		INSTRUCTION_BPL},
	/* 0x11	- ORA */	{&cpu6502_ORA,		&AddressingIndirectY,	2,		5,		INSTRUCTION_ORA},
	/* 0x12		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x13 -*SLO */	{&cpu6502_SLO,		&AddressingIndirectY,	2,		8,		INSTRUCTION_SLO},
	/* 0x14 -*DOP */	{&cpu6502_DOP,		&AddressingZeropageX,	2,		4,		INSTRUCTION_DOP},
	/* 0x15	- ORA */	{&cpu6502_ORA,		&AddressingZeropageX,	2,		4,		INSTRUCTION_ORA},
	/* 0x16	- ASL */	{&cpu6502_ASL,		&AddressingZeropageX,	2,		6,		INSTRUCTION_ASL},
	/* 0x17 -*SLO */	{&cpu6502_SLO,		&AddressingZeropageX,	2,		6,		INSTRUCTION_SLO},
	/* 0x18	- CLC */	{&cpu6502_CLC,		&AddressingImplied,		1,		2,		INSTRUCTION_CLC},
	/* 0x19	- ORA */	{&cpu6502_ORA,		&AddressingAbsoluteY,	3,		4,		INSTRUCTION_ORA},
	/* 0x1A -*NOP */	{&cpu6502_NOP,		&AddressingImplied,		1,		2,		INSTRUCTION_NOP2},
	/* 0x1B -*SLO */	{&cpu6502_SLO,		&AddressingAbsoluteY,	3,		7,		INSTRUCTION_SLO},
	/* 0x1C -*TOP */	{&cpu6502_TOP,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_TOP},
	/* 0x1D	- ORA */	{&cpu6502_ORA,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_ORA},
	/* 0x1E	- ASL */	{&cpu6502_ASL,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_ASL},
	/* 0x1F -*SLO */	{&cpu6502_SLO,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_SLO},
	/* 0x20	- JSR */	{&cpu6502_JSR,		&AddressingAbsolute,	3,		6,		INSTRUCTION_JSR},
	/* 0x21	- AND */	{&cpu6502_AND,		&AddressingIndirectX,	2,		6,		INSTRUCTION_AND},
	/* 0x22		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x23 -*RLA */	{&cpu6502_RLA,		&AddressingIndirectX,	2,		8,		INSTRUCTION_RLA},
	/* 0x24	- BIT */	{&cpu6502_BIT,		&AddressingZeropage,	2,		3,		INSTRUCTION_BIT},
	/* 0x25	- AND */	{&cpu6502_AND,		&AddressingZeropage,	2,		3,		INSTRUCTION_AND},
	/* 0x26	- ROL */	{&cpu6502_ROL,		&AddressingZeropage,	2,		5,		INSTRUCTION_ROL},
	/* 0x27 -*RLA */	{&cpu6502_RLA,		&AddressingZeropage,	2,		5,		INSTRUCTION_RLA},
	/* 0x28 - PLP */	{&cpu6502_PLP,		&AddressingImplied,		1,		4,		INSTRUCTION_PLP},
	/* 0x29	- AND */	{&cpu6502_AND,		&AddressingImmediate,	2,		2,		INSTRUCTION_AND},
	/* 0x2A	- ROL */	{&cpu6502_ROL,		&AddressingAccumulator,	1,		2,		INSTRUCTION_ROL},
	/* 0x2B		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x2C	- BIT */	{&cpu6502_BIT,		&AddressingAbsolute,	3,		4,		INSTRUCTION_BIT},
	/* 0x2D	- AND */	{&cpu6502_AND,		&AddressingAbsolute,	3,		4,		INSTRUCTION_AND},
	/* 0x2E	- ROL */	{&cpu6502_ROL,		&AddressingAbsolute,	3,		6,		INSTRUCTION_ROL},
	/* 0x2F -*RLA */	{&cpu6502_RLA,		&AddressingAbsolute,	3,		6,		INSTRUCTION_RLA},
	/* 0x30	- BMI */	{&cpu6502_BMI,		&AddressingRelative,	2,		2,		INSTRUCTION_BMI},
	/* 0x31	- AND */	{&cpu6502_AND,		&AddressingIndirectY,	2,		5,		INSTRUCTION_AND},
	/* 0x32		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x33 -*RLA */	{&cpu6502_RLA,		&AddressingIndirectY,	2,		8,		INSTRUCTION_RLA},
	/* 0x34 -*DOP */	{&cpu6502_DOP,		&AddressingZeropageX,	2,		4,		INSTRUCTION_DOP},
	/* 0x35	- AND */	{&cpu6502_AND,		&AddressingZeropageX,	2,		4,		INSTRUCTION_AND},
	/* 0x36	- ROL */	{&cpu6502_ROL,		&AddressingZeropageX,	2,		6,		INSTRUCTION_ROL},
	/* 0x37 -*RLA */	{&cpu6502_RLA,		&AddressingZeropageX,	2,		6,		INSTRUCTION_RLA},
	/* 0x38	- SEC */	{&cpu6502_SEC,		&AddressingImplied,		1,		2,		INSTRUCTION_SEC},
	/* 0x39	- AND */	{&cpu6502_AND,		&AddressingAbsoluteY,	3,		4,		INSTRUCTION_AND},
	/* 0x3A -*NOP */	{&cpu6502_NOP,		&AddressingImplied,		1,		2,		INSTRUCTION_NOP2},
	/* 0x3B -*RLA */	{&cpu6502_RLA,		&AddressingAbsoluteY,	3,		7,		INSTRUCTION_RLA},
	/* 0x3C -*TOP */	{&cpu6502_TOP,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_TOP},
	/* 0x3D	- AND */	{&cpu6502_AND,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_AND},
	/* 0x3E	- ROL */	{&cpu6502_ROL,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_ROL},
	/* 0x3F -*RLA */	{&cpu6502_RLA,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_RLA},
	/* 0x40	- RTI */	{&cpu6502_RTI,		&AddressingImplied,		1,		6,		INSTRUCTION_RTI},
	/* 0x41	- EOR */	{&cpu6502_EOR,		&AddressingIndirectX,	2,		6,		INSTRUCTION_EOR},
	/* 0x42		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x43 -*SRE */	{&cpu6502_SRE,		&AddressingIndirectX,	2,		8,		INSTRUCTION_SRE},
	/* 0x44 -*DOP */	{&cpu6502_DOP,		&AddressingZeropage,	2,		3,		INSTRUCTION_DOP},
	/* 0x45	- EOR */	{&cpu6502_EOR,		&AddressingZeropage,	2,		3,		INSTRUCTION_EOR},
	/* 0x46	- LSR */	{&cpu6502_LSR,		&AddressingZeropage,	2,		5,		INSTRUCTION_LSR},
	/* 0x47 -*SRE */	{&cpu6502_SRE,		&AddressingZeropage,	2,		5,		INSTRUCTION_SRE},
	/* 0x48	- PHA */	{&cpu6502_PHA,		&AddressingImplied,		1,		3,		INSTRUCTION_PHA},
	/* 0x49	- EOR */	{&cpu6502_EOR,		&AddressingImmediate,	2,		2,		INSTRUCTION_EOR},
	/* 0x4A	- LSR */	{&cpu6502_LSR,		&AddressingAccumulator,	1,		2,		INSTRUCTION_LSR},
	/* 0x4B		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x4C	- JMP */	{&cpu6502_JMP,		&AddressingAbsolute,	3,		3,		INSTRUCTION_JMP},
	/* 0x4D	- EOR */	{&cpu6502_EOR,		&AddressingAbsolute,	3,		4,		INSTRUCTION_EOR},
	/* 0x4E	- LSR */	{&cpu6502_LSR,		&AddressingAbsolute,	3,		6,		INSTRUCTION_LSR},
	/* 0x4F -*SRE */	{&cpu6502_SRE,		&AddressingAbsolute,	3,		6,		INSTRUCTION_SRE},
	/* 0x50	- BVC */	{&cpu6502_BVC,		&AddressingRelative,	2,		2,		INSTRUCTION_BVC},
	/* 0x51	- EOR */	{&cpu6502_EOR,		&AddressingIndirectY,	2,		5,		INSTRUCTION_EOR},
	/* 0x52		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x53 -*SRE */	{&cpu6502_SRE,		&AddressingIndirectY,	2,		8,		INSTRUCTION_SRE},
	/* 0x54 -*DOP */	{&cpu6502_DOP,		&AddressingZeropageX,	2,		4,		INSTRUCTION_DOP},
	/* 0x55	- EOR */	{&cpu6502_EOR,		&AddressingZeropageX,	2,		4,		INSTRUCTION_EOR},
	/* 0x56	- LSR */	{&cpu6502_LSR,		&AddressingZeropageX,	2,		6,		INSTRUCTION_LSR},
	/* 0x57 -*SRE */	{&cpu6502_SRE,		&AddressingZeropageX,	2,		6,		INSTRUCTION_SRE},
	/* 0x58	- CLI */	{&cpu6502_CLI,		&AddressingImplied,		1,		2,		INSTRUCTION_CLI},
	/* 0x59	- EOR */	{&cpu6502_EOR,		&AddressingAbsoluteY,	3,		4,		INSTRUCTION_EOR},
	/* 0x5A -*NOP */	{&cpu6502_NOP,		&AddressingImplied,		1,		2,		INSTRUCTION_NOP2},
	/* 0x5B -*SRE */	{&cpu6502_SRE,		&AddressingAbsoluteY,	3,		7,		INSTRUCTION_SRE},
	/* 0x5C -*TOP */	{&cpu6502_TOP,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_TOP},
	/* 0x5D	- EOR */	{&cpu6502_EOR,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_EOR},
	/* 0x5E	- LSR */	{&cpu6502_LSR,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_LSR},
	/* 0x5F -*SRE */	{&cpu6502_SRE,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_SRE},
	/* 0x60	- RTS */	{&cpu6502_RTS,		&AddressingImplied,		1,		6,		INSTRUCTION_RTS},
	/* 0x61	- ADC */	{&cpu6502_ADC,		&AddressingIndirectX,	2,		6,		INSTRUCTION_ADC},
	/* 0x62		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x63 -*RRA */	{&cpu6502_RRA,		&AddressingIndirectX,	2,		8,		INSTRUCTION_RRA },
	/* 0x64 -*DOP */	{&cpu6502_DOP,		&AddressingZeropage,	2,		3,		INSTRUCTION_DOP},
	/* 0x65	- ADC */	{&cpu6502_ADC,		&AddressingZeropage,	2,		3,		INSTRUCTION_ADC},
	/* 0x66	- ROR */	{&cpu6502_ROR,		&AddressingZeropage,	2,		5,		INSTRUCTION_ROR},
	/* 0x67 -*RRA */	{&cpu6502_RRA,		&AddressingZeropage,	2,		5,		INSTRUCTION_RRA},
	/* 0x68	- PLA */	{&cpu6502_PLA,		&AddressingImplied,		1,		4,		INSTRUCTION_PLA},
	/* 0x69	- ADC */	{&cpu6502_ADC,		&AddressingImmediate,	2,		2,		INSTRUCTION_ADC},
	/* 0x6A	- ROR */	{&cpu6502_ROR,		&AddressingAccumulator,	1,		2,		INSTRUCTION_ROR},
	/* 0x6B		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x6C	- JMP */	{&cpu6502_JMP,		&AddressingIndirect,	3,		5,		INSTRUCTION_JMP},
	/* 0x6D	- ADC */	{&cpu6502_ADC,		&AddressingAbsolute,	3,		4,		INSTRUCTION_ADC},
	/* 0x6E	- ROR */	{&cpu6502_ROR,		&AddressingAbsolute,	3,		6,		INSTRUCTION_ROR},
	/* 0x6F -*RRA */	{&cpu6502_RRA,		&AddressingAbsolute,	3,		6,		INSTRUCTION_RRA},
	/* 0x70	- BVS */	{&cpu6502_BVS,		&AddressingRelative,	2,		2,		INSTRUCTION_BVS},
	/* 0x71	- ADC */	{&cpu6502_ADC,		&AddressingIndirectY,	2,		5,		INSTRUCTION_ADC},
	/* 0x72		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x73 -*RRA */	{&cpu6502_RRA,		&AddressingIndirectY,	2,		8,		INSTRUCTION_RRA},
	/* 0x74 -*DOP */	{&cpu6502_DOP,		&AddressingZeropageX,	2,		4,		INSTRUCTION_DOP},
	/* 0x75	- ADC */	{&cpu6502_ADC,		&AddressingZeropageX,	2,		4,		INSTRUCTION_ADC},
	/* 0x76	- ROR */	{&cpu6502_ROR,		&AddressingZeropageX,	2,		6,		INSTRUCTION_ROR},
	/* 0x77 -*RRA */	{&cpu6502_RRA,		&AddressingZeropageX,	2,		6,		INSTRUCTION_RRA},
	/* 0x78	- SEI */	{&cpu6502_SEI,		&AddressingImplied,		1,		2,		INSTRUCTION_SEI},
	/* 0x79	- ADC */	{&cpu6502_ADC,		&AddressingAbsoluteY,	3,		4,		INSTRUCTION_ADC},
	/* 0x7A -*NOP */	{&cpu6502_NOP,		&AddressingImplied,		1,		2,		INSTRUCTION_NOP2},
	/* 0x7B -*RRA */	{&cpu6502_RRA,		&AddressingAbsoluteY,	3,		7,		INSTRUCTION_RRA},
	/* 0x7C -*TOP */	{&cpu6502_TOP,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_TOP},
	/* 0x7D	- ADC */	{&cpu6502_ADC,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_ADC},
	/* 0x7E	- ROR */	{&cpu6502_ROR,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_ROR},
	/* 0x7F -*RRA */	{&cpu6502_RRA,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_RRA},
	/* 0x80 -*DOP */	{&cpu6502_DOP,		&AddressingImmediate,	2,		2,		INSTRUCTION_DOP},
	/* 0x81	- STA */	{&cpu6502_STA,		&AddressingIndirectX,	2,		6,		INSTRUCTION_STA},
	/* 0x82 -*DOP */	{&cpu6502_DOP,		&AddressingImmediate,	2,		2,		INSTRUCTION_DOP},
	/* 0x83 -*AAX */	{&cpu6502_AAX,		&AddressingIndirectX,	2,		6,		INSTRUCTION_AAX},
	/* 0x84	- STY */	{&cpu6502_STY,		&AddressingZeropage,	2,		3,		INSTRUCTION_STY},
	/* 0x85	- STA */	{&cpu6502_STA,		&AddressingZeropage,	2,		3,		INSTRUCTION_STA},
	/* 0x86	- STX */	{&cpu6502_STX,		&AddressingZeropage,	2,		3,		INSTRUCTION_STX},
	/* 0x87 -*AAX */	{&cpu6502_AAX,		&AddressingZeropage,	2,		3,		INSTRUCTION_AAX},
	/* 0x88	- DEY */	{&cpu6502_DEY,		&AddressingImplied,		1,		2,		INSTRUCTION_DEY},
	/* 0x89 -*DOP */	{&cpu6502_DOP,		&AddressingImmediate,	2,		2,		INSTRUCTION_DOP},
	/* 0x8A	- TXA */	{&cpu6502_TXA,		&AddressingImplied,		1,		2,		INSTRUCTION_TXA},
	/* 0x8B		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x8C	- STY */	{&cpu6502_STY,		&AddressingAbsolute,	3,		4,		INSTRUCTION_STY},
	/* 0x8D	- STA */	{&cpu6502_STA,		&AddressingAbsolute,	3,		4,		INSTRUCTION_STA},
	/* 0x8E	- STX */	{&cpu6502_STX,		&AddressingAbsolute,	3,		4,		INSTRUCTION_STX},
	/* 0x8F -*AAX */	{&cpu6502_AAX,		&AddressingAbsolute,	3,		4,		INSTRUCTION_AAX},
	/* 0x90	- BCC */	{&cpu6502_BCC,		&AddressingRelative,	2,		2,		INSTRUCTION_BCC},
	/* 0x91	- STA */	{&cpu6502_STA,		&AddressingIndirectY,	2,		6,		INSTRUCTION_STA},
	/* 0x92		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x93		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x94	- STY */	{&cpu6502_STY,		&AddressingZeropageX,	2,		4,		INSTRUCTION_STY},
	/* 0x95	- STA */	{&cpu6502_STA,		&AddressingZeropageX,	2,		4,		INSTRUCTION_STA},
	/* 0x96	- STX */	{&cpu6502_STX,		&AddressingZeropageY,	2,		4,		INSTRUCTION_STX},
	/* 0x97 -*AAX */	{&cpu6502_AAX,		&AddressingZeropageY,	2,		4,		INSTRUCTION_AAX},
	/* 0x98	- TYA */	{&cpu6502_TYA,		&AddressingImplied,		1,		2,		INSTRUCTION_TYA},
	/* 0x99	- STA */	{&cpu6502_STA,		&AddressingAbsoluteY,	3,		5,		INSTRUCTION_STA},
	/* 0x9A	- TXS */	{&cpu6502_TXS,		&AddressingImplied,		1,		2,		INSTRUCTION_TXS},
	/* 0x9B		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x9C		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x9D	- STA */	{&cpu6502_STA,		&AddressingAbsoluteX,	3,		5,		INSTRUCTION_STA},
	/* 0x9E		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0x9F		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0xA0	- LDY */	{&cpu6502_LDY,		&AddressingImmediate,	2,		2,		INSTRUCTION_LDY},
	/* 0xA1	- LDA */	{&cpu6502_LDA,		&AddressingIndirectX,	2,		6,		INSTRUCTION_LDA},
	/* 0xA2	- LDX */	{&cpu6502_LDX,		&AddressingImmediate,	2,		2,		INSTRUCTION_LDX},
	/* 0xA3 -*LAX */	{&cpu6502_LAX,		&AddressingIndirectX,	2,		6,		INSTRUCTION_LAX},
	/* 0xA4	- LDY */	{&cpu6502_LDY,		&AddressingZeropage,	2,		3,		INSTRUCTION_LDY},
	/* 0xA5	- LDA */	{&cpu6502_LDA,		&AddressingZeropage,	2,		3,		INSTRUCTION_LDA},
	/* 0xA6	- LDX */	{&cpu6502_LDX,		&AddressingZeropage,	2,		3,		INSTRUCTION_LDX},
	/* 0xA7 -*LAX */	{&cpu6502_LAX,		&AddressingZeropage,	2,		3,		INSTRUCTION_LAX},
	/* 0xA8	- TAY */	{&cpu6502_TAY,		&AddressingImplied,		1,		2,		INSTRUCTION_TAY},
	/* 0xA9	- LDA */	{&cpu6502_LDA,		&AddressingImmediate,	2,		2,		INSTRUCTION_LDA},
	/* 0xAA	- TAX */	{&cpu6502_TAX,		&AddressingImplied,		1,		2,		INSTRUCTION_TAX},
	/* 0xAB		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0xAC	- LDY */	{&cpu6502_LDY,		&AddressingAbsolute,	3,		4,		INSTRUCTION_LDY},
	/* 0xAD	- LDA */	{&cpu6502_LDA,		&AddressingAbsolute,	3,		4,		INSTRUCTION_LDA},
	/* 0xAE	- LDX */	{&cpu6502_LDX,		&AddressingAbsolute,	3,		4,		INSTRUCTION_LDX},
	/* 0xAF -*LAX */	{&cpu6502_LAX,		&AddressingAbsolute,	3,		4,		INSTRUCTION_LAX},
	/* 0xB0	- BCS */	{&cpu6502_BCS,		&AddressingRelative,	2,		2,		INSTRUCTION_BCS},
	/* 0xB1	- LDA */	{&cpu6502_LDA,		&AddressingIndirectY,	2,		5,		INSTRUCTION_LDA},
	/* 0xB2		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0xB3 -*LAX */	{&cpu6502_LAX,		&AddressingIndirectY,	2,		5,		INSTRUCTION_LAX},
	/* 0xB4	- LDY */	{&cpu6502_LDY,		&AddressingZeropageX,	2,		4,		INSTRUCTION_LDY},
	/* 0xB5	- LDA */	{&cpu6502_LDA,		&AddressingZeropageX,	2,		4,		INSTRUCTION_LDA},
	/* 0xB6	- LDX */	{&cpu6502_LDX,		&AddressingZeropageY,	2,		4,		INSTRUCTION_LDX},
	/* 0xB7 -*LAX */	{&cpu6502_LAX,		&AddressingZeropageY,	2,		4,		INSTRUCTION_LAX},
	/* 0xB8	- CLV */	{&cpu6502_CLV,		&AddressingImplied,		1,		2,		INSTRUCTION_CLV},
	/* 0xB9	- LDA */	{&cpu6502_LDA,		&AddressingAbsoluteY,	3,		4,		INSTRUCTION_LDA},
	/* 0xBA	- TSX */	{&cpu6502_TSX,		&AddressingImplied,		1,		2,		INSTRUCTION_TSX},
	/* 0xBB		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0xBC	- LDY */	{&cpu6502_LDY,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_LDY},
	/* 0xBD	- LDA */	{&cpu6502_LDA,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_LDA},
	/* 0xBE	- LDX */	{&cpu6502_LDX,		&AddressingAbsoluteY,	3,		4,		INSTRUCTION_LDX},
	/* 0xBF -*LAX */	{&cpu6502_LAX,		&AddressingAbsoluteY,	3,		4,		INSTRUCTION_LAX},
	/* 0xC0	- CPY */	{&cpu6502_CPY,		&AddressingImmediate,	2,		2,		INSTRUCTION_CPY},
	/* 0xC1	- CMP */	{&cpu6502_CMP,		&AddressingIndirectX,	2,		6,		INSTRUCTION_CMP},
	/* 0xC2 -*DOP */	{&cpu6502_DOP,		&AddressingImmediate,	2,		2,		INSTRUCTION_DOP},
	/* 0xC3 -*DCP */	{&cpu6502_DCP,		&AddressingIndirectX,	2,		8,		INSTRUCTION_DCP},
	/* 0xC4	- CPY */	{&cpu6502_CPY,		&AddressingZeropage,	2,		3,		INSTRUCTION_CPY},
	/* 0xC5	- CMP */	{&cpu6502_CMP,		&AddressingZeropage,	2,		3,		INSTRUCTION_CMP},
	/* 0xC6	- DEC */	{&cpu6502_DEC,		&AddressingZeropage,	2,		5,		INSTRUCTION_DEC},
	/* 0xC7 -*DCP */	{&cpu6502_DCP,		&AddressingZeropage,	2,		5,		INSTRUCTION_DCP},
	/* 0xC8	- INY */	{&cpu6502_INY,		&AddressingImplied,		1,		2,		INSTRUCTION_INY},
	/* 0xC9	- CMP */	{&cpu6502_CMP,		&AddressingImmediate,	2,		2,		INSTRUCTION_CMP},
	/* 0xCA	- DEX */	{&cpu6502_DEX,		&AddressingImplied,		1,		2,		INSTRUCTION_DEX},
	/* 0xCB		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0xCC	- CPY */	{&cpu6502_CPY,		&AddressingAbsolute,	3,		4,		INSTRUCTION_CPY},
	/* 0xCD	- CMP */	{&cpu6502_CMP,		&AddressingAbsolute,	3,		4,		INSTRUCTION_CMP},
	/* 0xCE	- DEC */	{&cpu6502_DEC,		&AddressingAbsolute,	3,		6,		INSTRUCTION_DEC},
	/* 0xCF -*DCP */	{&cpu6502_DCP,		&AddressingAbsolute,	3,		6,		INSTRUCTION_DCP},
	/* 0xD0	- BNE */	{&cpu6502_BNE,		&AddressingRelative,	2,		2,		INSTRUCTION_BNE},
	/* 0xD1	- CMP */	{&cpu6502_CMP,		&AddressingIndirectY,	2,		5,		INSTRUCTION_CMP},
	/* 0xD2		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0xD3 -*DCP */	{&cpu6502_DCP,		&AddressingIndirectY,	2,		8,		INSTRUCTION_DCP},
	/* 0xD4 -*DOP */	{&cpu6502_DOP,		&AddressingZeropageX,	2,		4,		INSTRUCTION_DOP},
	/* 0xD5	- CMP */	{&cpu6502_CMP,		&AddressingZeropageX,	2,		4,		INSTRUCTION_CMP},
	/* 0xD6	- DEC */	{&cpu6502_DEC,		&AddressingZeropageX,	2,		6,		INSTRUCTION_DEC},
	/* 0xD7 -*DCP */	{&cpu6502_DCP,		&AddressingZeropageX,	2,		6,		INSTRUCTION_DCP},
	/* 0xD8	- CLD */	{&cpu6502_CLD,		&AddressingImplied,		1,		2,		INSTRUCTION_CLD},
	/* 0xD9	- CMP */	{&cpu6502_CMP,		&AddressingAbsoluteY,	3,		4,		INSTRUCTION_CMP},
	/* 0xDA -*NOP */	{&cpu6502_NOP,		&AddressingImplied,		1,		2,		INSTRUCTION_NOP2},
	/* 0xDB -*DCP */	{&cpu6502_DCP,		&AddressingAbsoluteY,	3,		7,		INSTRUCTION_DCP},
	/* 0xDC -*TOP */	{&cpu6502_TOP,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_TOP},
	/* 0xDD	- CMP */	{&cpu6502_CMP,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_CMP},
	/* 0xDE	- DEC */	{&cpu6502_DEC,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_DEC},
	/* 0xDF -*DCP */	{&cpu6502_DCP,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_DCP},
	/* 0xE0	- CPX */	{&cpu6502_CPX,		&AddressingImmediate,	2,		2,		INSTRUCTION_CPX},
	/* 0xE1	- SBC */	{&cpu6502_SBC,		&AddressingIndirectX,	2,		6,		INSTRUCTION_SBC},
	/* 0xE2 -*DOP */	{&cpu6502_DOP,		&AddressingImmediate,	2,		2,		INSTRUCTION_DOP},
	/* 0xE3 -*ISC */	{&cpu6502_ISC,		&AddressingIndirectX,	2,		8,		INSTRUCTION_ISC},
	/* 0xE4	- CPX */	{&cpu6502_CPX,		&AddressingZeropage,	2,		3,		INSTRUCTION_CPX},
	/* 0xE5	- SBC */	{&cpu6502_SBC,		&AddressingZeropage,	2,		3,		INSTRUCTION_SBC},
	/* 0xE6	- INC */	{&cpu6502_INC,		&AddressingZeropage,	2,		5,		INSTRUCTION_INC},
	/* 0xE7 -*ISC */	{&cpu6502_ISC,		&AddressingZeropage,	2,		5,		INSTRUCTION_ISC},
	/* 0xE8	- INX */	{&cpu6502_INX,		&AddressingImplied,		1,		2,		INSTRUCTION_INX},
	/* 0xE9	- SBC */	{&cpu6502_SBC,		&AddressingImmediate,	2,		2,		INSTRUCTION_SBC},
	/* 0xEA	- NOP */	{&cpu6502_NOP,		&AddressingImplied,		1,		2,		INSTRUCTION_NOP},
	/* 0xEB	-*SBC */	{&cpu6502_SBC,		&AddressingImmediate,	2,		2,		INSTRUCTION_SBC2},
	/* 0xEC	- CPX */	{&cpu6502_CPX,		&AddressingAbsolute,	3,		4,		INSTRUCTION_CPX},
	/* 0xED	- SBC */	{&cpu6502_SBC,		&AddressingAbsolute,	3,		4,		INSTRUCTION_SBC},
	/* 0xEE	- INC */	{&cpu6502_INC,		&AddressingAbsolute,	3,		6,		INSTRUCTION_INC},
	/* 0xEF -*ISC */	{&cpu6502_ISC,		&AddressingAbsolute,	3,		6,		INSTRUCTION_ISC},
	/* 0xF0	- BEQ */	{&cpu6502_BEQ,		&AddressingRelative,	2,		2,		INSTRUCTION_BEQ},
	/* 0xF1	- SBC */	{&cpu6502_SBC,		&AddressingIndirectY,	2,		5,		INSTRUCTION_SBC},
	/* 0xF2		  */	{&cpu6502_UNKNOWN,	NULL,					1,		1,		INSTRUCTION_UNKNOWN},
	/* 0xF3 -*ISC */	{&cpu6502_ISC,		&AddressingIndirectY,	2,		8,		INSTRUCTION_ISC},
	/* 0xF4 -*DOP */	{&cpu6502_DOP,		&AddressingZeropageX,	2,		4,		INSTRUCTION_DOP},
	/* 0xF5	- SBC */	{&cpu6502_SBC,		&AddressingZeropageX,	2,		4,		INSTRUCTION_SBC},
	/* 0xF6	- INC */	{&cpu6502_INC,		&AddressingZeropageX,	2,		6,		INSTRUCTION_INC},
	/* 0xF7 -*ISC */	{&cpu6502_ISC,		&AddressingZeropageX,	2,		6,		INSTRUCTION_ISC},
	/* 0xF8	- SED */	{&cpu6502_SED,		&AddressingImplied,		1,		2,		INSTRUCTION_SED},
	/* 0xF9	- SBC */	{&cpu6502_SBC,		&AddressingAbsoluteY,	3,		4,		INSTRUCTION_SBC},
	/* 0xFA -*NOP */	{&cpu6502_NOP,		&AddressingImplied,		1,		2,		INSTRUCTION_NOP2},
	/* 0xFB -*ISC */	{&cpu6502_ISC,		&AddressingAbsoluteY,	3,		7,		INSTRUCTION_ISC},
	/* 0xFC -*TOP */	{&cpu6502_TOP,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_TOP},
	/* 0xFD	- SBC */	{&cpu6502_SBC,		&AddressingAbsoluteX,	3,		4,		INSTRUCTION_SBC},
	/* 0xFE	- INC */	{&cpu6502_INC,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_INC},
	/* 0xFF -*ISC */	{&cpu6502_ISC,		&AddressingAbsoluteX,	3,		7,		INSTRUCTION_ISC},
};

static st_MemoryMap* MemoryMap = NULL;

uint8_t cpu6502AddressRead(uint16_t address)
{
	return MemoryMapRead(MemoryMap, address);
}

void cpu6502AddressWrite(uint16_t address, uint8_t value)
{
	MemoryMapWrite(MemoryMap, address, value);
}

/* Accumulator	OPC A
operand is AC (implied single byte instruction) */
uint16_t AddressingAccumulator(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	return (AC);
}

/* absolute	OPC $LLHH
operand is address $HHLL */
uint16_t AddressingAbsolute(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	uint16_t AddressingAbsoluteValue = 0u;
	AddressingAbsoluteValue = (_LL | (_HH << 8));
	return AddressingAbsoluteValue;
}

/* absolute, X-indexed
OPC $LLHH,X	operand is address; effective address is address incremented by X with carry */
uint16_t AddressingAbsoluteX(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	uint16_t AddressingAbsoluteXValue = 0u;
	AddressingAbsoluteXValue = (uint16_t)((_LL | (_HH << 8)) + _X);
	return AddressingAbsoluteXValue;
}

/* absolute, Y-indexed
OPC $LLHH,Y	operand is address; effective address is address incremented by Y with carry */
uint16_t AddressingAbsoluteY(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	uint16_t AddressingAbsoluteYValue = 0u;
	AddressingAbsoluteYValue = (uint16_t)((_LL | (_HH << 8)) + _Y);
	return AddressingAbsoluteYValue;
}

/* immediate
OPC #$BB	operand is byte BB */
uint16_t AddressingImmediate(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	return (_BB);
}

/* implied
OPC	operand implied */
uint16_t AddressingImplied(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	return (0x0);
}

/* indirect	OPC ($LLHH)
operand is address; effective address is contents of word at address: C.w($HHLL) */
uint16_t AddressingIndirect(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	uint16_t AddressingIndirectValue = 0;
	uint8_t ui8LowAddress = cpu6502AddressRead((uint16_t)(_LL | (_HH << 8)));
	uint8_t ui8HighAddress = 0;
	/* in case an indirect jump start at the last byte of the page,
	it will take the low byte of the address and the high byte from the beginning of the page */
	if (_LL == 0xFF)
	{
		ui8HighAddress = cpu6502AddressRead(((uint16_t)(_LL | ((_HH - 1) << 8)) + 1));
	}
	else
	{
		ui8HighAddress = cpu6502AddressRead(((uint16_t)(_LL | (_HH << 8)) + 1));
	}
	AddressingIndirectValue = (uint16_t)(ui8LowAddress | (ui8HighAddress << 8));
	return AddressingIndirectValue;
}

/* X-indexed, indirect
OPC ($LL,X)	operand is zeropage address; effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X) */
uint16_t AddressingIndirectX(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	uint8_t ui8LowAddress = cpu6502AddressRead((uint16_t)((_LL + _X) & 0xFF));
	uint8_t ui8HighAddress = cpu6502AddressRead((uint16_t)((_LL + _X + 1) & 0xFF));
	return (uint16_t)(ui8LowAddress | (ui8HighAddress << 8));
}

/* indirect, Y-indexed
OPC ($LL),Y	operand is zeropage address; effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y */
uint16_t AddressingIndirectY(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	uint8_t ui8LowAddress = cpu6502AddressRead((uint16_t)((_LL) & 0xFF));
	uint8_t ui8HighAddress = cpu6502AddressRead((uint16_t)((_LL + 1) & 0xFF));
	return (uint16_t)((ui8LowAddress | (ui8HighAddress << 8)) + _Y);
}

/* relative
OPC $BB	branch target is PC + signed offset BB */
uint16_t AddressingRelative(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	uint16_t AddressingRelativeValue = 0u;
	AddressingRelativeValue = PC.value + (int8_t)(_BB);
	return AddressingRelativeValue;
}

/* zeropage
OPC $LL	operand is zeropage address (hi-byte is zero, address = $00LL) */
uint16_t AddressingZeropage(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	return ((uint16_t)(_LL));
}

/* zeropage, X-indexed
OPC $LL,X	operand is zeropage address; effective address is address incremented by X without carry */
uint16_t AddressingZeropageX(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	uint16_t AddressingZeropageXValue = 0;
	AddressingZeropageXValue = ((_LL + _X) & 0xFF);
	return (AddressingZeropageXValue);
}

/* zeropage, Y-indexed
OPC $LL,Y	operand is zeropage address; effective address is address incremented by Y without carry */
uint16_t AddressingZeropageY(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB)
{
	uint16_t AddressingZeropageYValue = 0;
	AddressingZeropageYValue = ((_LL + _Y) & 0xFF);
	return (AddressingZeropageYValue);
}

uint16_t AddressingOperand8Address(func_addressing_type func_addressing)
{
	return func_addressing(
		X,
		Y, 
		AC, 
		cpu6502AddressRead(PC.value + 1), 
		cpu6502AddressRead(PC.value + 2), 
		cpu6502AddressRead(PC.value + 1));
}

uint8_t AddressingOperand8Value(func_addressing_type func_addressing)
{
	uint16_t addressing = 
		func_addressing(
			X, 
			Y, 
			AC, 
			cpu6502AddressRead(PC.value + 1), 
			cpu6502AddressRead(PC.value + 2), 
			cpu6502AddressRead(PC.value + 1));
	uint8_t val = 0;

	if ((func_addressing == AddressingImmediate) ||
		(func_addressing == AddressingAccumulator))
	{
		val = (uint8_t)(addressing);
	}
	else
	{
		val = cpu6502AddressRead(addressing);
	}

	return val;
}

uint16_t AddressingOperand16(func_addressing_type func_addressing)
{
	return func_addressing(
		X, 
		Y, 
		AC, 
		cpu6502AddressRead(PC.value + 1), 
		cpu6502AddressRead(PC.value + 2), 
		cpu6502AddressRead(PC.value + 1));
}

static uint8_t CalculateStatusN(uint8_t val)
{
	return (val & 0x80) >> 7;
}

static uint8_t CalculateStatusZ(uint8_t val)
{
	return (val == 0);
}

static uint8_t CalculateStatusC(uint16_t val)
{
	return (val & 100) >> 8;
}

static bool IsNegative(uint8_t val)
{
	return ((val & 0x80) == 0x80);
}

static uint8_t cpu6502_ADC_SBC_Helper(uint8_t arg1, uint8_t arg2)
{
	uint8_t AC_temp = arg1;
	uint8_t Ac_ret = 0;
	Ac_ret = arg1 + arg2 + SR.status.C;

	SR.status.N = CalculateStatusN(Ac_ret);
	SR.status.Z = CalculateStatusZ(Ac_ret);
	SR.status.C = 
		(uint16_t)((int16_t)(AC_temp)+(int16_t)arg2 + (int16_t)(SR.status.C)) > 0xFF;
	SR.status.V =
		((IsNegative(Ac_ret)) && (!IsNegative(AC_temp)) && (!IsNegative(arg2))) ||
		((!IsNegative(Ac_ret)) && (IsNegative(AC_temp)) && (IsNegative(arg2)));

	return Ac_ret;
}

/* ADC - Add Memory to Accumulator with Carry

A + M + C -> A, C
N	Z	C	I	D	V
+	+	+	-	-	+

addressing		assembler		opc		bytes	cyles
immediate		ADC #oper		69		2		2  
zeropage		ADC oper		65		2		3  
zeropage,X		ADC oper,X		75		2		4  
absolute		ADC oper		6D		3		4  
absolute,X		ADC oper,X		7D		3		4* 
absolute,Y		ADC oper,Y		79		3		4* 
(indirect,X)	ADC (oper,X)	61		2		6  
(indirect),Y	ADC (oper),Y	71		2		5* 
*/
uint8_t cpu6502_ADC(func_addressing_type func_addressing)
{
	uint8_t val = AddressingOperand8Value(func_addressing);
	AC = cpu6502_ADC_SBC_Helper(AC, val);

	return 0;
}

static uint8_t cpu6502_Bitwise_AND_Helper(uint8_t arg1, uint8_t arg2)
{
	return (arg1 & arg2);
}

static uint8_t cpu6502_Bitwise_XOR_Helper(uint8_t arg1, uint8_t arg2)
{
	return (arg1 ^ arg2);
}

static uint8_t cpu6502_Bitwise_OR_Helper(uint8_t arg1, uint8_t arg2)
{
	return (arg1 | arg2);
}

static uint8_t cpu6502_Bitwise_SHL_Helper(uint8_t arg1, uint8_t arg2)
{
	return (arg1 << arg2);
}

static uint8_t cpu6502_Bitwise_SHR_Helper(uint8_t arg1, uint8_t arg2)
{
	return (arg1 >> arg2);
}

static uint8_t cpu6502_Bitwise_Operator_Helper(uint8_t arg1, uint8_t arg2, uint8_t (*func)(uint8_t, uint8_t))
{
	uint8_t val = func(arg1, arg2);

	SR.status.N = CalculateStatusN(val);
	SR.status.Z = CalculateStatusZ(val);

	return val;
}

/* AND - AND Memory with Accumulator

A AND M -> A

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
immediate		AND #oper		29		2		2  
zeropage		AND oper		25		2		3  
zeropage,X		AND oper,X		35		2		4  
absolute		AND oper		2D		3		4  
absolute,X		AND oper,X		3D		3		4* 
absolute,Y		AND oper,Y		39		3		4* 
(indirect,X)	AND (oper,X)	21		2		6  
(indirect),Y	AND (oper),Y	31		2		5* 
*/
uint8_t cpu6502_AND(func_addressing_type func_addressing)
{
	AC = cpu6502_Bitwise_Operator_Helper(
		AC,
		AddressingOperand8Value(func_addressing),
		&cpu6502_Bitwise_AND_Helper);

	return 0;
}

/* ASL - Shift Left One Bit (Memory or Accumulator)

C <- [76543210] <- 0

N	Z	C	I	D	V
+	+	+	-	-	-
addressing		assembler		opc		bytes		cyles
accumulator		ASL A			0A		1			2
zeropage		ASL oper		06		2			5
zeropage,X		ASL oper,X		16		2			6
absolute		ASL oper		0E		3			6
absolute,X		ASL oper,X		1E		3			7
*/
uint8_t cpu6502_ASL(func_addressing_type func_addressing)
{
	uint8_t oper = AddressingOperand8Value(func_addressing);

	uint8_t val = cpu6502_Bitwise_Operator_Helper(
		AddressingOperand8Value(func_addressing),
		1,
		&cpu6502_Bitwise_SHL_Helper);

	if (func_addressing == &AddressingAccumulator)
	{
		AC = val;
	}
	else
	{
		cpu6502AddressWrite(AddressingOperand8Address(func_addressing), val);
	}

	SR.status.C = ((oper & 0x80) >> 7);

	return 0;
}

/* BCC - Branch on Carry Clear

branch on C = 0

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
relative		BCC oper		90		2		2**
*/
uint8_t cpu6502_BCC(func_addressing_type func_addressing)
{
	if (0 == SR.status.C)
	{
		PC.value = AddressingOperand16(func_addressing);
	}

	return 0;	/* no need to notify to outside loop as PC need to increment according to command bytes size */
}

/* BCS - Branch on Carry Set

branch on C = 1

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
relative		BCS oper		B0		2		2**
*/
uint8_t cpu6502_BCS(func_addressing_type func_addressing)
{
	if (1 == SR.status.C)
	{
		PC.value = AddressingOperand16(func_addressing);
	}

	return 0;	/* no need to notify to outside loop as PC need to increment according to command bytes size */
}

/* BEQ - Branch on Result Zero

branch on Z = 1

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
relative		BEQ oper		F0		2		2**
*/
uint8_t cpu6502_BEQ(func_addressing_type func_addressing)
{
	if (1 == SR.status.Z)
	{
		PC.value = AddressingOperand16(func_addressing);
	}

	return 0;	/* no need to notify to outside loop as PC need to increment according to command bytes size */
}

/* BIT - Test Bits in Memory with Accumulator

bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
the zero-flag is set to the result of operand AND accumulator.

A AND M, M7 -> N, M6 -> V

N	Z	C	I	D	V
M7	+	-	-	-	M6

addressing		assembler		opc		bytes	cyles
zeropage		BIT oper		24		2		3  
absolute		BIT oper		2C		3		4  
*/
uint8_t cpu6502_BIT(func_addressing_type func_addressing)
{
	uint8_t data = AddressingOperand8Value(func_addressing);
	SR.status.V = (data & 0x40) >> 6;
	SR.status.N = (data & 0x80) >> 7;

	SR.status.Z = CalculateStatusZ((AC & data));

	return 0;
}

/* BMI - Branch on Result Minus

branch on N = 1

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
relative		BMI oper		30		2		2**
*/
uint8_t cpu6502_BMI(func_addressing_type func_addressing)
{
	if (1 == SR.status.N)
	{
		PC.value = AddressingOperand16(func_addressing);
	}

	return 0;	/* no need to notify to outside loop as PC need to increment according to command bytes size */
}

/* BNE - Branch on Result not Zero

branch on Z = 0

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
relative		BNE oper		D0		2		2**
*/
uint8_t cpu6502_BNE(func_addressing_type func_addressing)
{
	if (0 == SR.status.Z)
	{
		PC.value = AddressingOperand16(func_addressing);
	}

	return 0;	/* no need to notify to outside loop as PC need to increment according to command bytes size */
}

/* BPL - Branch on Result Plus

branch on N = 0

N	Z	C	I	D	V
-	-	-	-	-	-
addressing		assembler		opc		bytes	cyles
relative		BPL oper		10		2		2**
*/
uint8_t cpu6502_BPL(func_addressing_type func_addressing)
{
	if (0 == SR.status.N)
	{
		PC.value = AddressingOperand16(func_addressing);
	}

	return 0;	/* no need to notify to outside loop as PC need to increment according to command bytes size */
}

/* BRK - Force Break

interrupt,
push PC+2, push SR

N	Z	C	I	D	V
-	-	-	1	-	-

addressing		assembler		opc		bytes	cyles
implied			BRK 00			1		1		7
*/
uint8_t cpu6502_BRK(func_addressing_type func_addressing)
{
	Reg16 temp = PC;
	temp.value = temp.value + 2;

	RegStatus P;
	P.value = SR.value;
	P.status.B = 1;
	P.status.spare = 1;
	
	cpu6502AddressWrite((uint16_t)((SP--) + 0x100), temp.PCH);
	cpu6502AddressWrite((uint16_t)((SP--) + 0x100), temp.PCL);
	cpu6502AddressWrite((uint16_t)((SP--) + 0x100), P.value);

	SR.status.I = 1;

	return 0;
}

/* BVC - Branch on Overflow Clear

branch on V = 0

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
relative		BVC oper		50		2		2**
*/
uint8_t cpu6502_BVC(func_addressing_type func_addressing)
{
	if (0 == SR.status.V)
	{
		PC.value = AddressingOperand16(func_addressing);
	}

	return 0;	/* no need to notify to outside loop as PC need to increment according to command bytes size */
}

/* BVS - Branch on Overflow Set

branch on V = 1

N	Z	C	I	D	V
-	-	-	-	-	-
addressing	assembler	opc	bytes	cyles
relative	BVC oper	70	2	2**
*/
uint8_t cpu6502_BVS(func_addressing_type func_addressing)
{
	if (1 == SR.status.V)
	{
		PC.value = AddressingOperand16(func_addressing);
	}

	return 0;	/* no need to notify to outside loop as PC need to increment according to command bytes size */
}

/* CLC - Clear Carry Flag

0 -> C
N	Z	C	I	D	V
-	-	0	-	-	-

addressing		assembler		opc		bytes	cyles
implied			CLC				18		1		2  
*/
uint8_t cpu6502_CLC(func_addressing_type func_addressing)
{
	SR.status.C = 0;

	return 0;
}

/* CLD - Clear Decimal Mode

0 -> D
N	Z	C	I	D	V
-	-	-	-	0	-

addressing		assembler		opc		bytes	cyles
implied			CLD				D8		1		2  
*/
uint8_t cpu6502_CLD(func_addressing_type func_addressing)
{
	SR.status.D = 0;

	return 0;
}

/* CLI - Clear Interrupt Disable Bit

0 -> I
N	Z	C	I	D	V
-	-	-	0	-	-

addressing		assembler		opc		bytes	cyles
implied			CLI				58		1		2  
*/
uint8_t cpu6502_CLI(func_addressing_type func_addressing)
{
	SR.status.I = 0;

	return 0;
}

/* CLV - Clear Overflow Flag

0 -> V
N	Z	C	I	D	V
-	-	-	-	-	0

addressing		assembler		opc		bytes	cyles
implied			CLV				B8		1		2  
*/
uint8_t cpu6502_CLV(func_addressing_type func_addressing)
{
	SR.status.V = 0;

	return 0;
}

static uint8_t cpu6502_CM_Helper(func_addressing_type func_addressing, uint8_t value)
{
	uint8_t addressing_value = AddressingOperand8Value(func_addressing);
	uint8_t temp8 = value - addressing_value;
	uint16_t temp16 = ((uint16_t)(value)) - ((uint16_t)(addressing_value));

	SR.status.N = CalculateStatusN(temp8);
	SR.status.Z = CalculateStatusZ(temp8);
	SR.status.C = (value >= addressing_value);

	return 0;
}

/* CMP - Compare Memory with Accumulator

A - M
N	Z	C	I	D	V
+	+	+	-	-	-

addressing		assembler		opc		bytes	cyles
immediate		CMP #oper		C9		2		2  
zeropage		CMP oper		C5		2		3  
zeropage,X		CMP oper,X		D5		2		4  
absolute		CMP oper		CD		3		4  
absolute,X		CMP oper,X		DD		3		4* 
absolute,Y		CMP oper,Y		D9		3		4* 
(indirect,X)	CMP (oper,X)	C1		2		6  
(indirect),Y	CMP (oper),Y	D1		2		5* 
*/
uint8_t cpu6502_CMP(func_addressing_type func_addressing)
{
	return cpu6502_CM_Helper(func_addressing, AC);
}

/* CPX - Compare Memory and Index X

X - M
N	Z	C	I	D	V
+	+	+	-	-	-

addressing		assembler		opc		bytes	cyles
immediate		CPX #oper		E0		2		2  
zeropage		CPX oper		E4		2		3  
absolute		CPX oper		EC		3		4  
*/
uint8_t cpu6502_CPX(func_addressing_type func_addressing)
{
	return cpu6502_CM_Helper(func_addressing, X);
}

/* CPY - Compare Memory and Index Y

Y - M
N	Z	C	I	D	V
+	+	+	-	-	-

addressing		assembler		opc		bytes	cyles
immediate		CPY #oper		C0		2		2  
zeropage		CPY oper		C4		2		3  
absolute		CPY oper		CC		3		4  
*/
uint8_t cpu6502_CPY(func_addressing_type func_addressing)
{
	return cpu6502_CM_Helper(func_addressing, Y);
}

static uint8_t cpu6502_Incrementer_Helper(uint8_t arg, uint8_t incrementer)
{
	uint8_t val = arg + incrementer;

	SR.status.N = CalculateStatusN(val);
	SR.status.Z = CalculateStatusZ(val);

	return val;
}

/* DEC - Decrement Memory by One

M - 1 -> M

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
zeropage		DEC oper		C6		2		5  
zeropage,X		DEC oper,X		D6		2		6  
absolute		DEC oper		CE		3		6  
absolute,X		DEC oper,X		DE		3		7  
*/
uint8_t cpu6502_DEC(func_addressing_type func_addressing)
{
	cpu6502AddressWrite(
		AddressingOperand8Address(func_addressing),
		cpu6502_Incrementer_Helper(AddressingOperand8Value(func_addressing), (-1))
		);

	return 0;
}

/* DEX - Decrement Index X by One

X - 1 -> X

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			DEX				CA		1		2  
*/
uint8_t cpu6502_DEX(func_addressing_type func_addressing)
{
	X = cpu6502_Incrementer_Helper(X, (-1));

	return 0;
}

/* DEY - Decrement Index Y by One

Y - 1 -> Y

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			DEY				88		1		2  
*/
uint8_t cpu6502_DEY(func_addressing_type func_addressing)
{
	Y = cpu6502_Incrementer_Helper(Y, (-1));

	return 0;
}

/* EOR - Exclusive-OR Memory with Accumulator

A EOR M -> A

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
immediate		EOR #oper		49		2		2  
zeropage		EOR oper		45		2		3  
zeropage,X		EOR oper,X		55		2		4  
absolute		EOR oper		4D		3		4  
absolute,X		EOR oper,X		5D		3		4* 
absolute,Y		EOR oper,Y		59		3		4* 
(indirect,X)	EOR (oper,X)	41		2		6  
(indirect),Y	EOR (oper),Y	51		2		5* 
*/
uint8_t cpu6502_EOR(func_addressing_type func_addressing)
{
	AC = cpu6502_Bitwise_Operator_Helper(
		AC,
		AddressingOperand8Value(func_addressing),
		&cpu6502_Bitwise_XOR_Helper);

	return 0;
}

/* INC - Increment Memory by One

M + 1 -> M

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
zeropage		INC oper		E6		2		5  
zeropage,X		INC oper,X		F6		2		6  
absolute		INC oper		EE		3		6  
absolute,X		INC oper,X		FE		3		7  
*/
uint8_t cpu6502_INC(func_addressing_type func_addressing)
{
	cpu6502AddressWrite(
		AddressingOperand8Address(func_addressing),
		cpu6502_Incrementer_Helper(AddressingOperand8Value(func_addressing), (+1))
	);

	return 0;
}

/* INX - Increment Index X by One

X + 1 -> X

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			INX				E8		1		2  
*/
uint8_t cpu6502_INX(func_addressing_type func_addressing)
{
	X = cpu6502_Incrementer_Helper(X, (+1));

	return 0;
}

/* INY - Increment Index Y by One

Y + 1 -> Y

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			INY				C8		1		2  
*/
uint8_t cpu6502_INY(func_addressing_type func_addressing)
{
	Y = cpu6502_Incrementer_Helper(Y, (+1));

	return 0;
}

static uint8_t cpu6502_JMP_Helper(func_addressing_type func_addressing)
{
	uint16_t val = AddressingOperand16(func_addressing);

	PC.PCL = (val & 0xFF);
	PC.PCH = ((val >> 0x8) & 0xFF);

	return 0xFF;	/* notify not to increment PC in outside loop */
}

/* JMP - Jump to New Location

(PC+1) -> PCL
(PC+2) -> PCH

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
absolute		JMP oper		4C		3		3  
indirect		JMP (oper)		6C		3		5  
*/
uint8_t cpu6502_JMP(func_addressing_type func_addressing)
{
	return cpu6502_JMP_Helper(func_addressing); 
}

/* JSR - Jump to New Location Saving Return Address

push (PC+2),
(PC+1) -> PCL
(PC+2) -> PCH

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
absolute		JSR oper		20		3		6  
*/
uint8_t cpu6502_JSR(func_addressing_type func_addressing)
{
	/* JSR pushes the address-1 of the next operation on to the stack */
	/* pushing PC+3 since PC was not incremented since pointing to opcode */
	cpu6502AddressWrite((uint16_t)((SP--) + 0x100), ((PC.value + 2) >> 0x8) & 0xFF);
	cpu6502AddressWrite((uint16_t)((SP--) + 0x100), (PC.value + 2) & 0xFF);
	
	return cpu6502_JMP_Helper(func_addressing);
}

static uint8_t cpu6502_LD_Helper(uint8_t arg)
{
	SR.status.N = CalculateStatusN(arg);
	SR.status.Z = CalculateStatusZ(arg);

	return arg;
}

/* LDA - Load Accumulator with Memory

M -> A

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
immediate		LDA #oper		A9		2		2  
zeropage		LDA oper		A5		2		3  
zeropage,X		LDA oper,X		B5		2		4  
absolute		LDA oper		AD		3		4  
absolute,X		LDA oper,X		BD		3		4* 
absolute,Y		LDA oper,Y		B9		3		4* 
(indirect,X)	LDA (oper,X)	A1		2		6  
(indirect),Y	LDA (oper),Y	B1		2		5* 
*/
uint8_t cpu6502_LDA(func_addressing_type func_addressing)
{
	AC = cpu6502_LD_Helper(AddressingOperand8Value(func_addressing));

	return 0;
}

/* LDX - Load Index X with Memory

M -> X

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
immediate		LDX #oper		A2		2		2  
zeropage		LDX oper		A6		2		3  
zeropage,Y		LDX oper,Y		B6		2		4  
absolute		LDX oper		AE		3		4  
absolute,Y		LDX oper,Y		BE		3		4* 
*/
uint8_t cpu6502_LDX(func_addressing_type func_addressing)
{
	X = cpu6502_LD_Helper(AddressingOperand8Value(func_addressing));

	return 0;
}

/* LDY - Load Index Y with Memory

M -> Y

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
immediate		LDY #oper		A0		2		2  
zeropage		LDY oper		A4		2		3  
zeropage,X		LDY oper,X		B4		2		4  
absolute		LDY oper		AC		3		4  
absolute,X		LDY oper,X		BC		3		4* 
*/
uint8_t cpu6502_LDY(func_addressing_type func_addressing)
{
	Y = cpu6502_LD_Helper(AddressingOperand8Value(func_addressing));

	return 0;
}

/* LSR - Shift One Bit Right (Memory or Accumulator)

0 -> [76543210] -> C

N	Z	C	I	D	V
0	+	+	-	-	-

addressing		assembler		opc		bytes	cyles
accumulator		LSR A			4A		1		2  
zeropage		LSR oper		46		2		5  
zeropage,X		LSR oper,X		56		2		6  
absolute		LSR oper		4E		3		6  
absolute,X		LSR oper,X		5E		3		7  
*/
uint8_t cpu6502_LSR(func_addressing_type func_addressing)
{
	uint8_t oper = AddressingOperand8Value(func_addressing);

	uint8_t val = cpu6502_Bitwise_Operator_Helper(
		AddressingOperand8Value(func_addressing),
		1,
		&cpu6502_Bitwise_SHR_Helper);

	if (func_addressing == &AddressingAccumulator)
	{
		AC = val;
	}
	else
	{
		cpu6502AddressWrite(AddressingOperand8Address(func_addressing), val);
	}

	SR.status.N = 0;
	SR.status.C = (oper & 0x01);

	return 0;
}

/* NOP - No Operation

---

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			NOP				EA		1		2  

  NOP (NOP) [NOP]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
No operation
Status flags: -

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Implied     |NOP        |$1A| 1 | 2
Implied     |NOP        |$3A| 1 | 2
Implied     |NOP        |$5A| 1 | 2
Implied     |NOP        |$7A| 1 | 2
Implied     |NOP        |$DA| 1 | 2
Implied     |NOP        |$FA| 1 | 2
*/
uint8_t cpu6502_NOP(func_addressing_type func_addressing)
{
	return 0;
}

/* ORA - OR Memory with Accumulator

A OR M -> A
N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
immediate		ORA #oper		09		2		2
zeropage		ORA oper		05		2		3
zeropage,X		ORA oper,X		15		2		4
absolute		ORA oper		0D		3		4
absolute,X		ORA oper,X		1D		3		4*
absolute,Y		ORA oper,Y		19		3		4*
(indirect,X)	ORA (oper,X)	01		2		6
(indirect),Y	ORA (oper),Y	11		2		5*
*/
uint8_t cpu6502_ORA(func_addressing_type func_addressing)
{
	AC = cpu6502_Bitwise_Operator_Helper(
		AC,
		AddressingOperand8Value(func_addressing),
		&cpu6502_Bitwise_OR_Helper);
	
	return 0;
}

/* PHA - Push Accumulator on Stack

push A

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			PHA				48		1		3  
*/
uint8_t cpu6502_PHA(func_addressing_type func_addressing)
{
	cpu6502AddressWrite((uint16_t)(SP + 0x100), AC);
	SP--;

	return 0;
}

/* PHP - Push Processor Status on Stack

push SR
N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			PHP				08		1		3 
*/
uint8_t cpu6502_PHP(func_addressing_type func_addressing)
{
	RegStatus P;
	P.value = SR.value;
	P.status.B = 1;
	P.status.spare = 1;
	cpu6502AddressWrite((uint16_t)(SP + 0x100), P.value);
	SP--;

	return 0;
}

/* PLA - Pull Accumulator from Stack

pull A

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			PLA				68		1		4  
*/
uint8_t cpu6502_PLA(func_addressing_type func_addressing)
{
	SP++;
	AC = cpu6502AddressRead((uint16_t)(SP + 0x100));
	
	SR.status.N = CalculateStatusN(AC);
	SR.status.Z = CalculateStatusZ(AC);

	return 0;
}

/* PLP - Pull Processor Status from Stack

pull SR

N	Z	C	I	D	V
from stack

addressing		assembler		opc		bytes	cyles
implied			PLP				28		1		4  
*/
uint8_t cpu6502_PLP(func_addressing_type func_addressing)
{
	SP++;
	SR.value = cpu6502AddressRead((uint16_t)(SP + 0x100));
	SR.status.B = 0;
	SR.status.spare = 1;

	return  0;
}

/* ROL - Rotate One Bit Left (Memory or Accumulator)

C <- [76543210] <- C

N	Z	C	I	D	V
+	+	+	-	-	-

addressing		assembler		opc		bytes	cyles
accumulator		ROL A			2A		1		2  
zeropage		ROL oper		26		2		5  
zeropage,X		ROL oper,X		36		2		6  
absolute		ROL oper		2E		3		6  
absolute,X		ROL oper,X		3E		3		7  
*/
uint8_t cpu6502_ROL(func_addressing_type func_addressing)
{
	uint8_t val = AddressingOperand8Value(func_addressing);
	uint8_t temp = SR.status.C;

	SR.status.C = (val & 0x80) >> 0x07;
	val = val << 1;
	val = val | temp;

	if (func_addressing == &AddressingAccumulator)
	{
		AC = val;
	}
	else
	{
		cpu6502AddressWrite(AddressingOperand8Address(func_addressing), val);
	}

	SR.status.N = CalculateStatusN(val);
	SR.status.Z = CalculateStatusZ(val);
	
	return 0;
}

/* ROR - Rotate One Bit Right (Memory or Accumulator)

C -> [76543210] -> C

N	Z	C	I	D	V
+	+	+	-	-	-

addressing		assembler		opc		bytes	cyles
accumulator		ROR A			6A		1		2  
zeropage		ROR oper		66		2		5  
zeropage,X		ROR oper,X		76		2		6  
absolute		ROR oper		6E		3		6  
absolute,X		ROR oper,X		7E		3		7  
*/
uint8_t cpu6502_ROR(func_addressing_type func_addressing)
{
	uint8_t val = AddressingOperand8Value(func_addressing);
	uint8_t temp = SR.status.C;

	SR.status.C = (val & 0x01);
	val = val >> 1;
	val = val | (temp << 0x07);

	if (func_addressing == &AddressingAccumulator)
	{
		AC = val;
	}
	else
	{
		cpu6502AddressWrite(AddressingOperand8Address(func_addressing), val);
	}

	SR.status.N = CalculateStatusN(val);
	SR.status.Z = CalculateStatusZ(val);

	return 0;
}

/* RTI - Return from Interrupt

pull SR, pull PC

N	Z	C	I	D	V
from stack

addressing		assembler		opc		bytes	cyles
implied			RTI				40		1		6  
*/
uint8_t cpu6502_RTI(func_addressing_type func_addressing)
{
	SR.value = cpu6502AddressRead((uint16_t)((++SP) + 0x100));
	PC.PCL = cpu6502AddressRead((uint16_t)((++SP) + 0x100));
	PC.PCH = cpu6502AddressRead((uint16_t)((++SP) + 0x100));

	SR.status.B = 0;
	SR.status.spare = 1;

	return 0xFF;		/* notify not to increment PC in outside loop */
}

/* RTS - Return from Subroutine

pull PC, PC+1 -> PC

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			RTS				60		1		6  
*/
uint8_t cpu6502_RTS(func_addressing_type func_addressing)
{
	PC.PCL = cpu6502AddressRead((uint16_t)((++SP) + 0x100));
	PC.PCH = cpu6502AddressRead((uint16_t)((++SP) + 0x100));

	PC.value++;
	
	return 0xFF;	/* notify not to increment PC in outside loop */
}

/* SBC - Subtract Memory from Accumulator with Borrow

A - M - C -> A

N	Z	C	I	D	V
+	+	+	-	-	+

addressing		assembler		opc		bytes	cyles
immediate		SBC #oper		E9		2		2  
zeropage		SBC oper		E5		2		3  
zeropage,X		SBC oper,X		F5		2		4  
absolute		SBC oper		ED		3		4  
absolute,X		SBC oper,X		FD		3		4* 
absolute,Y		SBC oper,Y		F9		3		4* 
(indirect,X)	SBC (oper,X)	E1		2		6  
(indirect),Y	SBC (oper),Y	F1		2		5*
*/
uint8_t cpu6502_SBC(func_addressing_type func_addressing)
{
	uint8_t val = AddressingOperand8Value(func_addressing);
	AC = cpu6502_ADC_SBC_Helper(AC, ~val);

	return 0;
}

/* SEC - Set Carry Flag

1 -> C

N	Z	C	I	D	V
-	-	1	-	-	-

addressing		assembler		opc		bytes	cyles
implied			SEC				38		1		2  
*/
uint8_t cpu6502_SEC(func_addressing_type func_addressing)
{
	SR.status.C = 1;

	return 0;
}

/* SED - Set Decimal Flag

1 -> D

N	Z	C	I	D	V
-	-	-	-	1	-

addressing		assembler		opc		bytes	cyles
implied			SED				F8		1		2  
*/
uint8_t cpu6502_SED(func_addressing_type func_addressing)
{
	SR.status.D = 1;

	return 0;
}

/* SEI - Set Interrupt Disable Status

1 -> I

N	Z	C	I	D	V
-	-	-	1	-	-

addressing		assembler		opc		bytes	cyles
implied			SEI				78		1		2  
*/
uint8_t cpu6502_SEI(func_addressing_type func_addressing)
{
	SR.status.I = 1;

	return 0;
}

/* STA - Store Accumulator in Memory

A -> M

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
zeropage		STA oper		85		2		3  
zeropage,X		STA oper,X		95		2		4  
absolute		STA oper		8D		3		4  
absolute,X		STA oper,X		9D		3		5  
absolute,Y		STA oper,Y		99		3		5  
(indirect,X)	STA (oper,X)	81		2		6  
(indirect),Y	STA (oper),Y	91		2		6  
*/
uint8_t cpu6502_STA(func_addressing_type func_addressing)
{
	cpu6502AddressWrite(AddressingOperand8Address(func_addressing), AC);

	return 0;
}

/* STX - Store Index X in Memory

X -> M

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
zeropage		STX oper		86		2		3  
zeropage,Y		STX oper,Y		96		2		4  
absolute		STX oper		8E		3		4  
*/
uint8_t cpu6502_STX(func_addressing_type func_addressing)
{
	cpu6502AddressWrite(AddressingOperand8Address(func_addressing), X);

	return 0;
}

/* STY - Sore Index Y in Memory

Y -> M

N	Z	C	I	D	V
-	-	-	-	-	-

addressing		assembler		opc		bytes	cyles
zeropage		STY oper		84		2		3  
zeropage,X		STY oper,X		94		2		4  
absolute		STY oper		8C		3		4  
*/
uint8_t cpu6502_STY(func_addressing_type func_addressing)
{
	cpu6502AddressWrite(AddressingOperand8Address(func_addressing), Y);

	return 0;
}

/* TAX - Transfer Accumulator to Index X

A -> X

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			TAX				AA		1		2  
*/
uint8_t cpu6502_TAX(func_addressing_type func_addressing)
{
	X = AC;
	SR.status.N = CalculateStatusN(X);
	SR.status.Z = CalculateStatusZ(X);

	return 0;
}

/* TAY - Transfer Accumulator to Index Y

A -> Y

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			TAY				A8		1		2  
*/
uint8_t cpu6502_TAY(func_addressing_type func_addressing)
{
	Y = AC;
	SR.status.N = CalculateStatusN(Y);
	SR.status.Z = CalculateStatusZ(Y);

	return 0;
}

/* TSX - Transfer Stack Pointer to Index X

SP -> X

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			TSX				BA		1		2  
*/
uint8_t cpu6502_TSX(func_addressing_type func_addressing)
{
	X = SP;
	SR.status.N = CalculateStatusN(X);
	SR.status.Z = CalculateStatusZ(X);

	return 0;
}

/* TXA - Transfer Index X to Accumulator

X -> A

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			TXA				8A		1		2  
*/
uint8_t cpu6502_TXA(func_addressing_type func_addressing)
{
	AC = X;
	SR.status.N = CalculateStatusN(AC);
	SR.status.Z = CalculateStatusZ(AC);

	return 0;
}

/* TXS - Transfer Index X to Stack Register

X -> SP

N	Z	C	I	D	V
-	-	-	-	-	-

addressing 	assembler		opc		bytes	cyles
implied		TXS				9A		1		2  
*/
uint8_t cpu6502_TXS(func_addressing_type func_addressing)
{
	SP = X;

	return 0;
}

/* TYA - Transfer Index Y to Accumulator

Y -> A

N	Z	C	I	D	V
+	+	-	-	-	-

addressing		assembler		opc		bytes	cyles
implied			TYA				98		1		2  
*/
uint8_t cpu6502_TYA(func_addressing_type func_addressing)
{
	AC = Y;
	SR.status.N = CalculateStatusN(AC);
	SR.status.Z = CalculateStatusZ(AC);

	return 0;
}

/* AAX (SAX) [AXS]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
AND X register with accumulator and store result in memory. Status
flags: N,Z

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Zero Page   |AAX arg    |$87| 2 | 3
Zero Page,Y |AAX arg,Y  |$97| 2 | 4
(Indirect,X)|AAX (arg,X)|$83| 2 | 6
Absolute    |AAX arg    |$8F| 3 | 4
*/
uint8_t cpu6502_AAX(func_addressing_type func_addressing)
{
	cpu6502AddressWrite(AddressingOperand8Address(func_addressing), (X & AC));

	/* according to: http://ist.uwaterloo.ca/~schepers/MJK/ascii/65xx_ill.txt,
	  status flags are not changed. */

	return 0;
}

/* DCP (DCP) [DCM]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
Subtract 1 from memory (without borrow).
Status flags: C

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Zero Page   |DCP arg    |$C7| 2 | 5
Zero Page,X |DCP arg,X  |$D7| 2 | 6
Absolute    |DCP arg    |$CF| 3 | 6
Absolute,X  |DCP arg,X  |$DF| 3 | 7
Absolute,Y  |DCP arg,Y  |$DB| 3 | 7
(Indirect,X)|DCP (arg,X)|$C3| 2 | 8
(Indirect),Y|DCP (arg),Y|$D3| 2 | 8
*/
uint8_t cpu6502_DCP(func_addressing_type func_addressing)
{
	cpu6502_DEC(func_addressing);
	cpu6502_CMP(func_addressing);

	return 0;
}

/* DOP (NOP) [SKB]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
No operation (double NOP). The argument has no signifi-cance. Status
flags: -

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Zero Page   |DOP arg    |$04| 2 | 3
Zero Page,X |DOP arg,X  |$14| 2 | 4
Zero Page,X |DOP arg,X  |$34| 2 | 4
Zero Page   |DOP arg    |$44| 2 | 3
Zero Page,X |DOP arg,X  |$54| 2 | 4
Zero Page   |DOP arg    |$64| 2 | 3
Zero Page,X |DOP arg,X  |$74| 2 | 4
Immediate   |DOP #arg   |$80| 2 | 2
Immediate   |DOP #arg   |$82| 2 | 2
Immediate   |DOP #arg   |$89| 2 | 2
Immediate   |DOP #arg   |$C2| 2 | 2
Zero Page,X |DOP arg,X  |$D4| 2 | 4
Immediate   |DOP #arg   |$E2| 2 | 2
Zero Page,X |DOP arg,X  |$F4| 2 | 4
*/
uint8_t cpu6502_DOP(func_addressing_type func_addressing)
{
	return 0;
}

/* ISC (ISB) [INS]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
Increase memory by one, then subtract memory from accu-mulator (with
borrow). Status flags: N,V,Z,C

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Zero Page   |ISC arg    |$E7| 2 | 5
Zero Page,X |ISC arg,X  |$F7| 2 | 6
Absolute    |ISC arg    |$EF| 3 | 6
Absolute,X  |ISC arg,X  |$FF| 3 | 7
Absolute,Y  |ISC arg,Y  |$FB| 3 | 7
(Indirect,X)|ISC (arg,X)|$E3| 2 | 8
(Indirect),Y|ISC (arg),Y|$F3| 2 | 8
*/
uint8_t cpu6502_ISC(func_addressing_type func_addressing)
{
	cpu6502_INC(func_addressing);
	cpu6502_SBC(func_addressing);

	return 0;
}

/* LAX (LAX) [LAX]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
Load accumulator and X register with memory.
Status flags: N,Z

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Zero Page   |LAX arg    |$A7| 2 | 3
Zero Page,Y |LAX arg,Y  |$B7| 2 | 4
Absolute    |LAX arg    |$AF| 3 | 4
Absolute,Y  |LAX arg,Y  |$BF| 3 | 4 *
(Indirect,X)|LAX (arg,X)|$A3| 2 | 6
(Indirect),Y|LAX (arg),Y|$B3| 2 | 5 *
*/
uint8_t cpu6502_LAX(func_addressing_type func_addressing)
{
	cpu6502_LDA(func_addressing);
	cpu6502_LDX(func_addressing);

	/* status flags are updated inside LDA/LDX functions */

	return 0;
}

/* RLA (RLA) [RLA]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
Rotate one bit left in memory, then AND accumulator with memory. Status
flags: N,Z,C

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Zero Page   |RLA arg    |$27| 2 | 5
Zero Page,X |RLA arg,X  |$37| 2 | 6
Absolute    |RLA arg    |$2F| 3 | 6
Absolute,X  |RLA arg,X  |$3F| 3 | 7
Absolute,Y  |RLA arg,Y  |$3B| 3 | 7
(Indirect,X)|RLA (arg,X)|$23| 2 | 8
(Indirect),Y|RLA (arg),Y|$33| 2 | 8
*/
uint8_t cpu6502_RLA(func_addressing_type func_addressing)
{
	cpu6502_ROL(func_addressing);
	cpu6502_AND(func_addressing);

	return 0;
}

/* RRA (RRA) [RRA]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
Rotate one bit right in memory, then add memory to accumulator (with
carry).

Status flags: N,V,Z,C

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Zero Page   |RRA arg    |$67| 2 | 5
Zero Page,X |RRA arg,X  |$77| 2 | 6
Absolute    |RRA arg    |$6F| 3 | 6
Absolute,X  |RRA arg,X  |$7F| 3 | 7
Absolute,Y  |RRA arg,Y  |$7B| 3 | 7
(Indirect,X)|RRA (arg,X)|$63| 2 | 8
(Indirect),Y|RRA (arg),Y|$73| 2 | 8
*/
uint8_t cpu6502_RRA(func_addressing_type func_addressing)
{
	cpu6502_ROR(func_addressing);
	cpu6502_ADC(func_addressing);

	return 0;
}

/* SLO (SLO) [ASO]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
Shift left one bit in memory, then OR accumulator with memory. =

Status flags: N,Z,C

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Zero Page   |SLO arg    |$07| 2 | 5
Zero Page,X |SLO arg,X  |$17| 2 | 6
Absolute    |SLO arg    |$0F| 3 | 6
Absolute,X  |SLO arg,X  |$1F| 3 | 7
Absolute,Y  |SLO arg,Y  |$1B| 3 | 7
(Indirect,X)|SLO (arg,X)|$03| 2 | 8
(Indirect),Y|SLO (arg),Y|$13| 2 | 8
*/
uint8_t cpu6502_SLO(func_addressing_type func_addressing)
{
	cpu6502_ASL(func_addressing);
	cpu6502_ORA(func_addressing);

	return 0;
}

/* SRE (SRE) [LSE]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
Shift right one bit in memory, then EOR accumulator with memory. Status
flags: N,Z,C

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Zero Page   |SRE arg    |$47| 2 | 5
Zero Page,X |SRE arg,X  |$57| 2 | 6
Absolute    |SRE arg    |$4F| 3 | 6
Absolute,X  |SRE arg,X  |$5F| 3 | 7
Absolute,Y  |SRE arg,Y  |$5B| 3 | 7
(Indirect,X)|SRE (arg,X)|$43| 2 | 8
(Indirect),Y|SRE (arg),Y|$53| 2 | 8
*/
uint8_t cpu6502_SRE(func_addressing_type func_addressing)
{
	cpu6502_LSR(func_addressing);
	cpu6502_EOR(func_addressing);

	return 0;
}

/* TOP (NOP) [SKW]
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
No operation (tripple NOP). The argument has no signifi-cance. Status
flags: -

Addressing  |Mnemonics  |Opc|Sz | n
------------|-----------|---|---|---
Absolute    |TOP arg    |$0C| 3 | 4
Absolute,X  |TOP arg,X  |$1C| 3 | 4 *
Absolute,X  |TOP arg,X  |$3C| 3 | 4 *
Absolute,X  |TOP arg,X  |$5C| 3 | 4 *
Absolute,X  |TOP arg,X  |$7C| 3 | 4 *
Absolute,X  |TOP arg,X  |$DC| 3 | 4 *
Absolute,X  |TOP arg,X  |$FC| 3 | 4 *
*/
uint8_t cpu6502_TOP(func_addressing_type func_addressing)
{
	return 0;
}


uint8_t cpu6502_UNKNOWN(func_addressing_type func_addressing)
{
	printf("UNKNOWN opcode number\n");
	return 0xAA;
}

Reg16 GetRegisterPC()
{
	return PC;
}

Reg8 GetRegisterAC()
{
	return AC;
}

Reg8 GetRegisterX()
{
	return X;
}

Reg8 GetRegisterY()
{
	return Y;
}

RegStatus GetRegisterSR()
{
	return SR;
}

Reg8 GetRegisterSP()
{
	return SP;
}

bool cpu6502Init(st_MemoryMap* map)
{
	MemoryMap = map;

	SR.value = 0;
	SR.status.I = 1;
	SR.status.spare = 1;

	/* Stack is on page 1 and works top down */
	SP = 0xFD;

	PC.value = 0x8000;

	return true;
}

bool cpu6502RunCycle(uint32_t cycle)
{
	uint8_t OpcodeRetVal = 0;

	/* Read command opcode to execute */
	uint8_t CurrentOp = cpu6502AddressRead(PC.value);

	/* Execute command */
	st_CPU6502_Instruction* pInstruction = &(CPU6502_Instruction_Set[CurrentOp]);

	if (PC.value == 0xC68B)
	{
		int a = 0;
	}

	/*cpu6502_DebugPrint(pInstruction);*/

	OpcodeRetVal = pInstruction->func_opcode(pInstruction->func_addressing);

	if (0xAA == OpcodeRetVal)
	{
		return false;
	}

	/* Increment program counter to next command only if it is not incremented in opcode func (return is 0) */
	if (0 == OpcodeRetVal)
	{
		PC.value = PC.value + pInstruction->bytes;
	}

	return true;
}

bool cpu6502Destroy()
{
	return true;
}

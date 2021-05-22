#ifndef __CPU6502_H__
#define __CPU6502_H__

#include <stdint.h>
#include <stdbool.h>
#include "memory.h"

typedef enum _tag_eInstruction
{
	INSTRUCTION_ADC,
	INSTRUCTION_AND,
	INSTRUCTION_ASL,
	INSTRUCTION_BCC,
	INSTRUCTION_BCS,
	INSTRUCTION_BEQ,
	INSTRUCTION_BIT,
	INSTRUCTION_BMI,
	INSTRUCTION_BNE,
	INSTRUCTION_BPL,
	INSTRUCTION_BRK,
	INSTRUCTION_BVC,
	INSTRUCTION_BVS,
	INSTRUCTION_CLC,
	INSTRUCTION_CLD,
	INSTRUCTION_CLI,
	INSTRUCTION_CLV,
	INSTRUCTION_CMP,
	INSTRUCTION_CPX,
	INSTRUCTION_CPY,
	INSTRUCTION_DEC,
	INSTRUCTION_DEX,
	INSTRUCTION_DEY,
	INSTRUCTION_EOR,
	INSTRUCTION_INC,
	INSTRUCTION_INX,
	INSTRUCTION_INY,
	INSTRUCTION_JMP,
	INSTRUCTION_JSR,
	INSTRUCTION_LDA,
	INSTRUCTION_LDX,
	INSTRUCTION_LDY,
	INSTRUCTION_LSR,
	INSTRUCTION_NOP,
	INSTRUCTION_ORA,
	INSTRUCTION_PHA,
	INSTRUCTION_PHP,
	INSTRUCTION_PLA,
	INSTRUCTION_PLP,
	INSTRUCTION_ROL,
	INSTRUCTION_ROR,
	INSTRUCTION_RTI,
	INSTRUCTION_RTS,
	INSTRUCTION_SBC,
	INSTRUCTION_SEC,
	INSTRUCTION_SED,
	INSTRUCTION_SEI,
	INSTRUCTION_STA,
	INSTRUCTION_STX,
	INSTRUCTION_STY,
	INSTRUCTION_TAX,
	INSTRUCTION_TAY,
	INSTRUCTION_TSX,
	INSTRUCTION_TXA,
	INSTRUCTION_TXS,
	INSTRUCTION_TYA,
	INSTRUCTION_AAX,
	INSTRUCTION_DCP,
	INSTRUCTION_DOP,
	INSTRUCTION_ISC,
	INSTRUCTION_LAX,
	INSTRUCTION_NOP2,
	INSTRUCTION_RLA,
	INSTRUCTION_RRA,
	INSTRUCTION_SBC2,
	INSTRUCTION_SLO,
	INSTRUCTION_SRE,
	INSTRUCTION_TOP,
	INSTRUCTION_UNKNOWN,
	INSTRUCTION_TOTAL,
} eInstruction;

typedef uint16_t (*func_addressing_type)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

typedef struct _tag_st_CPU6502_Instruction
{
	uint8_t (*func_opcode)(func_addressing_type);
	func_addressing_type func_addressing;
	uint8_t bytes;
	uint8_t cycles;
	eInstruction instr;
} st_CPU6502_Instruction;

typedef uint8_t Reg8;

typedef union _tag_Reg16
{
	uint16_t value;
	struct
	{
		Reg8 PCL;
		Reg8 PCH;
	};
} Reg16;

/* Status register
7  bit  0
---- ----
NVss DIZC
|||| ||||
|||| |||+- Carry
|||| ||+-- Zero
|||| |+--- Interrupt Disable
|||| +---- Decimal
||++------ No CPU effect, see: the B flag
|+-------- Overflow
+--------- Negative
*/
typedef struct _tag_RegStatusData
{
	/* bit 0 */	uint8_t C : 1;		/* Carry */
	/* bit 1 */	uint8_t Z : 1;		/* Zero */
	/* bit 2 */	uint8_t I : 1;		/* Interrupt (IRQ disable) */
	/* bit 3 */	uint8_t D : 1;		/* Decimal (use BCD for arithmetics) */
	/* bit 4 */	uint8_t B : 1;		/* Break */
	/* bit 5 */	uint8_t spare : 1;	/* ignored */
	/* bit 6 */	uint8_t V : 1;		/* Overflow */
	/* bit 7 */	uint8_t N : 1;		/* Negative */
} RegStatusData;

typedef union _tag_RegStatus
{
	Reg8 value;
	RegStatusData status;
} RegStatus;

uint8_t cpu6502AddressRead(uint16_t address);
void cpu6502AddressWrite(uint16_t address, uint8_t value);

uint16_t AddressingOperand8Address(func_addressing_type func_addressing);
uint8_t AddressingOperand8Value(func_addressing_type func_addressing);
uint16_t AddressingOperand16(func_addressing_type func_addressing);

uint16_t AddressingAccumulator(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingAbsolute(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingAbsoluteX(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingAbsoluteY(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingImmediate(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingImplied(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingIndirect(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingIndirectX(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingIndirectY(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingRelative(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingZeropage(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingZeropageX(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);
uint16_t AddressingZeropageY(uint8_t _X, uint8_t _Y, uint8_t _A, uint8_t _LL, uint8_t _HH, uint8_t _BB);

static uint8_t CalculateStatusN(uint8_t val);
static uint8_t CalculateStatusZ(uint8_t val);
static uint8_t CalculateStatusC(uint16_t val);

uint8_t cpu6502_ADC(func_addressing_type func_addressing);
uint8_t cpu6502_AND(func_addressing_type func_addressing);
uint8_t cpu6502_ASL(func_addressing_type func_addressing);
uint8_t cpu6502_BCC(func_addressing_type func_addressing);
uint8_t cpu6502_BCS(func_addressing_type func_addressing);
uint8_t cpu6502_BEQ(func_addressing_type func_addressing);
uint8_t cpu6502_BIT(func_addressing_type func_addressing);
uint8_t cpu6502_BMI(func_addressing_type func_addressing);
uint8_t cpu6502_BNE(func_addressing_type func_addressing);
uint8_t cpu6502_BPL(func_addressing_type func_addressing);
uint8_t cpu6502_BRK(func_addressing_type func_addressing);
uint8_t cpu6502_BVC(func_addressing_type func_addressing);
uint8_t cpu6502_BVS(func_addressing_type func_addressing);
uint8_t cpu6502_CLC(func_addressing_type func_addressing);
uint8_t cpu6502_CLD(func_addressing_type func_addressing);
uint8_t cpu6502_CLI(func_addressing_type func_addressing);
uint8_t cpu6502_CLV(func_addressing_type func_addressing);
uint8_t cpu6502_CMP(func_addressing_type func_addressing);
uint8_t cpu6502_CPX(func_addressing_type func_addressing);
uint8_t cpu6502_CPY(func_addressing_type func_addressing);
uint8_t cpu6502_DEC(func_addressing_type func_addressing);
uint8_t cpu6502_DEX(func_addressing_type func_addressing);
uint8_t cpu6502_DEY(func_addressing_type func_addressing);
uint8_t cpu6502_DOP(func_addressing_type func_addressing);
uint8_t cpu6502_EOR(func_addressing_type func_addressing);
uint8_t cpu6502_INC(func_addressing_type func_addressing);
uint8_t cpu6502_INX(func_addressing_type func_addressing);
uint8_t cpu6502_INY(func_addressing_type func_addressing);
uint8_t cpu6502_JMP(func_addressing_type func_addressing);
uint8_t cpu6502_JSR(func_addressing_type func_addressing);
uint8_t cpu6502_LDA(func_addressing_type func_addressing);
uint8_t cpu6502_LDX(func_addressing_type func_addressing);
uint8_t cpu6502_LDY(func_addressing_type func_addressing);
uint8_t cpu6502_LSR(func_addressing_type func_addressing);
uint8_t cpu6502_NOP(func_addressing_type func_addressing);
uint8_t cpu6502_ORA(func_addressing_type func_addressing);
uint8_t cpu6502_PHA(func_addressing_type func_addressing);
uint8_t cpu6502_PHP(func_addressing_type func_addressing);
uint8_t cpu6502_PLA(func_addressing_type func_addressing);
uint8_t cpu6502_PLP(func_addressing_type func_addressing);
uint8_t cpu6502_ROL(func_addressing_type func_addressing);
uint8_t cpu6502_ROR(func_addressing_type func_addressing);
uint8_t cpu6502_RTI(func_addressing_type func_addressing);
uint8_t cpu6502_RTS(func_addressing_type func_addressing);
uint8_t cpu6502_SBC(func_addressing_type func_addressing);
uint8_t cpu6502_SEC(func_addressing_type func_addressing);
uint8_t cpu6502_SED(func_addressing_type func_addressing);
uint8_t cpu6502_SEI(func_addressing_type func_addressing);
uint8_t cpu6502_STA(func_addressing_type func_addressing);
uint8_t cpu6502_STX(func_addressing_type func_addressing);
uint8_t cpu6502_STY(func_addressing_type func_addressing);
uint8_t cpu6502_TAX(func_addressing_type func_addressing);
uint8_t cpu6502_TAY(func_addressing_type func_addressing);
uint8_t cpu6502_TOP(func_addressing_type func_addressing);
uint8_t cpu6502_TSX(func_addressing_type func_addressing);
uint8_t cpu6502_TXA(func_addressing_type func_addressing);
uint8_t cpu6502_TXS(func_addressing_type func_addressing);
uint8_t cpu6502_TYA(func_addressing_type func_addressing);
uint8_t cpu6502_AAX(func_addressing_type func_addressing);
uint8_t cpu6502_DCP(func_addressing_type func_addressing);
uint8_t cpu6502_DOP(func_addressing_type func_addressing);
uint8_t cpu6502_ISC(func_addressing_type func_addressing);
uint8_t cpu6502_LAX(func_addressing_type func_addressing);
uint8_t cpu6502_RLA(func_addressing_type func_addressing);
uint8_t cpu6502_RRA(func_addressing_type func_addressing);
uint8_t cpu6502_SLO(func_addressing_type func_addressing);
uint8_t cpu6502_SRE(func_addressing_type func_addressing);
uint8_t cpu6502_TOP(func_addressing_type func_addressing);
uint8_t cpu6502_UNKNOWN(func_addressing_type func_addressing);

Reg16 GetRegisterPC();
Reg8 GetRegisterAC();
Reg8 GetRegisterX();
Reg8 GetRegisterY();
RegStatus GetRegisterSR();
Reg8 GetRegisterSP();


bool cpu6502Init(st_MemoryMap* map);
bool cpu6502RunCycle(uint32_t cycle);
bool cpu6502Destroy();

#endif	/* __CPU6502_H__ */

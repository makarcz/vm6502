
#include <string.h>
#include "MKCpu.h"
#include "MKGenException.h"

namespace MKBasic {

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */

/*
 *--------------------------------------------------------------------
 * Method:		MKCpu()
 * Purpose:		Default constructor.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
MKCpu::MKCpu()
{
	InitCpu();
}

/*
 *--------------------------------------------------------------------
 * Method:		MKCpu()
 * Purpose:		Custom constructor.
 * Arguments:	pmem - pointer to Memory object.
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
MKCpu::MKCpu(Memory *pmem)
{
	mpMem = pmem;
	InitCpu();
}

/*
 *--------------------------------------------------------------------
 * Method:		InitCpu()
 * Purpose:		Initialize internal variables and flags.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
	ADDRMODE_IMM = 0,
	ADDRMODE_ABS,
	ADDRMODE_ZP,
	ADDRMODE_IMP,
	ADDRMODE_IND,
	ADDRMODE_ABX,
	ADDRMODE_ABY,
	ADDRMODE_ZPX,
	ADDRMODE_ZPY,
	ADDRMODE_IZX,
	ADDRMODE_IZY,
	ADDRMODE_REL,
	ADDRMODE_ACC, 
 */
void MKCpu::InitCpu()
{
	string saArgFmtTbl[] = {"#$%02x", "$%04x", "$%02x", "     ", "($%04x)", "$%04x,X", "$%04x,Y",
													"$%02x,X", "$%02x,Y", "($%02x,X)", "($%02x),Y", "$%04x", "     ", "     "};
	int naAddrModesLen[] = {2, 3, 2, 1, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1};
	// Initialize instructions lengths table based on addressing modes
	// Initialize assembly argument formats table based on addressing modes
	for (int i=0; i<ADDRMODE_LENGTH; i++) {
		mAddrModesLen[i] = naAddrModesLen[i];
		mArgFmtTbl[i] = saArgFmtTbl[i];
	}
	// Initialize disassembly op-codes table/map.
	OpCodesMap myOpCodesMap {
		//opcode					opcode						addr. mode		time		symb.
		//----------------------------------------------------------------------------
		{OPCODE_BRK,			{OPCODE_BRK,			ADDRMODE_IMP,		7,		"BRK"}},
		{OPCODE_ORA_IZX,	{OPCODE_ORA_IZX,	ADDRMODE_IZX,		6,		"ORA"}},
		{OPCODE_ILL_02,		{OPCODE_ILL_02,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_03,		{OPCODE_ILL_03,		ADDRMODE_IZX,		8,		"SLO"}},
		{OPCODE_ILL_04,		{OPCODE_ILL_04,		ADDRMODE_ZP,		3,		"NOP"}},
		{OPCODE_ORA_ZP,		{OPCODE_ORA_ZP,		ADDRMODE_ZP,		3,		"ORA"}},
		{OPCODE_ASL_ZP,		{OPCODE_ASL_ZP,		ADDRMODE_ZP,		5,		"ASL"}},
		{OPCODE_ILL_07,		{OPCODE_ILL_07,		ADDRMODE_ZP,		5,		"SLO"}},
		{OPCODE_PHP,			{OPCODE_PHP,			ADDRMODE_IMP,		3,		"PHP"}},
		{OPCODE_ORA_IMM,	{OPCODE_ORA_IMM,	ADDRMODE_IMM,		2,		"ORA"}},
		{OPCODE_ASL,			{OPCODE_ASL,			ADDRMODE_ACC,		2,		"ASL"}},
		{OPCODE_ILL_0B,		{OPCODE_ILL_0B,		ADDRMODE_IMM,		2,		"ANC"}},
		{OPCODE_ILL_0C,		{OPCODE_ILL_0C,		ADDRMODE_ABS,		4,		"NOP"}},
		{OPCODE_ORA_ABS,	{OPCODE_ORA_ABS,	ADDRMODE_ABS,		4,		"ORA"}},
		{OPCODE_ASL_ABS,	{OPCODE_ASL_ABS,	ADDRMODE_ABS,		6,		"ASL"}},
		{OPCODE_ILL_0F,		{OPCODE_ILL_0F,		ADDRMODE_ABS,		6,		"SLO"}},
		{OPCODE_BPL_REL,	{OPCODE_BPL_REL,	ADDRMODE_REL,		2,		"BPL"}},
		{OPCODE_ORA_IZY,	{OPCODE_ORA_IZY,	ADDRMODE_IZY,		5,		"ORA"}},
		{OPCODE_ILL_12,		{OPCODE_ILL_12,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_13,		{OPCODE_ILL_13,		ADDRMODE_IZY,		8,		"SLO"}},
		{OPCODE_ILL_14,		{OPCODE_ILL_14,		ADDRMODE_ZPX,		4,		"NOP"}},
		{OPCODE_ORA_ZPX,	{OPCODE_ORA_ZPX,	ADDRMODE_ZPX,		4,		"ORA"}},
		{OPCODE_ASL_ZPX,	{OPCODE_ASL_ZPX,	ADDRMODE_ZPX,		6,		"ASL"}},
		{OPCODE_ILL_17,		{OPCODE_ILL_17,		ADDRMODE_ZPX,		6,		"SLO"}},
		{OPCODE_CLC,			{OPCODE_CLC,			ADDRMODE_IMP,		2,		"CLC"}},
		{OPCODE_ORA_ABY,	{OPCODE_ORA_ABY,	ADDRMODE_ABY,		4,		"ORA"}},
		{OPCODE_ILL_1A,		{OPCODE_ILL_1A,		ADDRMODE_IMP,		2,		"NOP"}},
		{OPCODE_ILL_1B,		{OPCODE_ILL_1B,		ADDRMODE_ABY,		7,		"SLO"}},
		{OPCODE_ILL_1C,		{OPCODE_ILL_1C,		ADDRMODE_ABX,		4,		"NOP"}},
		{OPCODE_ORA_ABX,	{OPCODE_ORA_ABX,	ADDRMODE_ABX,		4,		"ORA"}},
		{OPCODE_ASL_ABX,	{OPCODE_ASL_ABX,	ADDRMODE_ABX,		7,		"ASL"}},
		{OPCODE_ILL_1F,		{OPCODE_ILL_1F,		ADDRMODE_ABX,		7,		"SLO"}},
		{OPCODE_JSR_ABS,	{OPCODE_JSR_ABS,	ADDRMODE_ABS,		6,		"JSR"}},
		{OPCODE_AND_IZX,	{OPCODE_AND_IZX,	ADDRMODE_IZX,		6,		"AND"}},
		{OPCODE_ILL_22,		{OPCODE_ILL_22,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_23,		{OPCODE_ILL_23,		ADDRMODE_IZX,		8,		"RLA"}},
		{OPCODE_BIT_ZP,		{OPCODE_BIT_ZP,		ADDRMODE_ZP,		3,		"BIT"}},
		{OPCODE_AND_ZP,		{OPCODE_AND_ZP,		ADDRMODE_ZP,		3,		"AND"}},
		{OPCODE_ROL_ZP,		{OPCODE_ROL_ZP,		ADDRMODE_ZP,		5,		"ROL"}},
		{OPCODE_ILL_27,		{OPCODE_ILL_27,		ADDRMODE_ZP,		5,		"RLA"}},
		{OPCODE_PLP,			{OPCODE_PLP,			ADDRMODE_IMP,		4,		"PLP"}},
		{OPCODE_AND_IMM,	{OPCODE_AND_IMM,	ADDRMODE_IMM,		2,		"AND"}},
		{OPCODE_ROL,			{OPCODE_ROL,			ADDRMODE_ACC,		2,		"ROL"}},
		{OPCODE_ILL_2B,		{OPCODE_ILL_2B,		ADDRMODE_IMM,		2,		"ANC"}},
		{OPCODE_BIT_ABS,	{OPCODE_BIT_ABS,	ADDRMODE_ABS,		4,		"BIT"}},
		{OPCODE_AND_ABS,	{OPCODE_AND_ABS,	ADDRMODE_ABS,		4,		"AND"}},
		{OPCODE_ROL_ABS,	{OPCODE_ROL_ABS,	ADDRMODE_ABS,		6,		"ROL"}},
		{OPCODE_ILL_2F,		{OPCODE_ILL_2F,		ADDRMODE_ABS,		6,		"RLA"}},
		{OPCODE_BMI_REL,	{OPCODE_BMI_REL,	ADDRMODE_REL,		2,		"BMI"}},
		{OPCODE_AND_IZY,	{OPCODE_AND_IZY,	ADDRMODE_IZY,		5,		"AND"}},
		{OPCODE_ILL_32,		{OPCODE_ILL_32,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_33,		{OPCODE_ILL_33,		ADDRMODE_IZY,		8,		"RLA"}},
		{OPCODE_ILL_34,		{OPCODE_ILL_34,		ADDRMODE_ZPX,		4,		"NOP"}},
		{OPCODE_AND_ZPX,	{OPCODE_AND_ZPX,	ADDRMODE_ZPX,		4,		"AND"}},
		{OPCODE_ROL_ZPX,	{OPCODE_ROL_ZPX,	ADDRMODE_ZPX,		6,		"ROL"}},
		{OPCODE_ILL_37,		{OPCODE_ILL_37,		ADDRMODE_ZPX,		6,		"RLA"}},
		{OPCODE_SEC,			{OPCODE_SEC,			ADDRMODE_IMP,		2,		"SEC"}},
		{OPCODE_AND_ABY,	{OPCODE_AND_ABY,	ADDRMODE_ABY,		4,		"AND"}},
		{OPCODE_ILL_3A,		{OPCODE_ILL_3A,		ADDRMODE_IMP,		2,		"NOP"}},
		{OPCODE_ILL_3B,		{OPCODE_ILL_3B,		ADDRMODE_ABY,		7,		"RLA"}},
		{OPCODE_ILL_3C,		{OPCODE_ILL_3C,		ADDRMODE_ABX,		4,		"NOP"}},
		{OPCODE_AND_ABX,	{OPCODE_AND_ABX,	ADDRMODE_ABX,		4,		"AND"}},
		{OPCODE_ROL_ABX,	{OPCODE_ROL_ABX,	ADDRMODE_ABX,		7,		"ROL"}},
		{OPCODE_ILL_3F,		{OPCODE_ILL_3F,		ADDRMODE_ABX,		7,		"RLA"}},
		{OPCODE_RTI,			{OPCODE_RTI,			ADDRMODE_IMP,		6,		"RTI"}},
		{OPCODE_EOR_IZX,	{OPCODE_EOR_IZX,	ADDRMODE_IZX,		6,		"EOR"}},
		{OPCODE_ILL_42,		{OPCODE_ILL_42,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_43,		{OPCODE_ILL_43,		ADDRMODE_IZX,		8,		"SRE"}},
		{OPCODE_ILL_44,		{OPCODE_ILL_44,		ADDRMODE_ZP,		3,		"NOP"}},
		{OPCODE_EOR_ZP,		{OPCODE_EOR_ZP,		ADDRMODE_ZP,		3,		"EOR"}},
		{OPCODE_LSR_ZP,		{OPCODE_LSR_ZP,		ADDRMODE_ZP,		5,		"LSR"}},
		{OPCODE_ILL_47,		{OPCODE_ILL_47,		ADDRMODE_ZP,		5,		"SRE"}},
		{OPCODE_PHA,			{OPCODE_PHA,			ADDRMODE_IMP,		3,		"PHA"}},
		{OPCODE_EOR_IMM,	{OPCODE_EOR_IMM,	ADDRMODE_IMM,		2,		"EOR"}},
		{OPCODE_LSR,			{OPCODE_LSR,			ADDRMODE_ACC,		2,		"LSR"}},
		{OPCODE_ILL_4B,		{OPCODE_ILL_4B,		ADDRMODE_IMM,		2,		"ALR"}},
		{OPCODE_JMP_ABS,	{OPCODE_JMP_ABS,	ADDRMODE_ABS,		3,		"JMP"}},
		{OPCODE_EOR_ABS,	{OPCODE_EOR_ABS,	ADDRMODE_ABS,		4,		"EOR"}},
		{OPCODE_LSR_ABS,	{OPCODE_LSR_ABS,	ADDRMODE_ABS,		6,		"LSR"}},
		{OPCODE_ILL_4F,		{OPCODE_ILL_4F,		ADDRMODE_ABS,		6,		"SRE"}},
		{OPCODE_BVC_REL,	{OPCODE_BVC_REL,	ADDRMODE_REL,		2,		"BVC"}},
		{OPCODE_EOR_IZY,	{OPCODE_EOR_IZY,	ADDRMODE_IZY,		5,		"EOR"}},
		{OPCODE_ILL_52,		{OPCODE_ILL_52,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_53,		{OPCODE_ILL_53,		ADDRMODE_IZY,		8,		"SRE"}},
		{OPCODE_ILL_54,		{OPCODE_ILL_54,		ADDRMODE_ZPX,		4,		"NOP"}},
		{OPCODE_EOR_ZPX,	{OPCODE_EOR_ZPX,	ADDRMODE_ZPX,		4,		"EOR"}},
		{OPCODE_LSR_ZPX,	{OPCODE_LSR_ZPX,	ADDRMODE_ZPX,		6,		"LSR"}},
		{OPCODE_ILL_57,		{OPCODE_ILL_57,		ADDRMODE_ZPX,		6,		"SRE"}},
		{OPCODE_CLI,			{OPCODE_CLI,			ADDRMODE_IMP,		2,		"CLI"}},
		{OPCODE_EOR_ABY,	{OPCODE_EOR_ABY,	ADDRMODE_ABY,		4,		"EOR"}},
		{OPCODE_ILL_5A,		{OPCODE_ILL_5A,		ADDRMODE_IMP,		2,		"NOP"}},
		{OPCODE_ILL_5B,		{OPCODE_ILL_5B,		ADDRMODE_ABY,		7,		"SRE"}},
		{OPCODE_ILL_5C,		{OPCODE_ILL_5C,		ADDRMODE_ABX,		4,		"NOP"}},
		{OPCODE_EOR_ABX,	{OPCODE_EOR_ABX,	ADDRMODE_ABX,		4,		"EOR"}},
		{OPCODE_LSR_ABX,	{OPCODE_LSR_ABX,	ADDRMODE_ABX,		7,		"LSR"}},
		{OPCODE_ILL_5F,		{OPCODE_ILL_5F,		ADDRMODE_ABX,		7,		"SRE"}},
		{OPCODE_RTS,			{OPCODE_RTS,			ADDRMODE_IMP,		6,		"RTS"}},
		{OPCODE_ADC_IZX,	{OPCODE_ADC_IZX,	ADDRMODE_IZX,		6,		"ADC"}},
		{OPCODE_ILL_62,		{OPCODE_ILL_62,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_63,		{OPCODE_ILL_63,		ADDRMODE_IZX,		8,		"RRA"}},
		{OPCODE_ILL_64,		{OPCODE_ILL_64,		ADDRMODE_ZP,		3,		"NOP"}},
		{OPCODE_ADC_ZP,		{OPCODE_ADC_ZP,		ADDRMODE_ZP,		3,		"ADC"}},
		{OPCODE_ROR_ZP,		{OPCODE_ROR_ZP,		ADDRMODE_ZP,		5,		"ROR"}},
		{OPCODE_ILL_67,		{OPCODE_ILL_67,		ADDRMODE_ZP,		5,		"RRA"}},
		{OPCODE_PLA,			{OPCODE_PLA,			ADDRMODE_IMP,		4,		"PLA"}},
		{OPCODE_ADC_IMM,	{OPCODE_ADC_IMM,	ADDRMODE_IMM,		2,		"ADC"}},
		{OPCODE_ROR,			{OPCODE_ROR,			ADDRMODE_ACC,		2,		"ROR"}},
		{OPCODE_ILL_6B,		{OPCODE_ILL_6B,		ADDRMODE_IMM,		2,		"ARR"}},
		{OPCODE_JMP_IND,	{OPCODE_JMP_IND,	ADDRMODE_IND,		5,		"JMP"}},
		{OPCODE_ADC_ABS,	{OPCODE_ADC_ABS,	ADDRMODE_ABS,		4,		"ADC"}},
		{OPCODE_ROR_ABS,	{OPCODE_ROR_ABS,	ADDRMODE_ABS,		6,		"ROR"}},
		{OPCODE_ILL_6F,		{OPCODE_ILL_6F,		ADDRMODE_ABS,		6,		"RRA"}},
		{OPCODE_BVS_REL,	{OPCODE_BVS_REL,	ADDRMODE_REL,		2,		"BVS"}},
		{OPCODE_ADC_IZY,	{OPCODE_ADC_IZY,	ADDRMODE_IZY,		5,		"ADC"}},
		{OPCODE_ILL_72,		{OPCODE_ILL_72,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_73,		{OPCODE_ILL_73,		ADDRMODE_IZY,		8,		"RRA"}},
		{OPCODE_ILL_74,		{OPCODE_ILL_74,		ADDRMODE_ZPX,		4,		"NOP"}},
		{OPCODE_ADC_ZPX,	{OPCODE_ADC_ZPX,	ADDRMODE_ZPX,		4,		"ADC"}},
		{OPCODE_ROR_ZPX,	{OPCODE_ROR_ZPX,	ADDRMODE_ZPX,		6,		"ROR"}},
		{OPCODE_ILL_77,		{OPCODE_ILL_77,		ADDRMODE_ZPX,		6,		"RRA"}},
		{OPCODE_SEI,			{OPCODE_SEI,			ADDRMODE_IMP,		2,		"SEI"}},
		{OPCODE_ADC_ABY,	{OPCODE_ADC_ABY,	ADDRMODE_ABY,		4,		"ADC"}},
		{OPCODE_ILL_7A,		{OPCODE_ILL_7A,		ADDRMODE_IMP,		2,		"NOP"}},
		{OPCODE_ILL_7B,		{OPCODE_ILL_7B,		ADDRMODE_ABY,		7,		"RRA"}},
		{OPCODE_ILL_7C,		{OPCODE_ILL_7C,		ADDRMODE_ABX,		4,		"NOP"}},
		{OPCODE_ADC_ABX,	{OPCODE_ADC_ABX,	ADDRMODE_ABX,		4,		"ADC"}},
		{OPCODE_ROR_ABX,	{OPCODE_ROR_ABX,	ADDRMODE_ABX,		7,		"ROR"}},
		{OPCODE_ILL_7F,		{OPCODE_ILL_7F,		ADDRMODE_ABX,		7,		"RRA"}},
		{OPCODE_ILL_80,		{OPCODE_ILL_80,		ADDRMODE_IMM,		2,		"NOP"}},
		{OPCODE_STA_IZX,	{OPCODE_STA_IZX,	ADDRMODE_IZX,		6,		"STA"}},
		{OPCODE_ILL_82,		{OPCODE_ILL_82,		ADDRMODE_IMM,		2,		"NOP"}},
		{OPCODE_ILL_83,		{OPCODE_ILL_83,		ADDRMODE_IZX,		6,		"SAX"}},
		{OPCODE_STY_ZP,		{OPCODE_STY_ZP,		ADDRMODE_ZP,		3,		"STY"}},
		{OPCODE_STA_ZP,		{OPCODE_STA_ZP,		ADDRMODE_ZP,		3,		"STA"}},
		{OPCODE_STX_ZP,		{OPCODE_STX_ZP,		ADDRMODE_ZP,		3,		"STX"}},
		{OPCODE_ILL_87,		{OPCODE_ILL_87,		ADDRMODE_ZP,		3,		"SAX"}},
		{OPCODE_DEY,			{OPCODE_DEY,			ADDRMODE_IMP,		2,		"DEY"}},
		{OPCODE_ILL_89,		{OPCODE_ILL_89,		ADDRMODE_IMM,		2,		"NOP"}},
		{OPCODE_TXA,			{OPCODE_TXA,			ADDRMODE_IMP,		2,		"TXA"}},
		{OPCODE_ILL_8B,		{OPCODE_ILL_8B,		ADDRMODE_IMM,		2,		"XAA"}},
		{OPCODE_STY_ABS,	{OPCODE_STY_ABS,	ADDRMODE_ABS,		4,		"STY"}},
		{OPCODE_STA_ABS,	{OPCODE_STA_ABS,	ADDRMODE_ABS,		4,		"STA"}},
		{OPCODE_STX_ABS,	{OPCODE_STX_ABS,	ADDRMODE_ABS,		4,		"STX"}},
		{OPCODE_ILL_8F,		{OPCODE_ILL_8F,		ADDRMODE_ABS,		4,		"SAX"}},
		{OPCODE_BCC_REL,	{OPCODE_BCC_REL,	ADDRMODE_REL,		2,		"BCC"}},
		{OPCODE_STA_IZY,	{OPCODE_STA_IZY,	ADDRMODE_IZY,		6,		"STA"}},
		{OPCODE_ILL_92,		{OPCODE_ILL_92,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_93,		{OPCODE_ILL_93,		ADDRMODE_IZY,		6,		"AHX"}},
		{OPCODE_STY_ZPX,	{OPCODE_STY_ZPX,	ADDRMODE_ZPX,		4,		"STY"}},
		{OPCODE_STA_ZPX,	{OPCODE_STA_ZPX,	ADDRMODE_ZPX,		4,		"STA"}},
		{OPCODE_STX_ZPY,	{OPCODE_STX_ZPY,	ADDRMODE_ZPY,		4,		"STX"}},
		{OPCODE_ILL_97,		{OPCODE_ILL_97,		ADDRMODE_ZPY,		4,		"SAX"}},
		{OPCODE_TYA,			{OPCODE_TYA,			ADDRMODE_IMP,		2,		"TYA"}},
		{OPCODE_STA_ABY,	{OPCODE_STA_ABY,	ADDRMODE_ABY,		5,		"STA"}},
		{OPCODE_TXS,			{OPCODE_TXS,			ADDRMODE_IMP,		2,		"TXS"}},
		{OPCODE_ILL_9B,		{OPCODE_ILL_9B,		ADDRMODE_ABY,		5,		"TAS"}},
		{OPCODE_ILL_9C,		{OPCODE_ILL_9C,		ADDRMODE_ABX,		5,		"SHY"}},
		{OPCODE_STA_ABX,	{OPCODE_STA_ABX,	ADDRMODE_ABX,		5,		"STA"}},
		{OPCODE_ILL_9E,		{OPCODE_ILL_9E,		ADDRMODE_ABY,		5,		"SHX"}},
		{OPCODE_ILL_9F,		{OPCODE_ILL_9F,		ADDRMODE_ABY,		5,		"AHX"}},
		{OPCODE_LDY_IMM,	{OPCODE_LDY_IMM,	ADDRMODE_IMM,		2,		"LDY"}},
		{OPCODE_LDA_IZX,	{OPCODE_LDA_IZX,	ADDRMODE_IZX,		6,		"LDA"}},
		{OPCODE_LDX_IMM,	{OPCODE_LDX_IMM,	ADDRMODE_IMM,		2,		"LDX"}},
		{OPCODE_ILL_A3,		{OPCODE_ILL_A3,		ADDRMODE_IZX,		6,		"LAX"}},
		{OPCODE_LDY_ZP,		{OPCODE_LDY_ZP,		ADDRMODE_ZP,		3,		"LDY"}},
		{OPCODE_LDA_ZP,		{OPCODE_LDA_ZP,		ADDRMODE_ZP,		3,		"LDA"}},
		{OPCODE_LDX_ZP,		{OPCODE_LDX_ZP,		ADDRMODE_ZP,		3,		"LDX"}},
		{OPCODE_ILL_A7,		{OPCODE_ILL_A7,		ADDRMODE_ZP,		3,		"LAX"}},
		{OPCODE_TAY,			{OPCODE_TAY,			ADDRMODE_IMP,		2,		"TAY"}},
		{OPCODE_LDA_IMM,	{OPCODE_LDA_IMM,	ADDRMODE_IMM,		2,		"LDA"}},
		{OPCODE_TAX,			{OPCODE_TAX,			ADDRMODE_IMP,		2,		"TAX"}},
		{OPCODE_ILL_AB,		{OPCODE_ILL_AB,		ADDRMODE_IMM,		2,		"LAX"}},
		{OPCODE_LDY_ABS,	{OPCODE_LDY_ABS,	ADDRMODE_ABS,		4,		"LDY"}},
		{OPCODE_LDA_ABS,	{OPCODE_LDA_ABS,	ADDRMODE_ABS,		4,		"LDA"}},
		{OPCODE_LDX_ABS,	{OPCODE_LDX_ABS,	ADDRMODE_ABS,		4,		"LDX"}},
		{OPCODE_ILL_AF,		{OPCODE_ILL_AF,		ADDRMODE_ABS,		4,		"LAX"}},
		{OPCODE_BCS_REL,	{OPCODE_BCS_REL,	ADDRMODE_REL,		2,		"BCS"}},
		{OPCODE_LDA_IZY,	{OPCODE_LDA_IZY,	ADDRMODE_IZY,		5,		"LDA"}},
		{OPCODE_ILL_B2,		{OPCODE_ILL_B2,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_B3,		{OPCODE_ILL_B3,		ADDRMODE_IZY,		5,		"LAX"}},
		{OPCODE_LDY_ZPX,	{OPCODE_LDY_ZPX,	ADDRMODE_ZPX,		4,		"LDY"}},
		{OPCODE_LDA_ZPX,	{OPCODE_LDA_ZPX,	ADDRMODE_ZPX,		4,		"LDA"}},
		{OPCODE_LDX_ZPY,	{OPCODE_LDX_ZPY,	ADDRMODE_ZPY,		4,		"LDX"}},
		{OPCODE_ILL_B7,		{OPCODE_ILL_B7,		ADDRMODE_ZPY,		4,		"LAX"}},
		{OPCODE_CLV,			{OPCODE_CLV,			ADDRMODE_IMP,		2,		"CLV"}},
		{OPCODE_LDA_ABY,	{OPCODE_LDA_ABY,	ADDRMODE_ABY,		4,		"LDA"}},
		{OPCODE_TSX,			{OPCODE_TSX,			ADDRMODE_IMP,		2,		"TSX"}},
		{OPCODE_ILL_BB,		{OPCODE_ILL_BB,		ADDRMODE_ABY,		4,		"LAS"}},
		{OPCODE_LDY_ABX,	{OPCODE_LDY_ABX,	ADDRMODE_ABX,		4,		"LDY"}},
		{OPCODE_LDA_ABX,	{OPCODE_LDA_ABX,	ADDRMODE_ABX,		4,		"LDA"}},
		{OPCODE_LDX_ABY,	{OPCODE_LDX_ABY,	ADDRMODE_ABY,		4,		"LDX"}},
		{OPCODE_ILL_BF,		{OPCODE_ILL_BF,		ADDRMODE_ABY,		4,		"LAX"}},
		{OPCODE_CPY_IMM,	{OPCODE_CPY_IMM,	ADDRMODE_IMM,		2,		"CPY"}},
		{OPCODE_CMP_IZX,	{OPCODE_CMP_IZX,	ADDRMODE_IZX,		6,		"CMP"}},
		{OPCODE_ILL_C2,		{OPCODE_ILL_C2,		ADDRMODE_IMM,		2,		"NOP"}},
		{OPCODE_ILL_C3,		{OPCODE_ILL_C3,		ADDRMODE_IZX,		8,		"DCP"}},
		{OPCODE_CPY_ZP,		{OPCODE_CPY_ZP,		ADDRMODE_ZP,		3,		"CPY"}},
		{OPCODE_CMP_ZP,		{OPCODE_CMP_ZP,		ADDRMODE_ZP,		3,		"CMP"}},
		{OPCODE_DEC_ZP,		{OPCODE_DEC_ZP,		ADDRMODE_ZP,		5,		"DEC"}},
		{OPCODE_ILL_C7,		{OPCODE_ILL_C7,		ADDRMODE_ZP,		5,		"DCP"}},
		{OPCODE_INY,			{OPCODE_INY,			ADDRMODE_IMP,		2,		"INY"}},
		{OPCODE_CMP_IMM,	{OPCODE_CMP_IMM,	ADDRMODE_IMM,		2,		"CMP"}},
		{OPCODE_DEX,			{OPCODE_DEX,			ADDRMODE_IMP,		2,		"DEX"}},
		{OPCODE_ILL_CB,		{OPCODE_ILL_CB,		ADDRMODE_IMM,		2,		"AXS"}},
		{OPCODE_CPY_ABS,	{OPCODE_CPY_ABS,	ADDRMODE_ABS,		4,		"CPY"}},
		{OPCODE_CMP_ABS,	{OPCODE_CMP_ABS,	ADDRMODE_ABS,		4,		"CMP"}},
		{OPCODE_DEC_ABS,	{OPCODE_DEC_ABS,	ADDRMODE_ABS,		6,		"DEC"}},
		{OPCODE_ILL_CF,		{OPCODE_ILL_CF,		ADDRMODE_ABS,		6,		"DCP"}},
		{OPCODE_BNE_REL,	{OPCODE_BNE_REL,	ADDRMODE_REL,		2,		"BNE"}},
		{OPCODE_CMP_IZY,	{OPCODE_CMP_IZY,	ADDRMODE_IZY,		5,		"CMP"}},
		{OPCODE_ILL_D2,		{OPCODE_ILL_D2,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_D3,		{OPCODE_ILL_D3,		ADDRMODE_IZY,		8,		"DCP"}},
		{OPCODE_ILL_D4,		{OPCODE_ILL_D4,		ADDRMODE_ZPX,		4,		"NOP"}},
		{OPCODE_CMP_ZPX,	{OPCODE_CMP_ZPX,	ADDRMODE_ZPX,		4,		"CMP"}},
		{OPCODE_DEC_ZPX,	{OPCODE_DEC_ZPX,	ADDRMODE_ZPX,		6,		"DEC"}},
		{OPCODE_ILL_D7,		{OPCODE_ILL_D7,		ADDRMODE_ZPX,		6,		"DCP"}},
		{OPCODE_CLD,			{OPCODE_CLD,			ADDRMODE_IMP,		2,		"CLD"}},
		{OPCODE_CMP_ABY,	{OPCODE_CMP_ABY,	ADDRMODE_ABY,		4,		"CMP"}},
		{OPCODE_ILL_DA,		{OPCODE_ILL_DA,		ADDRMODE_IMP,		2,		"NOP"}},
		{OPCODE_ILL_DB,		{OPCODE_ILL_DB,		ADDRMODE_ABY,		7,		"DCP"}},
		{OPCODE_ILL_DC,		{OPCODE_ILL_DC,		ADDRMODE_ABX,		4,		"NOP"}},
		{OPCODE_CMP_ABX,	{OPCODE_CMP_ABX,	ADDRMODE_ABX,		4,		"CMP"}},
		{OPCODE_DEC_ABX,	{OPCODE_DEC_ABX,	ADDRMODE_ABX,		7,		"DEC"}},
		{OPCODE_ILL_DF,		{OPCODE_ILL_DF,		ADDRMODE_ABX,		7,		"DCP"}},
		{OPCODE_CPX_IMM,	{OPCODE_CPX_IMM,	ADDRMODE_IMM,		2,		"CPX"}},
		{OPCODE_SBC_IZX,	{OPCODE_SBC_IZX,	ADDRMODE_IZX,		6,		"SBC"}},
		{OPCODE_ILL_E2,		{OPCODE_ILL_E2,		ADDRMODE_IMM,		2,		"NOP"}},
		{OPCODE_ILL_E3,		{OPCODE_ILL_E3,		ADDRMODE_IZX,		8,		"ISC"}},
		{OPCODE_CPX_ZP,		{OPCODE_CPX_ZP,		ADDRMODE_ZP,		3,		"CPX"}},
		{OPCODE_SBC_ZP,		{OPCODE_SBC_ZP,		ADDRMODE_ZP,		3,		"SBC"}},
		{OPCODE_INC_ZP,		{OPCODE_INC_ZP,		ADDRMODE_ZP,		5,		"INC"}},
		{OPCODE_ILL_E7,		{OPCODE_ILL_E7,		ADDRMODE_ZP,		5,		"ISC"}},
		{OPCODE_INX,			{OPCODE_INX,			ADDRMODE_IMP,		2,		"INX"}},
		{OPCODE_SBC_IMM,	{OPCODE_SBC_IMM,	ADDRMODE_IMM,		2,		"SBC"}},
		{OPCODE_NOP,			{OPCODE_NOP,			ADDRMODE_IMP,		2,		"NOP"}},
		{OPCODE_ILL_EB,		{OPCODE_ILL_EB,		ADDRMODE_IMM,		2,		"SBC"}},
		{OPCODE_CPX_ABS,	{OPCODE_CPX_ABS,	ADDRMODE_ABS,		4,		"CPX"}},
		{OPCODE_SBC_ABS,	{OPCODE_SBC_ABS,	ADDRMODE_ABS,		4,		"SBC"}},
		{OPCODE_INC_ABS,	{OPCODE_INC_ABS,	ADDRMODE_ABS,		6,		"INC"}},
		{OPCODE_ILL_EF,		{OPCODE_ILL_EF,		ADDRMODE_ABS,		6,		"ISC"}},
		{OPCODE_BEQ_REL,	{OPCODE_BEQ_REL,	ADDRMODE_REL,		2,		"BEQ"}},
		{OPCODE_SBC_IZY,	{OPCODE_SBC_IZY,	ADDRMODE_IZY,		5,		"SBC"}},
		{OPCODE_ILL_F2,		{OPCODE_ILL_F2,		ADDRMODE_UND,		0,		"ILL"}},
		{OPCODE_ILL_F3,		{OPCODE_ILL_F3,		ADDRMODE_IZY,		8,		"ISC"}},
		{OPCODE_ILL_F4,		{OPCODE_ILL_F4,		ADDRMODE_ZPX,		4,		"NOP"}},
		{OPCODE_SBC_ZPX,	{OPCODE_SBC_ZPX,	ADDRMODE_ZPX,		4,		"SBC"}},
		{OPCODE_INC_ZPX,	{OPCODE_INC_ZPX,	ADDRMODE_ZPX,		6,		"INC"}},
		{OPCODE_ILL_F7,		{OPCODE_ILL_F7,		ADDRMODE_ZPX,		6,		"ISC"}},
		{OPCODE_SED,			{OPCODE_SED,			ADDRMODE_IMP,		2,		"SED"}},
		{OPCODE_SBC_ABY,	{OPCODE_SBC_ABY,	ADDRMODE_ABY,		4,		"SBC"}},
		{OPCODE_ILL_FA,		{OPCODE_ILL_FA,		ADDRMODE_IMP,		2,		"NOP"}},
		{OPCODE_ILL_FB,		{OPCODE_ILL_FB,		ADDRMODE_ABY,		7,		"ISC"}},
		{OPCODE_ILL_FC,		{OPCODE_ILL_FC,		ADDRMODE_ABX,		4,		"NOP"}},
		{OPCODE_SBC_ABX,	{OPCODE_SBC_ABX,	ADDRMODE_ABX,		4,		"SBC"}},
		{OPCODE_INC_ABX,	{OPCODE_INC_ABX,	ADDRMODE_ABX,		7,		"INC"}},
		{OPCODE_ILL_FF,		{OPCODE_ILL_FF,		ADDRMODE_ABX,		7,		"ISC"}}
	};
	mOpCodesMap = myOpCodesMap;
	mReg.Acc = 0;
	mReg.Acc16 = 0;
	mReg.Flags = FLAGS_UNUSED;
	mReg.IndX = 0;
	mReg.IndY = 0;
	mReg.Ptr16 = 0;
	mReg.PtrAddr = 0;
	mReg.PtrStack = 0xFF;	// top of stack	
	mReg.SoftIrq = false;
	mLocalMem = false;
	if (NULL == mpMem) {
		mpMem = new Memory();
		if (NULL == mpMem) {
			throw MKGenException("Unable to allocate memory!");
		}
		mLocalMem = true;
	}	
	// Set default BRK vector ($FFFE -> $FFF0)
	mpMem->Poke8bit(0xFFFE,0xF0); // LSB
	mpMem->Poke8bit(0xFFFF,0xFF); // MSB
	// Put RTI opcode at BRK procedure address.
	mpMem->Poke8bit(0xFFF0, OPCODE_RTI);
	// Set default RESET vector ($FFFC -> $0200)
	mpMem->Poke8bit(0xFFFC,0x00); // LSB
	mpMem->Poke8bit(0xFFFD,0x02); // MSB
	// Set BRK code at the RESET procedure address.
	mpMem->Poke8bit(0x0200,OPCODE_BRK);
}

/*
 *--------------------------------------------------------------------
 * Method:		SetFlags()
 * Purpose:		Set CPU status flags ZERO and SIGN based on Acc, X or Y
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void MKCpu::SetFlags(unsigned char reg)
{
	SetFlag((0 == reg), FLAGS_ZERO);
	SetFlag(((reg & FLAGS_SIGN) == FLAGS_SIGN), FLAGS_SIGN);
	SetFlag(true, FLAGS_UNUSED);
}

/*
 *--------------------------------------------------------------------
 * Method:		ShiftLeft()
 * Purpose:		Arithmetic shift left (1 bit), set Carry flag, shift 0
 *            into bit #0. Update flags NZ.
 * Arguments:	arg8 - 8-bit value
 * Returns:		8-bit value after shift
 *--------------------------------------------------------------------
 */
unsigned char MKCpu::ShiftLeft(unsigned char arg8)
{
	// set Carry flag based on original bit #7
	SetFlag(((arg8 & FLAGS_SIGN) == FLAGS_SIGN), FLAGS_CARRY);
	arg8 = arg8 << 1;	// shift left
	arg8 &= 0xFE;			// shift 0 into bit #0
	
	SetFlags(arg8);
	
	return arg8;
} 

/*
 *--------------------------------------------------------------------
 * Method:		ShiftRight()
 * Purpose:		Logical Shift Right, update flags NZC.
 * Arguments:	arg8 - byte value
 * Returns:		unsigned char (byte) - after shift
 *--------------------------------------------------------------------
 */
unsigned char MKCpu::ShiftRight(unsigned char arg8)
{
	SetFlag(((arg8 & 0x01) == 0x01), FLAGS_CARRY);
	arg8 = arg8 >> 1;
	arg8 &= 0x7F;	// unset bit #7
	SetFlags(arg8);
	
	return arg8;
}

/*
 *--------------------------------------------------------------------
 * Method:		RotateLeft()
 * Purpose:		Rotate left, Carry to bit 0, bit 7 to Carry, update
 							flags N and Z.
 * Arguments: arg8 - byte value to rotate
 * Returns:		unsigned char (byte) - rotated value
 *--------------------------------------------------------------------
 */
unsigned char MKCpu::RotateLeft(unsigned char arg8)
{
	unsigned char tmp8 = 0;
	
	tmp8 = arg8;
	arg8 = arg8 << 1;
	// Carry goes to bit #0.
	if (mReg.Flags & FLAGS_CARRY) {
		arg8 |= 0x01;
	} else {
		arg8 &= 0xFE;
	}
	// Original (before ROL) bit #7 goes to Carry.
	SetFlag(((tmp8 & FLAGS_SIGN) == FLAGS_SIGN), FLAGS_CARRY);
	SetFlags(arg8);	// set flags Z and N	
	
	return arg8;
}

/*
 *--------------------------------------------------------------------
 * Method:		RotateRight()
 * Purpose:		Rotate Right, Carry to bit 7, bit 0 to Carry, update
 							flags N and Z.
 * Arguments: arg8 - byte value to rotate
 * Returns:		unsigned char (byte) - rotated value
 *--------------------------------------------------------------------
 */
unsigned char MKCpu::RotateRight(unsigned char arg8)
{
	unsigned char tmp8 = 0;
	
	tmp8 = arg8;
	arg8 = arg8 >> 1;
	// Carry goes to bit #7.
	if (CheckFlag(FLAGS_CARRY)) {
		arg8 |= 0x80;
	} else {
		arg8 &= 0x7F;
	}
	// Original (before ROR) bit #0 goes to Carry.
	SetFlag(((tmp8 & 0x01) == 0x01), FLAGS_CARRY);
	SetFlags(arg8);	// set flags Z and N	
	
	return arg8;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetArg16()
 * Purpose:		Get 2-byte argument, add offset, increase PC.
 * Arguments:	addr - address of argument in memory
 *						offs - offset to be added to returned value
 * Returns:		16-bit address
 *--------------------------------------------------------------------
 */
unsigned short MKCpu::GetArg16(unsigned char offs)
{
	unsigned short ret = 0;
	
	ret = mpMem->Peek16bit(mReg.PtrAddr++);
	mReg.PtrAddr++;
	ret += offs;
	
	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		LogicOpAcc()
 * Purpose:		Perform logical bitwise operation between memory
 *						location and Acc, result in Acc. Set flags.
 * Arguments:	addr - memory location
 *						logop - logical operation code: LOGOP_OR, LOGOP_AND,
 *																						LOGOP_EOR
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void MKCpu::LogicOpAcc(unsigned short addr, int logop)
{
	unsigned char val = 0;
	
	val = mpMem->Peek8bit(addr);
	switch (logop) {
		case LOGOP_OR:
			mReg.Acc |= val;
			break;
		case LOGOP_AND:
			mReg.Acc &= val;
			break;
		case LOGOP_EOR:
			mReg.Acc ^= val;
			break;
		default:
			break;
	}
	SetFlags(mReg.Acc);			
}

/*
 *--------------------------------------------------------------------
 * Method:		ComputeRelJump()
 * Purpose:		Compute new PC based on relative offset.
 * Arguments:	offs - relative offset [-128 ($80)..127 ($7F)]
 * Returns:		unsigned short - new PC (Program Counter).
 * NOTE:
 * Program Counter (http://www.6502.org/tutorials/6502opcodes.html#PC)
 * When the 6502 is ready for the next instruction it increments the 
 * program counter before fetching the instruction. Once it has the op
 * code, it increments the program counter by the length of the
 * operand, if any. This must be accounted for when calculating
 * branches or when pushing bytes to create a false return address
 * (i.e. jump table addresses are made up of addresses-1 when it is
 * intended to use an RTS rather than a JMP).
 * The program counter is loaded least signifigant byte first.
 * Therefore the most signifigant byte must be pushed first when
 * creating a false return address.
 * When calculating branches a forward branch of 6 skips the following
 * 6 bytes so, effectively the program counter points to the address
 * that is 8 bytes beyond the address of the branch opcode;
 * and a backward branch of $FA (256-6) goes to an address 4 bytes
 * before the branch instruction. 
 *--------------------------------------------------------------------
 */
unsigned short MKCpu::ComputeRelJump(unsigned char offs)
{
	// NOTE: mReg.PtrAddr (PC) must be at the next opcode (after branch instr.) at this point.
	return ComputeRelJump(mReg.PtrAddr, offs);
}

/*
 *--------------------------------------------------------------------
 * Method:		ComputeRelJump()
 * Purpose:		Compute new address after branch based on relative
 *            offset.
 * Arguments:	addr - next opcode address (after branch instr.)
 *            offs - relative offset [-128 ($80)..127 ($7F)]
 * Returns:		unsigned short - new address for branch jump
 *--------------------------------------------------------------------
 */
unsigned short MKCpu::ComputeRelJump(unsigned short addr, unsigned char offs)
{
	unsigned short newpc = addr;
	
	if (offs < 0x80) {
		newpc += (unsigned short) offs;
	} else {
		newpc -= (unsigned short) ((unsigned char)(~offs + 1));  // use binary 2's complement instead of arithmetics
	}	
	
	return newpc;
}

/*
 *--------------------------------------------------------------------
 * Method:		Conv2Bcd()
 * Purpose:		Convert 16-bit unsigned number to 8-bit BCD
 *            representation.
 * Arguments:	v - 16-bit unsigned integer.
 * Returns:		byte representing BCD code of the 'v'.
 *--------------------------------------------------------------------
 */
unsigned char MKCpu::Conv2Bcd(unsigned short v)
{
   unsigned char arg8 = 0;
   arg8 = (unsigned char)((v/10) << 4);
   arg8 |= ((unsigned char)(v - (v/10)*10)) & 0x0F;
   return arg8;
}

/*
 *--------------------------------------------------------------------
 * Method:		Bcd2Num()
 * Purpose:		Convert 8-bit BCD code to a number.
 * Arguments:	v - BCD code.
 * Returns:		16-bit unsigned integer
 *--------------------------------------------------------------------
 */
unsigned short MKCpu::Bcd2Num(unsigned char v)
{
	unsigned short ret = 0;
	ret = 10 * ((v & 0xF0) >> 4) + (unsigned char)(v & 0x0F);
	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		CheckFlag()
 * Purpose:		Check if given bit flag in CPU status register is set
 *            or not.
 * Arguments:	flag - byte with the bit to be checked set and other
 *                   bits not set.
 * Returns:		bool
 *--------------------------------------------------------------------
 */
bool MKCpu::CheckFlag(unsigned char flag)
{
	return ((mReg.Flags & flag) == flag);
}

/*
 *--------------------------------------------------------------------
 * Method:		SetFlag()
 * Purpose:		Set or unset CPU status flag.
 * Arguments:	set - if true, set flag, if false, unset flag.
 *            flag - a byte with a bit set on the position of flag
 *                   being altered and zeroes on other positions.
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void MKCpu::SetFlag(bool set, unsigned char flag)
{
	if (set) {
		mReg.Flags |= flag;
	} else {
		mReg.Flags &= ~flag;
	}
}

/*
 *--------------------------------------------------------------------
 * Method:		AddWithCarry()
 * Purpose:		Add Acc + Mem with Carry, update flags and Acc.
 * Arguments:	mem8 - memory argument (byte)
 * Returns:		byte value Acc + Mem + Carry
 *--------------------------------------------------------------------
 */
unsigned char MKCpu::AddWithCarry(unsigned char mem8)
{
	// This algorithm was adapted from Frodo emulator code.
	unsigned short utmp16 = mReg.Acc + mem8 + (CheckFlag(FLAGS_CARRY) ? 1 : 0);
	if (CheckFlag(FLAGS_DEC)) {	// BCD mode
	
		unsigned short al = (mReg.Acc & 0x0F) + (mem8 & 0x0F) + (CheckFlag(FLAGS_CARRY) ? 1 : 0);
		if (al > 9) al += 6;
		unsigned short ah = (mReg.Acc >> 4) + (mem8 >> 4);
		if (al > 0x0F) ah++;
		SetFlag((utmp16 == 0), FLAGS_ZERO);
		SetFlag(((((ah << 4) ^ mReg.Acc) & 0x80) && !((mReg.Acc ^ mem8) & 0x80)), FLAGS_OVERFLOW);
		if (ah > 9) ah += 6;
		SetFlag((ah > 0x0F), FLAGS_CARRY);
		mReg.Acc = (ah << 4) | (al & 0x0f);
		SetFlag((mReg.Acc & FLAGS_SIGN) == FLAGS_SIGN, FLAGS_SIGN);
	} else {	// binary mode
	
		SetFlag((utmp16 > 0xff), FLAGS_CARRY);
		SetFlag((!((mReg.Acc ^ mem8) & 0x80) && ((mReg.Acc ^ utmp16) & 0x80)), FLAGS_OVERFLOW);
		mReg.Acc = utmp16 & 0xFF;
		SetFlag((mReg.Acc == 0), FLAGS_ZERO);
		SetFlag((mReg.Acc & FLAGS_SIGN) == FLAGS_SIGN, FLAGS_SIGN);
	}
	SetFlag(true, FLAGS_UNUSED);
	return mReg.Acc;
}

/*
 *--------------------------------------------------------------------
 * Method:		SubWithCarry()
 * Purpose:		Subtract Acc - Mem with Carry, update flags and Acc.
 * Arguments:	mem8 - memory argument (byte)
 * Returns:		byte value Acc - Mem - Carry
 *--------------------------------------------------------------------
 */
unsigned char MKCpu::SubWithCarry(unsigned char mem8)
{
	unsigned short utmp16 = mReg.Acc - mem8 - (CheckFlag(FLAGS_CARRY) ? 0 : 1);

	// This algorithm was adapted from Frodo emulator code.
	if (CheckFlag(FLAGS_DEC)) {	// BCD mode
	
		unsigned char al = (mReg.Acc & 0x0F) - (mem8 & 0x0F) - (CheckFlag(FLAGS_CARRY) ? 0 : 1);
		unsigned char ah = (mReg.Acc >> 4) - (mem8 >> 4);
		if (al & 0x10) {
			al -= 6; ah--;
		}
		if (ah & 0x10) ah -= 6;
		SetFlag((utmp16 < 0x100), FLAGS_CARRY);
		SetFlag(((mReg.Acc ^ utmp16) & 0x80) && ((mReg.Acc ^ mem8) & 0x80), FLAGS_OVERFLOW);
		SetFlag((utmp16 == 0), FLAGS_ZERO);
		mReg.Acc = (ah << 4) | (al & 0x0f);
		SetFlag((mReg.Acc & FLAGS_SIGN) == FLAGS_SIGN, FLAGS_SIGN);
		
	} else { // binary mode
	
		SetFlag((utmp16 < 0x100), FLAGS_CARRY);
		SetFlag(((mReg.Acc ^ utmp16) & 0x80) && ((mReg.Acc ^ mem8) & 0x80), FLAGS_OVERFLOW);
		mReg.Acc = utmp16 & 0xFF;
		SetFlag((mReg.Acc == 0), FLAGS_ZERO);
		SetFlag((mReg.Acc & FLAGS_SIGN) == FLAGS_SIGN, FLAGS_SIGN);
	
	}
	SetFlag(true, FLAGS_UNUSED);
	return mReg.Acc;
}

/*
 *--------------------------------------------------------------------
 * Method:		~MKCpu()
 * Purpose:		Class destructor.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
MKCpu::~MKCpu()
{
	if (mLocalMem) {
		if (NULL != mpMem)
			delete mpMem;
	}
}

/*
 *--------------------------------------------------------------------
 * Method:		GetAddrWithMode()
 * Purpose:		Get address of the argument with specified mode.
 *            Increment PC.
 * Arguments:	mode - code of the addressing mode, see eAddrModes.
 * Returns:		16-bit address
 *--------------------------------------------------------------------
 */
unsigned short MKCpu::GetAddrWithMode(int mode)
{
	unsigned short arg16 = 0;
	
	mReg.LastAddrMode = mode;
	switch (mode) {
		
		case ADDRMODE_IMM:
			arg16 = mReg.PtrAddr++;
			break;
			
		case ADDRMODE_ABS:
			mReg.LastArg = arg16 = GetArg16(0);
			break;
			
		case ADDRMODE_ZP:
			mReg.LastArg = arg16 = (unsigned short) mpMem->Peek8bit(mReg.PtrAddr++);
			break;
			
		case ADDRMODE_IMP:
			// DO NOTHING - implied mode operates on internal CPU register
			break;
			
		case ADDRMODE_IND:
			mReg.LastArg = arg16 = mpMem->Peek16bit(mReg.PtrAddr++);
			arg16 = mpMem->Peek16bit(arg16);
			break;
			
		case ADDRMODE_ABX:
			arg16 = GetArg16(mReg.IndX);
			mReg.LastArg = arg16 - mReg.IndX;
			break;
			
		case ADDRMODE_ABY:
			arg16 = GetArg16(mReg.IndY);
			mReg.LastArg = arg16 - mReg.IndY;
			break;
			
		case ADDRMODE_ZPX:
			mReg.LastArg = arg16 = mpMem->Peek8bit(mReg.PtrAddr++);
			arg16 = (arg16 + mReg.IndX) & 0xFF;
			break;
			
		case ADDRMODE_ZPY:
			mReg.LastArg = arg16 = mpMem->Peek8bit(mReg.PtrAddr++);
			arg16 = (arg16 + mReg.IndY) & 0xFF;
			break;
			
		case ADDRMODE_IZX:
			mReg.LastArg = arg16 = mpMem->Peek8bit(mReg.PtrAddr++);
			arg16 = (arg16 + mReg.IndX) & 0xFF;
			arg16 = mpMem->Peek16bit(arg16);			
			break;
			
		case ADDRMODE_IZY:
			mReg.LastArg = arg16 = mpMem->Peek8bit(mReg.PtrAddr++);
			arg16 = mpMem->Peek16bit(arg16) + mReg.IndY;			
			break;
			
		case ADDRMODE_REL:
			mReg.LastArg = arg16 = ComputeRelJump(mpMem->Peek8bit(mReg.PtrAddr++));
			break;
			
		case ADDRMODE_ACC:
			// DO NOTHING - acc mode operates on internal CPU register
			break;
			
		default:
			throw MKGenException("ERROR: Wrong addressing mode!");
			break;
	}
	
	return arg16;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetArgWithMode()
 * Purpose:		Get argument from address with specified mode.
 * Arguments:	addr - address in memory
 *            mode - code of the addressing mode, see eAddrModes.
 * Returns:		argument
 *--------------------------------------------------------------------
 */
unsigned short MKCpu::GetArgWithMode(unsigned short addr, int mode)
{
	unsigned short arg16 = 0;
	
	switch (mode) {
		
		case ADDRMODE_IMM:
			arg16 = mpMem->Peek8bit(addr);
			break;
			
		case ADDRMODE_ABS:
			arg16 = mpMem->Peek16bit(addr);
			break;
			
		case ADDRMODE_ZP:
			arg16 = (unsigned short) mpMem->Peek8bit(addr);
			break;
			
		case ADDRMODE_IMP:
			// DO NOTHING - implied mode operates on internal CPU register
			break;
			
		case ADDRMODE_IND:
			arg16 = mpMem->Peek16bit(addr);
			break;
			
		case ADDRMODE_ABX:
			arg16 = mpMem->Peek16bit(addr);
			break;
			
		case ADDRMODE_ABY:
			arg16 = mpMem->Peek16bit(addr);
			break;
			
		case ADDRMODE_ZPX:
			arg16 = mpMem->Peek8bit(addr);
			break;
			
		case ADDRMODE_ZPY:
			arg16 = mpMem->Peek8bit(addr);
			break;
			
		case ADDRMODE_IZX:
			arg16 = mpMem->Peek8bit(addr);
			break;
			
		case ADDRMODE_IZY:
			arg16 = mpMem->Peek8bit(addr);
			break;
			
		case ADDRMODE_REL:
			arg16 = ComputeRelJump(addr+1, mpMem->Peek8bit(addr));
			break;
			
		case ADDRMODE_ACC:
			// DO NOTHING - acc mode operates on internal CPU register
			break;
			
		default:
			break;
	}
	
	return arg16;
}

/*
 *--------------------------------------------------------------------
 * Method:		Disassemble()
 * Purpose:		Disassemble instruction and argument per addressing mode
 * Arguments:	n/a - internal
 * Returns:		0
 *--------------------------------------------------------------------
 */
unsigned short MKCpu::Disassemble()
{
	char sArg[40];
	char sFmt[20];

	strcpy(sFmt, "%s ");
	strcat(sFmt, mArgFmtTbl[mReg.LastAddrMode].c_str());
	sprintf(sArg, sFmt, 
								((mOpCodesMap[(eOpCodes)mReg.LastOpCode]).amf.length() > 0 
											? (mOpCodesMap[(eOpCodes)mReg.LastOpCode]).amf.c_str() : "???"),
								mReg.LastArg);
	for (unsigned int i=0; i<strlen(sArg); i++) sArg[i] = toupper(sArg[i]);
	mReg.LastInstr = sArg;
	
	return 0;
}

/*
 *--------------------------------------------------------------------
 * Method:		Disassemble()
 * Purpose:		Disassemble instruction in memory.
 * Arguments:	addr - address in memory
 *            instrbuf - pointer to a character buffer, this is where
 *                       instructions will be disassembled
 * Returns:		unsigned short - address of next instruction
 *--------------------------------------------------------------------
 */
unsigned short MKCpu::Disassemble(unsigned short opcaddr, char *instrbuf)
{
	char sArg[DISS_BUF_SIZE] = {0};
	char sBuf[10] = {0};
	char sFmt[40] = {0};
	int addr = opcaddr;
	int opcode = -1;
	int addrmode = -1;

	opcode = mpMem->Peek8bit(addr++);
	addrmode = (mOpCodesMap[(eOpCodes)opcode]).amf.length() > 0
							? (mOpCodesMap[(eOpCodes)opcode]).addrmode : -1;
							
  if (addrmode < 0 || NULL == instrbuf) return 0;
  switch (mAddrModesLen[addrmode])
  {
  	case 2:
  		sprintf(sBuf, "$%02x    ", mpMem->Peek8bit(addr));
  		break;
  		
  	case 3:
  		sprintf(sBuf, "$%02x $%02x", mpMem->Peek8bit(addr), mpMem->Peek8bit(addr+1));
  		break;
  		  	
  	default:
  		strcpy(sBuf, "       ");
  		break;
	}
	strcpy(sFmt, "$%04x: $%02x %s   %s ");
	strcat(sFmt, mArgFmtTbl[addrmode].c_str());
	sprintf(sArg, sFmt, opcaddr, mpMem->Peek8bit(opcaddr), sBuf,
								((mOpCodesMap[(eOpCodes)opcode]).amf.length() > 0 
											? (mOpCodesMap[(eOpCodes)opcode]).amf.c_str() : "???"),
								GetArgWithMode(addr,addrmode));
	for (unsigned int i=0; i<strlen(sArg); i++) sArg[i] = toupper(sArg[i]);
	strcpy(instrbuf, sArg);
	
	return opcaddr + mAddrModesLen[addrmode];
}

/*
 *--------------------------------------------------------------------
 * Method:		ExecOpcode()
 * Purpose:		Execute VM's opcode.
 * Arguments:	memaddr - address of code in virtual memory.
 * Returns:		Pointer to CPU registers and flags structure.
 *--------------------------------------------------------------------
 */
Regs *MKCpu::ExecOpcode(unsigned short memaddr)
{
	mReg.PtrAddr = memaddr;
	mReg.LastAddr = memaddr;
	unsigned char opcode = mpMem->Peek8bit(mReg.PtrAddr++);
	unsigned char arg8 = 0;
	unsigned short arg16 = 0;
	
	SetFlag(false, FLAGS_BRK);	// reset BRK flag - we want to detect
	mReg.SoftIrq = false;				// software interrupt each time it happens
															// (non-maskable)
	mReg.LastRTS = false;
	mReg.LastOpCode = opcode;
	mReg.LastAddrMode = ADDRMODE_UND;
	mReg.LastArg = 0;
				
	switch (opcode) {
		
		case OPCODE_BRK:				// software interrupt, Implied ($00 : BRK)
			mReg.LastAddrMode = ADDRMODE_IMP;
			if (!CheckFlag(FLAGS_IRQ)) {	// only if IRQ not masked
				arg16 = 0x100;
				arg16 += mReg.PtrStack--;
				// Note that BRK is really a 2-bytes opcode. Each BRK opcode should be padded by extra byte,
				// because the return address put on stack is PC + 1 (where PC is the next address after BRK).
				// That means the next opcode after BRK will not be executed upon return from interrupt, 
				// but the next after that will be.
				mReg.PtrAddr++;
				mpMem->Poke8bit(arg16, (unsigned char) (((mReg.PtrAddr) & 0xFF00) >> 8));	// HI(PC+1) - HI part of next instr. addr. + 1
				arg16 = 0x100;
				arg16 += mReg.PtrStack--;
				mpMem->Poke8bit(arg16, (unsigned char) ((mReg.PtrAddr) & 0x00FF));	// LO(PC+1) - LO part of next instr. addr. + 1
				arg16 = 0x100;
				arg16 += mReg.PtrStack--;
				SetFlag(true, FLAGS_BRK);		// The BRK flag that goes on stack is set.
				mpMem->Poke8bit(arg16, mReg.Flags);
				SetFlag(false, FLAGS_BRK);	// The BRK flag that remains in CPU status is unchanged, so unset after putting on stack.
				//mReg.SoftIrq = true;
				mReg.PtrAddr = mpMem->Peek16bit(0xFFFE);	// Load BRK vector into the PC.
				SetFlag(true,FLAGS_IRQ);				
			} else {
				mReg.PtrAddr++;
			}
			break;
		
		case OPCODE_NOP:				// NO oPeration, Implied ($EA : NOP)
			mReg.LastAddrMode = ADDRMODE_IMP;
			break;
			
		case OPCODE_LDA_IZX:		// LoaD Accumulator, Indexed Indirect ($A1 arg : LDA (arg,X) ;arg=0..$FF), MEM=&(arg+X)
			arg16 = GetAddrWithMode(ADDRMODE_IZX);
			mReg.Acc = mpMem->Peek8bit(arg16);
			SetFlags(mReg.Acc);
			break;			
			
		case OPCODE_LDA_ZP:			// LoaD Accumulator, Zero Page ($A5 arg : LDA arg ;arg=0..$FF), MEM=arg
			mReg.Acc = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_ZP));
			SetFlags(mReg.Acc);		
			break;
			
		case OPCODE_LDA_IMM:		// LoaD Accumulator, Immediate ($A9 arg : LDA #arg ;arg=0..$FF), MEM=PC+1			
			mReg.LastArg = mReg.Acc = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
			SetFlags(mReg.Acc);
			break;

		case OPCODE_LDA_ABS:		// LoaD Accumulator, Absolute ($AD addrlo addrhi : LDA addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			mReg.Acc = mpMem->Peek8bit(arg16);
			SetFlags(mReg.Acc);
			break;
			
		case OPCODE_LDA_IZY:		// LoaD Accumulator, Indirect Indexed ($B1 arg : LDA (arg),Y ;arg=0..$FF), MEM=&arg+Y	
			arg16 = GetAddrWithMode(ADDRMODE_IZY);
			mReg.Acc = mpMem->Peek8bit(arg16);
			SetFlags(mReg.Acc);
			break;
			
		case OPCODE_LDA_ZPX:		// LoaD Accumulator, Zero Page Indexed, X ($B5 arg : LDA arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			mReg.Acc = mpMem->Peek8bit(arg16);
			SetFlags(mReg.Acc);
			break;
			
		case OPCODE_LDA_ABY:		// LoaD Accumulator, Absolute Indexed, Y ($B9 addrlo addrhi : LDA addr,Y ;addr=0..$FFFF), MEM=addr+Y
			arg16 = GetAddrWithMode(ADDRMODE_ABY);
			mReg.Acc = mpMem->Peek8bit(arg16);
			SetFlags(mReg.Acc);		
			break;
			
		case OPCODE_LDA_ABX:		// LoaD Accumulator, Absolute Indexed, X ($BD addrlo addrhi : LDA addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			mReg.Acc = mpMem->Peek8bit(arg16);
			SetFlags(mReg.Acc);
			break;

		case OPCODE_LDX_IMM:		// LoaD X register, Immediate ($A2 arg : LDX #arg ;arg=0..$FF), MEM=PC+1
			mReg.LastArg = mReg.IndX = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
			SetFlags(mReg.IndX);
			break;
			
		case OPCODE_LDX_ZP:			// LoaD X register, Zero Page ($A6 arg : LDX arg ;arg=0..$FF), MEM=arg
			mReg.IndX = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_ZP));
			SetFlags(mReg.IndX);
			break;

		case OPCODE_LDX_ABS:		// LoaD X register, Absolute ($AE addrlo addrhi : LDX addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			mReg.IndX = mpMem->Peek8bit(arg16);
			SetFlags(mReg.IndX);
			break;
			
		case OPCODE_LDX_ZPY:		// LoaD X register, Zero Page Indexed, Y ($B6 arg : LDX arg,Y ;arg=0..$FF), MEM=arg+Y
			arg16 = GetAddrWithMode(ADDRMODE_ZPY);
			mReg.IndX = mpMem->Peek8bit(arg16);
			SetFlags(mReg.IndX);
			break;
			
		case OPCODE_LDX_ABY:		// LoaD X register, Absolute Indexed, Y ($BE addrlo addrhi : LDX addr,Y ;addr=0..$FFFF), MEM=addr+Y
			arg16 = GetAddrWithMode(ADDRMODE_ABY);
			mReg.IndX = mpMem->Peek8bit(arg16);
			SetFlags(mReg.IndX);
			break;
			
		case OPCODE_LDY_IMM:		// LoaD Y register, Immediate ($A0 arg : LDY #arg ;arg=0..$FF), MEM=PC+1
			mReg.LastArg = mReg.IndY = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
			SetFlags(mReg.IndY);
			break;
			
		case OPCODE_LDY_ZP:			// LoaD Y register, Zero Page ($A4 arg : LDY arg ;arg=0..$FF), MEM=arg
			mReg.IndY = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_ZP));
			SetFlags(mReg.IndY);		
			break;			

		case OPCODE_LDY_ABS:		// LoaD Y register, Absolute ($AC addrlo addrhi : LDY addr ;addr=0..$FFFF), MEM=addr			
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			mReg.IndY = mpMem->Peek8bit(arg16);
			SetFlags(mReg.IndY);
			break;
			
		case OPCODE_LDY_ZPX:		// LoaD Y register, Zero Page Indexed, X ($B4 arg : LDY arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			mReg.IndY = mpMem->Peek8bit(arg16);
			SetFlags(mReg.IndY);
			break;
			
		case OPCODE_LDY_ABX:		// LoaD Y register, Absolute Indexed, X ($BC addrlo addrhi : LDY addr,X ;addr=0..$FFFF), MEM=addr+X			
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			mReg.IndY = mpMem->Peek8bit(arg16);
			SetFlags(mReg.IndY);			
			break;
			
		case OPCODE_TAX:				// Transfer A to X, Implied ($AA : TAX)
			mReg.LastAddrMode = ADDRMODE_IMP;
			mReg.IndX = mReg.Acc;
			SetFlags(mReg.IndX);
			break;
			
		case OPCODE_TAY:				// Transfer A to Y, Implied ($A8 : TAY)			
			mReg.LastAddrMode = ADDRMODE_IMP;
			mReg.IndY = mReg.Acc;
			SetFlags(mReg.IndY);
			break;

		case OPCODE_TXA:				// Transfer X to A, Implied ($8A : TXA)
			mReg.LastAddrMode = ADDRMODE_IMP;
			mReg.Acc = mReg.IndX;
			SetFlags(mReg.Acc);
			break;

		case OPCODE_TYA:				// Transfer Y to A, Implied ($98 : TYA)
			mReg.LastAddrMode = ADDRMODE_IMP;
			mReg.Acc = mReg.IndY;
			SetFlags(mReg.Acc);
			break;
			
		case OPCODE_TSX:				// Transfer Stack ptr to X, Implied ($BA : TSX)
			mReg.LastAddrMode = ADDRMODE_IMP;
			mReg.IndX = mReg.PtrStack;
			SetFlags(mReg.IndX);
			break;
			
		case OPCODE_TXS:				// Transfer X to Stack ptr, Implied ($9A : TXS)
			mReg.LastAddrMode = ADDRMODE_IMP;
			mReg.PtrStack = mReg.IndX;
			break;

		case OPCODE_STA_IZX:		// STore Accumulator, Indexed Indirect ($81 arg : STA (arg,X) ;arg=0..$FF), MEM=&(arg+X)
			arg16 = GetAddrWithMode(ADDRMODE_IZX);
			mpMem->Poke8bit(arg16, mReg.Acc);
			break;

		case OPCODE_STA_ZP:			// STore Accumulator, Zero Page ($85 arg : STA arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			mpMem->Poke8bit(arg16, mReg.Acc);		
			break;					
			
		case OPCODE_STA_ABS:		// STore Accumulator, Absolute ($8D addrlo addrhi : STA addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			mpMem->Poke8bit(arg16, mReg.Acc);		
			break;

		case OPCODE_STA_IZY:		// STore Accumulator, Indirect Indexed ($91 arg : STA (arg),Y ;arg=0..$FF), MEM=&arg+Y
			arg16 = GetAddrWithMode(ADDRMODE_IZY);
			mpMem->Poke8bit(arg16, mReg.Acc);
			break;
			
		case OPCODE_STA_ZPX:		// STore Accumulator, Zero Page Indexed, X ($95 arg : STA arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			mpMem->Poke8bit(arg16, mReg.Acc);
			break;
			
		case OPCODE_STA_ABY:		// STore Accumulator, Absolute Indexed, Y ($99 addrlo addrhi : STA addr,Y ;addr=0..$FFFF), MEM=addr+Y
			arg16 = GetAddrWithMode(ADDRMODE_ABY);
			mpMem->Poke8bit(arg16, mReg.Acc);		
			break;			

		case OPCODE_STA_ABX:		// STore Accumulator, Absolute Indexed, X ($9D addrlo addrhi : STA addr,X ;addr=0..$FFFF), MEM=addr+X			
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			mpMem->Poke8bit(arg16, mReg.Acc);		
			break;
			
		case OPCODE_STX_ZP:			// STore X register, Zero Page ($86 arg : STX arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			mpMem->Poke8bit(arg16, mReg.IndX);		
			break;		
			
		case OPCODE_STX_ABS:		// STore X register, Absolute ($8E addrlo addrhi : STX addr ;addr=0..$FFFF), MEM=addr			
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			mpMem->Poke8bit(arg16, mReg.IndX);
			break;

		case OPCODE_STX_ZPY:		// STore X register, Zero Page Indexed, Y ($96 arg : STX arg,Y ;arg=0..$FF), MEM=arg+Y
			arg16 = GetAddrWithMode(ADDRMODE_ZPY);
			mpMem->Poke8bit(arg16, mReg.IndX);
			break;

		case OPCODE_STY_ZP:			// STore Y register, Zero Page ($84 arg : STY arg ;arg=0..$FF), MEM=arg			
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			mpMem->Poke8bit(arg16, mReg.IndY);		
			break;				

		case OPCODE_STY_ABS:		// STore Y register, Absolute ($8C addrlo addrhi : STY addr ;addr=0..$FFFF), MEM=addr		
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			mpMem->Poke8bit(arg16, mReg.IndY);			
			break;

		case OPCODE_STY_ZPX:		// STore Y register, Zero Page Indexed, X ($94 arg : STY arg,X ;arg=0..$FF), MEM=arg+X			
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			mpMem->Poke8bit(arg16, mReg.IndY);		
			break;
			
		case OPCODE_BNE_REL:		// Branch on Not Equal, Relative ($D0 signoffs : BNE signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])			
		  arg16 = GetAddrWithMode(ADDRMODE_REL);
			if (!CheckFlag(FLAGS_ZERO)) {
				mReg.PtrAddr = arg16;
			}
			break;

		case OPCODE_BEQ_REL:		// Branch on EQual, Relative ($F0 signoffs : BEQ signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
			arg16 = GetAddrWithMode(ADDRMODE_REL);
			if (CheckFlag(FLAGS_ZERO)) {
				mReg.PtrAddr = arg16;
			}
			break;
			
		case OPCODE_BPL_REL:		// Branch on PLus, Relative ($10 signoffs : BPL signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
			arg16 = GetAddrWithMode(ADDRMODE_REL);
			if (!CheckFlag(FLAGS_SIGN)) {
				mReg.PtrAddr = arg16;
			}		
			break;
			
		case OPCODE_BMI_REL:		// Branch on MInus, Relative ($30 signoffs : BMI signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
			arg16 = GetAddrWithMode(ADDRMODE_REL);
			if (CheckFlag(FLAGS_SIGN)) {
				mReg.PtrAddr = arg16;
			}
			break;
			
		case OPCODE_BVC_REL:		// Branch on oVerflow Clear, Relative ($50 signoffs : BVC signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
			arg16 = GetAddrWithMode(ADDRMODE_REL);
			if (!CheckFlag(FLAGS_OVERFLOW)) {
				mReg.PtrAddr = arg16;
			}		
			break;
			
		case OPCODE_BVS_REL:		// Branch on oVerflow Set, Relative ($70 signoffs : BVS signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
			arg16 = GetAddrWithMode(ADDRMODE_REL);
			if (CheckFlag(FLAGS_OVERFLOW)) {
				mReg.PtrAddr = arg16;
			}
			break;
			
		case OPCODE_BCC_REL:		// Branch on Carry Clear, Relative ($90 signoffs : BCC signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
			arg16 = GetAddrWithMode(ADDRMODE_REL);
			if (!CheckFlag(FLAGS_CARRY)) {
				mReg.PtrAddr = arg16;
			}
			break;
			
		case OPCODE_BCS_REL:		// Branch on Carry Set, Relative ($B0 signoffs : BCS signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
			arg16 = GetAddrWithMode(ADDRMODE_REL);
			if (CheckFlag(FLAGS_CARRY)) {
				mReg.PtrAddr = arg16;
			}		
			break;
		
		/***	
		case OPCODE_BRA:				// branch always to a relative 1-byte address offset -128 ($80)..127 ($7F) (OPCODE_BEQ reladdr : BEQ reladdr)
			arg8 = mpMem->Peek8bit(mReg.PtrAddr++);
			mReg.PtrAddr = ComputeRelJump(arg8);
			break;
			***/
			
		case OPCODE_INC_ZP:			// INCrement memory, Zero Page ($E6 arg : INC arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16) + 1;
			mpMem->Poke8bit(arg16, arg8);
			SetFlags(arg8);
			break;
			
		case OPCODE_INC_ABS:		// INCrement memory, Absolute ($EE addrlo addrhi : INC addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16) + 1;
			mpMem->Poke8bit(arg16, arg8);
			SetFlags(arg8);
			break;
			
		case OPCODE_INC_ZPX:		// INCrement memory, Zero Page Indexed, X ($F6 arg : INC arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			arg8 = mpMem->Peek8bit(arg16) + 1;
			mpMem->Poke8bit(arg16, arg8);
			SetFlags(arg8);
			break;
			
		case OPCODE_INC_ABX:		// INCrement memory, Absolute Indexed, X ($FE addrlo addrhi : INC addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			arg8 = mpMem->Peek8bit(arg16) + 1;
			mpMem->Poke8bit(arg16, arg8);
			SetFlags(arg8);
			break;
			
		case OPCODE_INX:				// INcrement X, Implied ($E8 : INX)			
			mReg.LastAddrMode = ADDRMODE_IMP;
			mReg.IndX++;
			SetFlags(mReg.IndX);
			break;
			
		case OPCODE_DEX:				// DEcrement X, Implied ($CA : DEX)
			mReg.LastAddrMode = ADDRMODE_IMP;
			mReg.IndX--;
			SetFlags(mReg.IndX);
			break;

		case OPCODE_INY:				// INcrement Y, Implied ($C8 : INY)
			mReg.LastAddrMode = ADDRMODE_IMP;
			mReg.IndY++;
			SetFlags(mReg.IndY);
			break;
			
		case OPCODE_DEY:				// DEcrement Y, Implied ($88 : DEY)
			mReg.LastAddrMode = ADDRMODE_IMP;
			mReg.IndY--;
			SetFlags(mReg.IndY);		
			break;

		case OPCODE_JMP_ABS:		// JuMP, Absolute ($4C addrlo addrhi : JMP addr ;addr=0..$FFFF), MEM=addr
			mReg.PtrAddr = GetAddrWithMode(ADDRMODE_ABS);
			break;

		case OPCODE_JMP_IND:		// JuMP, Indirect Absolute ($6C addrlo addrhi : JMP (addr) ;addr=0..FFFF), MEM=&addr
			mReg.PtrAddr = GetAddrWithMode(ADDRMODE_IND);
			break;
			
		case OPCODE_ORA_IZX:		// bitwise OR with Accumulator, Indexed Indirect ($01 arg : ORA (arg,X) ;arg=0..$FF), MEM=&(arg+X)
			arg16 = GetAddrWithMode(ADDRMODE_IZX);
			LogicOpAcc(arg16, LOGOP_OR);
			break;
			
		case OPCODE_ORA_ZP:			// bitwise OR with Accumulator, Zero Page ($05 arg : ORA arg ;arg=0..$FF), MEM=arg
			LogicOpAcc(GetAddrWithMode(ADDRMODE_ZP), LOGOP_OR);
			break;
			
		case OPCODE_ORA_IMM:		// bitwise OR with Accumulator, Immediate ($09 arg : ORA #arg ;arg=0..$FF), MEM=PC+1
			arg16 = GetAddrWithMode(ADDRMODE_IMM);
			mReg.LastArg = mpMem->Peek8bit(arg16);
			LogicOpAcc(arg16, LOGOP_OR);
			break;
			
		case OPCODE_ORA_ABS:		// bitwise OR with Accumulator, Absolute ($0D addrlo addrhi : ORA addr ;addr=0..$FFFF), MEM=addr			
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			LogicOpAcc(arg16, LOGOP_OR);
			break;
			
		case OPCODE_ORA_IZY:		// bitwise OR with Accumulator, Indirect Indexed ($11 arg : ORA (arg),Y ;arg=0..$FF), MEM=&arg+Y
			arg16 = GetAddrWithMode(ADDRMODE_IZY);
			LogicOpAcc(arg16, LOGOP_OR);
			break;
			
		case OPCODE_ORA_ZPX:		// bitwise OR with Accumulator, Zero Page Indexed, X ($15 arg : ORA arg,X ;arg=0..$FF), MEM=arg+X	
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			LogicOpAcc(arg16, LOGOP_OR);
			break;
			
		case OPCODE_ORA_ABY:		// bitwise OR with Accumulator, Absolute Indexed, Y ($19 addrlo addrhi : ORA addr,Y ;addr=0..$FFFF), MEM=addr+Y	
			arg16 = GetAddrWithMode(ADDRMODE_ABY);
			LogicOpAcc(arg16, LOGOP_OR);
			break;
			
		case OPCODE_ORA_ABX:		// bitwise OR with Accumulator, Absolute Indexed, X ($1D addrlo addrhi : ORA addr,X ;addr=0..$FFFF), MEM=addr+X			
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			LogicOpAcc(arg16, LOGOP_OR);
			break;		
			
		case OPCODE_ASL_ZP:			// Arithmetic Shift Left, Zero Page ($06 arg : ASL arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = ShiftLeft(arg8);
			mpMem->Poke8bit(arg16, arg8);
			break;
			
		case OPCODE_ASL:				// Arithmetic Shift Left, Accumulator ($0A : ASL)
			mReg.LastAddrMode = ADDRMODE_ACC;
			mReg.Acc = ShiftLeft(mReg.Acc);
			break;
		
		case OPCODE_ASL_ABS:		// Arithmetic Shift Left, Absolute ($0E addrlo addrhi : ASL addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = ShiftLeft(arg8);
			mpMem->Poke8bit(arg16, arg8);
			break;
			
		case OPCODE_ASL_ZPX:		// Arithmetic Shift Left, Zero Page Indexed, X ($16 arg : ASL arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = ShiftLeft(arg8);
			mpMem->Poke8bit(arg16, arg8);
			break;
			
		case OPCODE_ASL_ABX:		// Arithmetic Shift Left, Absolute Indexed, X ($1E addrlo addrhi : ASL addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = ShiftLeft(arg8);
			mpMem->Poke8bit(arg16, arg8);
			break;
			
		case OPCODE_JSR_ABS:	// Jump to SubRoutine, Absolute ($20 addrlo addrhi : JSR addr ;addr=0..$FFFF), MEM=addr
			// PC - next instruction address
			// Push PC-1 on stack (HI, then LO).
			// Currently PC (mReg.PtrAddr) is at the 1-st argument of JSR.
			// Therefore the actual PC-1 used in calculations equals: mReg.PtrAddr+1
			arg16 = 0x100;
			arg16 += mReg.PtrStack--;
			mpMem->Poke8bit(arg16, (unsigned char) (((mReg.PtrAddr+1) & 0xFF00) >> 8));	// HI(PC-1) - HI part of next instr. addr. - 1
			arg16 = 0x100;
			arg16 += mReg.PtrStack--;
			mpMem->Poke8bit(arg16, (unsigned char) ((mReg.PtrAddr+1) & 0x00FF));	// LO(PC-1) - LO part of next instr. addr. - 1
			mReg.PtrAddr = GetAddrWithMode(ADDRMODE_ABS);
			break;
			
		case OPCODE_AND_IZX:	// bitwise AND with accumulator, Indexed Indirect ($21 arg : AND (arg,X) ;arg=0..$FF), MEM=&(arg+X)
			arg16 = GetAddrWithMode(ADDRMODE_IZX);
			LogicOpAcc(arg16, LOGOP_AND);
			break;
			
		case OPCODE_AND_ZP:		// bitwise AND with accumulator, Zero Page ($25 arg : AND arg ;arg=0..$FF), MEM=arg
			LogicOpAcc(GetAddrWithMode(ADDRMODE_ZP), LOGOP_AND);
			break;
			
		case OPCODE_AND_IMM:	// bitwise AND with accumulator, Immediate ($29 arg : AND #arg ;arg=0..$FF), MEM=PC+1
			arg16 = GetAddrWithMode(ADDRMODE_IMM);
			mReg.LastArg = mpMem->Peek8bit(arg16);		
			LogicOpAcc(arg16, LOGOP_AND);
			break;
			
		case OPCODE_AND_ABS:	// bitwise AND with accumulator, Absolute ($2D addrlo addrhi : AND addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			LogicOpAcc(arg16, LOGOP_AND);
			break;
			
		case OPCODE_AND_IZY:	// bitwise AND with accumulator, Indirect Indexed ($31 arg : AND (arg),Y ;arg=0..$FF), MEM=&arg+Y
			arg16 = GetAddrWithMode(ADDRMODE_IZY);
			LogicOpAcc(arg16, LOGOP_AND);
			break;
			
		case OPCODE_AND_ZPX:	// bitwise AND with accumulator, Zero Page Indexed, X ($35 arg : AND arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			LogicOpAcc(arg16, LOGOP_AND);
			break;
			
		case OPCODE_AND_ABY:	// bitwise AND with accumulator, Absolute Indexed, Y ($39 addrlo addrhi : AND addr,Y ;addr=0..$FFFF), MEM=addr+Y
			arg16 = GetAddrWithMode(ADDRMODE_ABY);
			LogicOpAcc(arg16, LOGOP_AND);
			break;
			
		case OPCODE_AND_ABX:	// bitwise AND with accumulator, Absolute Indexed, X ($3D addrlo addrhi : AND addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			LogicOpAcc(arg16, LOGOP_AND);
			break;
			
		case OPCODE_BIT_ZP:		// test BITs, Zero Page ($24 arg : BIT arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((arg8 & FLAGS_OVERFLOW) == FLAGS_OVERFLOW, FLAGS_OVERFLOW);
			SetFlag((arg8 & FLAGS_SIGN) == FLAGS_SIGN, FLAGS_SIGN);
			arg8 &= mReg.Acc;
			SetFlag((arg8 == 0), FLAGS_ZERO);
			break;
			
		case OPCODE_BIT_ABS:	// test BITs, Absolute ($2C addrlo addrhi : BIT addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((arg8 & FLAGS_OVERFLOW) == FLAGS_OVERFLOW, FLAGS_OVERFLOW);
			SetFlag((arg8 & FLAGS_SIGN) == FLAGS_SIGN, FLAGS_SIGN);			
			arg8 &= mReg.Acc;
			SetFlag((arg8 == 0), FLAGS_ZERO);			
			break;
			
		case OPCODE_ROL_ZP:		// ROtate Left, Zero Page ($26 arg : ROL arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = RotateLeft(arg8);
			mpMem->Poke8bit(arg16, arg8);
			break;
			
		case OPCODE_ROL:			// ROtate Left, Accumulator ($2A : ROL)
			mReg.LastAddrMode = ADDRMODE_ACC;
			mReg.Acc = RotateLeft(mReg.Acc);
			break;
			
		case OPCODE_ROL_ABS:	// ROtate Left, Absolute ($2E addrlo addrhi : ROL addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = RotateLeft(arg8);
			mpMem->Poke8bit(arg16, arg8);
			break;
			
		case OPCODE_ROL_ZPX:	// ROtate Left, Zero Page Indexed, X ($36 arg : ROL arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = RotateLeft(arg8);
			mpMem->Poke8bit(arg16, arg8);
			break;
			
		case OPCODE_ROL_ABX:	// ROtate Left, Absolute Indexed, X ($3E addrlo addrhi : ROL addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = RotateLeft(arg8);
			mpMem->Poke8bit(arg16, arg8);
			break;

		case OPCODE_PHP:			// PusH Processor status on Stack, Implied ($08 : PHP)
			mReg.LastAddrMode = ADDRMODE_IMP;
			arg16 = 0x100;
			arg16 += mReg.PtrStack--;
			arg8 = mReg.Flags | FLAGS_BRK | FLAGS_UNUSED;
			mpMem->Poke8bit(arg16, arg8);
			break;
			
		case OPCODE_PHA:			// PusH Accumulator, Implied ($48 : PHA)
			mReg.LastAddrMode = ADDRMODE_IMP;
			arg16 = 0x100;
			arg16 += mReg.PtrStack--;
			mpMem->Poke8bit(arg16, mReg.Acc);		
			break;
						
		case OPCODE_PLP:			// PuLl Processor status, Implied ($28 : PLP)
			mReg.LastAddrMode = ADDRMODE_IMP;
			arg16 = 0x100;
			arg16 += ++mReg.PtrStack;
			mReg.Flags = mpMem->Peek8bit(arg16) | FLAGS_UNUSED;
			break;
			
		case OPCODE_PLA:			// PuLl Accumulator, Implied ($68 : PLA)
			mReg.LastAddrMode = ADDRMODE_IMP;
			arg16 = 0x100;
			arg16 += ++mReg.PtrStack;
			mReg.Acc = mpMem->Peek8bit(arg16);
			SetFlags(mReg.Acc);
			break;
			
		case OPCODE_CLC:			// CLear Carry, Implied ($18 : CLC)
			mReg.LastAddrMode = ADDRMODE_IMP;
			SetFlag(false, FLAGS_CARRY);
			break;
			
		case OPCODE_SEC:			// SEt Carry, Implied ($38 : SEC)
			mReg.LastAddrMode = ADDRMODE_IMP;
			SetFlag(true, FLAGS_CARRY);
			break;
			
		case OPCODE_CLI:			// CLear Interrupt, Implied ($58 : CLI)
			mReg.LastAddrMode = ADDRMODE_IMP;
			SetFlag(false, FLAGS_IRQ);
			break;
			
		case OPCODE_CLV:			// CLear oVerflow, Implied ($B8 : CLV)
			mReg.LastAddrMode = ADDRMODE_IMP;
			SetFlag(false, FLAGS_OVERFLOW);
			break;
			
		case OPCODE_CLD:			// CLear Decimal, Implied ($D8 : CLD)
			mReg.LastAddrMode = ADDRMODE_IMP;
			SetFlag(false, FLAGS_DEC);
			break;
			
		case OPCODE_SED:			// SEt Decimal, Implied ($F8 : SED)
			mReg.LastAddrMode = ADDRMODE_IMP;
			SetFlag(true, FLAGS_DEC);
			break;
			
		case OPCODE_SEI:			// SEt Interrupt, Implied ($78 : SEI)
			mReg.LastAddrMode = ADDRMODE_IMP;
			SetFlag(true, FLAGS_IRQ);
			break;
			
		/* 
		 * RTI retrieves the Processor Status Word (flags) and the Program Counter from the stack in that order
		 * (interrupts push the PC first and then the PSW). 
		 * Note that unlike RTS, the return address on the stack is the actual address rather than the address-1. 
		*/
		case OPCODE_RTI:			// ReTurn from Interrupt, Implied ($40 : RTI)
			mReg.LastAddrMode = ADDRMODE_IMP;
			arg16 = 0x100;
			arg16 += ++mReg.PtrStack;
			mReg.Flags = mpMem->Peek8bit(arg16);
			SetFlag(true,FLAGS_UNUSED);
			arg16++; mReg.PtrStack++;
			mReg.PtrAddr = mpMem->Peek8bit(arg16);
			arg16++; mReg.PtrStack++;
			mReg.PtrAddr += (mpMem->Peek8bit(arg16) * 0x100);
			mReg.SoftIrq = CheckFlag(FLAGS_BRK);
			SetFlag(false,FLAGS_IRQ);
			break;
			
		case OPCODE_RTS:			// ReTurn from Subroutine, Implied ($60 : RTS)
			mReg.LastAddrMode = ADDRMODE_IMP;
			if (mReg.PtrStack == 0xFF) {
				mReg.LastRTS = true;
				break;
			}
			arg16 = 0x100;
			arg16 += ++mReg.PtrStack;
			mReg.PtrAddr = mpMem->Peek8bit(arg16);
			arg16++; mReg.PtrStack++;
			mReg.PtrAddr += (mpMem->Peek8bit(arg16) * 0x100);
			mReg.PtrAddr++;
			break;
			
		case OPCODE_EOR_IZX:	// bitwise Exclusive OR, Indexed Indirect ($41 arg : EOR (arg,X) ;arg=0..$FF), MEM=&(arg+X)
			arg16 = GetAddrWithMode(ADDRMODE_IZX);
			LogicOpAcc(arg16, LOGOP_EOR);
			break;
			
		case OPCODE_EOR_ZP:		// bitwise Exclusive OR, Zero Page ($45 arg : EOR arg ;arg=0..$FF), MEM=arg
			LogicOpAcc(GetAddrWithMode(ADDRMODE_ZP), LOGOP_EOR);
			break;			
			
		case OPCODE_EOR_IMM:	// bitwise Exclusive OR, Immediate ($49 arg : EOR #arg ;arg=0..$FF), MEM=PC+1
			arg16 = GetAddrWithMode(ADDRMODE_IMM);
			mReg.LastArg = mpMem->Peek8bit(arg16);		
			LogicOpAcc(arg16, LOGOP_EOR);
			break;
			
		case OPCODE_EOR_ABS:	// bitwise Exclusive OR, Absolute ($4D addrlo addrhi : EOR addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			LogicOpAcc(arg16, LOGOP_EOR);		
			break;
			
		case OPCODE_EOR_IZY:	// bitwise Exclusive OR, Indirect Indexed ($51 arg : EOR (arg),Y ;arg=0..$FF), MEM=&arg+Y
			arg16 = GetAddrWithMode(ADDRMODE_IZY);
			LogicOpAcc(arg16, LOGOP_EOR);		
			break;
			
		case OPCODE_EOR_ZPX:	// bitwise Exclusive OR, Zero Page Indexed, X ($55 arg : EOR arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			LogicOpAcc(arg16, LOGOP_EOR);
			break;
			
		case OPCODE_EOR_ABY:	// bitwise Exclusive OR, Absolute Indexed, Y ($59 addrlo addrhi : EOR addr,Y ;addr=0..$FFFF), MEM=addr+Y
			arg16 = GetAddrWithMode(ADDRMODE_ABY);
			LogicOpAcc(arg16, LOGOP_EOR);
			break;
			
		case OPCODE_EOR_ABX:	// bitwise Exclusive OR, Absolute Indexed, X ($5D addrlo addrhi : EOR addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			LogicOpAcc(arg16, LOGOP_EOR);
			break;

		case OPCODE_LSR_ZP:		// Logical Shift Right, Zero Page ($46 arg : LSR arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = ShiftRight(arg8);
			mpMem->Poke8bit(arg16, arg8);		
			break;
			
		case OPCODE_LSR:			// Logical Shift Right, Accumulator ($4A : LSR)
			mReg.LastAddrMode = ADDRMODE_ACC;
			mReg.Acc = ShiftRight(mReg.Acc);
			break;
			
		case OPCODE_LSR_ABS:	// Logical Shift Right, Absolute ($4E addrlo addrhi : LSR addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = ShiftRight(arg8);
			mpMem->Poke8bit(arg16, arg8);		
			break;
			
		case OPCODE_LSR_ZPX:	// Logical Shift Right, Zero Page Indexed, X ($56 arg : LSR arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = ShiftRight(arg8);
			mpMem->Poke8bit(arg16, arg8);		
			break;
			
		case OPCODE_LSR_ABX:	// Logical Shift Right, Absolute Indexed, X ($5E addrlo addrhi : LSR addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = ShiftRight(arg8);
			mpMem->Poke8bit(arg16, arg8);		
			break;
			
		case OPCODE_ADC_IZX:	// ADd with Carry, Indexed Indirect ($61 arg : ADC (arg,X) ;arg=0..$FF), MEM=&(arg+X)
			arg16 = GetAddrWithMode(ADDRMODE_IZX);
			AddWithCarry(mpMem->Peek8bit(arg16));
			break;
			
		case OPCODE_ADC_ZP:		// ADd with Carry, Zero Page ($65 arg : ADC arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			AddWithCarry(mpMem->Peek8bit(arg16));
			break;
			
		case OPCODE_ADC_IMM:	// ADd with Carry, Immediate ($69 arg : ADC #arg ;arg=0..$FF), MEM=PC+1
			mReg.LastArg = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
			AddWithCarry(mReg.LastArg);
			break;
			
		case OPCODE_ADC_ABS:	// ADd with Carry, Absolute ($6D addrlo addrhi : ADC addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			AddWithCarry(mpMem->Peek8bit(arg16));
			break;
			
		case OPCODE_ADC_IZY:	// ADd with Carry, Indirect Indexed ($71 arg : ADC (arg),Y ;arg=0..$FF), MEM=&arg+Y
			arg16 = GetAddrWithMode(ADDRMODE_IZY);
			AddWithCarry(mpMem->Peek8bit(arg16));
			break;
			
		case OPCODE_ADC_ZPX:	// ADd with Carry, Zero Page Indexed, X ($75 arg : ADC arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			AddWithCarry(mpMem->Peek8bit(arg16));
			break;
			
		case OPCODE_ADC_ABY:	// ADd with Carry, Absolute Indexed, Y ($79 addrlo addrhi : ADC addr,Y ;addr=0..$FFFF), MEM=addr+Y
			arg16 = GetAddrWithMode(ADDRMODE_ABY);
			AddWithCarry(mpMem->Peek8bit(arg16));
			break;
			
		case OPCODE_ADC_ABX:	// ADd with Carry, Absolute Indexed, X ($7D addrlo addrhi : ADC addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			AddWithCarry(mpMem->Peek8bit(arg16));
			break;
			
		case OPCODE_ROR_ZP:		// ROtate Right, Zero Page ($66 arg : ROR arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16);
			mpMem->Poke8bit(arg16, RotateRight(arg8));		
			break;
			
		case OPCODE_ROR:			// ROtate Right, Accumulator ($6A : ROR)
			mReg.LastAddrMode = ADDRMODE_ACC;
			mReg.Acc = RotateRight(mReg.Acc);
			break;
		
		case OPCODE_ROR_ABS:	// ROtate Right, Absolute ($6E addrlo addrhi : ROR addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16);
			mpMem->Poke8bit(arg16, RotateRight(arg8));		
			break;
			
		case OPCODE_ROR_ZPX:	// ROtate Right, Zero Page Indexed, X ($76 arg : ROR arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			arg8 = mpMem->Peek8bit(arg16);
			mpMem->Poke8bit(arg16, RotateRight(arg8));		
			break;

		case OPCODE_ROR_ABX:	// ROtate Right, Absolute Indexed, X ($7E addrlo addrhi : ROR addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			arg8 = mpMem->Peek8bit(arg16);
			mpMem->Poke8bit(arg16, RotateRight(arg8));
			break;
			
		case OPCODE_CPY_IMM:	// ComPare Y register, Immediate ($C0 arg : CPY #arg ;arg=0..$FF), MEM=PC+1
			mReg.LastArg = arg8 = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
			SetFlag((mReg.IndY >= arg8), FLAGS_CARRY);
			arg8 = mReg.IndY - arg8;
			SetFlags(arg8);
			break;
			
		case OPCODE_CPY_ZP:		// ComPare Y register, Zero Page ($C4 arg : CPY arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.IndY >= arg8), FLAGS_CARRY);
			arg8 = mReg.IndY - arg8;
			SetFlags(arg8);			
			break;
			
		case OPCODE_CPY_ABS:	// ComPare Y register, Absolute ($CC addrlo addrhi : CPY addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.IndY >= arg8), FLAGS_CARRY);
			arg8 = mReg.IndY - arg8;
			SetFlags(arg8);			
			break;
			
		case OPCODE_CMP_IZX:	// CoMPare accumulator, Indexed Indirect ($A1 arg : LDA (arg,X) ;arg=0..$FF), MEM=&(arg+X)
			arg16 = GetAddrWithMode(ADDRMODE_IZX);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.Acc >= arg8), FLAGS_CARRY);
			arg8 = mReg.Acc - arg8;
			SetFlags(arg8);
			break;
			
		case OPCODE_CMP_ZP:		// CoMPare accumulator, Zero Page ($C5 arg : CMP arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.Acc >= arg8), FLAGS_CARRY);
			arg8 = mReg.Acc - arg8;
			SetFlags(arg8);			
			break;
			
		case OPCODE_CMP_IMM:	// CoMPare accumulator, Immediate ($C9 arg : CMP #arg ;arg=0..$FF), MEM=PC+1
			mReg.LastArg = arg8 = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
			SetFlag((mReg.Acc >= arg8), FLAGS_CARRY);
			arg8 = mReg.Acc - arg8;
			SetFlags(arg8);			
			break;
			
		case OPCODE_CMP_ABS:	// CoMPare accumulator, Absolute ($CD addrlo addrhi : CMP addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.Acc >= arg8), FLAGS_CARRY);
			arg8 = mReg.Acc - arg8;
			SetFlags(arg8);
			break;
			
		case OPCODE_CMP_IZY:	// CoMPare accumulator, Indirect Indexed ($D1 arg : CMP (arg),Y ;arg=0..$FF), MEM=&arg+Y
			arg16 = GetAddrWithMode(ADDRMODE_IZY);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.Acc >= arg8), FLAGS_CARRY);
			arg8 = mReg.Acc - arg8;
			SetFlags(arg8);			
			break;
			
		case OPCODE_CMP_ZPX:	// CoMPare accumulator, Zero Page Indexed, X ($D5 arg : CMP arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.Acc >= arg8), FLAGS_CARRY);
			arg8 = mReg.Acc - arg8;
			SetFlags(arg8);			
			break;
			
		case OPCODE_CMP_ABY:	// CoMPare accumulator, Absolute Indexed, Y ($D9 addrlo addrhi : CMP addr,Y ;addr=0..$FFFF), MEM=addr+Y
			arg16 = GetAddrWithMode(ADDRMODE_ABY);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.Acc >= arg8), FLAGS_CARRY);
			arg8 = mReg.Acc - arg8;
			SetFlags(arg8);			
			break;
			
		case OPCODE_CMP_ABX:	// CoMPare accumulator, Absolute Indexed, X ($DD addrlo addrhi : CMP addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.Acc >= arg8), FLAGS_CARRY);
			arg8 = mReg.Acc - arg8;
			SetFlags(arg8);
			break;
			
		case OPCODE_DEC_ZP:		// DECrement memory, Zero Page ($C6 arg : DEC arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16) - 1;
			mpMem->Poke8bit(arg16, arg8);
			SetFlags(arg8);
			break;
			
		case OPCODE_DEC_ABS:	// DECrement memory, Absolute ($CE addrlo addrhi : CMP addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16) - 1;
			mpMem->Poke8bit(arg16, arg8);
			SetFlags(arg8);
			break;
			
		case OPCODE_DEC_ZPX:	// DECrement memory, Zero Page Indexed, X ($D6 arg : DEC arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			arg8 = mpMem->Peek8bit(arg16) - 1;
			mpMem->Poke8bit(arg16, arg8);
			SetFlags(arg8);			
			break;
			
		case OPCODE_DEC_ABX:	// DECrement memory, Absolute Indexed, X ($DE addrlo addrhi : DEC addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			arg8 = mpMem->Peek8bit(arg16) - 1;
			mpMem->Poke8bit(arg16, arg8);
			SetFlags(arg8);	
			break;
			
		case OPCODE_CPX_IMM:	// ComPare X register, Immediate ($E0 arg : CPX #arg ;arg=0..$FF), MEM=PC+1
			mReg.LastArg = arg8 = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
			SetFlag((mReg.IndX >= arg8), FLAGS_CARRY);
			arg8 = mReg.IndX - arg8;
			SetFlags(arg8);		
			break;
			
		case OPCODE_CPX_ZP:		// ComPare X register, Zero Page ($E4 arg : CPX arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.IndX >= arg8), FLAGS_CARRY);
			arg8 = mReg.IndX - arg8;
			SetFlags(arg8);
			break;
			
		case OPCODE_CPX_ABS:	// ComPare X register, Absolute ($EC addrlo addrhi : CPX addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16);
			SetFlag((mReg.IndX >= arg8), FLAGS_CARRY);
			arg8 = mReg.IndX - arg8;
			SetFlags(arg8);
			break;
			
		case OPCODE_SBC_ZP:		// SuBtract with Carry, Zero Page ($E5 arg : SBC arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			SubWithCarry(mpMem->Peek8bit(arg16));		
			break;
			
		case OPCODE_SBC_ABS:	// SuBtract with Carry, Absolute ($ED addrlo addrhi : SBC addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			SubWithCarry(mpMem->Peek8bit(arg16));		
			break;
			
		case OPCODE_SBC_IZX:	// SuBtract with Carry, Indexed Indirect ($E1 arg : SBC (arg,X) ;arg=0..$FF), MEM=&(arg+X)
			arg16 = GetAddrWithMode(ADDRMODE_IZX);
			SubWithCarry(mpMem->Peek8bit(arg16));		
			break;
			
		case OPCODE_SBC_IZY:	// SuBtract with Carry, Indirect Indexed ($F1 arg : SBC (arg),Y ;arg=0..$FF), MEM=&arg+Y
			arg16 = GetAddrWithMode(ADDRMODE_IZY);
			SubWithCarry(mpMem->Peek8bit(arg16));		
			break;
			
		case OPCODE_SBC_ZPX:	// SuBtract with Carry, Zero Page Indexed, X ($F5 arg : SBC arg,X ;arg=0..$FF), MEM=arg+X
			arg16 = GetAddrWithMode(ADDRMODE_ZPX);
			SubWithCarry(mpMem->Peek8bit(arg16));		
			break;
			
		case OPCODE_SBC_ABY:	// SuBtract with Carry, Absolute Indexed, Y ($F9 addrlo addrhi : SBC addr,Y ;addr=0..$FFFF), MEM=addr+Y
			arg16 = GetAddrWithMode(ADDRMODE_ABY);
			SubWithCarry(mpMem->Peek8bit(arg16));		
			break;
			
		case OPCODE_SBC_ABX:	// SuBtract with Carry, Absolute Indexed, X ($FD addrlo addrhi : SBC addr,X ;addr=0..$FFFF), MEM=addr+X
			arg16 = GetAddrWithMode(ADDRMODE_ABX);
			SubWithCarry(mpMem->Peek8bit(arg16));		
			break;
			
		case OPCODE_SBC_IMM:	// SuBtract with Carry, Immediate ($E9 arg : SBC #arg ;arg=0..$FF), MEM=PC+1
			mReg.LastArg = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
			SubWithCarry(mReg.LastArg);
			break;
			
		default:
			// trap illegal opcode that has unidentified behavior
			{
				string s = ((mOpCodesMap[(eOpCodes)mReg.LastOpCode]).amf.length() > 0 
											? (mOpCodesMap[(eOpCodes)mReg.LastOpCode]).amf.c_str() : "???");
				if (s.compare("ILL") == 0) {
					mReg.SoftIrq = true;
				}											
			}
			break;
	}
	
	Disassemble();
	char histentry[80];
	sprintf(histentry, "$%04x: %-16s \t$%02x | $%02x | $%02x | $%02x | $%02x",
											mReg.LastAddr, mReg.LastInstr.c_str(), mReg.Acc, mReg.IndX, mReg.IndY, mReg.Flags, mReg.PtrStack);
	Add2History(histentry);
	
	return &mReg;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetRegs()
 * Purpose:		Return pointer to CPU registers and status.
 * Arguments:	n/a
 * Returns:		pointer to Regs structure.
 *--------------------------------------------------------------------
 */
Regs *MKCpu::GetRegs()
{
	return &mReg;
}

/*
 *--------------------------------------------------------------------
 * Method:		Add2History()
 * Purpose:		Add entry to execute history.
 * Arguments:	s - string (entry)
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void MKCpu::Add2History(string s)
{
	mExecHistory.push(s);
	while (mExecHistory.size() > 20) mExecHistory.pop();
}

/*
 *--------------------------------------------------------------------
 * Method:		GetExecHistory()
 * Purpose:		Return queue with execute history.
 * Arguments:	n/a
 * Returns:		queue<string>
 *--------------------------------------------------------------------
 */
queue<string>	MKCpu::GetExecHistory()
{
	return mExecHistory;
}

} // namespace MKBasic

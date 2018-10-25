 /*
  *--------------------------------------------------------------------
  * Project:    VM65 - Virtual Machine/CPU emulator programming
  *                   framework.  
  *
  * File:   		MKCpu.h
  *
  * Purpose: 		Prototype of MKCpu class. Data structures, enumerations
  *							and constants definitions supporting MKCpu class.
  *
  * Date:      	8/25/2016
  *
  * Copyright:  (C) by Marek Karcz 2016. All rights reserved.
  *
  * Contact:    makarcz@yahoo.com
  *
  * License Agreement and Warranty:

   This software is provided with No Warranty.
   I (Marek Karcz) will not be held responsible for any damage to
   computer systems, data or user's health resulting from use.
   Please proceed responsibly and exercise common sense.
   This software is provided in hope that it will be useful.
   It is free of charge for non-commercial and educational use.
   Distribution of this software in non-commercial and educational
   derivative work is permitted under condition that original
   copyright notices and comments are preserved. Some 3-rd party work
   included with this project may require separate application for
   permission from their respective authors/copyright owners.

 *--------------------------------------------------------------------
 */
#ifndef MKCPU_H
#define MKCPU_H

#include <string>
#include <map>
#include <queue>
#include "system.h"
#include "Memory.h"
#include "qfactory.hpp"

using namespace std;

namespace MKBasic {
	
#define DISS_BUF_SIZE 60	// disassembled instruction buffer size	
#define OPCO_HIS_SIZE 20	// size of op-code execute history queue

struct Regs {
	//qubit index 0: accumulator
	//qubit index 8: x register
	//qubit index 16: carry flag
	//qubit index 17: zero flag
	//qubit index 18: overflow flag
	//qubit index 19: negative flag
	unsigned char 	Acc;		// 8-bit accumulator
	unsigned short 	Acc16;		// 16-bit accumulator
	unsigned char 	IndX;		// 8-bit index register X
	unsigned char 	IndY;		// 8-bit index register Y
	unsigned short 	Ptr16;		// general purpose 16-bit register
	unsigned short 	PtrAddr;	// cpu code counter (PC) - current read/write address
	unsigned char 	PtrStack;	// 8-bit stack pointer (0-255).
	unsigned char 	Flags;		// CPU flags
	bool		SoftIrq;	// true when interrupted with BRK or trapped opcode
	bool            LastRTS;	// true if RTS encountered and stack empty.
	unsigned short	LastAddr;	// PC at the time of previous op-code
	//string	LastInstr;	// instruction and argument executed in previous step
	int		LastOpCode;	// op-code of last instruction
	unsigned short	LastArg;	// argument to the last instruction
	int		LastAddrMode;	// addressing mode of last instruction
	bool		IrqPending;	// pending Interrupt ReQuest (IRQ)
	int		CyclesLeft;	// # of cycles left to complete current opcode
	bool		PageBoundary;	// true if page boundary was crossed

	//Quantum metastatus flags:
	bool 		isAccQ;		// Is accumulator (and potentially carry flag) superposed in permutation basis?
	bool 		isXQ;		// Is X register superposed in permutation basis?
	bool 		isYQ;		// Is X register superposed in permutation basis?
	bool 		isCarryQ;	// Is the accumulator entangled with the X register?
};

#define REGS_ACC_Q		0
#define REGS_INDX_Q		8
#define REGS_INDY_Q		16
#define REG_LEN			8
#define FLAGS_CARRY_Q		24

/*
 * Virtual CPU, 6502 addressing modes:
 
      +---------------------+--------------------------+
      |      mode           |     assembler format     |
      +=====================+==========================+
      | Immediate           |          #aa             |
      | Absolute            |          aaaa            |
      | Zero Page           |          aa              |   Note:
      | Implied             |                          |
      | Indirect Absolute   |          (aaaa)          |     aa = 2 hex digits
      | Absolute Indexed,X  |          aaaa,X          |          as $FF
      | Absolute Indexed,Y  |          aaaa,Y          |
      | Zero Page Indexed,X |          aa,X            |     aaaa = 4 hex
      | Zero Page Indexed,Y |          aa,Y            |          digits as
      | Indexed Indirect    |          (aa,X)          |          $FFFF
      | Indirect Indexed    |          (aa),Y          |
      | Relative            |          aaaa            |     Can also be
      | Accumulator         |          A               |     assembler labels
      +---------------------+--------------------------+

Short notation:

imm = #$00
zp = $00
zpx = $00,X
zpy = $00,Y
izx = ($00,X)
izy = ($00),Y
abs = $0000
abx = $0000,X
aby = $0000,Y
ind = ($0000)
rel = $0000 (PC-relative)

See: 6502AssemblyInOneStep.txt for details.

 */
enum eAddrModes {
	ADDRMODE_IMM = 0,
	ADDRMODE_ABS,
	ADDRMODE_ZP,
	ADDRMODE_IMP,
	ADDRMODE_IND,
	//ADDRMODE_ABA,
	ADDRMODE_ABX,
	ADDRMODE_ABY,
	ADDRMODE_ZPX,
	ADDRMODE_ZPY,
	ADDRMODE_IZX,
	ADDRMODE_IZY,
	ADDRMODE_REL,
	ADDRMODE_ACC,
	ADDRMODE_UND,		// undetermined (for some illegal codes)
	ADDRMODE_NON,
	ADDRMODE_LENGTH	// should be always last
}; 
// assumed little-endian order of bytes (start with least significant)
// MEM - memory location from where the value is read/written, 
// & - reference operator (e.g.: &addr means: value under addr memory location)
// PC - program counter (PC+1 means - next memory location after opcode)
enum eOpCodes {
	OPCODE_BRK 			= 0x00,	// software interrupt, no arguments ($00 : BRK)
	
	/* full compatibility with 65C02 (illegal opcodes not supported, will be used for extended functions */
	OPCODE_ORA_IZX 	= 0x01,	// bitwise OR with Accumulator, Indexed Indirect ($01 arg : ORA (arg,X) ;arg=0..$FF), MEM=&(arg+X)
	OPCODE_HAD_A		= 0x02,	// 6502Q: Hadamard Accumulator
	OPCODE_HAD_X		= 0x03,	// 6502Q: Hadamard IndX
	OPCODE_HAD_Y		= 0x04,	// 6502Q: Hadamard IndY
	OPCODE_ORA_ZP		= 0x05,	// bitwise OR with Accumulator, Zero Page ($05 arg : ORA arg ;arg=0..$FF), MEM=arg
	OPCODE_ASL_ZP		= 0x06,	// Arithmetic Shift Left, Zero Page ($06 arg : ASL arg ;arg=0..$FF), MEM=arg
	OPCODE_ILL_07		= 0x07,	// illegal opcode
	OPCODE_PHP			= 0x08,	// PusH Processor status on Stack, Implied ($08 : PHP)
	OPCODE_ORA_IMM	= 0x09,	// bitwise OR with Accumulator, Immediate ($09 arg : ORA #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_ASL			= 0x0A,	// Arithmetic Shift Left, Accumulator ($0A : ASL)
	OPCODE_ILL_0B			= 0x0B,	// illegal opcode
	OPCODE_PAX_Y			= 0x0C,	// 6502Q: Pauli X on IndY
	OPCODE_ORA_ABS	= 0x0D,	// bitwise OR with Accumulator, Absolute ($0D addrlo addrhi : ORA addr ;addr=0..$FFFF), MEM=addr
	OPCODE_ASL_ABS	= 0x0E,	// Arithmetic Shift Left, Absolute ($0E addrlo addrhi : ASL addr ;addr=0..$FFFF), MEM=addr
	OPCODE_SEN			= 0x0F,	// 6502Q: SEt Negative
	OPCODE_BPL_REL	= 0x10,	// Branch on PLus, Relative ($10 signoffs : BPL signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
	OPCODE_ORA_IZY	= 0x11,	// bitwise OR with Accumulator, Indirect Indexed ($11 arg : ORA (arg),Y ;arg=0..$FF), MEM=&arg+Y
	OPCODE_PAX_A		= 0x12,	// 6502Q: Pauli X on Accumulator
	OPCODE_PAX_X		= 0x13,	// 6502Q: Pauli X on IndX
	OPCODE_PAX_C		= 0x14,	// 6502Q: Pauli X on Carry
	OPCODE_ORA_ZPX	= 0x15,	// bitwise OR with Accumulator, Zero Page Indexed, X ($15 arg : ORA arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_ASL_ZPX	= 0x16,	// Arithmetic Shift Left, Zero Page Indexed, X ($16 arg : ASL arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_HAD_C		= 0x17,	// 6502Q: Hadamard on Carry Flag
	OPCODE_CLC			= 0x18,	// CLear Carry, Implied ($18 : CLC)
	OPCODE_ORA_ABY	= 0x19,	// bitwise OR with Accumulator, Absolute Indexed, Y ($19 addrlo addrhi : ORA addr,Y ;addr=0..$FFFF), MEM=addr+Y
	OPCODE_PAY_A		= 0x1A,	// 6502Q: Pauli Y on Accumulator
	OPCODE_PAY_X		= 0x1B,	// 6502Q: Pauli Y on IndX
	OPCODE_PAY_Y		= 0x1C,	// 6502Q: Pauli Y on IndY
	OPCODE_ORA_ABX	= 0x1D,	// bitwise OR with Accumulator, Absolute Indexed, X ($1D addrlo addrhi : ORA addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_ASL_ABX	= 0x1E,	// Arithmetic Shift Left, Absolute Indexed, X ($1E addrlo addrhi : ASL addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_CLQ			= 0x1F,	// 6502Q: Clear Quantum Mode Flag
	OPCODE_JSR_ABS	= 0x20,	// Jump to SubRoutine, Absolute ($20 addrlo addrhi : JSR addr ;addr=0..$FFFF), MEM=addr
	OPCODE_AND_IZX	=	0x21,	// bitwise AND with accumulator, Indexed Indirect ($21 arg : AND (arg,X) ;arg=0..$FF), MEM=&(arg+X)
	OPCODE_ILL_22		= 0x22,	// illegal opcode
	OPCODE_ILL_23		= 0x23,	// illegal opcode
	OPCODE_BIT_ZP		= 0x24,	// test BITs, Zero Page ($24 arg : BIT arg ;arg=0..$FF), MEM=arg
	OPCODE_AND_ZP		=	0x25,	// bitwise AND with accumulator, Zero Page ($25 arg : AND arg ;arg=0..$FF), MEM=arg
	OPCODE_ROL_ZP		= 0x26,	// ROtate Left, Zero Page ($26 arg : ROL arg ;arg=0..$FF), MEM=arg
	OPCODE_SEV			= 0x27,	// 6502Q: SEt oVerflow
	OPCODE_PLP			= 0x28,	// PuLl Processor status, Implied ($28 : PLP)
	OPCODE_AND_IMM	= 0x29,	// bitwise AND with accumulator, Immediate ($29 arg : AND #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_ROL			= 0x2A,	// ROtate Left, Accumulator ($2A : ROL)
	OPCODE_SEZ			= 0x2B,	// 6502Q: SEt Zero
	OPCODE_BIT_ABS	= 0x2C,	// test BITs, Absolute ($2C addrlo addrhi : BIT addr ;addr=0..$FFFF), MEM=addr
	OPCODE_AND_ABS	= 0x2D,	// bitwise AND with accumulator, Absolute ($2D addrlo addrhi : AND addr ;addr=0..$FFFF), MEM=addr
	OPCODE_ROL_ABS	= 0x2E,	// ROtate Left, Absolute ($2E addrlo addrhi : ROL addr ;addr=0..$FFFF), MEM=addr
	OPCODE_CLN			= 0x2F,	// 6502Q: CLear Negative
	OPCODE_BMI_REL	= 0x30,	// Branch on MInus, Relative ($30 signoffs : BMI signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
	OPCODE_AND_IZY	= 0x31,	// bitwise AND with accumulator, Indirect Indexed ($31 arg : AND (arg),Y ;arg=0..$FF), MEM=&arg+Y
	OPCODE_PAZ_A		= 0x32,	// 6502Q: Pauli Z on Accumulator
	OPCODE_PAZ_X		= 0x33,	// 6502Q: Pauli Z on IndX
	OPCODE_PAZ_Y		= 0x34,	// 6502Q: Pauli Z on IndY
	OPCODE_AND_ZPX	= 0x35,	// bitwise AND with accumulator, Zero Page Indexed, X ($35 arg : AND arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_ROL_ZPX	= 0x36,	// ROtate Left, Zero Page Indexed, X ($36 arg : ROL arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_ILL_37		= 0x37,	// illegal opcode
	OPCODE_SEC			= 0x38,	// SEt Carry, Implied ($38 : SEC)
	OPCODE_AND_ABY	= 0x39,	// bitwise AND with accumulator, Absolute Indexed, Y ($39 addrlo addrhi : AND addr,Y ;addr=0..$FFFF), MEM=addr+Y
	OPCODE_ROT_A		= 0x3A,	// 6502Q: Quarter rotation on |1> axis for Accumulator
	OPCODE_ROT_X		= 0x3B,	// 6502Q: Quarter rotation on |1> axis for X register
	OPCODE_ROT_Y		= 0x3C,	// 6502Q: Quarter rotation on |1> axis for Y register	
	OPCODE_AND_ABX	=	0x3D,	// bitwise AND with accumulator, Absolute Indexed, X ($3D addrlo addrhi : AND addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_ROL_ABX	= 0x3E,	// ROtate Left, Absolute Indexed, X ($3E addrlo addrhi : ROL addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_SEQ			= 0x3F,	// 6502Q: Set Quantum Mode Flag
	OPCODE_RTI			= 0x40,	// ReTurn from Interrupt, Implied ($40 : RTI)
	OPCODE_EOR_IZX	= 0x41,	// bitwise Exclusive OR, Indexed Indirect ($41 arg : EOR (arg,X) ;arg=0..$FF), MEM=&(arg+X)
	OPCODE_ROTX_A		= 0x42,	// 6502Q: Quarter rotation on X axis for Accumulator
	OPCODE_ROTX_X		= 0x43,	// 6502Q: Quarter rotation on X axis for X register
	OPCODE_ROTX_Y		= 0x44,	// 6502Q: Quarter rotation on X axis for Y register
	OPCODE_EOR_ZP		= 0x45,	// bitwise Exclusive OR, Zero Page ($45 arg : EOR arg ;arg=0..$FF), MEM=arg
	OPCODE_LSR_ZP		= 0x46,	// Logical Shift Right, Zero Page ($46 arg : LSR arg ;arg=0..$FF), MEM=arg
	OPCODE_CLZ			= 0x47,	// 6502Q: CLear Zero
	OPCODE_PHA			= 0x48,	// PusH Accumulator, Implied ($48 : PHA)
	OPCODE_EOR_IMM	= 0x49,	// bitwise Exclusive OR, Immediate ($49 arg : EOR #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_LSR			= 0x4A,	// Logical Shift Right, Accumulator ($4A : LSR)
	OPCODE_ILL_4B		= 0x4B,	// illegal opcode
	OPCODE_JMP_ABS	= 0x4C,	// JuMP, Absolute ($4C addrlo addrhi : JMP addr ;addr=0..$FFFF), MEM=addr
	OPCODE_EOR_ABS	= 0x4D,	// bitwise Exclusive OR, Absolute ($4D addrlo addrhi : EOR addr ;addr=0..$FFFF), MEM=addr
	OPCODE_LSR_ABS	= 0x4E,	// Logical Shift Right, Absolute ($4E addrlo addrhi : LSR addr ;addr=0..$FFFF), MEM=addr
	OPCODE_ILL_4F		= 0x4F,	// illegal opcode
	OPCODE_BVC_REL	= 0x50,	// Branch on oVerflow Clear, Relative ($50 signoffs : BVC signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
	OPCODE_EOR_IZY	= 0x51,	// bitwise Exclusive OR, Indirect Indexed ($51 arg : EOR (arg),Y ;arg=0..$FF), MEM=&arg+Y
	OPCODE_ROTY_A		= 0x52,	// 6502Q: Quarter rotation on Y axis for Accumulator
	OPCODE_ROTY_X		= 0x53,	// 6502Q: Quarter rotation on Y axis for X register
	OPCODE_ROTY_Y		= 0x54,	// 6502Q: Quarter rotation on Y axis for Y register	
	OPCODE_EOR_ZPX	= 0x55,	// bitwise Exclusive OR, Zero Page Indexed, X ($55 arg : EOR arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_LSR_ZPX	= 0x56,	// Logical Shift Right, Zero Page Indexed, X ($56 arg : LSR arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_ILL_57		= 0x57,	// illegal opcode
	OPCODE_CLI			= 0x58,	// CLear Interrupt, Implied ($58 : CLI)
	OPCODE_EOR_ABY	= 0x59,	// bitwise Exclusive OR, Absolute Indexed, Y ($59 addrlo addrhi : EOR addr,Y ;addr=0..$FFFF), MEM=addr+Y
	OPCODE_ROTZ_A		= 0x5A,	// 6502Q: Quarter rotation on Z axis for Accumulator
	OPCODE_ROTZ_X		= 0x5B,	// 6502Q: Quarter rotation on Z axis for X register
	OPCODE_ROTZ_Y		= 0x5C,	// 6502Q: Quarter rotation on Z axis for Y register
	OPCODE_EOR_ABX	= 0x5D,	// bitwise Exclusive OR, Absolute Indexed, X ($5D addrlo addrhi : EOR addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_LSR_ABX	= 0x5E,	// Logical Shift Right, Absolute Indexed, X ($5E addrlo addrhi : LSR addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_ILL_5F		= 0x5F,	// illegal opcode
	OPCODE_RTS			= 0x60,	// ReTurn from Subroutine, Implied ($60 : RTS)
	OPCODE_ADC_IZX	= 0x61,	// ADd with Carry, Indexed Indirect ($61 arg : ADC (arg,X) ;arg=0..$FF), MEM=&(arg+X)
	OPCODE_QFT_A		= 0x62,	// 6502Q: Quantum Fourier Transform on Accumulator
	OPCODE_QFT_X		= 0x63,	// 6502Q: Quantum Fourier Transform on X register
	OPCODE_QFT_Y		= 0x64,	// 6502Q: Quantum Fourier Transform on Y register
	OPCODE_ADC_ZP		= 0x65,	// ADd with Carry, Zero Page ($65 arg : ADC arg ;arg=0..$FF), MEM=arg
	OPCODE_ROR_ZP		= 0x66,	// ROtate Right, Zero Page ($66 arg : ROR arg ;arg=0..$FF), MEM=arg
	OPCODE_ILL_67		= 0x67,	// illegal opcode
	OPCODE_PLA			= 0x68,	// PuLl Accumulator, Implied ($68 : PLA)
	OPCODE_ADC_IMM	= 0x69,	// ADd with Carry, Immediate ($69 arg : ADC #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_ROR			= 0x6A,	// ROtate Right, Accumulator ($6A : ROR)
	OPCODE_ILL_6B		= 0x6B,	// illegal opcode
	OPCODE_JMP_IND	= 0x6C,	// JuMP, Indirect Absolute ($6C addrlo addrhi : JMP (addr) ;addr=0..FFFF), MEM=&addr
	OPCODE_ADC_ABS	= 0x6D,	// ADd with Carry, Absolute ($6D addrlo addrhi : ADC addr ;addr=0..$FFFF), MEM=addr
	OPCODE_ROR_ABS	= 0x6E,	// ROtate Right, Absolute ($6E addrlo addrhi : ROR addr ;addr=0..$FFFF), MEM=addr
	OPCODE_ILL_6F		= 0x6F,	// illegal opcode
	OPCODE_BVS_REL	= 0x70,	// Branch on oVerflow Set, Relative ($70 signoffs : BVS signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
	OPCODE_ADC_IZY	= 0x71,	// ADd with Carry, Indirect Indexed ($71 arg : ADC (arg),Y ;arg=0..$FF), MEM=&arg+Y
	OPCODE_ILL_72		= 0x72,	// illegal opcode
	OPCODE_ILL_73		= 0x73,	// illegal opcode
	OPCODE_ILL_74		= 0x74,	// illegal opcode
	OPCODE_ADC_ZPX	= 0x75,	// ADd with Carry, Zero Page Indexed, X ($75 arg : ADC arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_ROR_ZPX	= 0x76,	// ROtate Right, Zero Page Indexed, X ($76 arg : ROR arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_ILL_77		= 0x77,	// illegal opcode
	OPCODE_SEI			= 0x78,	// SEt Interrupt, Implied ($78 : SEI)
	OPCODE_ADC_ABY	= 0x79,	// ADd with Carry, Absolute Indexed, Y ($79 addrlo addrhi : ADC addr,Y ;addr=0..$FFFF), MEM=addr+Y
	OPCODE_ILL_7A		= 0x7A,	// illegal opcode
	OPCODE_ILL_7B		= 0x7B,	// illegal opcode
	OPCODE_ILL_7C		= 0x7C,	// illegal opcode	
	OPCODE_ADC_ABX	= 0x7D,	// ADd with Carry, Absolute Indexed, X ($7D addrlo addrhi : ADC addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_ROR_ABX	=	0x7E,	// ROtate Right, Absolute Indexed, X ($7E addrlo addrhi : ROR addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_ILL_7F		= 0x7F,	// illegal opcode
	OPCODE_ILL_80		= 0x80,	// illegal opcode
	OPCODE_STA_IZX	= 0x81,	// STore Accumulator, Indexed Indirect ($81 arg : STA (arg,X) ;arg=0..$FF), MEM=&(arg+X)
	OPCODE_ILL_82		= 0x82,	// illegal opcode
	OPCODE_ILL_83		= 0x83,	// illegal opcode
	OPCODE_STY_ZP		= 0x84,	// STore Y register, Zero Page ($84 arg : STY arg ;arg=0..$FF), MEM=arg
	OPCODE_STA_ZP		= 0x85,	// STore Accumulator, Zero Page ($85 arg : STA arg ;arg=0..$FF), MEM=arg
	OPCODE_STX_ZP		= 0x86,	// STore X register, Zero Page ($86 arg : STX arg ;arg=0..$FF), MEM=arg
	OPCODE_ILL_87		= 0x87,	// illegal opcode
	OPCODE_DEY			= 0x88,	// DEcrement Y, Implied ($88 : DEY)
	OPCODE_ILL_89		= 0x89,	// illegal opcode
	OPCODE_TXA			= 0x8A,	// Transfer X to A, Implied ($8A : TXA)
	OPCODE_ILL_8B		= 0x8B,	// illegal opcode
	OPCODE_STY_ABS	= 0x8C,	// STore Y register, Absolute ($8C addrlo addrhi : STY addr ;addr=0..$FFFF), MEM=addr
	OPCODE_STA_ABS	= 0x8D,	// STore Accumulator, Absolute ($8D addrlo addrhi : STA addr ;addr=0..$FFFF), MEM=addr
	OPCODE_STX_ABS	= 0x8E,	// STore X register, Absolute ($8E addrlo addrhi : STX addr ;addr=0..$FFFF), MEM=addr
	OPCODE_ILL_8F		= 0x8F,	// illegal opcode
	OPCODE_BCC_REL	= 0x90,	// Branch on Carry Clear, Relative ($90 signoffs : BCC signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
	OPCODE_STA_IZY	= 0x91,	// STore Accumulator, Indirect Indexed ($91 arg : STA (arg),Y ;arg=0..$FF), MEM=&arg+Y
	OPCODE_ILL_92		= 0x92,	// illegal opcode
	OPCODE_ILL_93		= 0x93,	// illegal opcode
	OPCODE_STY_ZPX	= 0x94,	// STore Y register, Zero Page Indexed, X ($94 arg : STY arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_STA_ZPX	= 0x95,	// STore Accumulator, Zero Page Indexed, X ($95 arg : STA arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_STX_ZPY	= 0x96,	// STore X register, Zero Page Indexed, Y ($96 arg : STX arg,Y ;arg=0..$FF), MEM=arg+Y
	OPCODE_ILL_97		= 0x97,	// illegal opcode
	OPCODE_TYA			= 0x98,	// Transfer Y to A, Implied ($98 : TYA)
	OPCODE_STA_ABY	= 0x99,	// STore Accumulator, Absolute Indexed, Y ($99 addrlo addrhi : STA addr,Y ;addr=0..$FFFF), MEM=addr+Y
	OPCODE_TXS			= 0x9A,	// Transfer X to Stack ptr, Implied ($9A : TXS)
	OPCODE_ILL_9B		= 0x9B,	// illegal opcode
	OPCODE_ILL_9C		= 0x9C,	// illegal opcode	
	OPCODE_STA_ABX	= 0x9D,	// STore Accumulator, Absolute Indexed, X ($9D addrlo addrhi : STA addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_ILL_9E		= 0x9E,	// illegal opcode
	OPCODE_ILL_9F		= 0x9F,	// illegal opcode
	OPCODE_LDY_IMM	=	0xA0,	// LoaD Y register, Immediate ($A0 arg : LDY #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_LDA_IZX	= 0xA1,	// LoaD Accumulator, Indexed Indirect ($A1 arg : LDA (arg,X) ;arg=0..$FF), MEM=&(arg+X)
	OPCODE_LDX_IMM	= 0xA2,	// LoaD X register, Immediate ($A2 arg : LDX #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_ILL_A3		= 0xA3,	// illegal opcode
	OPCODE_LDY_ZP		= 0xA4,	// LoaD Y register, Zero Page ($A4 arg : LDY arg ;arg=0..$FF), MEM=arg
	OPCODE_LDA_ZP		= 0xA5,	// LoaD Accumulator, Zero Page ($A5 arg : LDA arg ;arg=0..$FF), MEM=arg
	OPCODE_LDX_ZP		= 0xA6,	// LoaD X register, Zero Page ($A6 arg : LDX arg ;arg=0..$FF), MEM=arg
	OPCODE_ILL_A7		= 0xA7,	// illegal opcode
	OPCODE_TAY			= 0xA8,	// Transfer A to Y, Implied ($A8 : TAY)
	OPCODE_LDA_IMM	= 0xA9,	// LoaD Accumulator, Immediate ($A9 arg : LDA #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_TAX			= 0xAA,	// Transfer A to X, Implied ($AA : TAX)
	OPCODE_ILL_AB		= 0xAB,	// illegal opcode
	OPCODE_LDY_ABS	= 0xAC,	// LoaD Y register, Absolute ($AC addrlo addrhi : LDY addr ;addr=0..$FFFF), MEM=addr
	OPCODE_LDA_ABS	= 0xAD,	// LoaD Accumulator, Absolute ($AD addrlo addrhi : LDA addr ;addr=0..$FFFF), MEM=addr
	OPCODE_LDX_ABS	= 0xAE,	// LoaD X register, Absolute ($AE addrlo addrhi : LDX addr ;addr=0..$FFFF), MEM=addr
	OPCODE_ILL_AF		= 0xAF,	// illegal opcode
	OPCODE_BCS_REL	= 0xB0,	// Branch on Carry Set, Relative ($B0 signoffs : BCS signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
	OPCODE_LDA_IZY	= 0xB1,	// LoaD Accumulator, Indirect Indexed ($B1 arg : LDA (arg),Y ;arg=0..$FF), MEM=&arg+Y
	OPCODE_ILL_B2		= 0xB2,	// illegal opcode
	OPCODE_ILL_B3		= 0xB3,	// illegal opcode	
	OPCODE_LDY_ZPX	= 0xB4,	// LoaD Y register, Zero Page Indexed, X ($B4 arg : LDY arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_LDA_ZPX	= 0xB5,	// LoaD Accumulator, Zero Page Indexed, X ($B5 arg : LDA arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_LDX_ZPY	= 0xB6,	// LoaD X register, Zero Page Indexed, Y ($B6 arg : LDX arg,Y ;arg=0..$FF), MEM=arg+Y
	OPCODE_ILL_B7		= 0xB7,	// illegal opcode	
	OPCODE_CLV			= 0xB8,	// CLear oVerflow, Implied ($B8 : CLV)
	OPCODE_LDA_ABY	= 0xB9,	// LoaD Accumulator, Absolute Indexed, Y ($B9 addrlo addrhi : LDA addr,Y ;addr=0..$FFFF), MEM=addr+Y
	OPCODE_TSX			= 0xBA,	// Transfer Stack ptr to X, Implied ($BA : TSX)
	OPCODE_ILL_BB		= 0xBB,	// illegal opcode	
	OPCODE_LDY_ABX	= 0xBC,	// LoaD Y register, Absolute Indexed, X ($BC addrlo addrhi : LDY addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_LDA_ABX	= 0xBD,	// LoaD Accumulator, Absolute Indexed, X ($BD addrlo addrhi : LDA addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_LDX_ABY	= 0xBE,	// LoaD X register, Absolute Indexed, Y ($BE addrlo addrhi : LDX addr,Y ;addr=0..$FFFF), MEM=addr+Y
	OPCODE_ILL_BF		= 0xBF,	// illegal opcode
	OPCODE_CPY_IMM	= 0xC0,	// ComPare Y register, Immediate ($C0 arg : CPY #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_CMP_IZX	= 0xC1,	// CoMPare accumulator, Indexed Indirect ($A1 arg : LDA (arg,X) ;arg=0..$FF), MEM=&(arg+X)
	OPCODE_ILL_C2		= 0xC2,	// illegal opcode
	OPCODE_ILL_C3		= 0xC3,	// illegal opcode		
	OPCODE_CPY_ZP		= 0xC4,	// ComPare Y register, Zero Page ($C4 arg : CPY arg ;arg=0..$FF), MEM=arg
	OPCODE_CMP_ZP		= 0xC5,	// CoMPare accumulator, Zero Page ($C5 arg : CMP arg ;arg=0..$FF), MEM=arg
	OPCODE_DEC_ZP		= 0xC6,	// DECrement memory, Zero Page ($C6 arg : DEC arg ;arg=0..$FF), MEM=arg
	OPCODE_ILL_C7		= 0xC7,	// illegal opcode
	OPCODE_INY			= 0xC8,	// INcrement Y, Implied ($C8 : INY)OPCODE_INY			= 0xC8,	// INcrement Y, Implied ($C8 : INY)
	OPCODE_CMP_IMM	= 0xC9,	// CoMPare accumulator, Immediate ($C9 arg : CMP #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_DEX			= 0xCA,	// DEcrement X, Implied ($CA : DEX)
	OPCODE_ILL_CB		= 0xCB,	// illegal opcode
	OPCODE_CPY_ABS	= 0xCC,	// ComPare Y register, Absolute ($CC addrlo addrhi : CPY addr ;addr=0..$FFFF), MEM=addr
	OPCODE_CMP_ABS	= 0xCD,	// CoMPare accumulator, Absolute ($CD addrlo addrhi : CMP addr ;addr=0..$FFFF), MEM=addr
	OPCODE_DEC_ABS	= 0xCE,	// DECrement memory, Absolute ($CE addrlo addrhi : CMP addr ;addr=0..$FFFF), MEM=addr
	OPCODE_ILL_CF		= 0xCF,	// illegal opcode
	OPCODE_BNE_REL	= 0xD0,	// Branch on Not Equal, Relative ($D0 signoffs : BNE signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
	OPCODE_CMP_IZY	= 0xD1,	// CoMPare accumulator, Indirect Indexed ($D1 arg : CMP (arg),Y ;arg=0..$FF), MEM=&arg+Y
	OPCODE_ILL_D2		= 0xD2,	// illegal opcode
	OPCODE_ILL_D3		= 0xD3,	// illegal opcode
	OPCODE_ILL_D4		= 0xD4,	// illegal opcode	
	OPCODE_CMP_ZPX	= 0xD5,	// CoMPare accumulator, Zero Page Indexed, X ($D5 arg : CMP arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_DEC_ZPX	= 0xD6,	// DECrement memory, Zero Page Indexed, X ($D6 arg : DEC arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_ILL_D7		= 0xD7,	// illegal opcode	
	OPCODE_CLD			= 0xD8,	// CLear Decimal, Implied ($D8 : CLD)
	OPCODE_CMP_ABY	= 0xD9,	// CoMPare accumulator, Absolute Indexed, Y ($D9 addrlo addrhi : CMP addr,Y ;addr=0..$FFFF), MEM=addr+Y
	OPCODE_ILL_DA		= 0xDA,	// illegal opcode
	OPCODE_ILL_DB		= 0xDB,	// illegal opcode
	OPCODE_ILL_DC		= 0xDC,	// illegal opcode	
	OPCODE_CMP_ABX	= 0xDD,	// CoMPare accumulator, Absolute Indexed, X ($DD addrlo addrhi : CMP addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_DEC_ABX	= 0xDE,	// DECrement memory, Absolute Indexed, X ($DE addrlo addrhi : DEC addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_ILL_DF		= 0xDF,	// illegal opcode
	OPCODE_CPX_IMM	= 0xE0,	// ComPare X register, Immediate ($E0 arg : CPX #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_SBC_IZX	= 0xE1,	// SuBtract with Carry, Indexed Indirect ($E1 arg : SBC (arg,X) ;arg=0..$FF), MEM=&(arg+X)
	OPCODE_ILL_E2		= 0xE2,	// illegal opcode
	OPCODE_ILL_E3		= 0xE3,	// illegal opcode	
	OPCODE_CPX_ZP		= 0xE4,	// ComPare X register, Zero Page ($E4 arg : CPX arg ;arg=0..$FF), MEM=arg
	OPCODE_SBC_ZP		= 0xE5,	// SuBtract with Carry, Zero Page ($E5 arg : SBC arg ;arg=0..$FF), MEM=arg
	OPCODE_INC_ZP		= 0xE6,	// INCrement memory, Zero Page ($E6 arg : INC arg ;arg=0..$FF), MEM=arg
	OPCODE_ILL_E7		= 0xE7,	// illegal opcode
	OPCODE_INX			= 0xE8,	// INcrement X, Implied ($E8 : INX)
	OPCODE_SBC_IMM	= 0xE9,	// SuBtract with Carry, Immediate ($E9 arg : SBC #arg ;arg=0..$FF), MEM=PC+1
	OPCODE_NOP			= 0xEA,	// NO oPeration, Implied ($EA : NOP)
	OPCODE_ILL_EB		= 0xEB,	// illegal opcode
	OPCODE_CPX_ABS	= 0xEC,	// ComPare X register, Absolute ($EC addrlo addrhi : CPX addr ;addr=0..$FFFF), MEM=addr
	OPCODE_SBC_ABS	= 0xED,	// SuBtract with Carry, Absolute ($ED addrlo addrhi : SBC addr ;addr=0..$FFFF), MEM=addr
	OPCODE_INC_ABS	= 0xEE,	// INCrement memory, Absolute ($EE addrlo addrhi : INC addr ;addr=0..$FFFF), MEM=addr
	OPCODE_ILL_EF		= 0xEF,	// illegal opcode
	OPCODE_BEQ_REL	= 0xF0,	// Branch on EQual, Relative ($F0 signoffs : BEQ signoffs ;signoffs=0..$FF [-128 ($80)..127 ($7F)])
	OPCODE_SBC_IZY	= 0xF1,	// SuBtract with Carry, Indirect Indexed ($F1 arg : SBC (arg),Y ;arg=0..$FF), MEM=&arg+Y
	OPCODE_ILL_F2		= 0xF2,	// illegal opcode
	OPCODE_ILL_F3		= 0xF3,	// illegal opcode
	OPCODE_ILL_F4		= 0xF4,	// illegal opcode
	OPCODE_SBC_ZPX	= 0xF5,	// SuBtract with Carry, Zero Page Indexed, X ($F5 arg : SBC arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_INC_ZPX	= 0xF6,	// INCrement memory, Zero Page Indexed, X ($F6 arg : INC arg,X ;arg=0..$FF), MEM=arg+X
	OPCODE_QZN_Z		= 0xF7,	// 6502Q: Apply Z operator to zero flag
	OPCODE_SED			= 0xF8,	// SEt Decimal, Implied ($F8 : SED)
	OPCODE_SBC_ABY	= 0xF9,	// SuBtract with Carry, Absolute Indexed, Y ($F9 addrlo addrhi : SBC addr,Y ;addr=0..$FFFF), MEM=addr+Y
	OPCODE_QZN_S		= 0xFA,	// 6502Q: Apply Z operator to negative flag
	OPCODE_QZN_C		= 0xFB,	// 6502Q: Apply Z operator to carry flag
	OPCODE_ILL_FC		= 0xFC,	// illegal opcode
	OPCODE_SBC_ABX	= 0xFD,	// SuBtract with Carry, Absolute Indexed, X ($FD addrlo addrhi : SBC addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_INC_ABX	= 0xFE,	// INCrement memory, Absolute Indexed, X ($FE addrlo addrhi : INC addr,X ;addr=0..$FFFF), MEM=addr+X
	OPCODE_ILL_FF		= 0xFF	// illegal opcode
};

class MKCpu;

typedef void (MKCpu::*OpCodeHdlrFn)();

struct OpCode {
	int 					code;			// the byte value of the opcode
	int 					addrmode;	// addressing mode (see eAddrModes)
	int 					time;			// # of cycles
	string				amf;			// assembler mnemonic
	OpCodeHdlrFn	pfun;			// opcoce handler function
};

typedef map<eOpCodes,OpCode> OpCodesMap;

/*
 *------------------------------------------------------------------------------
       bit ->   7                           0
              +---+---+---+---+---+---+---+---+
              | N | V |   | B | D | I | Z | C |  <-- flag, 0/1 = reset/set
              +---+---+---+---+---+---+---+---+
              
              
       N  =  NEGATIVE. Set if bit 7 of the accumulator is set.
       
       V  =  OVERFLOW. Set if the addition of two like-signed numbers or the
             subtraction of two unlike-signed numbers produces a result
             greater than +127 or less than -128.
             
       B  =  BRK COMMAND. Set if an interrupt caused by a BRK, reset if
             caused by an external interrupt.
             
       D  =  DECIMAL MODE. Set if decimal mode active.
       
       I  =  IRQ DISABLE.  Set if maskable interrupts are disabled.
             
       Z  =  ZERO.  Set if the result of the last operation (load/inc/dec/
             add/sub) was zero.
             
       C  =  CARRY. Set if the add produced a carry, or if the subtraction
             produced a borrow.  Also holds bits after a logical shift.
 *------------------------------------------------------------------------------             
 */
enum eCpuFlagMasks {
	FLAGS_CARRY			= 0x01,		// 0: C
	FLAGS_ZERO			= 0x02,		// 1: Z
	FLAGS_IRQ				= 0x04,		// 2: I
	FLAGS_DEC				= 0x08,		// 3: D
	FLAGS_BRK				= 0x10,		// 4: B (Clear if interrupt vectoring, set if BRK or PHP)
	FLAGS_QUANTUM		= 0x20,		// 5: 6502Q: Quantum mode flag
	FLAGS_OVERFLOW 	= 0x40,		// 6: V
	FLAGS_SIGN 			= 0x80		// 7: N
};

enum eLogicOps {
	LOGOP_OR,
	LOGOP_AND,
	LOGOP_EOR
};

class MKCpu
{
	public:

		bool mExitAtLastRTS;
		
		MKCpu();
		MKCpu(Memory *pmem);
		~MKCpu();
		
		Regs *ExecOpcode(unsigned short memaddr);
		Regs *GetRegs();
		void	SetRegs(Regs r);
		queue<string>	GetExecHistory();
		void	EnableExecHistory(bool enexehist);
		bool	IsExecHistoryEnabled();
		unsigned short Disassemble(unsigned short addr,
															 char *instrbuf);					// Disassemble instruction in memory, return next instruction addr.
		void Reset();																				// reset CPU		
		void Interrupt();																		// Interrupt ReQuest (IRQ)
		
	protected:
		
	private:

		// keeps all needed data to disassemble op-codes in execute history queue
		struct OpCodeHistItem {
			unsigned char 	Acc;					// 8-bit accumulator
			unsigned char 	IndX;					// 8-bit index register X
			unsigned char 	IndY;					// 8-bit index register Y
			unsigned char 	Flags;				// CPU flags			
			unsigned char 	PtrStack;			// 8-bit stack pointer (0-255).			
			unsigned short	LastAddr;			// PC at the time of previous op-code
			string					LastInstr;		// instruction and argument executed in previous step
			int							LastOpCode;		// op-code of last instruction
			unsigned short	LastArg;			// argument to the last instruction
			int							LastAddrMode;	// addressing mode of last instruction			
		};
		
		struct Regs mReg;						// CPU registers
		Memory 			*mpMem;					// pointer to memory object
		bool 				mLocalMem;			// true - memory locally allocated
		OpCodesMap	mOpCodesMap;		// hash table of all opcodes
		int					mAddrModesLen[ADDRMODE_LENGTH];	// array of instructions lengths per addressing mode
		string			mArgFmtTbl[ADDRMODE_LENGTH];		// array of instructions assembly formats per addressing mode
		queue<OpCodeHistItem> mExecHistory;					// keep the op-codes execute history
		bool				mEnableHistory;	// enable/disable execute history
		
		
		void InitCpu();
		unsigned char RotateClassical(unsigned char reg);								//Perform classical equivalent of quantum rotation
		void SetFlags(unsigned char reg);									// set CPU flags ZERO and SIGN based on reg
		void SetFlagsReg(unsigned char regStart);								// set CPU flags ZERO and SIGN based on Acc, X or Y
		void MeasureFlagsQ();										//Measure quantum flags
		void ShiftLeftQ();
		unsigned char ShiftLeft(unsigned char arg8);				// Arithmetic Shift Left, set Carry flag
		void ShiftRightQ();
		unsigned char ShiftRight(unsigned char arg8);				// Logical Shift Right, update flags NZC.
		void RotateLeftQ();
		unsigned char RotateLeft(unsigned char arg8);				// Rotate left, Carry to bit 0, bit 7 to Carry, update flags N and Z.
		void RotateRightQ();
		unsigned char RotateRight(unsigned char arg8);			// Rotate left, Carry to bit 7, bit 0 to Carry, update flags N and Z.
		unsigned short GetArg16(unsigned char offs);				// Get 2-byte argument, add offset, increase PC.
		void LogicOpAcc(unsigned short addr, int logop);		// Perform logical bitwise operation between memory at address and Acc.
		void CompareOpAcc(unsigned char val);				// Perform arithmetic compare between val and Acc
		void CompareOpIndX(unsigned char val);				// Perform arithmetic compare between val and IndX
		void CompareOpIndY(unsigned char val);				// Perform arithmetic compare between val and IndY
																												// Result in Acc. Set flags.
		unsigned short ComputeRelJump(unsigned char offs);	// Compute new PC based on relative offset.
		unsigned short ComputeRelJump(unsigned short addr,
																	unsigned char offs);	// Compute new address after branch based on relative offset.
		unsigned char Conv2Bcd(unsigned short v);						// Convert number to BCD representation.
		unsigned short Bcd2Num(unsigned char v);						// Convert BCD code to number.
		bool CheckFlag(unsigned char flag);									// Return true if given CPU status flag is set, false otherwise.
		bool CheckFlagQ(unsigned char flag);									// Return true if given quantum CPU status flag is set, false otherwise.
		void SetFlag(bool set, unsigned char flag);					// Set or unset processor status flag.
		unsigned char AddWithCarry(unsigned char mem8);			// Add With Carry, update flags and Acc.
		void AddWithCarryQ(unsigned char mem8);
		unsigned char SubWithCarry(unsigned char mem8);			// Subtract With Carry, update flags and Acc.
		void SubWithCarryQ(unsigned char mem8);	
		unsigned short GetAddrWithMode(int mode);						// Get address of the byte argument with specified addr. mode
		unsigned short GetArgWithMode(unsigned short opcaddr,
																	int mode);						// Get argument from address with specified addr. mode
		unsigned short Disassemble(OpCodeHistItem *histit);	// Disassemble op-code exec history item
		//void Add2History(string s);													// add entry to op-codes execute history
		void Add2History(OpCodeHistItem histitem);					// add entry to op-codes execute history
		bool PageBoundary(unsigned short startaddr,
											unsigned short endaddr);					// detect if page boundary was crossed
		void CollapseAccQ();	//Collapse Acc register state if it is in superposition
		void CollapseXQ();	//Collapse X register state if it is in superposition
		void CollapseYQ();	//Collapse Y register state if it is in superposition
		void CollapseCarryQ();	//Collapse carry flag state if it is in superposition
		void PrepareAccQ();	//Prepare Acc register qubit state for quantum operations
		void PrepareXQ();	//Prepare X register qubit state for quantum operations
		void PrepareYQ();	//Prepare Y register qubit state for quantum operations
		void PrepareCarryQ();	//Prepare carry flag qubit state for quantum operations

		// opcode execute methods
		void OpCodeBrk();
		void OpCodeNop();
		void OpCodeLdaIzx();
		void OpCodeLdaZp();
		void OpCodeLdaImm();
		void OpCodeLdaAbs();
		void OpCodeLdaIzy();
		void OpCodeLdaZpx();
		void OpCodeLdaAby();
		void OpCodeLdaAbx();
		void OpCodeLdxImm();
		void OpCodeLdxZp();
		void OpCodeLdxAbs();
		void OpCodeLdxZpy();
		void OpCodeLdxAby();
		void OpCodeLdyImm();
		void OpCodeLdyZp();
		void OpCodeLdyAbs();
		void OpCodeLdyZpx();
		void OpCodeLdyAbx();
		void OpCodeTax();
		void OpCodeTay();
		void OpCodeTxa();
		void OpCodeTya();
		void OpCodeTsx();
		void OpCodeTxs();
		void OpCodeStaIzx();
		void OpCodeStaZp();
		void OpCodeStaAbs();
		void OpCodeStaIzy();
		void OpCodeStaZpx();
		void OpCodeStaAby();
		void OpCodeStaAbx();
		void OpCodeStxZp();
		void OpCodeStxAbs();
		void OpCodeStxZpy();
		void OpCodeStyZp();
		void OpCodeStyAbs();
		void OpCodeStyZpx();
		void OpCodeBneRel();
		void OpCodeBeqRel();
		void OpCodeBplRel();
		void OpCodeBmiRel();
		void OpCodeBvcRel();
		void OpCodeBvsRel();
		void OpCodeBccRel();
		void OpCodeBcsRel();
		void OpCodeIncZp();
		void OpCodeIncAbs();
		void OpCodeIncZpx();
		void OpCodeIncAbx();
		void OpCodeInx();
		void OpCodeDex();
		void OpCodeIny();
		void OpCodeDey();
		void OpCodeJmpAbs();
		void OpCodeJmpInd();
		void OpCodeOraIzx();
		void OpCodeOraZp();
		void OpCodeOraImm();
		void OpCodeOraAbs();
		void OpCodeOraIzy();
		void OpCodeOraZpx();
		void OpCodeOraAby();
		void OpCodeOraAbx();
		void OpCodeAslZp();
		void OpCodeAslAcc();
		void OpCodeAslAbs();
		void OpCodeAslZpx();
		void OpCodeAslAbx();
		void OpCodeJsrAbs();
		void OpCodeAndIzx();
		void OpCodeAndZp();
		void OpCodeAndImm();
		void OpCodeAndAbs();
		void OpCodeAndIzy();
		void OpCodeAndZpx();
		void OpCodeAndAby();
		void OpCodeAndAbx();
		void OpCodeBitZp();
		void OpCodeBitAbs();
		void OpCodeRolZp();
		void OpCodeRolAcc();
		void OpCodeRolAbs();
		void OpCodeRolZpx();
		void OpCodeRolAbx();
		void OpCodePhp();
		void OpCodePha();
		void OpCodePlp();
		void OpCodePla();
		void OpCodeClc();
		void OpCodeSec();
		void OpCodeCli();
		void OpCodeClv();
		void OpCodeCld();
		void OpCodeSed();
		void OpCodeSei();
		void OpCodeRti();
		void OpCodeRts();
		void OpCodeEorIzx();
		void OpCodeEorZp();
		void OpCodeEorImm();
		void OpCodeEorAbs();
		void OpCodeEorIzy();
		void OpCodeEorZpx();
		void OpCodeEorAby();
		void OpCodeEorAbx();
		void OpCodeLsrZp();
		void OpCodeLsrAcc();
		void OpCodeLsrAbs();
		void OpCodeLsrZpx();
		void OpCodeLsrAbx();
		void OpCodeAdcIzx();
		void OpCodeAdcZp();
		void OpCodeAdcImm();
		void OpCodeAdcAbs();
		void OpCodeAdcIzy();
		void OpCodeAdcZpx();
		void OpCodeAdcAby();
		void OpCodeAdcAbx();
		void OpCodeRorZp();
		void OpCodeRorAcc();
		void OpCodeRorAbs();
		void OpCodeRorZpx();
		void OpCodeRorAbx();
		void OpCodeCpyImm();
		void OpCodeCpyZp();
		void OpCodeCpyAbs();
		void OpCodeCmpIzx();
		void OpCodeCmpZp();
		void OpCodeCmpImm();
		void OpCodeCmpAbs();
		void OpCodeCmpIzy();
		void OpCodeCmpZpx();
		void OpCodeCmpAby();
		void OpCodeCmpAbx();
		void OpCodeDecZp();
		void OpCodeDecAbs();
		void OpCodeDecZpx();
		void OpCodeDecAbx();
		void OpCodeCpxImm();
		void OpCodeCpxZp();
		void OpCodeCpxAbs();
		void OpCodeSbcZp();
		void OpCodeSbcAbs();
		void OpCodeSbcIzx();
		void OpCodeSbcIzy();
		void OpCodeSbcZpx();
		void OpCodeSbcAby();
		void OpCodeSbcAbx();
		void OpCodeSbcImm();
		void OpCodeDud();
//Quantum Opcodes
		void OpCodeHadA();
		void OpCodeHadX();
		void OpCodeHadY();
		void OpCodeXA();
		void OpCodeXX();
		void OpCodeXY();
		void OpCodeXC();
		void OpCodeYA();
		void OpCodeYX();
		void OpCodeYY();
		void OpCodeZA();
		void OpCodeZX();
		void OpCodeZY();
		void OpCodeRTA();
		void OpCodeRTX();
		void OpCodeRTY();
		void OpCodeRXA();
		void OpCodeRXX();
		void OpCodeRXY();
		void OpCodeRYA();
		void OpCodeRYX();
		void OpCodeRYY();
		void OpCodeRZA();
		void OpCodeRZX();
		void OpCodeRZY();
		void OpCodeFTA();
		void OpCodeFTX();
		void OpCodeFTY();
		void OpCodeLdaAba();
		void OpCodeHac();
		void OpCodeClq();
		void OpCodeSeq();
		void OpCodeQzZero();
		void OpCodeQzSign();
		void OpCodeQzCarry();
		void OpCodeQzOver();
		void OpCodeSen();
		void OpCodeCln();
		void OpCodeSev();
		void OpCodeSez();
		void OpCodeClz();
};

} // namespace MKBasic

#endif

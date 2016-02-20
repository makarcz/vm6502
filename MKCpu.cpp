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
 */
void MKCpu::InitCpu()
{
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
	// Set default BRK vector.
	mpMem->Poke8bit(0xFFFE,0xF0);
	mpMem->Poke8bit(0xFFFF,0xFF);
	// Put RTI opcode at BRK address.
	mpMem->Poke8bit(0xFFF0, OPCODE_RTI);
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
	unsigned short newpc = mReg.PtrAddr; // PtrAddr must be at the next
																			 // opcode at this point
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
	/* This algorithm was shamelessly ripped from Frodo emulator code.
		 Well, maybe not totally shamelessly, I put up a fight to roll my own.
		 I gave up after one day of trying to make it right based only on MOS documentation.
		 For my defense I have this - Frodo also does not work 100% as real metal MOS 6502).
		 And so doesn't Kowalski's emulator.
		 E.g.:
		 Real CPU - Rockwell 6502 AP in BCD mode:
		 80 + f0 and C=0 gives d0 and N=1 V=1 Z=0 C=1 (F9)
		 Kowalski's 6502 emulator gives: d0 and N=0, V=0, Z=0, C=1
		 My emulator gives: d0 and N=0, V=1, Z=0, C=1
	 */
	unsigned short utmp16 = mReg.Acc + mem8 + (CheckFlag(FLAGS_CARRY) ? 1 : 0);
	if (CheckFlag(FLAGS_DEC)) {	// BCD mode
	
		unsigned short al = (mReg.Acc & 0x0F) + (mem8 & 0x0F) + (CheckFlag(FLAGS_CARRY) ? 1 : 0);
		if (al > 9) al += 6;
		unsigned short ah = (mReg.Acc >> 4) + (mem8 >> 4);
		if (al > 0x0F) ah++;
		SetFlag((utmp16 == 0), FLAGS_ZERO);
		SetFlag((((ah << 4) & FLAGS_SIGN) == FLAGS_SIGN), FLAGS_SIGN);
		SetFlag(((((ah << 4) ^ mReg.Acc) & 0x80) && !((mReg.Acc ^ mem8) & 0x80)), FLAGS_OVERFLOW);
		if (ah > 9) ah += 6;
		SetFlag((ah > 0x0F), FLAGS_CARRY);
		mReg.Acc = (ah << 4) | (al & 0x0f);
	} else {	// binary mode
	
		SetFlag((utmp16 > 0xff), FLAGS_CARRY);
		SetFlag((!((mReg.Acc ^ mem8) & 0x80) && ((mReg.Acc ^ utmp16) & 0x80)), FLAGS_OVERFLOW);
		SetFlag((utmp16 == 0), FLAGS_ZERO);
		SetFlag(((utmp16 & 0xFF) & FLAGS_SIGN), FLAGS_SIGN);
		mReg.Acc = utmp16 & 0xFF;
	}
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

	/* This algorithm was shamelessly ripped from Frodo emulator code.
		 See my comments in AddWithCarry() method.
		 This method returned the same results when testing BCD mode as Rockwell 6502 AP CPU.
		 Kowalski's emulator returned different results.
		 My method also passes BCD mode behavior test by Bruce Clark (TestBCD.65s).
	 */
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
		SetFlag(((utmp16 & 0xFF) & FLAGS_SIGN) == FLAGS_SIGN, FLAGS_SIGN);
		mReg.Acc = (ah << 4) | (al & 0x0f);
		
	} else { // binary mode
	
		SetFlag((utmp16 < 0x100), FLAGS_CARRY);
		SetFlag(((mReg.Acc ^ utmp16) & 0x80) && ((mReg.Acc ^ mem8) & 0x80), FLAGS_OVERFLOW);
		SetFlag((utmp16 == 0), FLAGS_ZERO);
		SetFlag(((utmp16 & 0xFF) & FLAGS_SIGN) == FLAGS_SIGN, FLAGS_SIGN);
		mReg.Acc = utmp16 & 0xFF;
	
	}
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
	
	switch (mode) {
		
		case ADDRMODE_IMM:
			arg16 = mReg.PtrAddr++;
			break;
			
		case ADDRMODE_ABS:
			arg16 = GetArg16(0);
			break;
			
		case ADDRMODE_ZP:
			arg16 = (unsigned short) mpMem->Peek8bit(mReg.PtrAddr++);
			break;
			
		case ADDRMODE_IMP:
			// implied mode is an internal CPU register
			break;
			
		case ADDRMODE_IND:
			arg16 = mpMem->Peek16bit(mReg.PtrAddr++);
			arg16 = mpMem->Peek16bit(arg16);
			break;
			
		case ADDRMODE_ABX:
			arg16 = GetArg16(mReg.IndX);
			break;
			
		case ADDRMODE_ABY:
			arg16 = GetArg16(mReg.IndY);
			break;
			
		case ADDRMODE_ZPX:
			arg16 = (mpMem->Peek8bit(mReg.PtrAddr++) + mReg.IndX) & 0xFF;
			break;
			
		case ADDRMODE_ZPY:
			arg16 = (mpMem->Peek8bit(mReg.PtrAddr++) + mReg.IndY) & 0xFF;
			break;
			
		case ADDRMODE_IZX:
			arg16 = (mpMem->Peek8bit(mReg.PtrAddr++) + mReg.IndX) & 0xFF;
			arg16 = mpMem->Peek16bit(arg16);			
			break;
			
		case ADDRMODE_IZY:
			arg16 = mpMem->Peek8bit(mReg.PtrAddr++);
			arg16 = mpMem->Peek16bit(arg16) + mReg.IndY;			
			break;
			
		case ADDRMODE_REL:
			arg16 = ComputeRelJump(mpMem->Peek8bit(mReg.PtrAddr++));
			break;
			
		case ADDRMODE_ACC:
			// acc mode is an internal CPU register
			break;
			
		default:
			throw MKGenException("ERROR: Wrong addressing mode!");
			break;
	}
	
	return arg16;
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
	unsigned char opcode = mpMem->Peek8bit(mReg.PtrAddr++);
	unsigned char arg8 = 0;
	unsigned short arg16 = 0;
	
	SetFlag(false, FLAGS_BRK);	// reset BRK flag - we want to detect
	mReg.SoftIrq = false;				// software interrupt each time it happens
															// (non-maskable)
	mReg.LastRTS = false;
				
	switch (opcode) {
		
		case OPCODE_BRK:				// software interrupt, Implied ($00 : BRK)
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
			break;
		
		case OPCODE_NOP:				// NO oPeration, Implied ($EA : NOP)
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
			mReg.Acc = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
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
			mReg.IndX = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
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
			mReg.IndY = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
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
			mReg.IndX = mReg.Acc;
			SetFlags(mReg.IndX);
			break;
			
		case OPCODE_TAY:				// Transfer A to Y, Implied ($A8 : TAY)			
			mReg.IndY = mReg.Acc;
			SetFlags(mReg.IndY);
			break;

		case OPCODE_TXA:				// Transfer X to A, Implied ($8A : TXA)
			mReg.Acc = mReg.IndX;
			SetFlags(mReg.Acc);
			break;

		case OPCODE_TYA:				// Transfer Y to A, Implied ($98 : TYA)
			mReg.Acc = mReg.IndY;
			SetFlags(mReg.Acc);
			break;
			
		case OPCODE_TSX:				// Transfer Stack ptr to X, Implied ($BA : TSX)
			mReg.IndX = mReg.PtrStack;
			SetFlags(mReg.IndX);
			break;
			
		case OPCODE_TXS:				// Transfer X to Stack ptr, Implied ($9A : TXS)
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
			mReg.IndX++;
			SetFlags(mReg.IndX);
			break;
			
		case OPCODE_DEX:				// DEcrement X, Implied ($CA : DEX)
			mReg.IndX--;
			SetFlags(mReg.IndX);
			break;

		case OPCODE_INY:				// INcrement Y, Implied ($C8 : INY)
			mReg.IndY++;
			SetFlags(mReg.IndY);
			break;
			
		case OPCODE_DEY:				// DEcrement Y, Implied ($88 : DEY)
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
			LogicOpAcc(GetAddrWithMode(ADDRMODE_IMM), LOGOP_OR);
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
			LogicOpAcc(GetAddrWithMode(ADDRMODE_IMM), LOGOP_AND);
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
			arg8 &= mReg.Acc;
			SetFlags(arg8);
			mReg.Flags |= (arg8 & FLAGS_OVERFLOW);
			break;
			
		case OPCODE_BIT_ABS:	// test BITs, Absolute ($2C addrlo addrhi : BIT addr ;addr=0..$FFFF), MEM=addr
			arg16 = GetAddrWithMode(ADDRMODE_ABS);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 &= mReg.Acc;
			SetFlags(arg8);
			mReg.Flags |= (arg8 & FLAGS_OVERFLOW);			
			break;
			
		case OPCODE_ROL_ZP:		// ROtate Left, Zero Page ($26 arg : ROL arg ;arg=0..$FF), MEM=arg
			arg16 = GetAddrWithMode(ADDRMODE_ZP);
			arg8 = mpMem->Peek8bit(arg16);
			arg8 = RotateLeft(arg8);
			mpMem->Poke8bit(arg16, arg8);
			break;
			
		case OPCODE_ROL:			// ROtate Left, Accumulator ($2A : ROL)
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
			arg16 = 0x100;
			arg16 += mReg.PtrStack--;
			arg8 = mReg.Flags | FLAGS_BRK;
			mpMem->Poke8bit(arg16, arg8);
			break;
			
		case OPCODE_PHA:			// PusH Accumulator, Implied ($48 : PHA)
			arg16 = 0x100;
			arg16 += mReg.PtrStack--;
			mpMem->Poke8bit(arg16, mReg.Acc);		
			break;
						
		case OPCODE_PLP:			// PuLl Processor status, Implied ($28 : PLP)
			arg16 = 0x100;
			arg16 += ++mReg.PtrStack;
			mReg.Flags = mpMem->Peek8bit(arg16);
			break;
			
		case OPCODE_PLA:			// PuLl Accumulator, Implied ($68 : PLA)
			arg16 = 0x100;
			arg16 += ++mReg.PtrStack;
			mReg.Acc = mpMem->Peek8bit(arg16);
			SetFlags(mReg.Acc);
			break;
			
		case OPCODE_CLC:			// CLear Carry, Implied ($18 : CLC)
			SetFlag(false, FLAGS_CARRY);
			break;
			
		case OPCODE_SEC:			// SEt Carry, Implied ($38 : SEC)
			SetFlag(true, FLAGS_CARRY);
			break;
			
		case OPCODE_CLI:			// CLear Interrupt, Implied ($58 : CLI)
			SetFlag(false, FLAGS_IRQ);
			break;
			
		case OPCODE_CLV:			// CLear oVerflow, Implied ($B8 : CLV)
			SetFlag(false, FLAGS_OVERFLOW);
			break;
			
		case OPCODE_CLD:			// CLear Decimal, Implied ($D8 : CLD)
			SetFlag(false, FLAGS_DEC);
			break;
			
		case OPCODE_SED:			// SEt Decimal, Implied ($F8 : SED)
			SetFlag(true, FLAGS_DEC);
			break;
			
		case OPCODE_SEI:			// SEt Interrupt, Implied ($78 : SEI)
			SetFlag(true, FLAGS_IRQ);
			break;
			
		/* 
		 * RTI retrieves the Processor Status Word (flags) and the Program Counter from the stack in that order
		 * (interrupts push the PC first and then the PSW). 
		 * Note that unlike RTS, the return address on the stack is the actual address rather than the address-1. 
		*/
		case OPCODE_RTI:			// ReTurn from Interrupt, Implied ($40 : RTI)
			arg16 = 0x100;
			arg16 += ++mReg.PtrStack;
			mReg.Flags = mpMem->Peek8bit(arg16);
			arg16++; mReg.PtrStack++;
			mReg.PtrAddr = mpMem->Peek8bit(arg16);
			arg16++; mReg.PtrStack++;
			mReg.PtrAddr += (mpMem->Peek8bit(arg16) * 0x100);
			mReg.SoftIrq = CheckFlag(FLAGS_BRK);
			break;
			
		case OPCODE_RTS:			// ReTurn from Subroutine, Implied ($60 : RTS)
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
			LogicOpAcc(GetAddrWithMode(ADDRMODE_IMM), LOGOP_EOR);
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
			AddWithCarry(mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM)));
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
			arg8 = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
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
			arg8 = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
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
			arg8 = mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM));
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
			SubWithCarry(mpMem->Peek8bit(GetAddrWithMode(ADDRMODE_IMM)));
			break;
			
		default:
			break;
	}
	
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

} // namespace MKBasic

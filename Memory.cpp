#include "Memory.h"
#include <stdio.h>
#include <conio.h>

//#define DBG 1
#if defined (DBG)
#include <iostream>
using namespace std;
#endif

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */

namespace MKBasic {

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
Memory::Memory()
{
	Initialize();
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
Memory::~Memory()
{
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void Memory::Initialize()
{
	unsigned short addr = 0;
	for (int i=0; i < 0xFFFF; i++) {
		m8bitMem[addr++] = 0;
	}
	mCharIOAddr = CHARIO_ADDR;
	mCharIOActive = false;
	mIOEcho = false;
	mInBufDataBegin = mInBufDataEnd = 0;	
	mOutBufDataBegin = mOutBufDataEnd = 0;
	mROMBegin = ROM_BEGIN;
	mROMEnd = ROM_END;
	mROMActive = false;
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void Memory::EnableROM()
{
	mROMActive = true;
}
		
/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */		
void Memory::EnableROM(unsigned short start, unsigned short end)
{
	if (mROMEnd > mROMBegin) {
		mROMBegin = start;
		mROMEnd = end;
	}
	EnableROM();
}

/*
 *--------------------------------------------------------------------
 * Method:		ReadCharKb()
 * Purpose:		If char I/O active, read character from console
 *            (non-blocking) and put in an input  FIFO buffer.
 * Arguments: n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
unsigned char Memory::ReadCharKb()
{
	unsigned char ret = 0;
	if (mCharIOActive) {
		int c;
		putchar('?');
		while(!kbhit());
		c = getch();
		if (mIOEcho) putchar(c);
		mCharIOBufIn[mInBufDataEnd] = c;
		mInBufDataEnd++;
		if (mInBufDataEnd >= CHARIO_BUF_SIZE) mInBufDataEnd = 0;
		ret = c;
	}	
	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetCharIn()
 * Purpose:		Return character from the emulated character I/O FIFO
 *            input buffer or -1 if FIFO empty or char I/O not active.
 * Arguments: n/a
 * Returns:		character
 *--------------------------------------------------------------------
 */
char Memory::GetCharIn()
{
	char ret = -1;
	if (mCharIOActive) {
		if (mInBufDataEnd != mInBufDataBegin) {
			ret = mCharIOBufIn[mInBufDataBegin];	
			mInBufDataBegin++;
			if (mInBufDataBegin >= CHARIO_BUF_SIZE) mInBufDataBegin = 0;
		}
	}
	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetCharOut()
 * Purpose:		Return character from the emulated character I/O FIFO
 *            output buffer or -1 if FIFO empty or char I/O not 
 *            active.
 * Arguments: n/a
 * Returns:		character
 *--------------------------------------------------------------------
 */
char Memory::GetCharOut()
{
	char ret = -1;
	if (mCharIOActive) {
		if (mOutBufDataEnd != mOutBufDataBegin) {
			ret = mCharIOBufOut[mOutBufDataBegin];	
			mOutBufDataBegin++;
			if (mOutBufDataBegin >= CHARIO_BUF_SIZE) mOutBufDataBegin = 0;
		}
	}
	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		PutCharIO()
 * Purpose:		Put character in the output char I/O FIFO buffer.
 * Arguments: c - character
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void Memory::PutCharIO(char c)
{
	if (mCharIOActive) {
		mCharIOBufOut[mOutBufDataEnd] = c;
		mOutBufDataEnd++;
		if (mOutBufDataEnd >= CHARIO_BUF_SIZE) mOutBufDataEnd = 0;
	}
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
unsigned char Memory::Peek8bit(unsigned short addr)
{
	if (mCharIOActive && addr == mCharIOAddr) {
#if defined (DBG)		
		cout << "DBG: Peek8bit($" << hex << addr << ") BEFORE ReadCharKb()" << endl;
		cout << "DBG: m8bitMem[$" << hex << addr << "] = $" << hex << (unsigned short)m8bitMem[addr] << endl;
		for (unsigned int a  = 0xFFF0; a < 0x10000; a++) {
			cout << "DBG: m8bitMem[$" << hex << a << "] = $" << hex << (unsigned short)m8bitMem[a] << endl;
		}
#endif						
		m8bitMem[addr] = ReadCharKb();
#if defined (DBG)		
    cout << "************************" << endl;
		cout << "DBG: Peek8bit($" << hex << addr << ") AFTER ReadCharKb()" << endl;
		cout << "DBG: m8bitMem[$" << hex << addr << "] = $" << hex << (unsigned short)m8bitMem[addr] << endl;
		for (unsigned int a  = 0xFFF0; a < 0x10000; a++) {
			cout << "DBG: m8bitMem[$" << hex << a << "] = $" << hex << (unsigned short)m8bitMem[a] << endl;
		}
#endif				
	}
		
	return m8bitMem[addr];
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
unsigned short Memory::Peek16bit(unsigned short addr)
{
	unsigned short ret = 0;

	if (mCharIOActive && addr == mCharIOAddr) {
#if defined (DBG)		
		cout << "DBG: Peek16bit(" << addr << ")" << endl;
#endif		
		m8bitMem[addr] = ReadCharKb();
	}
			
	ret = m8bitMem[addr++];
	ret += m8bitMem[addr] * 256;

	return ret;
}
	
/*
 *--------------------------------------------------------------------
 * Method:		Poke8bit()
 * Purpose:		Write byte to specified memory location.
 *            If the memory location is mapped to character I/O,
 *            write value to output buffer and memory location.
 *            If the memory location is protected (ROM), do not
 *            write the value.
 * Arguments: addr - (0x0000..0xffff) memory address,
 *            val - value (0x00..0xff)
 * Returns:   n/a
 *--------------------------------------------------------------------
 */	
void Memory::Poke8bit(unsigned short addr, unsigned char val)
{
	if (mCharIOActive && addr == mCharIOAddr)
		PutCharIO(val);
	if (!mROMActive || (addr < ROM_BEGIN || addr > ROM_END)) {
		m8bitMem[addr] = val;
	}
}

/*
 *--------------------------------------------------------------------
 * Method:		SetCharIO()
 * Purpose:		Activates and sets an address of basic character I/O
 *            emulation (console).
 * Arguments:	addr - address of I/O area (0x0000..0xFFFF)
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void Memory::SetCharIO(unsigned short addr, bool echo)
{
	mCharIOAddr = addr;
	mCharIOActive = true;
	mIOEcho = echo;
}

/*
 *--------------------------------------------------------------------
 * Method:		DisableCharIO()
 * Purpose:		Deactivates basic character I/O emulation (console).
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void Memory::DisableCharIO()
{
	mCharIOActive = false;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetCharIOAddr()
 * Purpose:		Returns current address of basic character I/O area.
 * Arguments:	n/a
 * Returns:		address of I/O area
 *--------------------------------------------------------------------
 */
unsigned short Memory::GetCharIOAddr()
{
	return mCharIOAddr;
}

} // namespace MKBasic

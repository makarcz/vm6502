
#include "Memory.h"

#include <stdio.h>
#include <ctype.h>
#if defined(WINDOWS)
#include <conio.h>
#endif

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

#if defined(LINUX)
#include <stdlib.h>
#include <string.h>
#include <signal.h>

struct termios orig_termios;

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void Memory::set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
int Memory::kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
int Memory::getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}
#endif

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
void Memory::DisableROM()
{
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
void Memory::SetROM(unsigned short start, unsigned short end)
{
	if (mROMEnd > mROMBegin) {
		mROMBegin = start;
		mROMEnd = end;
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
void Memory::EnableROM(unsigned short start, unsigned short end)
{
	SetROM(start,end);
	EnableROM();
}

/*
 *--------------------------------------------------------------------
 * Method:		ReadCharKb()
 * Purpose:		If char I/O active, read character from console
 *            (non-blocking) and put in an input  FIFO buffer.
 * Arguments: nonblock - if true, works in non-blocking mode
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
unsigned char Memory::ReadCharKb(bool nonblock)
{
	unsigned char ret = 0;
	if (mCharIOActive) {
#if defined(LINUX)
    set_conio_terminal_mode();
#endif
		static int c = ' ';
		putchar('\n');
		if (mIOEcho && isprint(c)) putchar(c);
		else putchar(' ');
		fputs("<-Character Input (CTRL-Y to BREAK) ?\r",stdout);
		fflush(stdout);
		if (!nonblock) while(!kbhit());
		else c = 0;
		c = getch();
#if defined(LINUX)
		if (c == 3) { // capture CTRL-C in CONIO mode
			reset_terminal_mode();
			kill(getpid(),SIGINT);
		}
#endif
    fputs("                                      \r",stdout);
		fflush(stdout);
		mCharIOBufIn[mInBufDataEnd] = c;
		mInBufDataEnd++;
		if (mInBufDataEnd >= CHARIO_BUF_SIZE) mInBufDataEnd = 0;
		ret = c;
#if defined(LINUX)
    reset_terminal_mode();
#endif
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
		m8bitMem[addr] = ReadCharKb(false); // blocking mode input
	} else if (mCharIOActive && addr == mCharIOAddr+1) {
		m8bitMem[addr] = ReadCharKb(true);	// non-blocking mode input
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
		m8bitMem[addr] = ReadCharKb(false);	// blocking mode input
	} else if (mCharIOActive && addr == mCharIOAddr+1) {
		m8bitMem[addr] = ReadCharKb(true);	// non-blocking mode input
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
	if (!mROMActive || (addr < mROMBegin || addr > mROMEnd)) {
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

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
unsigned short Memory::GetROMBegin()
{
	return mROMBegin;
}
		
/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */		
unsigned short Memory::GetROMEnd()
{
	return mROMEnd;
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */		
bool Memory::IsROMEnabled()
{
	return mROMActive;
}

} // namespace MKBasic

/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:   			MemMapDev.h
 *
 * Purpose: 		Prototype of MemMapDev class and all supporting
 *							data structures, enumarators, constants and macros.
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
   Please proceed responsibly and apply common sense.
   This software is provided in hope that it will be useful.
   It is free of charge for non-commercial and educational use.
   Distribution of this software in non-commercial and educational
   derivative work is permitted under condition that original
   copyright notices and comments are preserved. Some 3-rd party work
   included with this project may require separate application for
   permission from their respective authors/copyright owners.

 *--------------------------------------------------------------------
 */
#ifndef MEMMAPDEV_H
#define MEMMAPDEV_H

#include <vector>
#include <string>
#include "system.h"
//#include "Memory.h"
#include "GraphDisp.h"
#include "Display.h"
#include "ConsoleIO.h"

#if defined(LINUX)
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#endif

// some default definitions
#define CHARIO_ADDR			0xE000
#define GRDISP_ADDR			0xE002
#define CHARIO_BUF_SIZE	256
#define CHARTBL_BANK		0x0B		// $B000
#define CHARTBL_LEN			0x1000	// 4 kB
#define TXTCRSR_MAXCOL	79
#define TXTCRSR_MAXROW	24

using namespace std;

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */

namespace MKBasic {

class Memory;	

// Definitions for memory mapped devices handling.

// memory address ranges
struct AddrRange {
	unsigned short start_addr, end_addr; // inclusive address range

	AddrRange(unsigned short staddr, unsigned short endaddr)
	{
		start_addr = staddr;
		end_addr = endaddr;
	}
};

// device parameters
struct DevPar {
	string name, value;

	DevPar(string n, string v)
	{
		name = n;
		value = v;
	}
};

typedef vector<AddrRange> MemAddrRanges;
typedef vector<DevPar>		DevParams;

class MemMapDev;

// Class MemMapDev implements functionality of all
// memory mapped devices.

// device register read, arguments: address
typedef int (MemMapDev::*ReadFunPtr)(int);
// device register write,	arguments: address, value
typedef void (MemMapDev::*WriteFunPtr)(int,int);

// Definition of device.
struct Device {
	int             num;						// device number
	string          name;						// device name
	MemAddrRanges 	addr_ranges;		// vector of memory address ranges for this device
	ReadFunPtr			read_fun_ptr;		// pointer to memory register read function
	WriteFunPtr			write_fun_ptr;	// pointer to memory register write function
	DevParams				params;					// list of device parameters

	Device()
	{
		num = -1;
	}

	Device(int dnum, 
				 string dname, 
				 MemAddrRanges addrranges, 
				 ReadFunPtr prdfun, 
				 WriteFunPtr pwrfun, 
				 DevParams parms) 
	{
		num = dnum;
		name = dname;
		addr_ranges = addrranges;
		read_fun_ptr = prdfun;
		write_fun_ptr = pwrfun;
		params = parms;
	}
};

typedef vector<Device> MemMappedDevices;

// currently supported devices
enum DevNums {
	DEVNUM_CHARIO = 0,	// character I/O device
	DEVNUM_GRDISP = 1		// raster graphics display device
};

/*
 * NOTE regarding device address ranges.
 * 
 * The emulated device is a model of a electronic device that is connected
 * to the CPU-s bus (address, memory, control signals).
 * Depending on the device, the memory address range may be as simple as
 * a single memory address or it may contain multiple memory addresses
 * or ranges of addresses positioned at various non-contiguous places.
 * The functions of these addresses are handled internally by the MemMapDev
 * class. The client (user) initializing the device, if providing custom
 * memory ranges must know more details about device in order to provide
 * accurate data. E.g.: if device requires a single address as an entry
 * point, just one memory range is needed with start_addr == end_addr.
 * If device is more complicated and requires range of addresses to access
 * its control registers, another range of addresses to map memory access
 * etc., device should be setup accordingly by calling SetupDevice method
 * with proper arguments. Device setup is not mandatory, each device that
 * is mainained is initialized wit default parameters.
 */

// offsets of raster graphics device registers
enum GraphDevRegs {
	GRAPHDEVREG_X 			= 0,
	GRAPHDEVREG_Y 			= 2,
	GRAPHDEVREG_PXCOL_R = 3,
	GRAPHDEVREG_PXCOL_G = 4,
	GRAPHDEVREG_PXCOL_B = 5,
	GRAPHDEVREG_BGCOL_R = 6,
	GRAPHDEVREG_BGCOL_G = 7,
	GRAPHDEVREG_BGCOL_B = 8,
	GRAPHDEVREG_CMD 		= 9,
	GRAPHDEVREG_X2			= 10,
	GRAPHDEVREG_Y2			= 12,
	GRAPHDEVREG_CHRTBL	= 13,	// set the 2 kB bank where char. table resides
	GRAPHDEVREG_TXTCURX	=	14,	// set text cursor position (column)
	GRAPHDEVREG_TXTCURY	= 15,	// set text cursor position (row)
	GRAPHDEVREG_PUTC		= 16,	// output char. to current pos. and move cursor
	GRAPHDEVREG_CRSMODE	= 17,	// set cursor mode (0 - not visible, 1 - block...)
	GRAPHDEVREG_TXTMODE = 18,	// set text mode (0 - normal, 1 - reverse)
	//---------------------------
	GRAPHDEVREG_END
};
/*
 * Note to GRAPHDEVREG_PUTC:
 * value put to register is not an ASCII code of the character, but rather
 * a screen code in order how character is positioned in character table.
 * Each character definition takes 8 bytes. There is 4096 bytes which defines
 * 512 characters. First 256 are normal color and next 256 are reverse color
 * definitions.
 */


// graphics display commands
enum GraphDevCmds {
	GRAPHDEVCMD_CLRSCR = 0,
	GRAPHDEVCMD_SETPXL = 1,
	GRAPHDEVCMD_CLRPXL = 2,
	GRAPHDEVCMD_SETBGC = 3,
	GRAPHDEVCMD_SETFGC = 4,
	GRAPHDEVCMD_DRAWLN = 5,
	GRAPHDEVCMD_ERASLN = 6
};

// Cursor modes.
// note: bit 7 will decide if cursor will blink (1)
#define CRS_BLINK	0x80
enum TextCursorModes {
	GRAPHDEVCRSMODE_BLANK = 0,	// not visible
	GRAPHDEVCRSMODE_BLOCK = 1,	// block
	GRAPHDEVCRSMODE_UND		= 2,	// underscore
	//------------------------------------------
	GRAPHDEVCRSMODE_END
};

// Text modes.
enum TextModes {
	GRAPHDEVTXTMODE_NORMAL	= 0,	// normal mode
	GRAPHDEVTXTMODE_REVERSE	=	1,	// reverse colors mode
	//------------------------------------------
	GRAPHDEVTXTMODE_END
};

struct GraphDeviceRegs {
	unsigned char mGraphDispLoX;
	unsigned char mGraphDispHiX;
	unsigned char mGraphDispY;
	unsigned char mGraphDispLoX2;
	unsigned char mGraphDispHiX2;
	unsigned char mGraphDispY2;	
	unsigned char mGraphDispPixColR;
	unsigned char mGraphDispPixColG;
	unsigned char mGraphDispPixColB;
	unsigned char mGraphDispBgColR;
	unsigned char mGraphDispBgColG;
	unsigned char mGraphDispBgColB;	
	unsigned char mGraphDispChrTbl;		// 2 kB char. table bank (0-31)
	unsigned char mGraphDispTxtCurX;	// text cursor column
	unsigned char mGraphDispTxtCurY;	// text cursor row
	unsigned char mGraphDispCrsMode;	// cursor mode
	unsigned char mGraphDispTxtMode;	// text mode
};

// Functionality of memory mapped devices
class MemMapDev {

	public:

		MemMapDev();
		MemMapDev(Memory *pmem);
		~MemMapDev();

		Device GetDevice(int devnum);
		int SetupDevice(int devnum, MemAddrRanges memranges, DevParams params);

		char GetCharIn();
		char GetCharOut();
		void CharIOFlush();
		unsigned short GetCharIOAddr();
		bool GetCharIOEchoOn();
		bool IsCharIOActive();
		Display* ActivateCharIO();
		Display* GetDispPtr();
		void DeactivateCharIO();

		int CharIODevice_Read(int addr);
		void CharIODevice_Write(int addr, int val);

		unsigned short GetGraphDispAddrBase();
		void ActivateGraphDisp();
		void DeactivateGraphDisp();

		int GraphDispDevice_Read(int addr);
		void GraphDispDevice_Write(int addr, int val);

		void GraphDisp_ReadEvents();
		void GraphDisp_Update();

		//void SetCharIODispPtr(Display *p, bool active);

	private:

		Memory *mpMem;	// pointer to memory object
		MemMappedDevices mDevices;
		char mCharIOBufIn[CHARIO_BUF_SIZE];
		char mCharIOBufOut[CHARIO_BUF_SIZE];
		unsigned int mInBufDataBegin;
		unsigned int mInBufDataEnd;
		unsigned int mOutBufDataBegin;
		unsigned int mOutBufDataEnd;
		unsigned int mCharIOAddr;
		bool mIOEcho;		
		unsigned int mGraphDispAddr;
		GraphDisp *mpGraphDisp;			// pointer to Graphics Device object
		Display   *mpCharIODisp;		// pointer to character I/O device object
		bool			mCharIOActive;		// indicate if character I/O is active
		GraphDeviceRegs mGrDevRegs;	// graphics display device registers
		unsigned int mCharTblAddr;	// start address of characters table
		ConsoleIO *mpConsoleIO;

		void Initialize();

		unsigned char ReadCharKb(bool nonblock);
		void PutCharIO(char c);		
		//void SetCurses();

};

}	// namespace MKBasic

#endif // MEMMAPDEV_H

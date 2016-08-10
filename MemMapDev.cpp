
#include "MemMapDev.h"
#include "Memory.h"
#include "MKGenException.h"

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
 * Method:		MemMapDev()
 * Purpose:		Class constructor.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
MemMapDev::MemMapDev()
{
	mpMem = NULL;
	Initialize();
}

/*
 *--------------------------------------------------------------------
 * Method:		MemMapDev()
 * Purpose:		Custom class constructor.
 * Arguments:	pmem - Pointer to Memory, pointer to Memory object.
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
MemMapDev::MemMapDev(Memory *pmem)
{
	mpMem = pmem;
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
MemMapDev::~MemMapDev()
{

}

/*
 *--------------------------------------------------------------------
 * Method:		Initialize()
 * Purpose:		Initialize class and all supported devices.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void MemMapDev::Initialize()
{
	mInBufDataBegin = mInBufDataEnd = 0;	
	mOutBufDataBegin = mOutBufDataEnd = 0;
	mIOEcho = false;
	mCharIOAddr = CHARIO_ADDR;
	mGraphDispAddr = GRDISP_ADDR;
	mpGraphDisp = NULL;
	AddrRange addr_range(CHARIO_ADDR, CHARIO_ADDR+1);
	DevPar dev_par("echo", "false");
	MemAddrRanges addr_ranges_chario;
	DevParams dev_params_chario;
	addr_ranges_chario.push_back(addr_range);
	dev_params_chario.push_back(dev_par);
	Device dev_chario(DEVNUM_CHARIO, 
						 				"Character I/O", 
									  addr_ranges_chario, 
									  &MemMapDev::CharIODevice_Read,
									  &MemMapDev::CharIODevice_Write,
									  dev_params_chario);
	mDevices.push_back(dev_chario);

	addr_range.start_addr = GRDISP_ADDR;
	addr_range.end_addr = GRDISP_ADDR + GRAPHDEVREG_END - 1;
	dev_par.name = "nil";
	dev_par.value = "nil";
	MemAddrRanges addr_ranges_grdisp;
	DevParams dev_params_grdisp;
	addr_ranges_grdisp.push_back(addr_range);
	dev_params_grdisp.push_back(dev_par);	
	Device dev_grdisp(DEVNUM_GRDISP, 
									  "Graphics Display", 
									  addr_ranges_grdisp, 
									  &MemMapDev::GraphDispDevice_Read,
									  &MemMapDev::GraphDispDevice_Write,
									  dev_params_grdisp);
	mDevices.push_back(dev_grdisp);	
}

/*
 *--------------------------------------------------------------------
 * Method:		GetDevice()
 * Purpose:		Get device by specified number.
 * Arguments:	devnum - device number
 * Returns:		Device structure value.
 *--------------------------------------------------------------------
 */
Device MemMapDev::GetDevice(int devnum)
{
	Device ret;
	for (MemMappedDevices::iterator devit = mDevices.begin();
		   devit != mDevices.end();
		   ++devit
		  ) {

		Device dev = *devit;	
		if (dev.num == devnum) {
			ret = dev;
			break;
		}
	}
	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		SetupDevice()
 * Purpose:		Configure device memory ranges and parameters.
 * Arguments:	devnum - device number
 *            memranges - memory address ranges vector
 *            params - parameters vector
 * Returns:		integer, 0 if OK, error number otherwise
 *--------------------------------------------------------------------
 */
int MemMapDev::SetupDevice(int devnum, 
													 MemAddrRanges memranges, 
													 DevParams params)
{
#if defined(DBG)			
	cout << "DBG: MemMapDev::SetupDevice()" << endl;
#endif				
	int ret = 0;
	Device dev = GetDevice(devnum);
	if (dev.num >= 0) {
		dev.addr_ranges = memranges;
		dev.params = params;
		MemMappedDevices devices_new;
		for (MemMappedDevices::iterator it = mDevices.begin();
				 it != mDevices.end();
				 ++it
				) {
			if ((*it).num != devnum) devices_new.push_back(*it);
		}
		devices_new.push_back(dev);
		mDevices = devices_new;
		// device specific post-processing
		if (DEVNUM_CHARIO == devnum) {
			MemAddrRanges::iterator it = memranges.begin();
			mCharIOAddr = (*it).start_addr;
			for (DevParams::iterator it = params.begin();
					 it != params.end();
					 ++it
					) {
				string par_name = (*it).name;
				string par_val = (*it).value;
				if (0 == par_name.compare("echo")) {
					if (0 == par_val.compare("true")) mIOEcho = true;
					else mIOEcho = false;
				}
			}
#if defined(DBG)			
			cout << "DBG: MemMapDev::SetupDevice() mCharIOAddr = $" << hex << mCharIOAddr << endl;
			cout << "DBG: MemMapDev::SetupDevice() mIOEcho = " << (mIOEcho?"true":"false") << endl;
#endif			
		} else if (DEVNUM_GRDISP == devnum) {
			MemAddrRanges::iterator it = memranges.begin();
			mGraphDispAddr = (*it).start_addr;
		}
		// finished device specific post-processing
	} else {
		ret++;	// error
#if defined(DBG)
		cout << "DBG: MemMapDev::SetupDevice() ERROR!" << endl;
#endif		
	}

#if defined(DBG)
	while (!getchar());
	cout << "Press [ENTER]...";
	getchar();
#endif	

	return ret;
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
void MemMapDev::set_conio_terminal_mode()
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
int MemMapDev::kbhit()
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
int MemMapDev::getch()
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
 * Method:		ReadCharKb()
 * Purpose:		If char I/O active, read character from console
 *            (non-blocking) and put in an input  FIFO buffer.
 * Arguments: nonblock - if true, works in non-blocking mode
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
unsigned char MemMapDev::ReadCharKb(bool nonblock)
{
	unsigned char ret = 0;
#if defined(LINUX)
    set_conio_terminal_mode();
#endif
		static int c = ' ';
		if (mIOEcho && isprint(c)) putchar(c);
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
		mCharIOBufIn[mInBufDataEnd] = c;
		mInBufDataEnd++;
		if (mInBufDataEnd >= CHARIO_BUF_SIZE) mInBufDataEnd = 0;
		ret = c;
#if defined(LINUX)
    reset_terminal_mode();
#endif
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
char MemMapDev::GetCharIn()
{
	char ret = -1;
	if (mInBufDataEnd != mInBufDataBegin) {
		ret = mCharIOBufIn[mInBufDataBegin];	
		mInBufDataBegin++;
		if (mInBufDataBegin >= CHARIO_BUF_SIZE) mInBufDataBegin = 0;
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
char MemMapDev::GetCharOut()
{
	char ret = -1;
	if (mOutBufDataEnd != mOutBufDataBegin) {
		ret = mCharIOBufOut[mOutBufDataBegin];	
		mOutBufDataBegin++;
		if (mOutBufDataBegin >= CHARIO_BUF_SIZE) mOutBufDataBegin = 0;
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
void MemMapDev::PutCharIO(char c)
{
	mCharIOBufOut[mOutBufDataEnd] = c;
	mOutBufDataEnd++;
	if (mOutBufDataEnd >= CHARIO_BUF_SIZE) mOutBufDataEnd = 0;
}

/*
 *--------------------------------------------------------------------
 * Method:		CharIODevice_Write()
 * Purpose:		Write value to Char I/O device register.
 * Arguments: addr - address, val - value (character)
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void MemMapDev::CharIODevice_Write(int addr, int val)
{
	if ((unsigned int)addr == mCharIOAddr) {
		PutCharIO ((char) val);
	}
}

/*
 *--------------------------------------------------------------------
 * Method:		CharIODevice_Read()
 * Purpose:		Read value from Char I/O device register.
 * Arguments: addr - address
 * Returns:   value
 *--------------------------------------------------------------------
 */
int MemMapDev::CharIODevice_Read(int addr)
{
	int ret = 0;
	if ((unsigned int)addr == mCharIOAddr) {
		ret = ReadCharKb(false);	// blocking mode input
	} else if ((unsigned int)addr == mCharIOAddr+1) {
		ret = ReadCharKb(true);		// non-blocking mode input
	}
	mpMem->Poke8bitImg((unsigned short)addr, (unsigned char)ret);
	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetCharIOAddr()
 * Purpose:		Returns current address of basic character I/O area.
 * Arguments:	n/a
 * Returns:		address of I/O area
 *--------------------------------------------------------------------
 */
unsigned short MemMapDev::GetCharIOAddr()
{
	return mCharIOAddr;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetCharIOEchoOn()
 * Purpose:		Returns current status of char I/O local echo flag.
 * Arguments:	n/a
 * Returns:		true if local echo is enabled, false if disabled
 *--------------------------------------------------------------------
 */
bool MemMapDev::GetCharIOEchoOn()
{
	return mIOEcho;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetGraphDispAddrBase()
 * Purpose:		Return base address of graphics display device.
 * Arguments:	n/a
 * Returns:		unsigned short - address ($0000 - $FFFF)
 *--------------------------------------------------------------------
 */
unsigned short MemMapDev::GetGraphDispAddrBase()
{
	return mGraphDispAddr;
}

/*
 *--------------------------------------------------------------------
 * Method:		GraphDispDevice_Read()
 * Purpose:		Read from specified graphics display device register.
 * Arguments:	int - address of the register in memory.
 * Returns:		int - read value.
 *--------------------------------------------------------------------
 */
int MemMapDev::GraphDispDevice_Read(int addr)
{
	// this device has no read registers, return 0
	GraphDisp_ReadEvents();
	GraphDisp_Update();
	return 0;
}

/*
 *--------------------------------------------------------------------
 * Method:		GraphDispDevice_Write()
 * Purpose:		Write a value to specified graphics display device
 *						register.
 * Arguments:	addr - address of the register in memory
 *						val - value to write
 * Returns:		n/a
 *--------------------------------------------------------------------
 */	
void MemMapDev::GraphDispDevice_Write(int addr, int val)
{
	if (NULL != mpGraphDisp) {	// only if device is active
		if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_X) {
			// setup X coordinate of the pixel or beginning of line,
			// less significant part
			mGrDevRegs.mGraphDispLoX = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_X + 1) {
			// setup X coordinate of the pixel, more significant part
			mGrDevRegs.mGraphDispHiX = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_Y) {
			// setup Y coordinate of the pixel
			mGrDevRegs.mGraphDispY = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_PXCOL_R) {
			// setup pixel RGB color Red component
			mGrDevRegs.mGraphDispPixColR = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_PXCOL_G) {
			// setup pixel RGB color Green component
			mGrDevRegs.mGraphDispPixColG = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_PXCOL_B) {
			// setup pixel RGB color Blue component
			mGrDevRegs.mGraphDispPixColB = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_BGCOL_R) {
			// setup background RGB color Red component
			mGrDevRegs.mGraphDispBgColR = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_BGCOL_G) {
			// setup background RGB color Green component
			mGrDevRegs.mGraphDispBgColG = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_BGCOL_B) {
			// setup background RGB color Blue component
			mGrDevRegs.mGraphDispBgColB = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_X2) {
			// setup X coordinate of the end of line, less significant part
			mGrDevRegs.mGraphDispLoX2 = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_X2 + 1) {
			// setup X coordinate of the end of line, more significant part
			mGrDevRegs.mGraphDispHiX2 = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_Y2) {
			// setup Y coordinate of the end of line
			mGrDevRegs.mGraphDispY2 = (unsigned char)val;
		} else if ((unsigned int)addr == mGraphDispAddr + GRAPHDEVREG_CMD) {
			// execute command
			switch (val) {
				
				case GRAPHDEVCMD_CLRSCR:
					mpGraphDisp->ClearScreen();
					break;

				case GRAPHDEVCMD_SETPXL:
					mpGraphDisp->SetPixel(mGrDevRegs.mGraphDispLoX 
																	+ 256 * mGrDevRegs.mGraphDispHiX,
																mGrDevRegs.mGraphDispY);
					break;

				case GRAPHDEVCMD_CLRPXL:
					mpGraphDisp->ErasePixel(mGrDevRegs.mGraphDispLoX
																		+ 256 * mGrDevRegs.mGraphDispHiX,
																	mGrDevRegs.mGraphDispY);
					break;

				case GRAPHDEVCMD_SETBGC:
					mpGraphDisp->SetBgColor(mGrDevRegs.mGraphDispBgColR,
																	mGrDevRegs.mGraphDispBgColG,
																	mGrDevRegs.mGraphDispBgColB);				
					break;

				case GRAPHDEVCMD_SETFGC:
					mpGraphDisp->SetFgColor(mGrDevRegs.mGraphDispPixColR,
																	mGrDevRegs.mGraphDispPixColG,
																	mGrDevRegs.mGraphDispPixColB);				
					break;					

				case GRAPHDEVCMD_DRAWLN:
					mpGraphDisp->DrawLine(mGrDevRegs.mGraphDispLoX 
																		+ 256 * mGrDevRegs.mGraphDispHiX,
																mGrDevRegs.mGraphDispY,
																mGrDevRegs.mGraphDispLoX2
																		+ 256 * mGrDevRegs.mGraphDispHiX2,
																mGrDevRegs.mGraphDispY2);
					break;

				case GRAPHDEVCMD_ERASLN:
					mpGraphDisp->EraseLine(mGrDevRegs.mGraphDispLoX 
																		+ 256 * mGrDevRegs.mGraphDispHiX,
																 mGrDevRegs.mGraphDispY,
																 mGrDevRegs.mGraphDispLoX2
																		+ 256 * mGrDevRegs.mGraphDispHiX2,
																 mGrDevRegs.mGraphDispY2);
					break;					

				default:
					break;

			}
		}
		GraphDisp_ReadEvents();
		GraphDisp_Update();
		//mpGraphDisp->Update();
	}	// if (NULL != mpGraphDisp)
}

/*
 *--------------------------------------------------------------------
 * Method:		ActivateGraphDisp()
 * Purpose:		Activate graphics display.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void MemMapDev::ActivateGraphDisp()
{
	if (NULL == mpGraphDisp) {
		mpGraphDisp = new GraphDisp();
		if (NULL == mpGraphDisp)
			throw MKGenException("Out of memory while initializing Graphics Display Device");
		mGrDevRegs.mGraphDispBgColR = 0;
		mGrDevRegs.mGraphDispBgColG = 0;
		mGrDevRegs.mGraphDispBgColB = 0;
		mGrDevRegs.mGraphDispPixColR = 0xFF;
		mGrDevRegs.mGraphDispPixColG = 0xFF;
		mGrDevRegs.mGraphDispPixColB = 0xFF;
		mpGraphDisp->SetBgColor(mGrDevRegs.mGraphDispBgColR,
														mGrDevRegs.mGraphDispBgColG,
														mGrDevRegs.mGraphDispBgColB);
		mpGraphDisp->SetFgColor(mGrDevRegs.mGraphDispPixColR,
														mGrDevRegs.mGraphDispPixColG,
														mGrDevRegs.mGraphDispPixColB);
		GraphDisp_Update();
		//mpGraphDisp->Start(mpGraphDisp);
	}
}

/*
 *--------------------------------------------------------------------
 * Method:		DeactivateGraphDisp()
 * Purpose:		Inactivate graphics display.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void MemMapDev::DeactivateGraphDisp()
{
#if defined(DBG)
	if (false == mpGraphDisp->IsMainLoopActive()) {
		cout << "DBG: ERROR: Main Loop is already inactive in Graphics Display." << endl;
	}
#endif	
	//mpGraphDisp->Stop();
	if (NULL != mpGraphDisp) delete mpGraphDisp;
	mpGraphDisp = NULL;
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void MemMapDev::GraphDisp_ReadEvents()
{
	if (NULL != mpGraphDisp) mpGraphDisp->ReadEvents();
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void MemMapDev::GraphDisp_Update()
{
	if (NULL != mpGraphDisp) mpGraphDisp->Update();
}

} // namespace MKBasic

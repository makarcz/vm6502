
#include "Memory.h"

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
	if (NULL != mpMemMapDev) delete mpMemMapDev;
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
	mROMBegin = ROM_BEGIN;
	mROMEnd = ROM_END;
	mROMActive = false;
	mpMemMapDev = new MemMapDev(this);
	mGraphDispActive = false;
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
 * Method:		Peek8bit()
 * Purpose:		Get/read 8-bit value from memory. If the memory address
 *            is within the range of any active devices, call
 *            handling method for this device.
 * Arguments:	addr - memory address
 * Returns:		unsigned char value read from specified memory address
 *--------------------------------------------------------------------
 */
unsigned char Memory::Peek8bit(unsigned short addr)
{
	// if memory address is in range of any active memory mapped
	// devices, call corresponding device handling function
	bool cont_loop = true;
	for (vector<int>::iterator it = mActiveDevNumVec.begin();
		   it != mActiveDevNumVec.end() && cont_loop;
		   ++it
		  ) {

		Device dev = mpMemMapDev->GetDevice(*it);
		if (dev.num >= 0) {
			for (MemAddrRanges::iterator memrangeit = dev.addr_ranges.begin();
				   memrangeit != dev.addr_ranges.end();
				   ++memrangeit
				  ) {
				AddrRange addr_range = *memrangeit;
				if (addr >= addr_range.start_addr && addr <= addr_range.end_addr) {
					ReadFunPtr pfun = dev.read_fun_ptr;
					if (pfun != NULL) {
						cont_loop = false;
						(mpMemMapDev->*pfun)((int)addr);
						break;
					}
				}
			}
		}
		mDispOp = (DEVNUM_GRDISP == dev.num);		
	}
	/*
	if (mCharIOActive && addr == mCharIOAddr) {
		m8bitMem[addr] = ReadCharKb(false); // blocking mode input
	} else if (mCharIOActive && addr == mCharIOAddr+1) {
		m8bitMem[addr] = ReadCharKb(true);	// non-blocking mode input
	}*/
		
	return m8bitMem[addr];
}

/*
 *--------------------------------------------------------------------
 * Method:		Peek8bitImg()
 * Purpose:		Get/read 8-bit value from memory image only.
 *            Memory mapped devices are not affected.
 * Arguments:	addr - memory address
 * Returns:		unsigned char value read from specified memory address
 *--------------------------------------------------------------------
 */
unsigned char Memory::Peek8bitImg(unsigned short addr)
{
	return m8bitMem[addr];
}

/*
 *--------------------------------------------------------------------
 * Method:		Peek16bit()
 * Purpose:		Get/read 16-bit value from memory. If the memory address
 *            is within the range of any active devices, call
 *            handling method for this device.
 * Arguments:	addr - memory address
 * Returns:		unsigned short value read secified from memory address
 *            and next memory address (16-bit, little endian)
 *--------------------------------------------------------------------
 */
unsigned short Memory::Peek16bit(unsigned short addr)
{
	unsigned short ret = 0;

	// if memory address is in range of any active memory mapped
	// devices, call corresponding device handling function
	bool cont_loop = true;
	for (vector<int>::iterator it = mActiveDevNumVec.begin();
		   it != mActiveDevNumVec.end() && cont_loop;
		   ++it
		  ) {
		Device dev = mpMemMapDev->GetDevice(*it);
		if (dev.num >= 0) {
			for (MemAddrRanges::iterator memrangeit = dev.addr_ranges.begin();
				   memrangeit != dev.addr_ranges.end();
				   ++memrangeit
				  ) {
				AddrRange addr_range = *memrangeit;
				if (addr >= addr_range.start_addr && addr <= addr_range.end_addr) {
					ReadFunPtr pfun = dev.read_fun_ptr;
					if (pfun != NULL) {
						cont_loop = false;
						(mpMemMapDev->*pfun)((int)addr);
						break;
					}
				}
			}
		}
		mDispOp = (DEVNUM_GRDISP == dev.num);
	}

	/*
	if (mCharIOActive && addr == mCharIOAddr) {
		m8bitMem[addr] = ReadCharKb(false);	// blocking mode input
	} else if (mCharIOActive && addr == mCharIOAddr+1) {
		m8bitMem[addr] = ReadCharKb(true);	// non-blocking mode input
	}*/
			
	ret = m8bitMem[addr++];
	ret += m8bitMem[addr] * 256;

	return ret;
}
	
/*
 *--------------------------------------------------------------------
 * Method:		Poke8bit()
 * Purpose:		Write byte to specified memory location.
 *            If the memory location is mapped to an active device,
 *            call corresponding handling function.
 *            If the memory location is protected (ROM), do not
 *            write the value.
 * Arguments: addr - (0x0000..0xffff) memory address,
 *            val - value (0x00..0xff)
 * Returns:   n/a
 *--------------------------------------------------------------------
 */	
void Memory::Poke8bit(unsigned short addr, unsigned char val)
{
	// if memory address is in range of any active memory mapped
	// devices, call corresponding device handling function
	bool cont_loop = true;
	for (vector<int>::iterator it = mActiveDevNumVec.begin();
		   it != mActiveDevNumVec.end() && cont_loop;
		   ++it
		  ) {	
		Device dev = mpMemMapDev->GetDevice(*it);
		if (dev.num >= 0) {
			for (MemAddrRanges::iterator memrangeit = dev.addr_ranges.begin();
				   memrangeit != dev.addr_ranges.end();
				   ++memrangeit
				  ) {
				AddrRange addr_range = *memrangeit;
				if (addr >= addr_range.start_addr && addr <= addr_range.end_addr) {
					WriteFunPtr pfun = dev.write_fun_ptr;
					if (pfun != NULL) {
						cont_loop = false;
						(mpMemMapDev->*pfun)((int)addr,(int)val);
						m8bitMem[addr] = val;
						break;
					}
				}
			}
		}
		mDispOp = (DEVNUM_GRDISP == dev.num);
	}
	/*
	if (mCharIOActive && addr == mCharIOAddr)
		PutCharIO(val);
		*/
	if (!mROMActive || (addr < mROMBegin || addr > mROMEnd)) {
		m8bitMem[addr] = val;
	}
}

/*
 *--------------------------------------------------------------------
 * Method:		Poke8bitImg()
 * Purpose:		Write byte to specified memory location.
 * Arguments: addr - (0x0000..0xffff) memory address,
 *            val - value (0x00..0xff)
 * Returns:   n/a
 *--------------------------------------------------------------------
 */	
void Memory::Poke8bitImg(unsigned short addr, unsigned char val)
{
	m8bitMem[addr] = val;
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
	//mCharIOActive = true;
	mIOEcho = echo;

	AddrRange addr_range(addr, addr+1);
	MemAddrRanges memaddr_ranges;
	DevPar dev_par("echo",(echo?"true":"false"));
	DevParams dev_params;

	dev_params.push_back(dev_par);
	memaddr_ranges.push_back(addr_range);	

	if (false == mCharIOActive) AddDevice(DEVNUM_CHARIO);
	mCharIOActive = true;
	SetupDevice(DEVNUM_CHARIO, memaddr_ranges, dev_params);
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
	DeleteDevice(DEVNUM_CHARIO);
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
 * Method:		SetGraphDisp()
 * Purpose:		Setup and activate graphics display device.
 * Arguments:	addr - base address of display device
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void Memory::SetGraphDisp(unsigned short addr)
{
	AddrRange addr_range(addr, addr + GRAPHDEVREG_END - 1);
	MemAddrRanges memaddr_ranges;
	DevPar dev_par("nil","nil");
	DevParams dev_params;

	dev_params.push_back(dev_par);
	memaddr_ranges.push_back(addr_range);	

	if (false == mGraphDispActive) AddDevice(DEVNUM_GRDISP);
	mGraphDispActive = true;
	SetupDevice(DEVNUM_GRDISP, memaddr_ranges, dev_params);
	mpMemMapDev->ActivateGraphDisp();
}

/*
 *--------------------------------------------------------------------
 * Method:		DisableGraphDisp()
 * Purpose:		Inactivate graphics display device.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void Memory::DisableGraphDisp()
{
	mGraphDispActive = false;
	mpMemMapDev->DeactivateGraphDisp();
	DeleteDevice(DEVNUM_GRDISP);
}

/*
 *--------------------------------------------------------------------
 * Method:		GetGraphDispAddr()
 * Purpose:		Return base address of graphics display device.
 * Arguments:	n/a
 * Returns:		unsigned short - address ($0000 - $FFFF)
 *--------------------------------------------------------------------
 */
unsigned short Memory::GetGraphDispAddr()
{
	return mpMemMapDev->GetGraphDispAddrBase();
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

/*
 *--------------------------------------------------------------------
 * Method:		AddDevice()
 * Purpose:		Add device number to active devices list.
 * Arguments:	devnum - device number
 * Returns:		0
 *--------------------------------------------------------------------
 */		
int Memory::AddDevice(int devnum)
{
	mActiveDevNumVec.push_back(devnum);

	return 0;
}

/*
 *--------------------------------------------------------------------
 * Method:		DeleteDevice()
 * Purpose:		Delete device number from active devices list.
 * Arguments:	devnum - device number
 * Returns:		0
 *--------------------------------------------------------------------
 */		
int Memory::DeleteDevice(int devnum)
{
	vector<int> active_new;

	for (vector<int>::iterator it = mActiveDevNumVec.begin();
			 it != mActiveDevNumVec.end();
			 ++it
			) {
		if (*it != devnum) {
			active_new.push_back(*it);
		}
	}
	mActiveDevNumVec.clear();
	mActiveDevNumVec = active_new;

	return 0;
}

/*
 *--------------------------------------------------------------------
 * Method:		SetupDevice()
 * Purpose:		Configure device address ranges and parameters.
 * Arguments:	devnum - device number, must be on active dev. list
 *            memranges - memory address ranges vector
 *            params - parameters vector
 * Returns:		n/a
 *--------------------------------------------------------------------
 */		
void Memory::SetupDevice(int devnum, 
												 MemAddrRanges memranges, 
												 DevParams params)
{
	for (vector<int>::iterator it = mActiveDevNumVec.begin();
			 it != mActiveDevNumVec.end();
			 ++it
			) {
		if (devnum == *it) {
			mpMemMapDev->SetupDevice(devnum, memranges, params);
			break;
		}
	}
	if (DEVNUM_CHARIO == devnum) {
		mCharIOAddr = mpMemMapDev->GetCharIOAddr();
		mIOEcho = mpMemMapDev->GetCharIOEchoOn();
	}
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
	return mpMemMapDev->GetCharIn();
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
	return mpMemMapDev->GetCharOut();
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void Memory::GraphDisp_ReadEvents()
{
	mpMemMapDev->GraphDisp_ReadEvents();
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void Memory::GraphDisp_Update()
{
	mpMemMapDev->GraphDisp_Update();
}

/*
 *--------------------------------------------------------------------
 * Method:		GraphDispOp()
 * Purpose:		Status of last operation being perf. on Graphics Display
 *            or not.
 * Arguments: n/a
 * Returns:   bool, true if last operation was performed on Graphics
 *                  Display device.
 *--------------------------------------------------------------------
 */
bool Memory::GraphDispOp()
{
	return mDispOp;
}

} // namespace MKBasic

/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:   			Memory.cpp.
 *
 * Purpose: 		Implementation of class Memory.
 *							The Memory class implements the highest abstraction
 *							layer of Visrtual Machine's memory. in this particular
 *							case the Virtual Machine emulates a computer system
 *							based on a 8-bit microprocessor. Therefore it
 *              implements image size and addressing space adequate
 *              for the specific architecture of such microprocessor.
 *							The Memory class also interfaces with the MemMapDev
 *              API (Memory Mapped Devices).
 *
 * Date:      
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
#include "Memory.h"
#include "MKGenException.h"

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
	for (int i=0; i < MEM_PAGE_SIZE; i++) {
		mMemPageDev[i] = -1;
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
	int mempg = addr / MEM_PAGE_SIZE;
	if (mMemPageDev[mempg] >= 0) {
		bool cont_loop = true;
		for (vector<Device>::iterator devit = mActiveDeviceVec.begin();
				 devit != mActiveDeviceVec.end() && cont_loop;
				 ++devit
				) {		   

			if (devit->num >= 0) {
				for (MemAddrRanges::iterator memrangeit = devit->addr_ranges.begin();
					   memrangeit != devit->addr_ranges.end();
					   ++memrangeit
					  ) {				   
					if (addr >= memrangeit->start_addr && addr <= memrangeit->end_addr) {
						ReadFunPtr pfun = devit->read_fun_ptr;
						if (pfun != NULL) {
							cont_loop = false;
							(mpMemMapDev->*pfun)((int)addr);
							break;
						}
					}
				}
				mDispOp = (DEVNUM_GRDISP == devit->num);
			}
		}
	}
		
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
	int mempg = addr / MEM_PAGE_SIZE;
	if (mMemPageDev[mempg] >= 0) {	
		bool cont_loop = true;
		for (vector<Device>::iterator devit = mActiveDeviceVec.begin();
			   devit != mActiveDeviceVec.end() && cont_loop;
			   ++devit
			  ) {		   
			if (devit->num >= 0) {
				for (MemAddrRanges::iterator memrangeit = devit->addr_ranges.begin();
					   memrangeit != devit->addr_ranges.end();
					   ++memrangeit
					  ) {				   
					if (addr >= memrangeit->start_addr && addr <= memrangeit->end_addr) {
						ReadFunPtr pfun = devit->read_fun_ptr;
						if (pfun != NULL) {
							cont_loop = false;
							(mpMemMapDev->*pfun)((int)addr);
							break;
						}
					}
				}
				mDispOp = (DEVNUM_GRDISP == devit->num);
			}
		}
	}

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
	int mempg = addr / MEM_PAGE_SIZE;
	if (mMemPageDev[mempg] >= 0) {	
		bool cont_loop = true;
		for (vector<Device>::iterator devit = mActiveDeviceVec.begin();
			   devit != mActiveDeviceVec.end() && cont_loop;
			   ++devit
			  ) {			   
			if (devit->num >= 0) {
				for (MemAddrRanges::iterator memrangeit = devit->addr_ranges.begin();
					   memrangeit != devit->addr_ranges.end();
					   ++memrangeit
					  ) {				   
					if (addr >= memrangeit->start_addr && addr <= memrangeit->end_addr) {
						WriteFunPtr pfun = devit->write_fun_ptr;
						if (pfun != NULL) {
							cont_loop = false;
							(mpMemMapDev->*pfun)((int)addr,(int)val);
							break;
						}
					}
				}
				mDispOp = (DEVNUM_GRDISP == devit->num);
			}
		}
	}

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
	mIOEcho = echo;

	AddrRange addr_range(addr, addr+1);
	MemAddrRanges memaddr_ranges;
	DevPar dev_par("echo",(echo?"true":"false"));
	DevParams dev_params;

	dev_params.push_back(dev_par);
	memaddr_ranges.push_back(addr_range);	

	SetupDevice(DEVNUM_CHARIO, memaddr_ranges, dev_params);
	if (false == mCharIOActive) AddDevice(DEVNUM_CHARIO);
	mCharIOActive = true;	
	mpMemMapDev->ActivateCharIO();
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
	mpMemMapDev->DeactivateCharIO();
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

	SetupDevice(DEVNUM_GRDISP, memaddr_ranges, dev_params);
	if (false == mGraphDispActive) AddDevice(DEVNUM_GRDISP);
	mGraphDispActive = true;	
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
 * Method:		GetROMBegin()
 * Purpose:		Get starting address of read-only memory.
 * Arguments:	
 * Returns:		unsigned short - address ($0000-$FFFF)
 *--------------------------------------------------------------------
 */
unsigned short Memory::GetROMBegin()
{
	return mROMBegin;
}
		
/*
 *--------------------------------------------------------------------
 * Method:		GetROMEnd()
 * Purpose:		Get end address of read-only memory.
 * Arguments:
 * Returns:		unsigned short - address ($0000-$FFFF)
 *--------------------------------------------------------------------
 */		
unsigned short Memory::GetROMEnd()
{
	return mROMEnd;
}

/*
 *--------------------------------------------------------------------
 * Method:		IsROMEnabled()
 * Purpose:		Get status of ROM.
 * Arguments:	
 * Returns:		bool - true if enabled.
 *--------------------------------------------------------------------
 */		
bool Memory::IsROMEnabled()
{
	return mROMActive;
}

/*
 *--------------------------------------------------------------------
 * Method:		AddDevice()
 * Purpose:		Add device to active devices cache.
 * Arguments:	devnum - device number
 * Returns:		-1 if device is not supported OR already cached
 *            devnum - device number if it was found
 *--------------------------------------------------------------------
 */		
int Memory::AddDevice(int devnum)
{
	int ret = -1;
	bool found = false;
	Device dev = mpMemMapDev->GetDevice(devnum);
	if (dev.num >= 0) {
		for (vector<Device>::iterator devit = mActiveDeviceVec.begin();
				 devit != mActiveDeviceVec.end();
				 ++devit
				) {

			if (devit->num == devnum) {
				found = true;
				break;
			}
		}
		// if device not found in local cache, add it
		if (!found) {
			mActiveDeviceVec.push_back(dev);
			ret = devnum;

			// update the device usage flag in memory pages devices array mMemPageDev
			for (MemAddrRanges::iterator memrangeit = dev.addr_ranges.begin();
				   memrangeit != dev.addr_ranges.end();
				   ++memrangeit
				  ) {			

				int pgnum = memrangeit->start_addr / MEM_PAGE_SIZE;
				while (pgnum < MEM_PAGE_SIZE) {
					mMemPageDev[pgnum] = devnum;
					pgnum++;
					if (pgnum * MEM_PAGE_SIZE > memrangeit->end_addr) break;
				}
			}
		}
	}	// END if (dev.num >= 0)
	// else device with such number is not supported

	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		DeleteDevice()
 * Purpose:		Delete device from active devices cache.
 * Arguments:	devnum - device number
 * Returns:		>=0 if device was found in local cache and deleted
 *            -1 if device was not found
 *--------------------------------------------------------------------
 */		
int Memory::DeleteDevice(int devnum)
{
	vector<Device> actdev_new;
	int ret = -1;

	for (int i=0; i < MEM_PAGE_SIZE; i++) {
		mMemPageDev[i] = -1;
	}
	// device is deleted by refreshing local active devices cache
	// the device to be deleted is skipped and not re-added to refreshed
	// cache vector
	for (vector<Device>::iterator devit = mActiveDeviceVec.begin();
			 devit != mActiveDeviceVec.end();
			 ++devit
			) {

		if (devit->num != devnum) {

			Device dev = mpMemMapDev->GetDevice(devit->num);

			if (dev.num < 0) 
				throw MKGenException("Unsupported device in local cache");

			actdev_new.push_back(mpMemMapDev->GetDevice(devit->num));

			// update the device number in memory pages devices array mMemPageDev
			for (MemAddrRanges::iterator memrangeit = devit->addr_ranges.begin();
				   memrangeit != devit->addr_ranges.end();
				   ++memrangeit
				  ) {			

				int pgnum = memrangeit->start_addr / MEM_PAGE_SIZE;
				while (pgnum < MEM_PAGE_SIZE) {
					mMemPageDev[pgnum] = devit->num;
					pgnum++;
					if (pgnum * MEM_PAGE_SIZE > memrangeit->end_addr) break;
				}
			}			

		} else ret++;	// indicating that the device was found in cache
	}
	// refresh local active devices cache
	mActiveDeviceVec.clear();
	mActiveDeviceVec = actdev_new;

	return ret;
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
	Device dev = mpMemMapDev->GetDevice(devnum);
	if (dev.num >= 0) {
		mpMemMapDev->SetupDevice(devnum, memranges, params);
		// reload device to local vector because its setup has changed
		// but only if device was cached locally already (active)
		if (0 <= DeleteDevice(devnum)) AddDevice(devnum);		
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
 * Method:		GraphDisp_ReadEvents()
 * Purpose:		Read events from the graphics display window.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void Memory::GraphDisp_ReadEvents()
{
	mpMemMapDev->GraphDisp_ReadEvents();
}

/*
 *--------------------------------------------------------------------
 * Method:		GraphDisp_Update()
 * Purpose:		Trigger update handler of the graphics display window.
 * Arguments:	n/a
 * Returns:		n/a
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

/*
 *--------------------------------------------------------------------
 * Method:		GetMemMapDevPtr()
 * Purpose:		Get the pointer to MemMapDev object.
 * Arguments:	n/a
 * Returns:		Pointer to MemMapDev object.
 *--------------------------------------------------------------------
 */
MemMapDev *Memory::GetMemMapDevPtr()
{
	return mpMemMapDev;
}

} // namespace MKBasic

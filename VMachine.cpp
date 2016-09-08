/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:   			VMachine.cpp
 *
 * Purpose: 		Implementation of VMachine class.
 *							The VMachine class implements the Virtual Machine
 *							in its entirety. It creates all the objects that
 *							emulate the component's of the whole system and
 *							implements the methods to execute the code on the
 *							emulated platform.
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
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include "system.h"
#include "VMachine.h"
#include "MKGenException.h"

/*
#if defined(WINDOWS)
#include <conio.h>
#endif
*/

using namespace std;

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
 * Method:		VMachine()
 * Purpose:		Default class constructor.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */ 
VMachine::VMachine()
{
	InitVM();
}

/*
 *--------------------------------------------------------------------
 * Method:		VMachine()
 * Purpose:		Custom class constructor.
 * Arguments:	romfname - name of the ROM definition file
 *						ramfname - name of the RAM definition file
 * Returns:		n/a
 *--------------------------------------------------------------------
 */ 
VMachine::VMachine(string romfname, string ramfname)
{
	InitVM();
	LoadROM(romfname);
	LoadRAM(ramfname);
}

/*
 *--------------------------------------------------------------------
 * Method:		~VMachine()
 * Purpose:		Class destructor.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
VMachine::~VMachine()
{
	//delete mpDisp;
	delete mpCPU;
	delete mpROM;
	delete mpRAM;
	delete mpConIO;
}

/*
 *--------------------------------------------------------------------
 * Method:		InitVM()
 * Purpose:		Initialize class.
 * Arguments:	n/a
 * Returns:		n/a
f *--------------------------------------------------------------------
 */
void VMachine::InitVM()
{
	mOpInterrupt = false;
	mpRAM = new Memory();

	mPerfStats.cycles = 0;
	mPerfStats.perf_onemhz = 0;
	mPerfStats.prev_cycles = 0;
	mPerfStats.prev_usec = 0;
	mOldStyleHeader = false;
	mError = VMERR_OK;
	mAutoExec = false;	
	mAutoReset = false;
	mCharIOAddr = CHARIO_ADDR;
	mCharIOActive = mCharIO = false;
	mGraphDispActive = false;
	mPerfStatsActive = false;
	mDebugTraceActive = false;
	if (NULL == mpRAM) {
		throw MKGenException("Unable to initialize VM (RAM).");
	}
	mRunAddr = mpRAM->Peek16bit(0xFFFC);	// address under RESET vector
	mpROM = new Memory();
	if (NULL == mpROM) {
		throw MKGenException("Unable to initialize VM (ROM).");
	}	
	mpCPU = new MKCpu(mpRAM);
	if (NULL == mpCPU) {
		throw MKGenException("Unable to initialize VM (CPU).");
	}
	/*
	mpDisp = new Display();
	if (NULL == mpDisp) {
		throw MKGenException("Unable to initialize VM (Display).");
	}	*/	
	mpConIO = new ConsoleIO();
	if (NULL == mpConIO) {
		throw MKGenException("Unable to initialize VM (ConsoleIO)");
	}
	mBeginTime = high_resolution_clock::now();
}

/*
 *--------------------------------------------------------------------
 * Method:		ClearScreen()
 * Purpose:		Clear the working are of the VM - DOS. 
 *            This is not a part of virtual display emulation.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::ClearScreen()
{
	mpConIO->ClearScreen();
}

/*
 *--------------------------------------------------------------------
 * Method:		ScrHome()
 * Purpose:		Bring the console cursor to home position - DOS.
 *            This is not a part of virtual display emulation.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::ScrHome()
{
	mpConIO->ScrHome();
}

/*
 *--------------------------------------------------------------------
 * Method:		ShowDisp()
 * Purpose:		Show the emulated virtual text display device contents.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::ShowDisp()
{
	if (mCharIOActive && NULL != mpDisp) {
			ScrHome();
			mpDisp->ShowScr();
	}	
}


/*
 *--------------------------------------------------------------------
 * Method:		CalcCurrPerf()
 * Purpose:		Calculate CPU emulation performance based on 1 MHz model
 *						CPU.
 * Arguments:	n/a
 * Returns:		Integer, the % of speed as compared to 1 MHz CPU.
 *--------------------------------------------------------------------
 */
int VMachine::CalcCurrPerf()
{
	if (!mPerfStatsActive) return 0;

	auto lap = high_resolution_clock::now();
	long usec = duration_cast<microseconds>(lap-mPerfStats.begin_time).count();

	if (usec > 0) {
		int currperf = (int)(((double)mPerfStats.cycles / (double)usec) * 100.0);
		if (mPerfStats.perf_onemhz == 0)
			mPerfStats.perf_onemhz = currperf;
		else
			mPerfStats.perf_onemhz = (mPerfStats.perf_onemhz + currperf) / 2;

		mPerfStats.prev_cycles = mPerfStats.cycles;
		mPerfStats.prev_usec = usec;
		mPerfStats.cycles = 0;
		mPerfStats.begin_time = lap;
		if (mDebugTraceActive) {	// prepare and log some debug traces
			stringstream sscp, ssap;
			string msg, avgprf, curprf;
			ssap << mPerfStats.perf_onemhz;
			ssap >> avgprf;
			sscp << currperf;
			sscp >> curprf;
			msg = "Perf. measured. Curr.: " + curprf + " %, Avg.: " + avgprf + " %";
			AddDebugTrace(msg);
		}
	}

	return mPerfStats.perf_onemhz;
} 

/*
 *--------------------------------------------------------------------
 * Method:		PERFSTAT_LAP (macro)
 * Purpose:		Calculate emulation performace at pre-defined interval
 *            of real time and clock ticks.
 * Arguments:	cycles - long : number of clock ticks executed so far
 *            begin - time_point<high_resolution_clock> : the moment
 *                    when time measurement started
 * Returns:		n/a
 * Remarks:		Call inside emulation execute loop.
 *--------------------------------------------------------------------
 */
#define PERFSTAT_LAP(cycles,begin) \
{	\
	if (mPerfStatsActive && cycles%PERFSTAT_CYCLES == 0) {	\
		long usec = duration_cast<microseconds>	\
							(high_resolution_clock::now()-begin).count();	\
		if (usec >= PERFSTAT_INTERVAL) CalcCurrPerf();	\
	}	\
}

/*
 *--------------------------------------------------------------------
 * Method:		Run()
 * Purpose:		Run VM until software break instruction.
 * Arguments:	n/a
 * Returns:		Pointer to CPU registers and flags.
 *--------------------------------------------------------------------
 */
Regs *VMachine::Run()
{
	Regs *cpureg = NULL;

	AddDebugTrace("Running code at: $" + Addr2HexStr(mRunAddr));
	mOpInterrupt = false;
	ClearScreen();
	ShowDisp();
	mPerfStats.cycles = 0;
	mPerfStats.begin_time = high_resolution_clock::now();	
	while (true) {
		mPerfStats.cycles++;		
		cpureg = Step();
		if (cpureg->CyclesLeft == 0 && mCharIO)	ShowDisp();
		if (cpureg->SoftIrq || mOpInterrupt) break;
		PERFSTAT_LAP(mPerfStats.cycles,mPerfStats.begin_time);
	}
	CalcCurrPerf();

	ShowDisp();	
	
	return cpureg;
}

/*
 *--------------------------------------------------------------------
 * Method:		Run()
 * Purpose:		Run VM from specified address until software break 
 *						instruction.
 * Arguments:	addr - start execution address
 * Returns:		Pointer to CPU registers and flags.
 *--------------------------------------------------------------------
 */
Regs *VMachine::Run(unsigned short addr)
{
	mRunAddr = addr;
	return Run();
}

/*
 *--------------------------------------------------------------------
 * Method:		Exec()
 * Purpose:		Run VM from current address until last RTS (if enabled).
 *            NOTE: Stack must be empty for last RTS to be trapped.
 * Arguments:	n/a
 * Returns:		Pointer to CPU registers and flags.
 *--------------------------------------------------------------------
 */
Regs *VMachine::Exec()
{
	Regs *cpureg = NULL;

	AddDebugTrace("Executing code at: $" + Addr2HexStr(mRunAddr));
	mOpInterrupt = false;
	ClearScreen();
	ShowDisp();
	mPerfStats.cycles = 0;
	mPerfStats.begin_time = high_resolution_clock::now();
	while (true) {
		mPerfStats.cycles++;
		cpureg = Step();
		if (cpureg->LastRTS || mOpInterrupt) break;
		PERFSTAT_LAP(mPerfStats.cycles,mPerfStats.begin_time);
	}
	CalcCurrPerf();

	ShowDisp();	
	
	return cpureg;
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
PerfStats VMachine::GetPerfStats()
{
	return mPerfStats;
}

/*
 *--------------------------------------------------------------------
 * Method:		Exec()
 * Purpose:		Run VM from specified address until RTS.
 * Arguments:	addr - start execution address
 * Returns:		Pointer to CPU registers and flags.
 *--------------------------------------------------------------------
 */
Regs *VMachine::Exec(unsigned short addr)
{
	mRunAddr = addr;
	return Exec();
}

/*
 *--------------------------------------------------------------------
 * Method:		Step()
 * Purpose:		Execute single opcode.
 * Arguments:	n/a
 * Returns:		Pointer to CPU registers and flags.
 *--------------------------------------------------------------------
 */
Regs *VMachine::Step()
{
	Regs *cpureg = NULL;	
	
	cpureg = mpCPU->ExecOpcode(mRunAddr);
	if (mGraphDispActive && cpureg->CyclesLeft == 0) {
		mpRAM->GraphDisp_ReadEvents();
	}
	mRunAddr = cpureg->PtrAddr;
	
	return cpureg;
}

/*
 *--------------------------------------------------------------------
 * Method:		Step()
 * Purpose:		Execute single opcode.
 * Arguments:	addr (unsigned short) - opcode address
 * Returns:		Pointer to CPU registers and flags.
 *--------------------------------------------------------------------
 */
Regs *VMachine::Step(unsigned short addr)
{
	mRunAddr = addr;
	return Step();
}

/*
 *--------------------------------------------------------------------
 * Method:		LoadROM()
 * Purpose:		Load data from memory definition file to the memory.
 * Arguments:	romfname - name of the ROM file definition
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::LoadROM(string romfname)
{
	LoadMEM(romfname, mpROM);
}

/*
 *--------------------------------------------------------------------
 * Method:		LoadRAM()
 * Purpose:		Load data from memory definition file to the memory.
 * Arguments:	ramfname - name of the RAM file definition
 * Returns:		int - error code
 *--------------------------------------------------------------------
 */
int VMachine::LoadRAM(string ramfname)
{
	int err = 0;
	eMemoryImageTypes memimg_type = GetMemoryImageType(ramfname);
	switch (memimg_type) {
		case MEMIMG_VM65DEF:		err = LoadMEM(ramfname, mpRAM); break;
		case MEMIMG_INTELHEX:		err = LoadRAMHex(ramfname); break;
		case MEMIMG_BIN:
		default:	// unknown type, try to read as binary
							// and hope for the best
			err = LoadRAMBin(ramfname);
			break;
	}
	mError = err;
	if (mDebugTraceActive && err) {
		stringstream sserr;
		string msg, strerr;
		sserr << err;
		sserr >> strerr;
		msg = "ERROR: LoadRAM, error code: " + strerr;
		AddDebugTrace(msg);
	}		

	return err;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetMemoryImageType()
 * Purpose:		Detect format of memory image file.
 * Arguments:	ramfname - name of the RAM file definition
 * Returns:		eMemoryImageTypes - code of the memory image format
 *--------------------------------------------------------------------
 */
eMemoryImageTypes VMachine::GetMemoryImageType(string ramfname)
{
	eMemoryImageTypes ret = MEMIMG_UNKNOWN;
	char buf[256] = {0};
	FILE *fp = NULL;
	int n = 0;

	if ((fp = fopen(ramfname.c_str(), "rb")) != NULL) {
		memset(buf, 0, 256);
		while (0 == feof(fp) && 0 == ferror(fp)) {
			unsigned char val = fgetc(fp);
			buf[n++] = val;
			if (n >= 256) break;
		}
		fclose(fp);
	}
	bool possibly_intelhex = true;
	for (int i=0; i<256; i++) {
		char *pc = buf+i;
		if (isspace(buf[i])) continue;
		if (i < 256-9 // 256 less the length of the longest expected keyword
				&&
				(!strncmp(pc, "ADDR", 4)
				|| !strncmp(pc, "ORG", 3)
				|| !strncmp(pc, "IOADDR", 6)
				|| !strncmp(pc, "ROMBEGIN", 8)
				|| !strncmp(pc, "ROMEND", 6)
				|| !strncmp(pc, "ENROM", 5)
				|| !strncmp(pc, "ENIO", 4)
				|| !strncmp(pc, "EXEC", 4)
				|| !strncmp(pc, "RESET", 5)
				|| !strncmp(pc, "ENGRAPH", 7)
				|| !strncmp(pc, "GRAPHADDR", 9))
			 )
		{
			ret = MEMIMG_VM65DEF;
			break;
		}
		if (buf[i] != ':'
			  && buf[i] != '0'
			  && buf[i] != '1'
			  && buf[i] != '2'
			  && buf[i] != '3'
			  && buf[i] != '4'
			  && buf[i] != '5'
			  && buf[i] != '6'
			  && buf[i] != '7'
			  && buf[i] != '8'
			  && buf[i] != '9'
			  && tolower(buf[i]) != 'a'
			  && tolower(buf[i]) != 'b'
			  && tolower(buf[i]) != 'c'
			  && tolower(buf[i]) != 'd'
			  && tolower(buf[i]) != 'e'
			  && tolower(buf[i]) != 'f') 
		{
			possibly_intelhex = false;
		}
	}
	if (ret == MEMIMG_UNKNOWN && possibly_intelhex)
		ret = MEMIMG_INTELHEX;

	return ret;	
}

/*
 *--------------------------------------------------------------------
 * Method:		HasHdrData()
 * Purpose:		Check for header in the binary memory image.
 * Arguments:	File pointer.
 * Returns:		true if magic keyword found at the beginning of the
 *						memory image file, false otherwise
 *--------------------------------------------------------------------
 */
bool VMachine::HasHdrData(FILE *fp)
{
	bool ret = false;
	int n = 0, l = strlen(HDRMAGICKEY);
	char buf[20];

	memset(buf, 0, 20);
	
	rewind(fp);
	while (0 == feof(fp) && 0 == ferror(fp)) {
		unsigned char val = fgetc(fp);
		buf[n] = val;
		n++;
		if (n >= l) break;
	}
	ret = (0 == strncmp(buf, HDRMAGICKEY, l));

	AddDebugTrace(((ret) ? "HasHdrData: YES" : "HasHdrData: NO"));

	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		HasOldHdrData()
 * Purpose:		Check for previous version header in the binary memory
 *            image.
 * Arguments:	File pointer.
 * Returns:		true if magic keyword found at the beginning of the
 *						memory image file, false otherwise
 *--------------------------------------------------------------------
 */
bool VMachine::HasOldHdrData(FILE *fp)
{
	bool ret = false;
	int n = 0, l = strlen(HDRMAGICKEY_OLD);
	char buf[20];

	memset(buf, 0, 20);
	
	rewind(fp);
	while (0 == feof(fp) && 0 == ferror(fp)) {
		unsigned char val = fgetc(fp);
		buf[n] = val;
		n++;
		if (n >= l) break;
	}
	ret = (0 == strncmp(buf, HDRMAGICKEY_OLD, l));

	AddDebugTrace(((ret) ? "HasOldHdrData: YES" : "HasOldHdrData: NO"));

	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		LoadHdrData()
 * Purpose:		Load data from binary image header.
 * Arguments:	File pointer.
 * Returns:		bool, true if success, false if error
 *
 * Details:
 *    Header of the binary memory image consists of magic keyword
 * string followed by the 128 bytes of data (unsigned char values). 
 * It has following format:
 *
 * MAGIC_KEYWORD
 * aabbccddefghijklmm[remaining unused bytes]
 *
 * Where:
 *    MAGIC_KEYWORD - text string indicating header, may vary between
 *                    versions thus rendering headers from previous
 *                    versions incompatible - currently: "SNAPSHOT2"
 *                    NOTE: Previous version of header is currently
 *                          recognized and can be read, the magic
 *                          keyword of previous version: "SNAPSHOT".
 *                          Old header had only 15 bytes of data.
 *                          This backwards compatibility will be
 *                          removed in next version as the new
 *                          format of header with 128 bytes for
 *                          data leaves space for expansion without
 *                          the need of changing file format.
 *    aa - low and hi bytes of execute address (PC)
 *    bb - low and hi bytes of char IO address
 *    cc - low and hi bytes of ROM begin address
 *    dd - low and hi bytes of ROM end address
 *    e - 0 if char IO is disabled, 1 if enabled
 *    f - 0 if ROM is disabled, 1 if enabled
 *    g - value in CPU Acc (accumulator) register
 *    h - value in CPU X (X index) register
 *    i - value in CPU Y (Y index) register
 *    j - value in CPU PS (processor status/flags)
 *    k - value in CPU SP (stack pointer) register
 *    l - 0 if generic graphics display device is disabled,
 *        1 if graphics display is enabled
 *    mm - low and hi bytes of graphics display base address
 *
 * NOTE:
 *   If magic keyword was detected, this part is already read and file
 *   pointer position is at the 1-st byte of data. Therefore this
 *   method does not have to read and skip the magic keyword.
 *--------------------------------------------------------------------
 */
bool VMachine::LoadHdrData(FILE *fp)
{
	int n = 0, l = 0, hdrdtlen = HDRDATALEN;
	unsigned short rb = 0, re = 0;
	Regs r;
	bool ret = false;

	if (mOldStyleHeader) hdrdtlen = HDRDATALEN_OLD;
	while (0 == feof(fp) && 0 == ferror(fp) && n < hdrdtlen) {
		unsigned char val = fgetc(fp);
		switch (n)
		{
			case 1:		mRunAddr = l + 256 * val;
								ADD_DBG_LDMEMPARHEX("LoadHdrData : mRunAddr",mRunAddr);
								break;
			case 3:		mCharIOAddr = l + 256 * val;
								ADD_DBG_LDMEMPARHEX("LoadHdrData : mCharIOAddr",mCharIOAddr);
								break;
			case 5:		rb = l + 256 * val;
								break;
			case 7: 	re = l + 256 * val;
								break;
			case 8: 	mCharIOActive = (val != 0);
								ADD_DBG_LDMEMPARVAL("LoadHdrData : mCharIOActive",mCharIOActive);
								break;
			case 9: 	if (val != 0) {
									mpRAM->EnableROM(rb, re);
								} else {
									mpRAM->SetROM(rb, re);
								}
								ADD_DBG_LDMEMPARHEX("LoadHdrData : ROM begin",rb);
								ADD_DBG_LDMEMPARHEX("LoadHdrData : ROM end",re);
								ADD_DBG_LDMEMPARVAL("LoadHdrData : ROM enable",((val!=0)?1:0));
								break;
			case 10:	r.Acc = val;
								break;
			case 11:	r.IndX = val;
								break;
			case 12:	r.IndY = val;
								break;
			case 13:	r.Flags = val;
								break;
			case 14:	r.PtrStack = val;
								ret = true;
								break;
			case 15:	mGraphDispActive = (val != 0);
								ADD_DBG_LDMEMPARVAL("LoadHdrData : mGraphDispActive",mGraphDispActive);
								break;
			case 17:	if (mGraphDispActive) SetGraphDisp(l + 256 * val);
								else DisableGraphDisp();
								ADD_DBG_LDMEMPARHEX("LoadHdrData : Graph. Disp. addr",(l + 256 * val));
								break;
			default: 	break;
		}
		l = val;
		n++;
	}
	if (ret) {
		r.PtrAddr = mRunAddr;
		mpCPU->SetRegs(r);
	}

	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		SaveHdrData()
 * Purpose:		Save header data to binary file (memory snapshot).
 * Arguments:	File pointer, must be opened for writing in binary mode.
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::SaveHdrData(FILE *fp)
{
	char buf[20] = {0};
	int n = HDRDATALEN;

	strcpy(buf, HDRMAGICKEY);
	for (unsigned int i = 0; i < strlen(HDRMAGICKEY); i++) {
		fputc(buf[i], fp);
	}
	Regs *reg = mpCPU->GetRegs();
	unsigned char lo = 0, hi = 0;
	lo = (unsigned char) (reg->PtrAddr & 0x00FF);
	hi = (unsigned char) ((reg->PtrAddr & 0xFF00) >> 8);
	SAVE_HDR_DATA(lo,fp,n);
	SAVE_HDR_DATA(hi,fp,n);
	lo = (unsigned char) (mCharIOAddr & 0x00FF);
	hi = (unsigned char) ((mCharIOAddr & 0xFF00) >> 8);
	SAVE_HDR_DATA(lo,fp,n);
	SAVE_HDR_DATA(hi,fp,n);	
	lo = (unsigned char) (GetROMBegin() & 0x00FF);
	hi = (unsigned char) ((GetROMBegin() & 0xFF00) >> 8);
	SAVE_HDR_DATA(lo,fp,n);
	SAVE_HDR_DATA(hi,fp,n);	
	lo = (unsigned char) (GetROMEnd() & 0x00FF);
	hi = (unsigned char) ((GetROMEnd() & 0xFF00) >> 8);
	SAVE_HDR_DATA(lo,fp,n);
	SAVE_HDR_DATA(hi,fp,n);	
	lo = (mCharIOActive ? 1 : 0);
	SAVE_HDR_DATA(lo,fp,n);	
	lo = (IsROMEnabled() ? 1 : 0);
	SAVE_HDR_DATA(lo,fp,n);	
	Regs *pregs = mpCPU->GetRegs();
	if (pregs != NULL) {
		SAVE_HDR_DATA(pregs->Acc,fp,n);
		SAVE_HDR_DATA(pregs->IndX,fp,n);
		SAVE_HDR_DATA(pregs->IndY,fp,n);
		SAVE_HDR_DATA(pregs->Flags,fp,n);
		SAVE_HDR_DATA(pregs->PtrStack,fp,n);
	}
	lo = (mGraphDispActive ? 1 : 0);
	SAVE_HDR_DATA(lo,fp,n);
	lo = (unsigned char) (GetGraphDispAddr() & 0x00FF);
	hi = (unsigned char) ((GetGraphDispAddr() & 0xFF00) >> 8);
	SAVE_HDR_DATA(lo,fp,n);
	SAVE_HDR_DATA(hi,fp,n);		
	// fill up the unused slots of header data with 0-s
	for (int i = n; i > 0; i--) fputc(0, fp);
}

/*
 *--------------------------------------------------------------------
 * Method:		SaveSnapshot()
 * Purpose:		Save current state of the VM and memory image.
 * Arguments: String - file name.
 * Returns:		int, 0 if successful, greater then 0 if not (# of bytes
 *            not written).
 *--------------------------------------------------------------------
 */
int VMachine::SaveSnapshot(string fname)
{
	FILE *fp = NULL;
	int ret = MAX_8BIT_ADDR+1;

	if ((fp = fopen(fname.c_str(), "wb")) != NULL) {
		SaveHdrData(fp);
		for (int addr = 0; addr < MAX_8BIT_ADDR+1; addr++) {
			if (addr != mCharIOAddr && addr != mCharIOAddr+1) {
				unsigned char b = mpRAM->Peek8bitImg((unsigned short)addr);
				if (EOF != fputc(b, fp)) ret--;
				else break;
			} else {
				if (EOF != fputc(0, fp)) ret--;
				else break;
			}
		}
		fclose(fp);
	}
	if (0 != ret) mError = VMERR_SAVE_SNAPSHOT;
	if (mDebugTraceActive && ret) {
		stringstream sserr;
		string msg, strerr;
		sserr << ret;
		sserr >> strerr;
		msg = "ERROR: SaveSnapshot, error code: " + strerr;
		AddDebugTrace(msg);
	}		

	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		LoadRAMBin()
 * Purpose:		Load data from binary image file to the memory.
 * Arguments:	ramfname - name of the RAM file definition
 * Returns:		int - error code
 *            MEMIMGERR_OK - OK
 * 						MEMIMGERR_RAMBIN_EOF
 *             - WARNING: Unexpected EOF (image shorter than 64kB).
 *						MEMIMGERR_RAMBIN_OPEN
 *             - WARNING: Unable to open memory image file.
 *						MEMIMGERR_RAMBIN_HDR
 *             - WARNING: Problem with binary image header.
 *						MEMIMGERR_RAMBIN_NOHDR
 *             - WARNING: No header found in binary image.
 *						MEMIMGERR_RAMBIN_HDRANDEOF
 *             - WARNING: Problem with binary image header and
 *                        Unexpected EOF (image shorter than 64kB).
 *						MEMIMGERR_RAMBIN_NOHDRANDEOF
 *             - WARNING: No header found in binary image and
 *                        Unexpected EOF (image shorter than 64kB).
 * TO DO:
 *  - Add fixed size header to binary image with emulator
 *    configuration data. Presence of the header will be detected
 *    by magic key at the beginning. Header should also include
 *    snapshot info, so the program can continue from the place
 *    where it was frozen/saved.
 *--------------------------------------------------------------------
 */
int VMachine::LoadRAMBin(string ramfname)
{
	FILE *fp = NULL;
	unsigned short addr = 0x0000;
	int n = 0;
	Memory *pm = mpRAM;
	int ret = MEMIMGERR_RAMBIN_OPEN;

	AddDebugTrace("LoadRAMBin : " + ramfname);
	mOldStyleHeader = false;	
	if ((fp = fopen(ramfname.c_str(), "rb")) != NULL) {
		if (HasHdrData(fp) || (mOldStyleHeader = HasOldHdrData(fp))) {
			ret = (LoadHdrData(fp) ? MEMIMGERR_OK : MEMIMGERR_RAMBIN_HDR);
		} else {
			ret = MEMIMGERR_RAMBIN_NOHDR;
			rewind(fp);
		}
		// temporarily disable emulation facilities to allow
		// proper memory image initialization
		bool tmp1 = mCharIOActive, tmp2 = mpRAM->IsROMEnabled();
		DisableCharIO();
		DisableROM();
		while (0 == feof(fp) && 0 == ferror(fp)) {
			unsigned char val = fgetc(fp);
			pm->Poke8bitImg(addr, val);
			addr++; n++;
		}
		fclose(fp);
		// restore emulation facilities status
		if (tmp1) SetCharIO(mCharIOAddr, false);
		if (tmp2) EnableROM();
		if (n <= 0xFFFF) {
			switch (ret) {

				case MEMIMGERR_OK: 
					ret = MEMIMGERR_RAMBIN_EOF; 
					break;

				case MEMIMGERR_RAMBIN_HDR: 
					ret = MEMIMGERR_RAMBIN_HDRANDEOF; 
					break;

				case MEMIMGERR_RAMBIN_NOHDR: 
					ret = MEMIMGERR_RAMBIN_NOHDRANDEOF; 
					break;

				default: break;
			}
		}
	}
	mError = ret;

	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		LoadRAMHex()
 * Purpose:		Load data from Intel HEX file format to memory.
 * Arguments:	hexfname - name of the HEX file
 * Returns:		int, MEMIMGERR_OK if OK, otherwise error code:
 *               MEMIMGERR_INTELH_OPEN - unable to open file
 *							 MEMIMGERR_INTELH_SYNTAX - syntax error
 *							 MEMIMGERR_INTELH_FMT - hex format error
 *--------------------------------------------------------------------
 */
int VMachine::LoadRAMHex(string hexfname)
{
		char line[256] = {0};
		FILE *fp = NULL;
		int ret = 0;
		unsigned int addr = 0;

		bool tmp1 = mCharIOActive, tmp2 = mpRAM->IsROMEnabled();
		DisableCharIO();
		DisableROM();
		if ((fp = fopen(hexfname.c_str(), "r")) != NULL) {
			while (0 == feof(fp) && 0 == ferror(fp)) {
				line[0] = '\0';
				fgets(line, 256, fp);
				if (line[0] == ':') {
					if (0 == strcmp(line, HEXEOF)) {
						break;	// EOF, we are done here.
					}
					char blen[3] = {0,0,0};
					char baddr[5] = {0,0,0,0,0};
					char brectype[3] = {0,0,0};
					blen[0] = line[1];
					blen[1] = line[2];
					blen[2] = 0;
					baddr[0] = line[3];
					baddr[1] = line[4];
					baddr[2] = line[5];
					baddr[3] = line[6];
					baddr[4] = 0;
					brectype[0] = line[7];
					brectype[1] = line[8];
					brectype[2] = 0;
					unsigned int reclen = 0, rectype = 0;
					sscanf(blen, "%02x", &reclen);
					sscanf(baddr, "%04x", &addr);
					sscanf(brectype, "%02x", &rectype);
					if (reclen == 0 && rectype == 1) break;	// EOF, we are done here.
					if (rectype != 0) continue;	// not a data record, next!
					for (unsigned int i=9; i<reclen*2+9; i+=2,addr++) {
						if (i>=strlen(line)-3) {
							ret = MEMIMGERR_INTELH_FMT;	// hex format error
							break;
						}
						char dbuf[3] = {0,0,0};
						unsigned int byteval = 0;
						Memory *pm = mpRAM;
						dbuf[0] = line[i];
						dbuf[1] = line[i+1];
						dbuf[2] = 0;
						sscanf(dbuf, "%02x", &byteval);
						pm->Poke8bitImg(addr, (unsigned char)byteval&0x00FF);
					}
				} else {
					ret = MEMIMGERR_INTELH_SYNTAX;	// syntax error
					break;
				}
			}
			fclose(fp);
		} else {
			ret = MEMIMGERR_INTELH_OPEN;	// unable to open file
		}
		if (tmp1) SetCharIO(mCharIOAddr, false);
		if (tmp2) EnableROM();					

		mError = ret;

		return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		LoadRAMDef()
 * Purpose:		Load RAM from VM65 memory definition file.
 * Arguments:	memfname - file name
 * Returns:		int - error code.
 *--------------------------------------------------------------------
 */
int VMachine::LoadRAMDef(string memfname)
{
	return LoadMEM(memfname, mpRAM);
}

/*
 *--------------------------------------------------------------------
 * Method:		LoadMEM()
 * Purpose:		Load data from VM65 memory definition file to the
 *            provided memory device.
 * Arguments: memfname - name of memory definition file
 *						pmem - pointer to memory object
 * Returns:		int - error code
 * Details:
 *    Format of the memory definition file:
 * [; comment]
 * [ADDR
 * address]
 * [data]
 * [ORG
 * address]
 * [data]
 * [IOADDR
 * address]
 * [ROMBEGIN
 * address]
 * [ROMEND
 * address]
 * [ENIO]
 * [ENROM]
 * [EXEC
 * addrress]
 * [ENGRAPH]
 * [GRAPHADDR
 * address]
 * [RESET]
 *
 * Where:
 * [] - optional token
 * ADDR - label indicating that starting address will follow in next
 *        line, it also defines run address
 * ORG - label indicating that the address counter will change to the
 *       value provided in next line
 * IOADDR - label indicating that char IO trap address will be defined
 *          in the next line
 * ROMBEGIN - label indicating that ROM begin address will be defined
 *            in the next line
 * ROMEND - label indicating that ROM end address will be defined
 *          in the next line
 * ENIO - label enabling char IO emulation
 * ENROM - label enabling ROM emulation
 * EXEC - label enabling auto-execute of code, address follows in the
 *        next line
 * ENGRAPH - enable generic graphics display device emulation with
 *           default base address
 * GRAPHADDR - label indicating that base address for generic graphics
 *             display device will follow in next line,
 *             also enables generic graphics device emulation, but
 *             with the customized base address
 * RESET     - initiate CPU reset sequence after loading memory definition file
 * address - decimal or hexadecimal (prefix $) address in memory
 * E.g:
 * ADDR
 * $200
 * 
 * or
 *
 * ADDR
 * 512
 * 
 * changes the default start address (256) to 512.
 *
 * ORG
 * 49152
 *
 * moves address counter to address 49152, following data will be
 * loaded from that address forward
 *
 * data - the multi-line stream of decimal of hexadecimal ($xx) values
 *        of size unsigned char (byte: 0-255) separated with spaces
 *        or commas. 
 * E.g.: 
 * $00 $00 $00 $00
 * $00 $00 $00 $00
 *
 * or
 *
 * $00,$00,$00,$00
 *
 * or
 *
 * 0 0 0 0
 *
 * or
 *
 * 0,0,0,0
 * 0 0 0 0 
 *--------------------------------------------------------------------
 */
int VMachine::LoadMEM(string memfname, Memory *pmem)
{
	FILE *fp = NULL;
	char line[256] = "\0";
	int lc = 0, errc = 0;
	unsigned short addr = 0, rombegin = 0, romend = 0;
	unsigned int nAddr, graphaddr = GRDISP_ADDR;
	bool enrom = false, enio = false, runset = false;
	bool ioset = false, execset = false, rombegset = false;
	bool romendset = false, engraph = false, graphset = false;
	Memory *pm = pmem;
	int err = MEMIMGERR_OK;

	if (mDebugTraceActive) {
		string msg = "LoadMEM: " + memfname;
		AddDebugTrace(msg);
	}
	if ((fp = fopen(memfname.c_str(), "r")) != NULL) {
		DisableROM();
		DisableCharIO();
		while (0 == feof(fp) && 0 == ferror(fp))
		{
			line[0] = '\0';			
			fgets(line, 256, fp);
			lc++;
			// change run address (can be done only once)
			if (0 == strncmp(line, "ADDR", 4)) {
				line[0] = '\0';
				fgets(line, 256, fp);
				lc++;
				if (!runset) {
					if (*line == '$') {
						sscanf(line+1, "%04x", &nAddr);
						addr = nAddr;
					} else {
						addr = (unsigned short) atoi(line);				
					}
					mRunAddr = addr;
					runset = true;
				} else {
					err = MEMIMGERR_VM65_IGNPROCWRN;
					errc++;
					ADD_DBG_LOADMEM(lc," WARNING: Run address was already set. Ignoring...");
					//cout << "LINE #" << dec << lc << " WARNING: Run address was already set. Ignoring..." << endl;
				}
				ADD_DBG_LDMEMPARHEX("ADDR",addr);
				continue;
			}
			// change address counter
			if (0 == strncmp(line, "ORG", 3)) {
				line[0] = '\0';
				fgets(line, 256, fp);
				lc++;
				if (*line == '$') {
					sscanf(line+1, "%04x", &nAddr);
					addr = nAddr;
				} else {
					addr = (unsigned short) atoi(line);				
				}	
				ADD_DBG_LDMEMPARHEX("ORG",addr);
				continue;
			}
			// define I/O emulation address (once)
			if (0 == strncmp(line, "IOADDR", 6)) {
				line[0] = '\0';
				fgets(line, 256, fp);
				lc++;
				if (!ioset) {
					if (*line == '$') {
						sscanf(line+1, "%04x", &nAddr);
						mCharIOAddr = nAddr;
					} else {
						mCharIOAddr = (unsigned short) atoi(line);				
					}	
					ioset = true;
				} else {
					err = MEMIMGERR_VM65_IGNPROCWRN;
					errc++;
					ADD_DBG_LOADMEM(lc," WARNING: I/O address was already set. Ignoring...");
					//cout << "LINE #" << dec << lc << " WARNING: I/O address was already set. Ignoring..." << endl;
				}
				ADD_DBG_LDMEMPARHEX("IOADDR",mCharIOAddr);
				continue;
			}
			// define generic graphics display device base address (once)
			if (0 == strncmp(line, "GRAPHADDR", 9)) {
				line[0] = '\0';
				fgets(line, 256, fp);
				lc++;
				if (!graphset) {
					if (*line == '$') {
						sscanf(line+1, "%04x", &nAddr);
						graphaddr = nAddr;
					} else {
						graphaddr = (unsigned short) atoi(line);				
					}	
					graphset = true;
				} else {
					err = MEMIMGERR_VM65_IGNPROCWRN;
					errc++;
					ADD_DBG_LOADMEM(lc," WARNING: graphics device base address was already set. Ignoring...");
					//cout << "LINE #" << dec << lc << " WARNING: graphics device base address was already set. Ignoring..." << endl;
				}
				ADD_DBG_LDMEMPARHEX("GRAPHADDR",graphaddr);
				continue;
			}			
			// enable character I/O emulation
			if (0 == strncmp(line, "ENIO", 4)) {
				enio = true;
				ADD_DBG_LDMEMPARVAL("ENIO",enio);
				continue;
			}
			// enable generic graphics display emulation
			if (0 == strncmp(line, "ENGRAPH", 7)) {
				engraph = true;
				ADD_DBG_LDMEMPARVAL("ENIO",engraph);
				continue;
			}			
			// enable ROM emulation
			if (0 == strncmp(line, "ENROM", 5)) {
				enrom = true;
				ADD_DBG_LDMEMPARVAL("ENROM",enrom);
				continue;
			}			
			// auto execute from address
			if (0 == strncmp(line, "EXEC", 4)) {
				mAutoExec = true;
				line[0] = '\0';
				fgets(line, 256, fp);
				lc++;
				if (!execset) {
					if (*line == '$') {
						sscanf(line+1, "%04x", &nAddr);
						mRunAddr = nAddr;
					} else {
						mRunAddr = (unsigned short) atoi(line);				
					}	
					execset = true;
				} else {
					err = MEMIMGERR_VM65_IGNPROCWRN;
					errc++;
					ADD_DBG_LOADMEM(lc," WARNING: auto-exec address was already set. Ignoring...");
					//cout << "LINE #" << dec << lc << " WARNING: auto-exec address was already set. Ignoring..." << endl;
				}
				ADD_DBG_LDMEMPARHEX("EXEC",mRunAddr);
				continue;
			}
			// auto reset
			if (0 == strncmp(line, "RESET", 5)) {
				mAutoReset = true;
				ADD_DBG_LDMEMPARVAL("RESET",mAutoReset);
				continue;
			}			
			// define ROM begin address
			if (0 == strncmp(line, "ROMBEGIN", 8)) {
				line[0] = '\0';
				fgets(line, 256, fp);
				lc++;
				if (!rombegset) {
					if (*line == '$') {
						sscanf(line+1, "%04x", &nAddr);
						rombegin = nAddr;
					} else {
						rombegin = (unsigned short) atoi(line);
					}	
					rombegset = true;
				} else {
					err = MEMIMGERR_VM65_IGNPROCWRN;
					errc++;
					ADD_DBG_LOADMEM(lc," WARNING: ROM-begin address was already set. Ignoring...");
					//cout << "LINE #" << dec << lc << " WARNING: ROM-begin address was already set. Ignoring..." << endl;
				}
				ADD_DBG_LDMEMPARHEX("ROMBEGIN",rombegin);
				continue;
			}
			// define ROM end address
			if (0 == strncmp(line, "ROMEND", 6)) {
				line[0] = '\0';
				fgets(line, 256, fp);
				lc++;
				if (!romendset) {
					if (*line == '$') {
						sscanf(line+1, "%04x", &nAddr);
						romend = nAddr;
					} else {
						romend = (unsigned short) atoi(line);
					}	
					romendset = true;
				} else {
					err = MEMIMGERR_VM65_IGNPROCWRN;
					errc++;
					ADD_DBG_LOADMEM(lc," WARNING: ROM-end address was already set. Ignoring...");
					//cout << "LINE #" << dec << lc << " WARNING: ROM-end address was already set. Ignoring..." << endl;
				}
				ADD_DBG_LDMEMPARHEX("ROMEND",romend);
				continue;
			}			
			if (';' == *line) continue; // skip comment lines
			char *s = strtok (line, " ,");
			while (NULL != s) {
				unsigned int nVal;
				if (*s == '$') {
					sscanf(s+1, "%02x", &nVal);
					pm->Poke8bitImg(addr++, (unsigned short)nVal);
				} else {
					pm->Poke8bitImg(addr++, (unsigned short)atoi(s));
				}
				s = strtok(NULL, " ,");
			}
		}
		fclose(fp);
		if (rombegin > MIN_ROM_BEGIN && romend > rombegin) {
			if (enrom)
				pm->EnableROM(rombegin, romend);
			else
				pm->SetROM(rombegin, romend);
		} else {
			if (enrom) EnableROM();
		}
		if (enio) {
			SetCharIO(mCharIOAddr, false);
		}
		if (engraph || graphset) {
			SetGraphDisp(graphaddr);
		}
	}
	else {
		err = MEMIMGERR_VM65_OPEN;
		cout << "WARNING: Unable to open memory definition file: ";
		cout << memfname << endl;
		errc++;
	}	
	if (errc) {
		cout << "Found " << dec << errc << ((errc > 1) ? " problems." : " problem.") << endl;
		cout << "Press [ENTER] to continue...";
		getchar();		
	}	

	mError = err;

	return err;
}

/*
 *--------------------------------------------------------------------
 * Method:		MemPeek8bit()
 * Purpose:		Read value from specified RAM address.
 * Arguments:	addr - RAM address (0..0xFFFF)
 * Returns:		unsigned short - value read from specified RAM address
 *--------------------------------------------------------------------
 */
unsigned short VMachine::MemPeek8bit(unsigned short addr)
{
	unsigned short ret = 0;
	
	ret = (unsigned short)mpRAM->Peek8bit(addr);
	
	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		MemPoke8bit()
 * Purpose:		Write value to specified RAM address.
 * Arguments:	addr - RAM address (0..0xFFFF)
 *            v - 8-bit byte value
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::MemPoke8bit(unsigned short addr, unsigned char v)
{
	mpRAM->Poke8bit(addr, v);
}

/*
 *--------------------------------------------------------------------
 * Method:		GetRegs()
 * Purpose:		Return pointer to CPU status register.
 * Arguments:	n/a
 * Returns:		pointer to status register
 *--------------------------------------------------------------------
 */
Regs *VMachine::GetRegs()
{
	return mpCPU->GetRegs();
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
void VMachine::SetCharIO(unsigned short addr, bool echo)
{
	mCharIOAddr = addr;
	mCharIOActive = true;
	mpRAM->SetCharIO(addr, echo);
	mpDisp = mpRAM->GetMemMapDevPtr()->GetDispPtr();
	if (mDebugTraceActive) {
		string msg;
		msg = "Char I/O activated at: $" + Addr2HexStr(addr) + ", echo: "
					+ ((echo) ? "ON" : "OFF");
		AddDebugTrace(msg);
	}
}

/*
 *--------------------------------------------------------------------
 * Method:		DisableCharIO()
 * Purpose:		Deactivates basic character I/O emulation (console).
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::DisableCharIO()
{
	mCharIOActive = false;
	mpRAM->DisableCharIO();
	AddDebugTrace("Char I/O DISABLED");
	//mpRAM->GetMemMapDevPtr()->DeactivateCharIO();
}

/*
 *--------------------------------------------------------------------
 * Method:		GetCharIOAddr()
 * Purpose:		Returns current address of basic character I/O area.
 * Arguments:	n/a
 * Returns:		address of I/O area
 *--------------------------------------------------------------------
 */
unsigned short VMachine::GetCharIOAddr()
{
	return mCharIOAddr;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetCharIOActive()
 * Purpose:		Returns status of character I/O emulation.
 * Arguments:	n/a
 * Returns:		true if I/O emulation active
 *--------------------------------------------------------------------
 */
bool VMachine::GetCharIOActive()
{
	return mCharIOActive;
}

/*
 *--------------------------------------------------------------------
 * Method:		SetGraphDisp()
 * Purpose:		Set graphics device address and enable.
 * Arguments:	addr - unsigned short : device base address.
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::SetGraphDisp(unsigned short addr)
{
	mGraphDispActive = true;
	mpRAM->SetGraphDisp(addr);
	if (mDebugTraceActive) {
		string msg;
		msg = "Graphics Device set at: $" + Addr2HexStr(addr) + ".";
		AddDebugTrace(msg);
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
void VMachine::DisableGraphDisp()
{
	mGraphDispActive = false;
	mpRAM->DisableGraphDisp();
	AddDebugTrace("Graphics Device DISABLED.");
}

/*
 *--------------------------------------------------------------------
 * Method:		GetGraphDispAddr()
 * Purpose:		Return base address of graphics display.
 * Arguments:	n/a
 * Returns:		unsigned short - address ($0000 - $FFFF)
 *--------------------------------------------------------------------
 */
unsigned short VMachine::GetGraphDispAddr()
{
	return mpRAM->GetGraphDispAddr();
}

/*
 *--------------------------------------------------------------------
 * Method:		GetGraphDispActive()
 * Purpose:		Returns status of graphics display emulation.
 * Arguments:	n/a
 * Returns:		true if graphics display emulation is active
 *--------------------------------------------------------------------
 */
bool VMachine::GetGraphDispActive()
{
	return mGraphDispActive;
}

/*
 *--------------------------------------------------------------------
 * Method:		ShowIO()
 * Purpose:		Show contents of emulated char I/O.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::ShowIO()
{
	if (mCharIOActive && NULL != mpDisp)
		mpDisp->ShowScr();
}

/*
 *--------------------------------------------------------------------
 * Method:		IsAutoExec()
 * Purpose:		Return status of auto-execute flag.
 * Arguments:	n/a
 * Returns:		bool - true if auto-exec flag is enabled.
 *--------------------------------------------------------------------
 */
bool VMachine::IsAutoExec()
{
	return mAutoExec;
}

/*
 *--------------------------------------------------------------------
 * Method:		IsAutoReset()
 * Purpose:		Return status of auto-reset flag.
 * Arguments:	n/a
 * Returns:		bool - true if auto-exec flag is enabled.
 *--------------------------------------------------------------------
 */
bool VMachine::IsAutoReset()
{
	return mAutoReset;
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::EnableROM()
{
	mpRAM->EnableROM();
	AddDebugTrace("ROM ENABLED.");
}
		
/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */		
void VMachine::DisableROM()
{
	mpRAM->DisableROM();
	AddDebugTrace("ROM DISABLED.");
}
		
/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */		
void VMachine::SetROM(unsigned short start, unsigned short end)
{
	mpRAM->SetROM(start, end);
	if (mDebugTraceActive) {
		string msg;
		msg = "ROM SET, Start: $" + Addr2HexStr(start) + ", End: $";
		msg += Addr2HexStr(end) + ".";		
		AddDebugTrace(msg);
	}	
}
		
/*
 *--------------------------------------------------------------------
 * Method:		EnableROM()
 * Purpose:		Sets and enables Read Only Memory range.
 * Arguments:	start, end - unsigned short : start and end addresses
 *												 of the ROM.
 * Returns:		n/a
 *--------------------------------------------------------------------
 */		
void VMachine::EnableROM(unsigned short start, unsigned short end)
{
	mpRAM->EnableROM(start, end);
	if (mDebugTraceActive) {
		string msg;
		msg = "ROM ENABLED, Start: $" + Addr2HexStr(start) + ", End: $";
		msg += Addr2HexStr(end) + ".";		
		AddDebugTrace(msg);
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
unsigned short VMachine::GetROMBegin()
{
	return mpRAM->GetROMBegin();
}
		
/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */		
unsigned short VMachine::GetROMEnd()
{
	return mpRAM->GetROMEnd();
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */		
bool VMachine::IsROMEnabled()
{
	return mpRAM->IsROMEnabled();
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */		
unsigned short VMachine::GetRunAddr()
{
	return mRunAddr;
}

/*
 *--------------------------------------------------------------------
 * Method:		SetOpInterrupt()
 * Purpose:		Set the flag indicating operator interrupt.
 * Arguments:	bool - new value of the flag
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::SetOpInterrupt(bool opint)
{
	mOpInterrupt = opint;
}

/*
 *--------------------------------------------------------------------
 * Method:		IsOpInterrupt()
 * Purpose:		Return the flag indicating operator interrupt status.
 * Arguments:	n/a
 * Returns:		bool - true if operator interrupt flag was set
 *--------------------------------------------------------------------
 */
bool VMachine::IsOpInterrupt()
{
	return mOpInterrupt;
}

/*
 *--------------------------------------------------------------------
 * Method:		GetExecHistory()
 * Purpose:		Return history of executed opcodes (last 20).
 * Arguments:	n/a
 * Returns:		queue<string>
 *--------------------------------------------------------------------
 */
queue<string> VMachine::GetExecHistory()
{
	return mpCPU->GetExecHistory();
}

/*
 *--------------------------------------------------------------------
 * Method:		Disassemble()
 * Purpose:		Disassemble code in memory. Return next instruction
 *            address.
 * Arguments:	addr - address in memory
 *            buf - character buffer for disassembled instruction
 * Returns:		unsigned short - address of next instruction
 *--------------------------------------------------------------------
 */
unsigned short VMachine::Disassemble(unsigned short addr, char *buf)
{
	return mpCPU->Disassemble(addr, buf);
}

/*
 *--------------------------------------------------------------------
 * Method:		Reset()
 * Purpose:		Reset VM and CPU (should never return except operator
 *            induced interrupt).
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void VMachine::Reset()
{
	mpCPU->Reset();
	Exec(mpCPU->GetRegs()->PtrAddr);
	mpCPU->mExitAtLastRTS = true;
	AddDebugTrace("*** CPU RESET ***");
}

/*
 *--------------------------------------------------------------------
 * Method:		Interrupt()
 * Purpose:		Send Interrupt ReQuest to CPU (set the IRQ line LOW).
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void VMachine::Interrupt()
{
	mpCPU->Interrupt();
}

/*
 *--------------------------------------------------------------------
 * Method:		GetLastError()
 * Purpose:		Return error code set by last operation. Reset error
 *            code to OK.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
int VMachine::GetLastError()
{
	int ret = mError;
	mError = MEMIMGERR_OK;
	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::EnableExecHistory(bool enexehist)
{
	mpCPU->EnableExecHistory(enexehist);
	if (mDebugTraceActive) {
		string msg;
		msg = "The op-code execute history " 
					+ (string)((enexehist) ? "ENABLED" : "DISABLED");
		msg += ".";
		AddDebugTrace(msg);
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
bool VMachine::IsExecHistoryActive()
{
	return mpCPU->IsExecHistoryEnabled();
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::EnableDebugTrace()
{
	if (!mDebugTraceActive) {
		mDebugTraceActive = true;
		while (!mDebugTraces.empty()) mDebugTraces.pop();
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
void VMachine::DisableDebugTrace()
{
	mDebugTraceActive = false;
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
bool VMachine::IsDebugTraceActive()
{
	return mDebugTraceActive;
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
queue<string> VMachine::GetDebugTraces()
{
	return mDebugTraces;
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::EnablePerfStats()
{
	if (!mPerfStatsActive) {
		mPerfStatsActive = true;
		mPerfStats.cycles = 0;
		mPerfStats.prev_cycles = 0;
		mPerfStats.prev_usec = 0;
		mPerfStats.perf_onemhz = 0;
		AddDebugTrace("Performance stats ENABLED.");
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
void VMachine::DisablePerfStats()
{
	mPerfStatsActive = false;
	AddDebugTrace("Performance stats DISABLED.");
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
bool VMachine::IsPerfStatsActive()
{
	return mPerfStatsActive;
}

/*
 *--------------------------------------------------------------------
 * Method:		AddDebugTrace()
 * Purpose:		Add string to the debug messages queue.  String is
 *            prefixed with time stamp (number of microseconds since
 *            start of program. Queue is maintained to not exceed
 *            DBG_TRACE_SIZE. If the size is exceeded with the next
 *            added debug message, the first one in the queue is
 *            deleted (FIFO).
 * Arguments:	msg - string : debug message.
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::AddDebugTrace(string msg)
{
	if (mDebugTraceActive) {
		stringstream ss;
		string mmsg;

		// add timestamp in front (micro seconds)
		auto lap = high_resolution_clock::now();
		unsigned long usec = duration_cast<microseconds>(lap-mBeginTime).count();
		ss << usec;
		ss >> mmsg;
		mmsg += " : " + msg;
		mDebugTraces.push(mmsg);
		while (mDebugTraces.size() > DBG_TRACE_SIZE) mDebugTraces.pop();
	}
}

/*
 *--------------------------------------------------------------------
 * Method:		Addr2HexStr()
 * Purpose:   Convert 16-bit address to a hex notation string.
 * Arguments: addr - 16-bit unsigned
 * Returns:   string
 *--------------------------------------------------------------------
 */
string VMachine::Addr2HexStr(unsigned short addr)
{
	stringstream ss;
	string ret;

	ss << hex << addr;
	ss >> ret;

	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		Addr2DecStr()
 * Purpose:   Convert 16-bit address to a decimal notation string.
 * Arguments: addr - 16-bit unsigned
 * Returns:   string
 *--------------------------------------------------------------------
 */
string VMachine::Addr2DecStr(unsigned short addr)
{
	stringstream ss;
	string ret;
	
	ss << addr;
	ss >> ret;

	return ret;
}

} // namespace MKBasic

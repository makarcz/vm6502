/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:   			VMachine.h
 *
 * Purpose: 		Prototype of VMachine class and all supporting data
 *							structures, enumerations, constants and macros.
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
#ifndef VMACHINE_H
#define VMACHINE_H

#include <string>
#include <queue>
#include <chrono>
#include "system.h"
#include "MKCpu.h"
#include "Memory.h"
#include "Display.h"
#include "ConsoleIO.h"

//#define WINDOWS 1
#if defined (WINDOWS)
#include <windows.h>
#endif

#define IOREFRESH 32
#define OPINTERRUPT 25	// operator interrupt code (CTRL-Y)
#define HDRMAGICKEY "SNAPSHOT2"
#define HDRMAGICKEY_OLD "SNAPSHOT"
#define HDRDATALEN	128
#define HDRDATALEN_OLD	15
#define HEXEOF	":00000001FF"
// take emulation speed measurement every 2 minutes (120,000,000 usec)
#define PERFSTAT_INTERVAL	120000000
// but not more often than 30,000,000 clock ticks
#define PERFSTAT_CYCLES 30000000
#define DBG_TRACE_SIZE	200	// maximum size of debug messages queue

using namespace std;
using namespace chrono;

// Macros for debug log.
#define ADD_DBG_LOADMEM(lc,txt) \
	if (mDebugTraceActive)	\
	{	\
		stringstream ss;	\
		string msg, s;		\
		ss << lc;					\
		ss >> s;					\
		msg = "LINE #" + s + txt;	\
		AddDebugTrace(msg);	\
	}

#define ADD_DBG_LDMEMPARHEX(name,value) \
	if (mDebugTraceActive)	\
	{	\
		string msg;						\
		msg = (string)name + " = $" + Addr2HexStr(value);	\
		AddDebugTrace(msg);		\
	}	

#define ADD_DBG_LDMEMPARVAL(name,value) \
	if (mDebugTraceActive)	\
	{	\
		string msg;							\
		msg = (string)name + " = " + Addr2DecStr(value);	\
		AddDebugTrace(msg);			\
	}	

// Macro to save header data: v - value, fp - file pointer, n - data counter (dec)
#define SAVE_HDR_DATA(v,fp,n) {fputc(v, fp); n--;}	

namespace MKBasic {

// Types of memory image definition file.
enum eMemoryImageTypes {
	MEMIMG_UNKNOWN = 0,
	MEMIMG_VM65DEF,
	MEMIMG_INTELHEX,
	MEMIMG_BIN
};

// Types of memory image load errors
enum eMemImgLoadErrors {
	MEMIMGERR_OK = 0,							// all is good
	// binary format
	MEMIMGERR_RAMBIN_OPEN,				// unable to open file
	MEMIMGERR_RAMBIN_EOF,					// unexpected EOF (image shorter then 64 kB)
	MEMIMGERR_RAMBIN_HDR,					// header problem
	MEMIMGERR_RAMBIN_NOHDR,				// no header found
	MEMIMGERR_RAMBIN_HDRANDEOF,		// header problem and unexpected EOF
	MEMIMGERR_RAMBIN_NOHDRANDEOF,	// header not found and unexoected EOF
	// Intel HEX format
	MEMIMGERR_INTELH_OPEN, 				// unable to open file
	MEMIMGERR_INTELH_SYNTAX,			// syntax error
	MEMIMGERR_INTELH_FMT,					// format error
	// VM65 memory definition
	MEMIMGERR_VM65_OPEN,					// unable to open file
	MEMIMGERR_VM65_IGNPROCWRN,		// processing warnings (ignored, not critical)
	//-------------------------------------------------------------------------
	MEMIMGERR_UNKNOWN
};

// Types of other errors
enum eVMErrors {
	VMERR_OK = 0,																// all is good
	VMERR_SAVE_SNAPSHOT = MEMIMGERR_UNKNOWN+1,	// problem saving memory image
																							// snapshot
	//-------------------------------------------------------------------------
	VMERR_UNKNOWN																// unknown error
};

struct PerfStats {
	time_point<high_resolution_clock> 
			 begin_time;				// the moment of time count start
	long cycles;						// performance stats
	long prev_cycles;				// previously measured stats
	long prev_usec;					// previously measured stats
	int  perf_onemhz;				// avg. % perf. based on 1MHz CPU.
};

class VMachine
{
	public:
		VMachine();
		VMachine(string romfname, string ramfname);
		~VMachine();
		
		void InitVM();
		Regs *Run();
		Regs *Run(unsigned short addr);
		Regs *Exec();
		Regs *Exec(unsigned short addr);		
		Regs *Step();
		Regs *Step(unsigned short addr);
		void LoadROM(string romfname);
		int  LoadRAM(string ramfname);
		int  LoadRAMBin(string ramfname);
		int  LoadRAMHex(string hexfname);
		int  LoadRAMDef(string memfname);
		unsigned short MemPeek8bit(unsigned short addr);
		void MemPoke8bit(unsigned short addr, unsigned char v);
		Regs *GetRegs();
		void SetCharIO(unsigned short addr, bool echo);
		void DisableCharIO();
		unsigned short GetCharIOAddr();
		bool GetCharIOActive();
		bool GetGraphDispActive();
		void ShowIO();
		void ClearScreen();
		void ScrHome();
		bool IsAutoExec();
		bool IsAutoReset();
		void EnableROM();
		void DisableROM();
		void SetROM(unsigned short start, unsigned short end);
		void EnableROM(unsigned short start, unsigned short end);
		unsigned short GetROMBegin();
		unsigned short GetROMEnd();
		bool IsROMEnabled();
		unsigned short GetRunAddr();		
		void SetOpInterrupt(bool opint);
		bool IsOpInterrupt();
		queue<string> GetExecHistory();
		unsigned short Disassemble(unsigned short addr, char *buf);
		void Reset();
		void Interrupt();
		int SaveSnapshot(string fname);
		int GetLastError();
		void SetGraphDisp(unsigned short addr);
		void DisableGraphDisp();
		unsigned short GetGraphDispAddr();
		PerfStats GetPerfStats();	// returns performance stats based on 1 million
															// cycles per second (1 MHz CPU).
		void EnableExecHistory(bool enexehist);
		bool IsExecHistoryActive();
		void EnableDebugTrace();
		void DisableDebugTrace();
		bool IsDebugTraceActive();
		void EnablePerfStats();
		void DisablePerfStats();
		bool IsPerfStatsActive();
		queue<string> GetDebugTraces();

		
	protected:
		
	private:
		
		MKCpu		*mpCPU;			// object maintained locally
		Memory	*mpROM;			// object maintained locally
		Memory	*mpRAM;			// object maintained locally
		Display	*mpDisp;		// just a pointer
		ConsoleIO *mpConIO;	// object maintained locally
		unsigned short mRunAddr;
		unsigned short mCharIOAddr;
		bool mCharIOActive;
		bool mCharIO;
		bool mOpInterrupt; // operator interrupt from console
		bool mAutoExec;
		bool mAutoReset;
		int  mError;			 // last error code
		bool mGraphDispActive;
		bool mOldStyleHeader;
		PerfStats mPerfStats;
		queue<string> mDebugTraces;
		bool mPerfStatsActive;
		bool mDebugTraceActive;
		time_point<high_resolution_clock> mBeginTime;
		
		int  LoadMEM(string memfname, Memory *pmem);
		void ShowDisp();
		bool HasHdrData(FILE *fp);
		bool HasOldHdrData(FILE *fp);
		bool LoadHdrData(FILE *fp);
		void SaveHdrData(FILE *fp);
		eMemoryImageTypes GetMemoryImageType(string ramfname);
		int CalcCurrPerf();
		void AddDebugTrace(string msg);
		string Addr2HexStr(unsigned short addr);
		string Addr2DecStr(unsigned short addr);
};

} // namespace MKBasic

#endif

#ifndef VMACHINE_H
#define VMACHINE_H

#include <string>
#include <queue>
#include "system.h"
#include "MKCpu.h"
#include "Memory.h"
#include "Display.h"

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

using namespace std;

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
		
	protected:
		
	private:
		
		MKCpu		*mpCPU;
		Memory	*mpROM;
		Memory	*mpRAM;
		Display	*mpDisp;
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
		
		int  LoadMEM(string memfname, Memory *pmem);
		void ShowDisp();
		bool HasHdrData(FILE *fp);
		bool HasOldHdrData(FILE *fp);
		bool LoadHdrData(FILE *fp);
		void SaveHdrData(FILE *fp);
		eMemoryImageTypes GetMemoryImageType(string ramfname);
};

} // namespace MKBasic

#endif

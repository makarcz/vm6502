#ifndef VMACHINE_H
#define VMACHINE_H

#include <string>
#include "MKCpu.h"
#include "Memory.h"
#include "Display.h"

#define WINDOWS 1
#if defined (WINDOWS)
#include <windows.h>
#endif
#define IOREFRESH 32

using namespace std;

namespace MKBasic {

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
		void LoadRAM(string ramfname);
		unsigned short MemPeek8bit(unsigned short addr);
		void MemPoke8bit(unsigned short addr, unsigned char v);
		Regs *GetRegs();
		void SetCharIO(unsigned short addr, bool echo);
		void DisableCharIO();
		unsigned short GetCharIOAddr();
		bool GetCharIOActive();
		void ShowIO();
		void ClearScreen();
		void ScrHome();
		
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
		
		void LoadMEM(string memfname, Memory *pmem);
		void ShowDisp();
};

} // namespace MKBasic

#endif

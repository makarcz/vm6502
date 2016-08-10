#include <cstdlib>
#include <iostream>
#include <bitset>
#include <chrono>
#include <thread>
#include <string.h>
#include "system.h"
#include "MKCpu.h"
#include "Memory.h"
#include "Display.h"
#include "VMachine.h"
#include "GraphDisp.h"
#include "MemMapDev.h"
#include "MKGenException.h"

using namespace std;
using namespace MKBasic;

#define ANIM_DELAY 250

const bool ClsIfDirty = true;

VMachine *pvm = NULL;
Regs *preg = NULL;
bool ioecho = false, opbrk = false, needhelp = false;
bool loadbin = false, loadhex = false, reset = false, execvm = false;
int g_stackdisp_lines = 1;
string ramfile = "dummy.ram";

bool ShowRegs(Regs *preg, VMachine *pvm, bool ioecho, bool showiostat);
void ShowHelp();
void CmdArgHelp(string prgname);
void CopyrightBanner();

/*
 *--------------------------------------------------------------------
 * Method:		RunSingleInstr()
 * Purpose:		Execute single instruction of the CPU (all cycles).
 * Arguments:	addr - unsigned short, instruction address
 * Returns:		pointer to CPU registers
 *--------------------------------------------------------------------
 */
 Regs *RunSingleInstr(unsigned short addr)
 {
 		Regs *ret = NULL;

 		do {
    	ret = pvm->Step(addr);
  	} while (ret->CyclesLeft > 0);

  	return ret;
 }

 /*
 *--------------------------------------------------------------------
 * Method:		RunSingleCurrInstr()
 * Purpose:		Execute single instruction of the CPU (all cycles)
 *            at current address.
 * Arguments:	n/a
 * Returns:		pointer to CPU registers
 *--------------------------------------------------------------------
 */
 Regs *RunSingleCurrInstr()
 {
 		Regs *ret = NULL;

 		do {
    	ret = pvm->Step();
  	} while (ret->CyclesLeft > 0);

  	return ret;
 }

/*
 *--------------------------------------------------------------------
 * Method:		PrintVMErr()
 * Purpose:		Print the warning/error message.
 * Arguments:	err - integer, error code
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void PrintVMErr(int err)
{
	bool pressenter = true;
	switch (err) {
		case MEMIMGERR_RAMBIN_EOF: 
			cout << "WARNING: Unexpected EOF (image shorter than 64kB).";
			cout << endl;
			break;
		case MEMIMGERR_RAMBIN_OPEN: 
			cout << "WARNING: Unable to open memory image file." << endl;
			break;
		case MEMIMGERR_RAMBIN_HDR: 
			cout << "WARNING: Problem with binary image header." << endl;
			break;
		case MEMIMGERR_RAMBIN_NOHDR: 
			cout << "WARNING: No header found in binary image." << endl;
			break;
		case MEMIMGERR_RAMBIN_HDRANDEOF: 
			cout << "WARNING: Problem with binary image header." << endl;
			cout << "WARNING: Unexpected EOF (image shorter than 64kB).";
			cout << endl;
			break;
		case MEMIMGERR_RAMBIN_NOHDRANDEOF: 
			cout << "WARNING: No header found in binary image." << endl;
			cout << "WARNING: Unexpected EOF (image shorter than 64kB).";
			cout << endl;
			break;
		case MEMIMGERR_INTELH_OPEN: 
			cout << "WARNING: Unable to open Intel HEX file." << endl;
			break;
		case MEMIMGERR_INTELH_SYNTAX: 
			cout << "ERROR: Syntax error." << endl;
			break;
		case MEMIMGERR_INTELH_FMT: 
			cout << "ERROR: Intel HEX format error." << endl;
			break;			
		case MEMIMGERR_VM65_OPEN:
			cout << "ERROR: Unable to open memory definition file.";
			cout << endl;
			break;
		case MEMIMGERR_VM65_IGNPROCWRN:
			cout << "WARNING: There were problems while processing";
			cout << " memory definition file." << endl;
			break;	
		case VMERR_SAVE_SNAPSHOT:
			cout << "WARNING: There was a problem saving memory snapshot.";
			cout << endl;
			break;
		default: pressenter = false; break;
	}	
	if (pressenter) {
		cout << "Press [ENTER]...";
		getchar();
	}
}

#if defined(LINUX)

#include <signal.h>

void trap_signal(int signum);

/*
 *--------------------------------------------------------------------
 * Method:		trap_signal()
 * Purpose:		handle signal
 * Arguments:	signum - signal #
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void trap_signal(int signum)
{
   cout << "Signal caught: " << dec << signum << endl;
   if (NULL != pvm && NULL != preg) {
  		pvm->SetOpInterrupt(true);
  		opbrk = true;
	 }
   //exit(signum);
   return;
}

#endif

#if defined(WINDOWS)
#include <windows.h>

BOOL CtrlHandler(DWORD fdwCtrlType);

/*
 *--------------------------------------------------------------------
 * Method:		CtrlHandler()
 * Purpose:		handle signal
 * Arguments:	fdwCtrlType - event type
 * Returns:		BOOL - TRUE if event handled, FALSE if needs further
 *                   processing.
 *--------------------------------------------------------------------
 */
BOOL CtrlHandler(DWORD fdwCtrlType)
{ 
  switch( fdwCtrlType ) 
  { 
    case CTRL_C_EVENT: 
      //Beep( 750, 300 ); 
    	if (NULL != pvm && NULL != preg) {
    		pvm->SetOpInterrupt(true);
    		opbrk = true;
	  	}      
      return TRUE;
      
 
    case CTRL_CLOSE_EVENT: 
      //Beep( 600, 200 ); 
      cout << "Ctrl-Close event" << endl;
      return TRUE ; 
 
    case CTRL_BREAK_EVENT: 
      //Beep( 900, 200 ); 
    	if (NULL != pvm && NULL != preg) {
				pvm->SetOpInterrupt(true);
				opbrk = true;
	  	}       
      return TRUE; 
 
    case CTRL_LOGOFF_EVENT: 
      //Beep( 1000, 200 ); 
      cout << "Ctrl-Logoff event" << endl;
      return FALSE; 
 
    case CTRL_SHUTDOWN_EVENT: 
      Beep( 750, 500 ); 
      cout << "Ctrl-Shutdown event" << endl;
      return FALSE; 
 
    default: 
      return FALSE; 
  } 
}

#endif

/*
 *--------------------------------------------------------------------
 * Method:		PromptNewAddress()
 * Purpose:		Prompt user to enter 16-bit address (hex) in console.
 * Arguments:	prompt - prompt text
 * Returns:		unsigned int - address entered by user
 *--------------------------------------------------------------------
 */
unsigned int PromptNewAddress(string prompt)
{
	unsigned int newaddr = 0x10000;
	
	while (newaddr > 0xFFFF) {
		cout << prompt;
		cin >> hex >> newaddr;
	}
	
	return newaddr;
}

/*
 *--------------------------------------------------------------------
 * Thank you stackoverflow.com.
 * http://stackoverflow.com/questions/111928/
 * is-there-a-printf-converter-to-print-in-binary-format
 *--------------------------------------------------------------------
 */
#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0)

/*
 *--------------------------------------------------------------------
 * Method:		ShowRegs()
 * Purpose:		Display status of CPU registers on DOS console.
 * Arguments:	preg - pointer to registers structure
 *            pvm - pointer to VM
 *            ioaddr - address setup for char I/O emulation
 *            ioecho - local I/O echo flag
 *            showiostat - if true, I/O emulation status is shown
 * Returns:   boolean - true if the stack pointer was longer than
 *                      15 (the screen must be cleared).
 *--------------------------------------------------------------------
 */
bool ShowRegs(Regs *preg, VMachine *pvm, bool ioecho, bool showiostat)
{
	bool ret = false;
	char sBuf[80] = {0};
	
	sprintf(sBuf, "|  PC: $%04x  |  Acc: $%02x (" BYTETOBINARYPATTERN ")  |  X: $%02x  |  Y: $%02x  |",
								preg->PtrAddr, preg->Acc, BYTETOBINARY(preg->Acc), preg->IndX, preg->IndY);
	cout << "*-------------*-----------------------*----------*----------*" << endl;								
	cout << sBuf << endl;
	cout << "*-------------*-----------------------*----------*----------*" << endl;
	cout << "|  NV-BDIZC   |" << endl;
	cout << "|  " << bitset<8>((int)preg->Flags) << "   |";
	cout << " Last instr.: " << preg->LastInstr << "          " << endl;
	cout << "*-------------*" << endl;
	cout << endl;
	cout << "Stack: $" << hex << (unsigned short)preg->PtrStack << "   " << endl;
	cout << "                                                                               \r";
	// display stack contents
	cout << "       [";
	int j = 0, stacklines = 1;
	for (unsigned int addr = 0x0101 + preg->PtrStack; addr < 0x0200; addr++) {
		unsigned int hv = (unsigned int)pvm->MemPeek8bit(addr);
		if (hv < 16) {
			cout << 0;
		}
		cout << hex << hv << " ";
		j++;
		if (j > 15) {
			cout << "]" << endl;
			cout << "       [";
			j=0;
			stacklines++;
		}
	}
	cout << "]                    " << endl;
	ret = (stacklines < g_stackdisp_lines);
	g_stackdisp_lines = stacklines;
	// end display stack contents

	if (showiostat) {
		cout << endl << "I/O status: " << (pvm->GetCharIOActive() ? "enabled" : "disabled") << ", ";
		cout << " at: $" << hex << pvm->GetCharIOAddr() << ", ";
		cout << " local echo: " << (ioecho ? "ON" : "OFF") << "." << endl;
		cout << "Graphics status: " << (pvm->GetGraphDispActive() ? "enabled" : "disabled") << ", ";
		cout << " at: $" << hex << pvm->GetGraphDispAddr() << endl;
		cout << "ROM: " << ((pvm->IsROMEnabled()) ? "enabled." : "disabled.") << " ";
		cout << "Range: $" << hex << pvm->GetROMBegin() << " - $" << hex << pvm->GetROMEnd() << "." << endl;
	}
	cout << "                                                                               \r";

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
void ShowMenu()
{
	cout << "------------------------------------+----------------------------------------" << endl;
	cout << "   C - continue,  S - step          |    A - set address for next step" << endl;
	cout << "   G - go/cont. from new address    |    N - go number of steps, P - IRQ" << endl;
	cout << "   I - toggle char I/O emulation    |    X - execute from new address" << endl;
	cout << "   T - show I/O console             |    B - blank (clear) screen" << endl;
	cout << "   E - toggle I/O local echo        |    F - toggle registers animation" << endl;
	cout << "   J - set animation delay          |    M - dump memory, W - write memory" << endl;	
	cout << "   K - toggle ROM emulation         |    R - show registers, Y - snapshot" << endl;
	cout << "   L - load memory image            |    O - display op-codes history" << endl;
	cout << "   D - disassemble code in memory   |    Q - quit, 0 - reset, H - help" << endl;
	cout << "   V - toggle graphics emulation    |" << endl;
	cout << "------------------------------------+----------------------------------------" << endl;
} 


/*
 *--------------------------------------------------------------------
 * Method:		RUNSTEPS() - macro
 * Purpose:		Execute multiple steps of CPU emulation.
 * Arguments:
 *            step - boolean flag, true if step by step mode
 *            nsteps - # if steps
 *            brk - current status of break flag
 *            preg - pointer to CPU registers
 *            stct - step counter
 *            pvm - pointer to VM
 *            lrts - status of last RTS flag
 *            anim - boolean flag, true - registers animation mode
 *            delay - delay for anim mode
 *            enrom - rom enabled/disabled flag
 *            rombegin - begin address of emulated ROM
 *            romend - end address of emulated ROM
 *            
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
#define RUNSTEPS(step,nsteps,brk,preg,stct,pvm,lrts,anim,delay)             \
{                                                                           \
	bool cls = false;                                                         \
	brk = preg->SoftIrq;                                                      \
	lrts = preg->LastRTS;                                                     \
	while(step && nsteps > 1 && !brk && !lrts && !opbrk) {                    \
		cout << "addr: $" << hex << preg->PtrAddr << ", step: " << dec << stct; \
		cout  << "    \r";                                                      \
		preg = RunSingleCurrInstr();                                            \
		if (anim) {                                                             \
			if (cls & ClsIfDirty) { pvm->ClearScreen(); cls = false; }            \
			pvm->ScrHome();                                                       \
			cls = ShowRegs(preg,pvm,false,false);                                 \
			cout << endl;                                                         \
			this_thread::sleep_for(chrono::milliseconds(delay));                  \
		}                                                                       \
		brk = preg->SoftIrq;                                                    \
		lrts = preg->LastRTS;                                                   \
		nsteps--;                                                               \
		stct++;                                                                 \
	}                                                                         \
}

/*
 *--------------------------------------------------------------------
 * Method:		LoadArgs()
 * Purpose:		Parse command line arguments.
 * Arguments:	int argc, char *argv[], standard C command line args.
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
 void LoadArgs(int argc, char *argv[])
 {
 		for (int i=1; i<argc; i++) {
 			if (!strcmp(argv[i], "-r")) {
 				reset = true;
 				execvm = true;
 			} else if (!strcmp(argv[i], "-b")) {
 				loadbin = true;
 			} else if (!strcmp(argv[i], "-x")) {
 				loadhex = true;
 			} else if (!strcmp(argv[i], "-h")) {
 				needhelp = true;
 			} else {
 				ramfile = argv[i];
 			}
 		}
 }


/************ corrected in makefile

// Quick and dirty SDL2 workaround to 'undefined reference to WinMain'

#ifdef main
#undef main
#endif

*****************/

/*
 *--------------------------------------------------------------------
 * Method:		main()
 * Purpose:		Application entry point/main loop.
 * Arguments:	int argc, char *argv[], standard C command line args.
 * Returns:		int - general principle is to return 0 if OK, non-zero
 *                  otherwise
 *--------------------------------------------------------------------
 */

int main(int argc, char *argv[]) {
#if defined(LINUX)
	signal(SIGINT, trap_signal);
  signal(SIGTERM, trap_signal);
#endif
#if defined(WINDOWS)
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE );
#endif
	string romfile("dummy.rom");
	LoadArgs(argc, argv);
	if (needhelp) { CmdArgHelp(argv[0]); exit(0); }
	if (loadbin && loadhex) {
		cout << "ERROR: Can't load both formats at the same time." << endl;
		exit(-1);
	}	
	try {
		cout << endl;
		if (loadbin) {
			pvm = new VMachine(romfile, "dummy.ram");
			if (NULL != pvm) {
				PrintVMErr (pvm->LoadRAMBin(ramfile));
				if (!reset) { reset = execvm = pvm->IsAutoReset(); }
			}
		} else if (loadhex) {
			pvm = new VMachine(romfile, "dummy.ram");
			if (NULL != pvm) PrintVMErr (pvm->LoadRAMHex(ramfile));
		}
		else {
			pvm = new VMachine(romfile, ramfile);
			if (NULL != pvm) PrintVMErr(pvm->GetLastError());
			if (NULL != pvm && !reset) { reset = execvm = pvm->IsAutoReset(); }
		}
		if (NULL == pvm) {
			throw MKGenException("Out of memory");
		}
		pvm->ClearScreen();
		CopyrightBanner();
		string cmd;
		bool runvm = false, step = false, brk = false, execaddr = false, stop = true;
		bool lrts = false, anim = false, enrom = pvm->IsROMEnabled();
		unsigned int newaddr = pvm->GetRunAddr(), ioaddr = pvm->GetCharIOAddr(), tmpaddr = 0x0000;
		unsigned int graddr = pvm->GetGraphDispAddr();
		unsigned int rombegin = pvm->GetROMBegin(), romend = pvm->GetROMEnd(), delay = ANIM_DELAY;
		int nsteps = 0;
		if (pvm->IsAutoExec()) {
			execvm = true;
		}
		if (newaddr == 0) newaddr = 0x10000;
		while (true) {
			preg = pvm->GetRegs();
			if (runvm) {
				if (anim) pvm->ClearScreen();
				int stct = 1;
				if (execaddr) {
					preg = ((step) ? RunSingleInstr(newaddr) : pvm->Run(newaddr));
					RUNSTEPS(step,nsteps,brk,preg,stct,pvm,lrts,anim,delay);
					execaddr = false;
				} else {
					preg = ((step) ? RunSingleCurrInstr() : pvm->Run());
					RUNSTEPS(step,nsteps,brk,preg,stct,pvm,lrts,anim,delay);
				}
				if (step)
					cout << "\rExecuted " << dec << stct << ((stct == 1) ? " step." : " steps.") << "                 " << endl;				
				nsteps = 0;
				runvm = step = false;
				newaddr = 0x10000;				
			} else if (execvm) {
				if (reset) {
					pvm->Reset();
					preg = pvm->GetRegs();
					reset = false;
				} else {
					preg = (execaddr ? pvm->Exec(newaddr) : pvm->Exec());
				}
				execvm = false;
				execaddr = false;
				brk = preg->SoftIrq;
				lrts = preg->LastRTS;
				newaddr = 0x10000;
			}
			if (brk || opbrk || stop || lrts) {
				pvm->ClearScreen();
				pvm->ShowIO();
				cout << endl;
				if (opbrk) {
					cout << "Interrupted at " << hex << preg->PtrAddr << endl;
				} else if (brk) {
					cout << "BRK at " << hex << preg->PtrAddr << endl;
				} else if (lrts) {
					cout << "FINISHED at " << hex << ((newaddr > 0xFFFF) ? preg->PtrAddr : newaddr) << endl;
				}	else if (stop) {
					cout << "STOPPED at " << hex << ((newaddr > 0xFFFF) ? preg->PtrAddr : newaddr) << endl;
				}
				opbrk = brk = stop = lrts = false;
				pvm->SetOpInterrupt(false);
				ShowRegs(preg,pvm,ioecho,true);
			}
			ShowMenu();
			cout << "> ";
			cin >> cmd;
			char c = tolower(cmd.c_str()[0]);
			if (c == 'h') {	// display help
				ShowHelp();
			} else if (c == 'p') {	// Interrupt ReQuest
				pvm->Interrupt();
				cout << "OK" << endl;
			} else if (c == 'y') {	// save snapshot of current CPU and memory in binary image
				string name;
				cout << "Enter file name: ";
				cin >> name;
				cout << " [" << name << "]" << endl;
				if (0 == pvm->SaveSnapshot(name)) {
					cout << "OK" << endl;
				} else {
					cout << "ERROR!" << endl;
					cout << "errno=" << errno << endl;
				}
			} else if (c == '0') {	// reset CPU
				reset = true;
				execvm = true;
				runvm = false;
			} else if (c == 'o') {
				queue<string> exechist(pvm->GetExecHistory());
				cout << "PC   : INSTR                    ACC |  X  |  Y  | PS  | SP" << endl;
				cout << "------------------------------------+-----+-----+-----+-----" << endl;
				while (exechist.size()) {
					cout << exechist.front() << endl;
					exechist.pop();
				}
			} else if (c == 'l') {	// load memory image
				char typ = 0;
				for (char c = tolower(typ); 
						 c != 'a' && c != 'b' && c != 'h' && c != 'd';
						 c = tolower(typ)) {
					cout << "Type (A - auto/B - binary/H - Intel HEX/D - definition): ";
					cin >> typ;
				}
				cout << " [";
				switch (tolower(typ)) {
					case 'a': cout << "auto"; break;
					case 'b': cout << "binary"; break;
					case 'h': cout << "Intel HEX"; break;
					case 'd': cout << "definition"; break;
					default: break;	// should never happen
				}
				cout << "]" << endl;
				string name;
				cout << "Memory Image File Name: ";
				cin >> name;
				cout << " [" << name << "]" << endl;
				if (typ == 'b') PrintVMErr (pvm->LoadRAMBin(name));
				else if (typ == 'h') PrintVMErr (pvm->LoadRAMHex(name));
				else if (typ == 'd') {
					PrintVMErr (pvm->LoadRAMDef(name));
					if (pvm->IsAutoExec()) execvm = true;
					if (newaddr == 0) newaddr = 0x10000;					
				}
				else {	// automatic file format detection
					pvm->LoadRAM(name);
					if (pvm->IsAutoExec()) execvm = true;
					if (newaddr == 0) newaddr = 0x10000;
				}
			} else if (c == 'k') {	// toggle ROM emulation
				if (!enrom) {
					enrom = true;
					do {
						rombegin = PromptNewAddress("ROM begin     (0200..FFFF): ");
					} while (rombegin < 0x0200);
					cout << " [" << hex << rombegin << "]" << endl;
					do {
						romend   = PromptNewAddress("ROM end (ROMBEGIN+1..FFFF): ");
					} while (romend <= rombegin);
					cout << " [" << hex << romend << "]" << endl;
					pvm->EnableROM(rombegin, romend);					
					cout << "ROM activated." << endl;
				} else {
					enrom = false;
					pvm->DisableROM();
					cout << "ROM deactivated." << endl;					
				} 
			}	else if (c == 'j') {	// set registers animation delay
					cout << "Delay [ms]: ";
					cin >> dec >> delay;
					cout << " [" << dec << delay << "]" << endl;					
			}	else if (c == 'f') {	// toggle registers animation in step mode
				anim = !anim;
				cout << "Registers status animation " << ((anim) ? "enabled." : "disabled.") << endl;
			}	else if (c == 'b') {	// clear screen
				pvm->ClearScreen();
			} else if (c == 'r') {	// show registers
				stop = true;
			} else if (c == 'e') {	// toggle local echo for I/O console
				if (pvm->GetCharIOActive()) {
					ioecho = !ioecho;
					cout << "I/O echo is " << (ioecho ? "activated." : "deactivated.") << endl;
					pvm->SetCharIO(ioaddr, ioecho);					
				} else {
					cout << "ERROR: I/O is deactivated." << endl;
				}
			} else if (c == 't') {	// show I/O console
				if (pvm->GetCharIOActive()) {
					pvm->ShowIO();
				} else {
					cout << "ERROR: I/O is deactivated." << endl;
				}
			} else if (c == 'i') {	// toggle I/O
				if (pvm->GetCharIOActive()) {
					pvm->DisableCharIO();
					cout << "I/O deactivated." << endl;
				} else {
					ioaddr = PromptNewAddress("Address (0..FFFF): ");
					cout << " [" << hex << ioaddr << "]" << endl;
					pvm->SetCharIO(ioaddr, ioecho);
					cout << "I/O activated." << endl;
				}
			} else if (c == 'v') { // toggle graphics display
				if (pvm->GetGraphDispActive()) {
					pvm->DisableGraphDisp();
					cout << "Graphics display deactivated." << endl;
				} else {
					graddr = PromptNewAddress("Address (0..FFFF): ");
					cout << " [" << hex << graddr << "]" << endl;
					pvm->SetGraphDisp(graddr);
					cout << "Graphics display activated." << endl;
				}
			} else if (c == 'w') {	// write to memory
				tmpaddr = PromptNewAddress("Address (0..FFFF): ");
				cout << " [" << hex << tmpaddr << "]" << endl;
				cout << "Enter hex bytes [00..FF] values separated with NL or spaces, end with [100]:" << endl;
				unsigned short v = 0;
				while (true) {
					cin >> hex >> v;
					cout << " " << hex << v;
					if (v > 0xFF) break;
					pvm->MemPoke8bit(tmpaddr++, v & 0xFF);
				};
				cout << endl;
			} else if (c == 'a') {	// change run address
				execaddr = stop = true;
				newaddr = PromptNewAddress("Address (0..FFFF): ");
				cout << " [" << hex << newaddr << "]" << endl;				
			} else if (c == 's') {
				runvm = step = stop = true;
			} else if (c == 'n') {	// execute # of steps
				nsteps = 0;
				while (nsteps < 1) {
					cout << "# of steps [n>1]: ";
					cin >> dec >> nsteps;
				}
				cout << " [" << dec << nsteps << "]" << endl;
				runvm = step = stop = true;				
			} else if (c == 'c') {	// continue running code
				runvm = true;
			} else if (c == 'g') {	// run from new address until BRK
				runvm = true;
				execaddr = true;
				newaddr = PromptNewAddress("Address (0..FFFF): ");
				cout << " [" << hex << newaddr << "]" << endl;
			} else if (c == 'x') {	// execute code at address
				execvm = true;
				execaddr = true;
				newaddr = PromptNewAddress("Address (0..FFFF): ");
				cout << " [" << hex << newaddr << "]" << endl;				
			} else if (c == 'q') {	// quit
				break;
			} else if (c == 'd') {	// disassemble code in memory
				unsigned int addrbeg = 0x10000, addrend = 0x10000;
				cout << "Enter address range (0..0xFFFF)..." << endl;
				addrbeg = PromptNewAddress("Start address (0..FFFF): ");
				cout << " [" << hex << addrbeg << "]" << endl;
				addrend = PromptNewAddress("End address   (0..FFFF): ");
				cout << " [" << hex << addrend << "]" << endl;
				cout << endl;
				for (unsigned int addr = addrbeg; addr <= addrend;) {
					char instrbuf[DISS_BUF_SIZE];
					addr = pvm->Disassemble((unsigned short)addr, instrbuf);
					cout << instrbuf << endl;
				}
			} else if (c == 'm') {	// dump memory
				unsigned int addrbeg = 0x10000, addrend = 0x10000;
				cout << "Enter address range (0..0xFFFF)..." << endl;
				addrbeg = PromptNewAddress("Start address (0..FFFF): ");
				cout << " [" << hex << addrbeg << "]" << endl;
				addrend = PromptNewAddress("End address   (0..FFFF): ");
				cout << " [" << hex << addrend << "]" << endl;
				cout << endl;
				for (unsigned int addr = addrbeg; addr <= addrend; addr+=16) {
					cout << "\t|";
					for (unsigned int j=0; j < 16; j++) {
						unsigned int hv = (unsigned int)pvm->MemPeek8bit(addr+j);
						if (hv < 16) {
							cout << 0;
						}
						cout << hex << hv << " ";
					}
					cout << "|";
					for (int j=0; j < 16; j++) {
						char cc = (char)pvm->MemPeek8bit(addr+j);
						if (isprint(cc))
							cout << cc;
						else
							cout << "?";	
					}
					cout << '\r';
					cout << hex << addr;
					cout << endl;
				}
			}
		}
	}
	catch (MKGenException& ex) {
		cout << ex.GetCause() << endl;
	}
	catch (...) {
		cout << "ERROR: Fatal." << endl;
	}
	return 0;
}

/*
 *--------------------------------------------------------------------
 * Method:    CopyrightBanner()
 * Purpose:   Display copyright information.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void CopyrightBanner()
{
	cout << "Virtual Machine/CPU Emulator (MOS 6502) and Debugger." << endl;
	cout << "Copyright (C) by Marek Karcz 2016. All rights reserved." << endl;			
}

/*
 *--------------------------------------------------------------------
 * Method:    CmdArgHelp()
 * Purpose:   Display command line arguments help/Usage.
 * Arguments: prgname - string, program name
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void CmdArgHelp(string prgname)
{
	CopyrightBanner();

	cout << endl << endl;
	cout << "Usage:" << endl << endl;
	cout << "\t" << prgname;
	cout << " [-h] | [ramdeffile] [-b | -x] [-r]" << endl;
	cout << endl << endl;
	cout << "Where:" << endl << endl;
	cout << "\tramdeffile    - RAM definition file name" << endl;
	cout << "\t-b            - specify input format as binary" << endl;
	cout << "\t-x            - specify input format as Intel HEX" << endl;
	cout << "\t-r            - after loading, perform CPU RESET" << endl;
	cout << "\t-h            - print this help screen" << endl;
	cout << R"(

When ran with no arguments, program will load default memory
definition files: default.rom, default.ram and will enter the debug
console menu.
When ramdeffile argument is provided with no input format specified,
program will attempt to automatically detect input format and load the
memory definition from the file, set the flags and parameters depending
on the contents of the memory definition file and enter the corresponding
mode of operation as defined in that file.
If input format is specified (-b|-x), program will load memory from the
provided image file and enter the debug console menu.

)";
	cout << endl;
}

/*
 *--------------------------------------------------------------------
 * Method:    ShowHelp()
 * Purpose:   Display Debugger Console Command Reference help.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void ShowHelp()
{
  cout << R"(Debugger Console Command Reference.
  
S - step
    Executes single opcode at current address.
C - continue
    Continues code execution from current address until BRK.
M - dump memory
    Usage: M [startaddr] [endaddr]
    Where: startaddr,endaddr - memory addr. in hexadecimal format [0000..FFFF].
    Dumps contents of memory, hexadecimal and ASCII formats."
G - go/continue from new address until BRK
    Usage: G [address]
    Where: address - memory addr. in hexadecimal format [0000.FFFF].
    Executes code at provided address, interrupted by BRK opcode.
X - execute code from new address until RTS
    Usage: X [address]
    Where: address - memory addr. in hexadecimal format [0000.FFFF].
    Executes code at provided address, until RTS (last one).
Q - quit
    Exits from the emulator/debugger.
A - set address for next step
    Usage: A [address]
    Where: address - memory addr. in hexadecimal format [0000.FFFF].
    Sets current address to a new value.
N - go number of steps
    Usage: N [steps]
    Where: steps - number of steps in decimal format
    Execute number of opcodes provided in steps argument starting
    from current address.
P - IRQ
    Send maskable interrupt request to CPU (set the IRQ line LOW).    
W - write to memory
    Usage: W [address] [hexval] [hexval] ... 100
    Where: address - memory addr. in hexadecimal format [0000.FFFF],
           hexval - byte value in hexadecimal format [00.FF].
    Writes provided values to memory starting at specified address.
I - toggle char I/O emulation
    Usage: I [address]
    Where: address - memory addr. in hexadecimal format [0000.FFFF],
    Toggles basic character I/O emulation. When enabled, all writes
    to the specified memory address also writes a character code to
    to a virtual console. All reads from specified memory address
    are interpreted as console character input.
V - toggle graphics display (video) emulation
    Usage: V [address]
    Where: address - memory addr. in hexadecimal format [0000.FFFF],
    Toggles basic raster (pixel) based RGB graphics display emulation.
    When enabled, window with graphics screen will open and several
    registers are available to control the device starting at provided
    base address. Read programmers reference for detailed documentation
    regarding the available registers and their functions.
R - show registers
    Displays CPU registers, flags and stack.
Y - snapshot
    Usage: Y [file_name]
    Where: file_name - the name of the output file.
    Save snapshot of current CPU and memory in a binary file.
T - show I/O console
    Displays/prints the contents of the virtual console screen.
    Note that in run mode (commands X, G or C), virtual screen is
    displayed automatically in real-time if I/O emulation is enabled.
E - toggle I/O local echo
    Toggles local echo on/off when I/O emulation is enabled.
B - blank (clear) screen
    Clears the screen, useful when after exiting I/O emulation or
    registers animation (long stack) your screen is messed up.
F - toggle registers animation mode
    When in multi-step debug mode (command: N), displaying registers
    can be suppressed or, when animation mode is enabled - they will
    be continuously displayed after each executed step.
J - set registers status animation delay
    Usage: J [delay]
    Where: delay - time of delay in milliseconds,
    Sets the time added at the end of each execution step in multi
    step mode (command: N). The default value is 250 ms.
K - toggle ROM emulation
    Usage: K [rombegin] [romend] - to enable,
           K - to disable,
           (OR just use 'K' in both cases and be prompted for arguments.)
    Where:
       rombegin - hexadecimal address [0200..FFFF],
       romend   - hexadecimal address [rombegin+1..FFFF].
    Enable/disable ROM emulation and define address range to which the ROM
    (read-only memory) will be mapped. Default range: $D000-$DFFF.
L - load memory image
    Usage: L [image_type] [image_name]
    Where: 
       image_type - A - (auto), B (binary), H (Intel HEX) OR D (definition),
       image_name - name of the image file.
    This function allows to load new memory image from either binary
    image file, Intel HEX format file or the ASCII definition file.
    With option 'A' selected, automatic input format detection will be
    attempted.
    The binary image is always loaded from address 0x0000 and can be up to
    64kB long. The definition file format is a plain text file that can
    contain following keywords and data:
      
      ADDR      This keyword defines the run address of the executable code.
                It is optional, but if exists, it must be the 1-st keyword
                in the definition file.
                Address in decimal or hexadecimal ($xxxx) format must follow
                in the next line.
                
      ORG       Changes the current address counter. The line that follows
                sets the new address in decimal or hexadecimal format.
                Data that follows will be put in memory starting from that
                address. This keyword is optional and can be used multiple
                times in the definition file.
                
      IOADDR    Defines the address of the character I/O emulation. The
                next line sets the address of I/O emulation in decimal or
                hexadecimal format. If the I/O emulation is enabled
                (see ENIO keyword), then any character written to this
                address will be sent to the virtual console. The reading
                from that address will invoke character input from the
                emulated console. That input procedure is of blocking
                type. To invoke non-blocking character procedure, reading
                should be performed from IOADDR+1.
                
      ROMBEGIN  Defines the address in memory where the beginning of the
                Read Only memory is mapped. The next line that follows this
                keyword sets the address in decimal or hexadecimal format.
                
      ROMEND    Defines the address in memory where the end of the Read
                Only Memory is mapped. The next line that follows this
                keyword sets the address in decimal or hexadecimal format.
                
      ENIO      Putting this keyword in memory definition file enables
                rudimentary character I/O emulation and virtual console
                emulation.
                
      ENROM     Putting this keyword in memory definition file enables
                emulation of Read Only Memory, in range of addresses
                defined by ROMBEGIN and ROMEND keywords.
                
      EXEC      Define starting address of code which will be automatically
                executed after the memory image is loaded.
                The next line that follows this keyword sets the address
                in decimal or hexadecimal format.

      RESET     Enables auto-reset of the CPU. After loading the memory
                definition file, the CPU reset sequence will be initiated.                      
                
O - display op-codes history
    Show the history of last executed op-codes/instructions, full with
    disassembled mnemonic and argument.
D - diassemble code in memory
    Usage: D [startaddr] [endaddr]
    Where: startaddr,endaddr - hexadecimal address [0000..FFFF].
    Attempt to disassemble code in specified address range and display
    the results (print) on the screen in symbolic form.
0 - reset
    Run the processor initialization sequence, just like the real CPU
    when its RTS signal is set to LOW and HIGH again. CPU will disable
    interrupts, copy address from vector $FFFC to processors PC and will
    start executing code. Programmer must put initialization routine
    under address pointed by $FFFC vector, which will set the arithmetic
    mode, initialize stack, I/O devices and enable IRQ if needed before
    jumping to main loop. The reset routine disables trapping last RTS
    opcode if stack is empty, so the VM will never return from opcodes
    execution loop unless user interrupts with CTRL-C or CTRL-Break.
                    
NOTE:
    1. If no arguments provided, each command will prompt user to enter
       missing data.
    2. It is possible to exit from running program to debugger console
       by pressing CTRL-C or CTRL-Pause/Break, which will generate
       a "Operator Interrupt". However in the character input mode
       use CTRL-Y combination or CTRL-Break (DOS), CTRL-C (Linux).
       You may need to press ENTER after that in input mode (DOS).
)";
  cout << endl; 
}

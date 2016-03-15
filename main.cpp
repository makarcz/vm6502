#include <cstdlib>
#include <iostream>
#include <bitset>
#include <chrono>
#include <thread>
#include "system.h"
#include "MKCpu.h"
#include "Memory.h"
#include "Display.h"
#include "VMachine.h"
#include "MKGenException.h"

using namespace std;
using namespace MKBasic;

#define ANIM_DELAY 250

const bool ClsIfDirty = true;

VMachine *pvm = NULL;
Regs *preg = NULL;
bool ioecho = false, opbrk = false;
int g_stackdisp_lines = 1;

bool ShowRegs(Regs *preg, VMachine *pvm, bool ioecho, bool showiostat);
void ShowHelp();

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
	//cout << "Last instr.: " << preg->LastInstr << "          " << endl;
	cout << endl;
	/*
	cout << "Registers:" << endl;
	cout << "   Acc: $" << hex << (unsigned short)preg->Acc << "\t(%" << bitset<8>((int)preg->Acc) << ")" << endl;
	cout << "     X: $" << hex << (unsigned short)preg->IndX << "   " << endl;
	cout << "     Y: $" << hex << (unsigned short)preg->IndY << "   " << endl;
	cout << "    PC: $" << hex << preg->PtrAddr << "   " << endl;
	//cout << " Acc16: $" << hex << preg->Acc16 << "   " << endl;
	//cout << " Ptr16: $" << hex << preg->Ptr16 << "   " << endl;
	*/
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
		cout << "ROM: " << ((pvm->IsROMEnabled()) ? "enabled." : "disabled.") << " ";
		cout << "Range: $" << hex << pvm->GetROMBegin() << " - $" << hex << pvm->GetROMEnd() << "." << endl;
	}
	cout << "                                                                               \r";
	// cout << "-------------------------------------------------------------------------------" << endl;	
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
	cout << "   G - go/cont. from new address    |    N - go number of steps" << endl;
	cout << "   I - toggle char I/O emulation    |    X - execute from new address" << endl;
	cout << "   T - show I/O console             |    B - blank (clear) screen" << endl;
	cout << "   E - toggle I/O local echo        |    F - toggle registers animation" << endl;
	cout << "   J - set animation delay          |    M - dump memory, W - write memory" << endl;	
	cout << "   K - toggle ROM emulation         |    R - show registers" << endl;
	cout << "   L - load memory image            |    O - display op-codes history" << endl;
	cout << "   D - disassemble code in memory   |    Q - quit, H - help" << endl;
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
		preg = pvm->Step();                                                     \
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

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char** argv) {
#if defined(LINUX)
	signal(SIGINT, trap_signal);
  signal(SIGTERM, trap_signal);
#endif
#if defined(WINDOWS)
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE );
#endif
	string romfile("dummy.rom"), ramfile("dummy.ram");
	if (argc > 1) {
		ramfile = argv[1];
	}
	try {
		cout << endl;
		pvm = new VMachine(romfile, ramfile);
		pvm->ClearScreen();
		cout << "Virtual Machine/CPU Emulator (MOS 6502) and Debugger." << endl;
		cout << "Copyright (C) by Marek Karcz 2016. All rights reserved." << endl;		
		string cmd;
		bool runvm = false, step = false, brk = false, execaddr = false, stop = true;
		bool lrts = false, execvm = false, anim = false, enrom = pvm->IsROMEnabled();
		unsigned int newaddr = pvm->GetRunAddr(), ioaddr = pvm->GetCharIOAddr(), tmpaddr = 0x0000;
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
					preg = ((step) ? pvm->Step(newaddr) : pvm->Run(newaddr));
					RUNSTEPS(step,nsteps,brk,preg,stct,pvm,lrts,anim,delay);
					execaddr = false;
				} else {
					preg = ((step) ? pvm->Step() : pvm->Run());
					RUNSTEPS(step,nsteps,brk,preg,stct,pvm,lrts,anim,delay);
				}
				if (step)
					cout << "\rExecuted " << dec << stct << ((stct == 1) ? " step." : " steps.") << "                 " << endl;				
				nsteps = 0;
				runvm = step = false;
				newaddr = 0x10000;				
			} else if (execvm) {
				preg = (execaddr ? pvm->Exec(newaddr) : pvm->Exec());
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
				while (tolower(typ) != 'b' && tolower(typ) != 'd') {
					cout << "Type (B - binary/D - definition): ";
					cin >> typ;
				}
				cout << " [" << ((tolower(typ) == 'b') ? "binary" : "definition") << "]" << endl;
				string name;
				cout << "Memory Image File Name: ";
				cin >> name;
				cout << " [" << name << "]" << endl;
				if (typ == 'b') pvm->LoadRAMBin(name);
				else {
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
 * Method:    ShowHel2p()
 * Purpose:   Display commands help.
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
R - show registers
    Displays CPU registers, flags and stack.
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
       image_type - B (binary) OR D (definition),
       image_name - name of the image file.
    This function allows to load new memory image from either binary
    image file or the ASCII definition file. The binary image is always
    loaded from address 0x0000 and can be up to 64kB long. The definition
    file format is a plain text file that can contain following keywords
    and data:
      
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
                
O - display op-codes history
    Show the history of last executed op-codes/instructions, full with
    disassembled mnemonic and argument.
D - diassemble code in memory
    Usage: D [startaddr] [endaddr]
    Where: startaddr,endaddr - hexadecimal address [0000..FFFF].
    Attempt to disassemble code in specified address range and display
    the results (print) on the screen in symbolic form.
                    
NOTE:
    1. If no arguments provided, each command will prompt user to enter
       missing data.
    2. It is possible to exit from running program to debugger console
       by pressing CTRL-C or CTRL-Pause/Break, which will generate
       a "Operator Interrupt". However in the character input mode
       use CTRL-Y combination or CTRL-Break (DOS), CTRL-C (Linux).
       You may need to press ENTER after that in input mode (DOS)
)";
  cout << endl; 
}
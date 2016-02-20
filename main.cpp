#include <cstdlib>
#include <iostream>
#include <bitset>
#include "MKCpu.h"
#include "Memory.h"
#include "Display.h"
#include "VMachine.h"
#include "MKGenException.h"

using namespace std;
using namespace MKBasic;

/*
 *--------------------------------------------------------------------
 * Method:		ShowHelp()
 * Purpose:		Display commands help.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void ShowHelp()
{
	cout << "Virtual Machine/CPU emulator/Debugger Command Reference." << endl << endl;
	cout << "S - step" << endl;
	cout << "    Executes single opcode at current address." << endl;
	cout << "C - continue" << endl;
	cout << "    Continues code execution from current address until BRK." << endl;
	cout << "D - dump memory" << endl;
	cout << "    Usage: D [startaddr] [endaddr]" << endl;
	cout << "    Where: startaddr,endaddr - memory addr. in hexadecimal format [0000..FFFF]." << endl;
	cout << "    Dumps contents of memory, hexadecimal and ASCII formats." << endl;
	cout << "G - go/continue from new address until BRK" << endl;
	cout << "    Usage: G [address]" << endl;
	cout << "    Where: address - memory addr. in hexadecimal format [0000.FFFF]." << endl;
	cout << "    Executes code at provided address, interrupted by BRK opcode." << endl;
	cout << "X - execute code from new address until RTS" << endl;
	cout << "    Usage: X [address]" << endl;
	cout << "    Where: address - memory addr. in hexadecimal format [0000.FFFF]." << endl;
	cout << "    Executes code at provided address, until RTS (last one)." << endl;	
	cout << "Q - quit" << endl;
	cout << "    Exits from the emulator/debugger." << endl;
	cout << "A - set address for next step" << endl;
	cout << "    Usage: A [address]" << endl;
	cout << "    Where: address - memory addr. in hexadecimal format [0000.FFFF]." << endl;
	cout << "    Sets current address to a new value." << endl;
	cout << "N - go number of steps" << endl;
	cout << "    Usage: N [steps]" << endl;
	cout << "    Where: steps - number of steps in decimal format" << endl;
	cout << "    Execute number of opcodes provided in steps argument starting" << endl;
	cout << "    from current address." << endl;
	cout << "W - write to memory" << endl;
	cout << "    Usage: W [address] [hexval] [hexval] ... 100" << endl;
	cout << "    Where: address - memory addr. in hexadecimal format [0000.FFFF]," << endl;
	cout << "           hexval - byte value in hexadecimal format [00.FF]." << endl;
	cout << "    Writes provided values to memory starting at specified address." << endl;
	cout << "I - toggle char I/O emulation" << endl;
	cout << "    Usage: I [address]" << endl;
	cout << "    Where: address - memory addr. in hexadecimal format [0000.FFFF]," << endl;
	cout << "    Toggles basic character I/O emulation. When enabled, all writes" << endl;
	cout << "    to the specified memory address also writes a character code to" << endl;
	cout << "    to a virtual console. All reads from specified memory address" << endl;
	cout << "    are interpreted as console character input." << endl;
	cout << "R - regs" << endl;
	cout << "    Displays CPU registers and flags." << endl;
	cout << "T - show I/O console" << endl;
	cout << "    Displays/prints the contents of the virtual console screen." << endl;
	cout << "    Note that in run mode (commands X, G or C), virtual screen is" << endl;
	cout << "    displayed automatically in real-time if I/O emulation is enabled." << endl;
	cout << "E - toggle I/O local echo" << endl;
	cout << "    Toggles local echo on/off when I/O emulation is enabled." << endl;
	cout << "B - blank (clear) screen" << endl;
	cout << "    Clears the screen, useful when after exiting I/O emulation" << endl;
	cout << "    your screen is messed up." << endl;
	cout << "NOTE:" << endl;
	cout << "    If no arguments provided, each command will prompt user to enter" << endl;
	cout << "    missing data." << endl;
	cout << endl;	
}

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
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void ShowRegs(Regs *preg, VMachine *pvm, unsigned short ioaddr, bool ioecho)
{
	cout << "Registers:" << endl;
	cout << "   Acc: $" << hex << (unsigned short)preg->Acc << "\t(%" << bitset<8>((int)preg->Acc) << ")" << endl;
	cout << "     X: $" << hex << (unsigned short)preg->IndX << endl;
	cout << "     Y: $" << hex << (unsigned short)preg->IndY << endl;
	cout << "  Addr: $" << hex << preg->PtrAddr << endl;
	cout << " Acc16: $" << hex << preg->Acc16 << endl;
	cout << " Ptr16: $" << hex << preg->Ptr16 << endl;
	cout << " Stack: $" << hex << (unsigned short)preg->PtrStack << endl;
	cout << " Flags: NV-BDIZC" << endl;
	cout << "        " << bitset<8>((int)preg->Flags) << endl;
	cout << endl << "I/O status: " << (pvm->GetCharIOActive() ? "enabled" : "disabled") << ", ";
	cout << " at: $" << hex << ioaddr << ", ";
	cout << " local echo: " << (ioecho ? "ON" : "OFF") << "." << endl;
	// cout << "-------------------------------------------------------------------------------" << endl;	
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
	cout << "---------------------------------------------------------------------------" << endl;
	cout << "S - step | C - continue, D - dump memory | G - go/continue from new address" << endl;
	cout << "Q - quit | A - set address for next step | N - go number of steps" << endl;
	cout << "H - help | I - toggle char I/O emulation | W - write to memory" << endl;
	cout << "R - regs | T - show I/O console          | E - toggle I/O local echo" << endl;
	cout << "         | X - execute from new address  | B - blank (clear) screen" << endl;	
	cout << "---------------------------------------------------------------------------" << endl;
} 


/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
#define RUNSTEPS(step,nsteps,brk,preg,stct,pvm,lrts)																\
	brk = preg->SoftIrq;																															\
	lrts = preg->LastRTS;																															\
	while(step && nsteps > 1 && !brk && !lrts) {																			\
		cout << "addr: $" << hex << preg->PtrAddr << ", step: " << dec << stct << "\r";	\
		preg = pvm->Step();																															\
		brk = preg->SoftIrq;																														\
		nsteps--;																																				\
		stct++;																																					\
	}

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char** argv) {
	string romfile("dummy.rom"), ramfile("dummy.ram");
	if (argc > 1) {
		ramfile = argv[1];
	}
	try {
		cout << endl;
		VMachine *pvm = new VMachine(romfile, ramfile);
		pvm->ClearScreen();
		cout << "Welcome to Virtual Machine/CPU Emulator (6502)/Debugger." << endl;
		cout << "Copyright (C) by Marek Karcz 2016. All rights reserved." << endl;		
		string cmd;
		bool runvm = false, step = false, brk = false, execaddr = false, stop = true;
		bool ioecho = false, lrts = false, execvm = false;
		unsigned int newaddr = 0x10000, ioaddr = 0xE000, tmpaddr = 0x0000;
		int nsteps = 0;
		while (true) {
			Regs *preg = pvm->GetRegs();
			if (runvm) {
				int stct = 1;
				if (execaddr) {
					preg = ((step) ? pvm->Step(newaddr) : pvm->Run(newaddr));
					RUNSTEPS(step,nsteps,brk,preg,stct,pvm,lrts);
					execaddr = false;
					newaddr = 0x10000;
				} else {
					preg = ((step) ? pvm->Step() : pvm->Run());
					RUNSTEPS(step,nsteps,brk,preg,stct,pvm,lrts);
				}
				if (step)
					cout << "\rExecuted " << dec << stct << ((stct == 1) ? " step." : " steps.") << "                 " << endl;				
				nsteps = 0;
				runvm = step = false;
			} else if (execvm) {
				preg = (execaddr ? pvm->Exec(newaddr) : pvm->Exec());
				execvm = false;
				brk = preg->SoftIrq;
				lrts = preg->LastRTS;
			}
			if (brk || stop || lrts) {
				cout << endl;
				if (brk) {
					cout << "BRK at " << hex << preg->PtrAddr << endl;
					brk = stop = lrts = false;
				} else if (lrts) {
					cout << "FINISHED at " << hex << ((newaddr > 0xFFFF) ? preg->PtrAddr : newaddr) << endl;
					brk = stop = lrts = false;					
				}	else if (stop) {
					cout << "STOPPED at " << hex << ((newaddr > 0xFFFF) ? preg->PtrAddr : newaddr) << endl;
					brk = stop = lrts = false;
				}
				ShowRegs(preg,pvm,ioaddr,ioecho);
			}
			ShowMenu();
			cout << "> ";
			cin >> cmd;
			char c = tolower(cmd.c_str()[0]);
			if (c == 'h') {
				ShowHelp();
			} else if (c == 'b') {
				pvm->ClearScreen();
			} else if (c == 'r') {
				stop = true;
			} else if (c == 'e') {
				if (pvm->GetCharIOActive()) {
					ioecho = !ioecho;
					cout << "I/O echo is " << (ioecho ? "activated." : "deactivated.") << endl;
					pvm->SetCharIO(ioaddr, ioecho);					
				} else {
					cout << "ERROR: I/O is deactivated." << endl;
				}
			} else if (c == 't') {
				if (pvm->GetCharIOActive()) {
					pvm->ShowIO();
				} else {
					cout << "ERROR: I/O is deactivated." << endl;
				}
			} else if (c == 'i') {
				if (pvm->GetCharIOActive()) {
					pvm->DisableCharIO();
					cout << "I/O deactivated." << endl;
				} else {
					ioaddr = PromptNewAddress("Address (0..FFFF): ");
					cout << " [" << hex << ioaddr << "]" << endl;					
					pvm->SetCharIO(ioaddr, ioecho);
					cout << "I/O activated." << endl;
				}
			} else if (c == 'w') {
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
			} else if (c == 'a') {
				execaddr = stop = true;
				newaddr = PromptNewAddress("Address (0..FFFF): ");
				cout << " [" << hex << newaddr << "]" << endl;				
			} else if (c == 's') {
				runvm = step = stop = true;
			} else if (c == 'n') {
				nsteps = 0;
				while (nsteps < 1) {
					cout << "# of steps [n>1]: ";
					cin >> dec >> nsteps;
				}
				cout << " [" << dec << nsteps << "]" << endl;
				runvm = step = stop = true;				
			} else if (c == 'c') {
				runvm = true;
			} else if (c == 'g') {
				runvm = true;
				execaddr = true;
				newaddr = PromptNewAddress("Address (0..FFFF): ");
				cout << " [" << hex << newaddr << "]" << endl;
			} else if (c == 'x') {
				execvm = true;
				execaddr = true;
				newaddr = PromptNewAddress("Address (0..FFFF): ");
				cout << " [" << hex << newaddr << "]" << endl;				
			} else if (c == 'q') {
				break;
			} else if (c == 'd') {
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
		cout << "ERROR: " << ex.GetCause() << endl;
	}
	catch (...) {
		cout << "ERROR: Fatal." << endl;
	}
	return 0;
}

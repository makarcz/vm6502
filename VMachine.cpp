
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "system.h"
#include "VMachine.h"
#include "MKGenException.h"

#if defined(WINDOWS)
#include <conio.h>
#endif

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
	delete mpDisp;
	delete mpCPU;
	delete mpROM;
	delete mpRAM;
}

/*
 *--------------------------------------------------------------------
 * Method:		InitVM()
 * Purpose:		Initialize class.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::InitVM()
{
	mOpInterrupt = false;
	mpRAM = new Memory();

	mAutoExec = false;	
	mCharIOAddr = CHARIO_ADDR;
	mCharIOActive = mCharIO = false;
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
	mpDisp = new Display();
	if (NULL == mpDisp) {
		throw MKGenException("Unable to initialize VM (Display).");
	}		
}

#if defined(WINDOWS)

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::ClearScreen()
{
  HANDLE                     hStdOut;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  DWORD                      count;
  DWORD                      cellCount;
  COORD                      homeCoords = { 0, 0 };

  hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
  if (hStdOut == INVALID_HANDLE_VALUE) return;

  /* Get the number of cells in the current buffer */
  if (!GetConsoleScreenBufferInfo( hStdOut, &csbi )) return;
  cellCount = csbi.dwSize.X *csbi.dwSize.Y;

  /* Fill the entire buffer with spaces */
  if (!FillConsoleOutputCharacter(
    hStdOut,
    (TCHAR) ' ',
    cellCount,
    homeCoords,
    &count
    )) return;

  /* Fill the entire buffer with the current colors and attributes */
  if (!FillConsoleOutputAttribute(
    hStdOut,
    csbi.wAttributes,
    cellCount,
    homeCoords,
    &count
    )) return;

  /* Move the cursor home */
  SetConsoleCursorPosition( hStdOut, homeCoords );
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::ScrHome()
{
  HANDLE                     hStdOut;
  COORD                      homeCoords = { 0, 0 };

  hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
  if (hStdOut == INVALID_HANDLE_VALUE) return;

  /* Move the cursor home */
  SetConsoleCursorPosition( hStdOut, homeCoords );
}

#endif

#if defined(LINUX)

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::ClearScreen()
{
   system("clear");
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::ScrHome()
{
   cout << "\033[1;1H";
}

#endif

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::ShowDisp()
{
	if (mCharIOActive) {
			ScrHome();
			mpDisp->ShowScr();
	}	
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

	mOpInterrupt = false;
	ClearScreen();
	ShowDisp();
	while (true) {
		cpureg = Step();
		if (mCharIO) {
			ShowDisp();
		}
		if (cpureg->SoftIrq || mOpInterrupt)
			break;
	}

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
 * Purpose:		Run VM from current address until last RTS.
 *            NOTE: Stack must be empty!
 * Arguments:	n/a
 * Returns:		Pointer to CPU registers and flags.
 *--------------------------------------------------------------------
 */
Regs *VMachine::Exec()
{
	Regs *cpureg = NULL;

	mOpInterrupt = false;
	ClearScreen();
	ShowDisp();
	while (true) {
		cpureg = Step();
		if (mCharIO) {
			ShowDisp();
		}
		if (cpureg->LastRTS || mOpInterrupt) break;
	}

	ShowDisp();	
	
	return cpureg;
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
	unsigned short addr = mRunAddr;
	Regs *cpureg = NULL;	
	
	cpureg = mpCPU->ExecOpcode(addr);
	addr = cpureg->PtrAddr;
	mRunAddr = addr;
	
	if (mCharIOActive && !mOpInterrupt) {
		char c = -1;
		mCharIO = false;
		while ((c = mpRAM->GetCharOut()) != -1) {
			mOpInterrupt = mOpInterrupt || (c == OPINTERRUPT);
			if (!mOpInterrupt) {
				mpDisp->PutChar(c);
				mCharIO = true;				
			}
		}
	}
	
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
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::LoadRAM(string ramfname)
{
	LoadMEM(ramfname, mpRAM);
	//mpRAM->EnableROM();
}

/*
 *--------------------------------------------------------------------
 * Method:		LoadRAMBin()
 * Purpose:		Load data from binary image file to the memory.
 * Arguments:	ramfname - name of the RAM file definition
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::LoadRAMBin(string ramfname)
{
	FILE *fp = NULL;
	unsigned short addr = 0x0000;
	int n = 0;
	Memory *pm = mpRAM;
	
	if ((fp = fopen(ramfname.c_str(), "rb")) != NULL) {
		while (0 == feof(fp) && 0 == ferror(fp)) {
			unsigned char val = fgetc(fp);
			pm->Poke8bit(addr, val);
			addr++; n++;
		}
		fclose(fp);
		if (n <= 0xFFFF) {
			cout << "WARNING: Unexpected EOF." << endl;
		}
	}
	else {
		cout << "WARNING: Unable to open memory image file: " << ramfname << endl;
		cout << "Press [ENTER]...";
		getchar();
	}		
}

/*
 *--------------------------------------------------------------------
 * Method:		LoadMEM()
 * Purpose:		Load data from memory definition file to the memory.
 * Arguments: memfname - name of memory definition file
 *						pmem - pointer to memory object
 * Returns:		n/a
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
void VMachine::LoadMEM(string memfname, Memory *pmem)
{
	FILE *fp = NULL;
	char line[256] = "\0";
	int lc = 0, errc = 0;
	unsigned short addr = 0, rombegin = 0, romend = 0;
	unsigned int nAddr;
	bool enrom = false, enio = false, runset = false;
	bool ioset = false, execset = false, rombegset = false;
	bool romendset = false;
	Memory *pm = pmem;
	
	if ((fp = fopen(memfname.c_str(), "r")) != NULL) {
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
					errc++;
					cout << "LINE #" << dec << lc << " WARNING: Run address was already set. Ignoring..." << endl;
				}
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
					errc++;
					cout << "LINE #" << dec << lc << " WARNING: I/O address was already set. Ignoring..." << endl;
				}
				continue;
			}
			// enable character I/O emulation
			if (0 == strncmp(line, "ENIO", 4)) {
				enio = true;
				continue;
			}
			// enable ROM emulation
			if (0 == strncmp(line, "ENROM", 5)) {
				enrom = true;
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
					errc++;
					cout << "LINE #" << dec << lc << " WARNING: auto-exec address was already set. Ignoring..." << endl;
				}
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
					errc++;
					cout << "LINE #" << dec << lc << " WARNING: ROM-begin address was already set. Ignoring..." << endl;
				}
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
					errc++;
					cout << "LINE #" << dec << lc << " WARNING: ROM-end address was already set. Ignoring..." << endl;
				}
				continue;
			}			
			if (';' == *line) continue; // skip comment lines
			char *s = strtok (line, " ,");
			while (NULL != s) {
				unsigned int nVal;
				if (*s == '$') {
					sscanf(s+1, "%02x", &nVal);
					pm->Poke8bit(addr++, (unsigned short)nVal);
				} else {
					pm->Poke8bit(addr++, (unsigned short)atoi(s));
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
			if (enrom) pm->EnableROM();
		}
		if (enio) {
			SetCharIO(mCharIOAddr, false);
		}
	}
	else {
		cout << "WARNING: Unable to open memory definition file: " << memfname << endl;
		errc++;
	}	
	if (errc) {
		cout << "Found " << dec << errc << ((errc > 1) ? " problems." : " problem.") << endl;
		cout << "Press [ENTER] to continue...";
		getchar();		
	}	
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
	mpDisp->ClrScr();
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
 * Method:		ShowIO()
 * Purpose:		Show contents of emulated char I/O.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void VMachine::ShowIO()
{
	if (mCharIOActive)
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
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void VMachine::EnableROM()
{
	mpRAM->EnableROM();
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
}
		
/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */		
void VMachine::EnableROM(unsigned short start, unsigned short end)
{
	mpRAM->EnableROM(start, end);
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

} // namespace MKBasic

#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <string.h>
#include "VMachine.h"
#include "MKGenException.h"

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
	mpRAM = new Memory();
	mRunAddr = 0x200;
	mCharIOAddr = CHARIO_ADDR;
	mCharIOActive = mCharIO = false;
	if (NULL == mpRAM) {
		throw MKGenException("Unable to initialize VM (RAM).");
	}
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
#if defined (WINDOWS)			
			//ClearScreen();
			ScrHome();
#elif defined (LINUX)
			system("clear");
#endif						
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

#if defined (WINDOWS)			
			ClearScreen();
#elif defined (LINUX)
			system("clear");
#endif		
	ShowDisp();
	while (true) {
		cpureg = Step();
		if (mCharIO) {
			ShowDisp();
		}
		if (cpureg->SoftIrq)
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

#if defined (WINDOWS)			
			ClearScreen();
#elif defined (LINUX)
			system("clear");
#endif	
	ShowDisp();
	while (true) {
		cpureg = Step();
		if (mCharIO) {
			ShowDisp();
		}
		if (cpureg->LastRTS) break;
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
	
	if (mCharIOActive) {
		char c = -1;
		mCharIO = false;
		while ((c = mpRAM->GetCharOut()) != -1) {
			mpDisp->PutChar(c);
			mCharIO = true;
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
	mpRAM->EnableROM();
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
 * ; comment
 * ADDR
 * address
 * data
 * ORG
 * address
 *
 * Where:
 * ADDR - label indicating that starting address will follow in next
 *        line
 * ORG - label indicating that the address counter will change to the
 *       value provided in next line
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
	unsigned short addr = 0x200;
	unsigned int nAddr;
	Memory *pm = pmem;
	
	if ((fp = fopen(memfname.c_str(), "r")) != NULL) {
		fgets(line, 256, fp);
		if (0 == strcmp(line, "ADDR")) {
			line[0] = '\0';
			fgets(line, 256, fp);
			if (*line == '$') {
				sscanf(line+1, "%04x", &nAddr);
				addr = nAddr;
			} else {
				addr = (unsigned short) atoi(line);				
			}
			mRunAddr = addr;
		}
		while (0 == feof(fp) && 0 == ferror(fp))
		{
			line[0] = '\0';			
			fgets(line, 256, fp);
			if (0 == strncmp(line, "ORG", 3)) {
				line[0] = '\0';
				fgets(line, 256, fp);
				if (*line == '$') {
					sscanf(line+1, "%04x", &nAddr);
					addr = nAddr;
				} else {
					addr = (unsigned short) atoi(line);				
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
	}
	else {
		cout << "WARNING: Unable to open memory definition file: " << memfname << endl;
		cout << "Press [ENTER]...";
		getchar();
		//throw MKGenException("Unable to open memory definition file: " + memfname);
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

} // namespace MKBasic

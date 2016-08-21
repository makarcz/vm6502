#ifndef MEMMAPDEV_H
#define MEMMAPDEV_H

#include <vector>
#include <string>
#include "system.h"
//#include "Memory.h"
#include "GraphDisp.h"

#if defined(LINUX)
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#endif

// some default definitions
#define CHARIO_ADDR			0xE000
#define GRDISP_ADDR			0xE002
#define CHARIO_BUF_SIZE	256

using namespace std;

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */

namespace MKBasic {

class Memory;	

// Definitions for memory mapped devices handling.

// memory address ranges
struct AddrRange {
	unsigned short start_addr, end_addr; // inclusive address range

	AddrRange(unsigned short staddr, unsigned short endaddr)
	{
		start_addr = staddr;
		end_addr = endaddr;
	}
};

// device parameters
struct DevPar {
	string name, value;

	DevPar(string n, string v)
	{
		name = n;
		value = v;
	}
};

typedef vector<AddrRange> MemAddrRanges;
typedef vector<DevPar>		DevParams;

class MemMapDev;

// Class MemMapDev implements functionality of all
// memory mapped devices.

// device register read, arguments: address
typedef int (MemMapDev::*ReadFunPtr)(int);
// device register write,	arguments: address, value
typedef void (MemMapDev::*WriteFunPtr)(int,int);

// Definition of device.
struct Device {
	int             num;						// device number
	string          name;						// device name
	MemAddrRanges 	addr_ranges;		// vector of memory address ranges for this device
	ReadFunPtr			read_fun_ptr;		// pointer to memory register read function
	WriteFunPtr			write_fun_ptr;	// pointer to memory register write function
	DevParams				params;					// list of device parameters

	Device()
	{
		num = -1;
	}

	Device(int dnum, 
				 string dname, 
				 MemAddrRanges addrranges, 
				 ReadFunPtr prdfun, 
				 WriteFunPtr pwrfun, 
				 DevParams parms) 
	{
		num = dnum;
		name = dname;
		addr_ranges = addrranges;
		read_fun_ptr = prdfun;
		write_fun_ptr = pwrfun;
		params = parms;
	}
};

typedef vector<Device> MemMappedDevices;

enum DevNums {
	DEVNUM_CHARIO = 0,	// character I/O device
	DEVNUM_GRDISP = 1,	// raster graphics display device
};

/*
 * NOTE regarding device address ranges.
 * 
 * The emulated device is a model of a electronic device that is connected
 * to the CPU-s bus (address, memory, control signals).
 * Depending on the device, the memory address range maybe as simple as
 * a single memory address or it may contain multiple memory addresses
 * or ranges of addresses positioned at various non-contiguous places.
 * The functions of these addresses are handled internally by the MemMapDev
 * class. The client (user) initializing the device, if providing custom
 * memory ranges must know more details about device in order to provide
 * accurate data. E.g.: if device requires a single address as an entry
 * point, just one memory range is needed with start_addr == end_addr.
 * If device is more complicated and requires range of addresses to access
 * its control registers, another range of addresses to map memory access
 * etc., device should be setup accordingly by calling SetupDevice method
 * with proper arguments. Device setup is not mandatory, each device that
 * is mainained is initialized wit default parameters.
 */

// offsets of raster graphics device registers
enum GraphDevRegs {
	GRAPHDEVREG_X 			= 0,
	GRAPHDEVREG_Y 			= 2,
	GRAPHDEVREG_PXCOL_R = 3,
	GRAPHDEVREG_PXCOL_G = 4,
	GRAPHDEVREG_PXCOL_B = 5,
	GRAPHDEVREG_BGCOL_R = 6,
	GRAPHDEVREG_BGCOL_G = 7,
	GRAPHDEVREG_BGCOL_B = 8,
	GRAPHDEVREG_CMD 		= 9,
	GRAPHDEVREG_X2			= 10,
	GRAPHDEVREG_Y2			= 12,
	//---------------------------
	GRAPHDEVREG_END
};

// graphics display commands
enum GraphDevCmds {
	GRAPHDEVCMD_CLRSCR = 0,
	GRAPHDEVCMD_SETPXL = 1,
	GRAPHDEVCMD_CLRPXL = 2,
	GRAPHDEVCMD_SETBGC = 3,
	GRAPHDEVCMD_SETFGC = 4,
	GRAPHDEVCMD_DRAWLN = 5,
	GRAPHDEVCMD_ERASLN = 6
};

struct GraphDeviceRegs {
	unsigned char mGraphDispLoX;
	unsigned char mGraphDispHiX;
	unsigned char mGraphDispY;
	unsigned char mGraphDispLoX2;
	unsigned char mGraphDispHiX2;
	unsigned char mGraphDispY2;	
	unsigned char mGraphDispPixColR;
	unsigned char mGraphDispPixColG;
	unsigned char mGraphDispPixColB;
	unsigned char mGraphDispBgColR;
	unsigned char mGraphDispBgColG;
	unsigned char mGraphDispBgColB;	
};

// Functionality of memory mapped devices
class MemMapDev {

	public:

		MemMapDev();
		MemMapDev(Memory *pmem);
		~MemMapDev();

		Device GetDevice(int devnum);
		int SetupDevice(int devnum, MemAddrRanges memranges, DevParams params);

		char GetCharIn();
		char GetCharOut();
		unsigned short GetCharIOAddr();
		bool GetCharIOEchoOn();

		int CharIODevice_Read(int addr);
		void CharIODevice_Write(int addr, int val);

		unsigned short GetGraphDispAddrBase();
		void ActivateGraphDisp();
		void DeactivateGraphDisp();

		int GraphDispDevice_Read(int addr);
		void GraphDispDevice_Write(int addr, int val);

		void GraphDisp_ReadEvents();
		void GraphDisp_Update();

	private:

		Memory *mpMem;	// pointer to memory object
		MemMappedDevices mDevices;
		char mCharIOBufIn[CHARIO_BUF_SIZE];
		char mCharIOBufOut[CHARIO_BUF_SIZE];
		unsigned int mInBufDataBegin;
		unsigned int mInBufDataEnd;
		unsigned int mOutBufDataBegin;
		unsigned int mOutBufDataEnd;
		unsigned int mCharIOAddr;
		bool mIOEcho;		
		unsigned int mGraphDispAddr;
		GraphDisp *mpGraphDisp;			// pointer to Graphics Device object
		GraphDeviceRegs mGrDevRegs;	// graphics display device registers

		void Initialize();

		unsigned char ReadCharKb(bool nonblock);
		void PutCharIO(char c);		

#if defined(LINUX)

		void set_conio_terminal_mode();
		int kbhit();
		int getch();		

#endif		

};

}	// namespace MKBasic

#endif // MEMMAPDEV_H
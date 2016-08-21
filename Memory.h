#ifndef MEMORY_H
#define MEMORY_H

#include "system.h"
#include "MemMapDev.h"

#define MAX_8BIT_ADDR 	0xFFFF
#define MEM_PAGE_SIZE		0x100
#define ROM_BEGIN				0xD000
#define ROM_END					0xDFFF
#define MIN_ROM_BEGIN		0x0200

using namespace std;

namespace MKBasic {

class Memory
{
	public:
		
		Memory();
		~Memory();
		
		void	Initialize();
		unsigned char Peek8bit(unsigned short addr);
		unsigned char Peek8bitImg(unsigned short addr);
		unsigned short Peek16bit(unsigned short addr);
		void Poke8bit(unsigned short addr, unsigned char val);		// write to memory and call memory mapped device handle
		void Poke8bitImg(unsigned short addr, unsigned char val);	// write to memory image only
		void SetCharIO(unsigned short addr, bool echo);
		void DisableCharIO();
		unsigned short GetCharIOAddr();		
		char GetCharIn();
		char GetCharOut();
		void EnableROM();
		void DisableROM();
		void SetROM(unsigned short start, unsigned short end);
		void EnableROM(unsigned short start, unsigned short end);
		unsigned short GetROMBegin();
		unsigned short GetROMEnd();
		bool IsROMEnabled();
		int AddDevice(int devnum);
		int DeleteDevice(int devnum);
		void SetupDevice(int devnum, MemAddrRanges memranges, DevParams params);

		void SetGraphDisp(unsigned short addr);
		void DisableGraphDisp();
		unsigned short GetGraphDispAddr();
		void GraphDisp_ReadEvents();
		void GraphDisp_Update();
		bool GraphDispOp();
		
	protected:
		
	private:
		
		unsigned char m8bitMem[MAX_8BIT_ADDR+1];
		// array of device usage for each memory page
		// this is performance optimization array that keeps values >= 0 under the
		// indexes of memory pages where the memory mapped device is active or -1
		// if there is no active device on given memory page
		int mMemPageDev[MEM_PAGE_SIZE];
		unsigned short mCharIOAddr;
		bool mCharIOActive;
		bool mIOEcho;
		unsigned short mROMBegin;
		unsigned short mROMEnd;
		bool mROMActive;
		vector<Device> mActiveDeviceVec;	// active devices
		MemMapDev *mpMemMapDev;						// pointer to MemMapDev object
		bool mGraphDispActive;
		bool mDispOp;
		
		unsigned char ReadCharKb(bool nonblock);
		void PutCharIO(char c);
};

} // namespace MKBasic

#endif

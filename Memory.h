#ifndef MEMORY_H
#define MEMORY_H

#include "system.h"

#if defined(LINUX)
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#endif

#define MAX_8BIT_ADDR 	0xFFFF
#define CHARIO_ADDR			0xE000
#define CHARIO_BUF_SIZE	256
#define ROM_BEGIN				0xD000
#define ROM_END					0xDFFF
#define MIN_ROM_BEGIN		0x0200

namespace MKBasic {

class Memory
{
	public:
		
		Memory();
		~Memory();
		
		void	Initialize();
		unsigned char Peek8bit(unsigned short addr);
		unsigned short Peek16bit(unsigned short addr);
		void Poke8bit(unsigned short addr, unsigned char val);
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
		
	protected:
		
	private:
		
		unsigned char m8bitMem[MAX_8BIT_ADDR+1];
		char mCharIOBufIn[CHARIO_BUF_SIZE];
		char mCharIOBufOut[CHARIO_BUF_SIZE];
		unsigned int mInBufDataBegin;
		unsigned int mInBufDataEnd;
		unsigned int mOutBufDataBegin;
		unsigned int mOutBufDataEnd;		
		unsigned short mCharIOAddr;
		bool mCharIOActive;
		bool mIOEcho;
		unsigned short mROMBegin;
		unsigned short mROMEnd;
		bool mROMActive;
		
		unsigned char ReadCharKb(bool nonblock);
		void PutCharIO(char c);

#if defined(LINUX)

		void set_conio_terminal_mode();
		int kbhit();
		int getch();

#endif
};

} // namespace MKBasic

#endif

#ifndef MEMORY_H
#define MEMORY_H

#define MAX_8BIT_ADDR 	0xFFFF
#define CHARIO_ADDR			0xE000
#define CHARIO_BUF_SIZE	256
#define ROM_BEGIN				0xD000
#define ROM_END					0xDFFF

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
		void EnableROM(unsigned short start, unsigned short end);
		
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
		
		unsigned char ReadCharKb();
		void PutCharIO(char c);
};

} // namespace MKBasic

#endif

#ifndef DISPLAY_H
#define DISPLAY_H

#include "system.h"

#define TABSIZE 4

namespace MKBasic {
	
enum eScreenDimensions {
	SCREENDIM_COL = 80,
	SCREENDIM_ROW = 24
};

enum eScreenSpecChars {
	SCREENSPECCHARS_NL = (int)'\n',	// new line
	SCREENSPECCHARS_CR = (int)'\r',	// caret
	SCREENSPECCHARS_TB = (int)'\t',	// tab
	SCREENSPECCHARS_BS = (int)'\b',	// backspace
	SCREENSPECCHARS_BE = (int)'\a'	// bell
};
	
struct CursorCoord {
	unsigned int row;
	unsigned int col;
};	

class Display
{
	public:
		
		Display();
		~Display();
		void GotoXY(unsigned int col, unsigned int row);
		void PutChar(char c);
		void ClrScr();
		char GetCharAt(unsigned int col, unsigned int row);
		void ShowScr();
		CursorCoord *GetCursorCoord();		
				
	protected:
		
	private:
		
		char mScreen[SCREENDIM_COL][SCREENDIM_ROW];
		CursorCoord mCursorCoord;
		unsigned int mShellConsoleWidth;
		unsigned int mScrLines;
		unsigned int mScrColumns;
		
		void InitScr();
		void ScrollUp();
		bool IsSpecChar(char c);
		int  GetConsoleWidth();

};

} // namespace MKBasic

#endif

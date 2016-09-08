/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:   			Display.h
 *
 * Purpose: 		Prototype of Display class and all supportig data
 *							structures, enumerations, constants and macros.
 *
 * Date:      	8/25/2016
 *
 * Copyright:  (C) by Marek Karcz 2016. All rights reserved.
 *
 * Contact:    makarcz@yahoo.com
 *
 * License Agreement and Warranty:

   This software is provided with No Warranty.
   I (Marek Karcz) will not be held responsible for any damage to
   computer systems, data or user's health resulting from use.
   Please proceed responsibly and apply common sense.
   This software is provided in hope that it will be useful.
   It is free of charge for non-commercial and educational use.
   Distribution of this software in non-commercial and educational
   derivative work is permitted under condition that original
   copyright notices and comments are preserved. Some 3-rd party work
   included with this project may require separate application for
   permission from their respective authors/copyright owners.

 *--------------------------------------------------------------------
 */
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
		char GetLastChar();
				
	protected:
		
	private:
		
		char mScreen[SCREENDIM_ROW][SCREENDIM_COL];
		char mLastChar;
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

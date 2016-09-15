/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:   			Display.cpp
 *
 * Purpose: 		Implementation of Display class.
 *							The Display class emulates the character I/O device.
 *							It defines the abstract device. The actual
 *							input/output to the screen of the platform on which
 *							the emulator runs is defined in MemMapDev class
 *							or on higher level of abstraction.
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
#include <ctype.h>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include "Display.h"
#include "MKGenException.h"

using namespace std;


#if defined(LINUX)
#include <ncurses.h>

extern bool g_initialized;

#endif

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */

namespace MKBasic {

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
Display::Display()
{
	InitScr();
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
Display::~Display()
{
}

/*
 *--------------------------------------------------------------------
 * Method:		InitScr()
 * Purpose:		Initialize screen.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void Display::InitScr()
{
	mpConIO = new ConsoleIO();
	if (NULL == mpConIO)
		throw MKGenException("Display::InitScr() : Out of memory - ConsoleIO");
	mLastChar = 0;
	mScrLines = SCREENDIM_ROW;
	mScrColumns = SCREENDIM_COL;
	mShellConsoleWidth = GetConsoleWidth();
	if (mScrColumns > mShellConsoleWidth) {
		mScrColumns = mShellConsoleWidth;
	}
	ClrScr();
}

#if defined(WINDOWS)

#include <windows.h>
#include <conio.h>

/*
 *--------------------------------------------------------------------
 * Method:		GetConsoleWidth()
 * Purpose:		Obtain the width of shell console (the real one, not
 *            the emulated one) on Windows.
 * Arguments:	n/a
 * Returns:		int - width of the shell console.
 *--------------------------------------------------------------------
 */
int Display::GetConsoleWidth()
{
  HANDLE                     hStdOut;
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
  if (hStdOut == INVALID_HANDLE_VALUE) return -1;

  if (!GetConsoleScreenBufferInfo( hStdOut, &csbi )) return -2;

  return csbi.dwSize.X;
}

#endif

#if defined(LINUX)

#include <termcap.h>

/*
 *--------------------------------------------------------------------
 * Method:		GetConsoleWidth()
 * Purpose:		Obtain the width of shell console (the real one, not
 *            the emulated one) on Linux.
 * Arguments:	n/a
 * Returns:		int - width of the shell console.
 *--------------------------------------------------------------------
 */
int Display::GetConsoleWidth()
{
	unsigned int conwidth = SCREENDIM_COL;
  char *termtype = getenv("TERM");
  static char termbuf[2048];

  if (tgetent(termbuf, termtype) < 0) {
  	cout << "WARNING: Could not access the termcap data base." << endl;
  	cout << "         Unable to determine console width." << endl;
  } else {
  	conwidth = tgetnum("co");
  }

	return conwidth;
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
void Display::ScrollUp()
{
	for (unsigned int row=0; row<mScrLines-1; row++) {	
		for (unsigned int col=0; col<mScrColumns; col++) {
			mScreen[row][col] = mScreen[row+1][col];
		}
	}
	for (unsigned int col=0; col<mScrColumns; col++) {
		mScreen[mScrLines-1][col] = ' ';
	}
}

/*
 *--------------------------------------------------------------------
 * Method:		GotoXY()
 * Purpose:		Move cursor to new coordinates.
 * Arguments:	col, row - integer values, new cursor coordinates
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void Display::GotoXY(unsigned int col, unsigned int row)
{
	if (col < mScrColumns && row < mScrLines) {
		mCursorCoord.col = col;
		mCursorCoord.row = row;
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
bool Display::IsSpecChar(char c)
{
	bool ret = false;
	char sct[] = {SCREENSPECCHARS_NL,
								SCREENSPECCHARS_CR,
								SCREENSPECCHARS_TB,
								SCREENSPECCHARS_BS,
								SCREENSPECCHARS_BE,
								0};
								
	for (unsigned int i=0; i<strlen(sct); i++) {
		if (c == sct[i]) {
			ret = true;
			break;
		}
	}								
	
	return ret;
}

/*
 *--------------------------------------------------------------------
 * Method:		PutChar()
 * Purpose:		Output character to console. If going outside
 *            lower-right corner, scroll the contents up.
 * Arguments: c - character to output.
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void Display::PutChar(char c)
{
	if (isalnum(c) || ispunct(c) || isspace(c) || IsSpecChar(c)) 
	{
		if (c == SCREENSPECCHARS_NL) {
			mLastChar = SCREENSPECCHARS_NL;
			//mCursorCoord.col = 0;
			mCursorCoord.row++;			
			if (mCursorCoord.row >= mScrLines) {
				ScrollUp();
				mCursorCoord.row = mScrLines-1;
			}
		} else if (c == SCREENSPECCHARS_CR) {
			mLastChar = SCREENSPECCHARS_CR;
			mCursorCoord.col = 0;
		} else if (c == SCREENSPECCHARS_TB) {
			mLastChar = SCREENSPECCHARS_TB;
			mCursorCoord.col += DISP_TABSIZE;
			if (mCursorCoord.col >= mScrColumns) {
				mCursorCoord.col = mScrColumns-1; // must work on it some more
			}
		} else if (c == SCREENSPECCHARS_BS) {
			mLastChar = SCREENSPECCHARS_BS;
			if (mCursorCoord.col > 0) mCursorCoord.col--;
		} else if (c == SCREENSPECCHARS_BE) {
			mLastChar = SCREENSPECCHARS_BE;
			// no action
		}
		else {
			mScreen[mCursorCoord.row][mCursorCoord.col] = c;
			mLastChar = c;
			mCursorCoord.col++;
			if (mCursorCoord.col >= mScrColumns) {
				mCursorCoord.col = 0;
				mCursorCoord.row++;
				if (mCursorCoord.row >= mScrLines) {
					ScrollUp();
					mCursorCoord.row = mScrLines-1;
				}
			}
		}
	}
}

/*
 *--------------------------------------------------------------------
 * Method:		ClrScr()
 * Purpose:   Fill the screen with spaces. Set cursor in left-upper
 *            corner.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void Display::ClrScr()
{
	for (unsigned int col=0; col<mScrColumns; col++) {
		for (unsigned int row=0; row<mScrLines; row++) {
			mScreen[row][col] = ' ';
		}
	}	
	mCursorCoord.col = mCursorCoord.row = 0;	
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
char Display::GetCharAt(unsigned int col, unsigned int row)
{
	char c = -1;
	
	if (col < mScrColumns && row < mScrLines)
		c = mScreen[row][col];
	
	return c;
}

/*
 *--------------------------------------------------------------------
 * Method:		ShowScr()
 * Purpose:   Display contents of the emulated console on a... well,
 *            real console.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void Display::ShowScr()
{
	char buf[SCREENDIM_COL] = {0};
	string scr;
	scr.clear();
	for (unsigned int row=0; row<mScrLines; row++) {
		char *linebuf = &(mScreen[row][0]);
		memset(buf, 0, mScrColumns);
		strncpy(buf, linebuf, mScrColumns);
		buf[mScrColumns] = 0;
		if (mCursorCoord.row == row) {
			buf[mCursorCoord.col] = '_';
		}
		string line(buf);
		// add extra NL if the real console is wider than emulated one
		if (mShellConsoleWidth > mScrColumns)	line = line + "\n";
		scr = scr + line;
	}
  mpConIO->PrintString(scr);
}

/*
 *--------------------------------------------------------------------
 * Method:		GetCursorCoord()
 * Purpose:   Get cursor coordinates.
 * Arguments: n/a
 * Returns:   pointer to cursor coordinates
 *--------------------------------------------------------------------
 */
CursorCoord *Display::GetCursorCoord()
{
	return &mCursorCoord;
}

/*
 *--------------------------------------------------------------------
 * Method:
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
 char Display::GetLastChar()
 {
 	return mLastChar;
 }

} // namespace MKBasic

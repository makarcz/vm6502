#include "Display.h"
#include <ctype.h>
#include <cstdlib>
#include <iostream>
#include <string.h>

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
			mScreen[col][row] = mScreen[col][row+1];
		}
	}
	for (unsigned int col=0; col<mScrColumns; col++) {
		mScreen[col][mScrLines-1] = ' ';
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
			//mCursorCoord.col = 0;
			mCursorCoord.row++;			
			if (mCursorCoord.row >= mScrLines) {
				ScrollUp();
				mCursorCoord.row = mScrLines-1;
			}
		} else if (c == SCREENSPECCHARS_CR) {
			mCursorCoord.col = 0;
		} else if (c == SCREENSPECCHARS_TB) {
			mCursorCoord.col += TABSIZE;
			if (mCursorCoord.col >= mScrColumns) {
				mCursorCoord.col = mScrColumns-1; // must work on it some more
			}
		} else if (c == SCREENSPECCHARS_BS) {
			if (mCursorCoord.col > 0) mCursorCoord.col--;
		} else if (c == SCREENSPECCHARS_BE) {
			// no action
		}
		else {
			mScreen[mCursorCoord.col][mCursorCoord.row] = c;
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
			mScreen[col][row] = ' ';
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
		c = mScreen[col][row];
	
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
	for (unsigned int row=0; row<mScrLines; row++) {
		string line;
		line.clear();
		for (unsigned int col=0; col<mScrColumns; col++) {
			char c = mScreen[col][row];
			if (mCursorCoord.col == col && mCursorCoord.row == row) {
				c = '_';
			}
			line = line + c;
		}
		cout << line;
		// add extra NL if the real console is wider than emulated one
		if (mShellConsoleWidth > mScrColumns)	cout << endl;
	}		
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

} // namespace MKBasic

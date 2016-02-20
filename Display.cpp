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
	ClrScr();
}

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
	for (int row=0; row<SCREENDIM_ROW-1; row++) {	
		for (int col=0; col<SCREENDIM_COL; col++) {
			mScreen[col][row] = mScreen[col][row+1];
		}
	}
	for (int col=0; col<SCREENDIM_COL; col++) {
		mScreen[col][SCREENDIM_ROW-1] = ' ';
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
	if (col < SCREENDIM_COL && row < SCREENDIM_ROW) {
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
			if (mCursorCoord.row >= SCREENDIM_ROW) {
				ScrollUp();
				mCursorCoord.row = SCREENDIM_ROW-1;
			}
		} else if (c == SCREENSPECCHARS_CR) {
			mCursorCoord.col = 0;
		} else if (c == SCREENSPECCHARS_TB) {
			mCursorCoord.col += TABSIZE;
			if (mCursorCoord.col >= SCREENDIM_COL) {
				mCursorCoord.col = SCREENDIM_COL-1; // must work on it some more
			}
		} else if (c == SCREENSPECCHARS_BS) {
			if (mCursorCoord.col > 0) mCursorCoord.col--;
		} else if (c == SCREENSPECCHARS_BE) {
			// no action
		}
		else {
			mScreen[mCursorCoord.col][mCursorCoord.row] = c;
			mCursorCoord.col++;
			if (mCursorCoord.col >= SCREENDIM_COL) {
				mCursorCoord.col = 0;
				mCursorCoord.row++;
				if (mCursorCoord.row >= SCREENDIM_ROW) {
					ScrollUp();
					mCursorCoord.row = SCREENDIM_ROW-1;
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
	for (int col=0; col<SCREENDIM_COL; col++) {
		for (int row=0; row<SCREENDIM_ROW; row++) {
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
	
	if (col < SCREENDIM_COL && row < SCREENDIM_ROW)
		c = mScreen[col][row];
	
	return c;
}

/*
 *--------------------------------------------------------------------
 * Method:		ShowScr()
 * Purpose:   Display contents of the emulated console on... well,
 *            real console.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void Display::ShowScr()
{
	for (int row=0; row<SCREENDIM_ROW; row++) {
		string line;
		line.clear();
		for (int col=0; col<SCREENDIM_COL; col++) {
			char c = mScreen[col][row];
			if (mCursorCoord.col == col && mCursorCoord.row == row) {
				c = '_';
			}
			line = line + c;
			//putchar(mScreen[col][row]);
		}
		cout << line;
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

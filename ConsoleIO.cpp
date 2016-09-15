/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:        ConsoleIO.cpp
 *
 * Purpose:     Implementation of ConsoleIO class.
 *              The ConsoleIO class has methods helpful when
 *              UI is utilizing STDIO (DOS) console OR curses on
 *							Linux.
 *
 * Date:        8/26/2016
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
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include "system.h"
#include "ConsoleIO.h"

#include "MKGenException.h"

#if defined(WINDOWS)
#include <conio.h>
#endif

#if defined(LINUX)
#include <termios.h>
#include <sys/ioctl.h>
#include <asm-generic/ioctls.h>
#include <ncurses.h>

static WINDOW *g_pWin = NULL;
#endif

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
ConsoleIO::ConsoleIO()
{

}  

/*
 *--------------------------------------------------------------------
 * Method:    
 * Purpose:   
 * Arguments: 
 * Returns:   
 *--------------------------------------------------------------------
 */
ConsoleIO::~ConsoleIO()
{

}

#if defined(WINDOWS)

/*
 *--------------------------------------------------------------------
 * Method:     ClearScreen()
 * Purpose:    Clear the console screen.
 * Arguments:  n/a
 * Returns:    n/a
 *--------------------------------------------------------------------
 */
void ConsoleIO::ClearScreen()
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
 * Method:     ScrHome()
 * Purpose:    Bring the console cursor to home position.
 * Arguments:  n/a
 * Returns:    n/a
 *--------------------------------------------------------------------
 */
void ConsoleIO::ScrHome()
{
  HANDLE                     hStdOut;
  COORD                      homeCoords = { 0, 0 };

  hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
  if (hStdOut == INVALID_HANDLE_VALUE) return;

  /* Move the cursor home */
  SetConsoleCursorPosition( hStdOut, homeCoords );
}

#endif // WINDOWS

#if defined(LINUX)

/*
 *--------------------------------------------------------------------
 * Method:     ClearScreen()
 * Purpose:    Clear the console screen.
 * Arguments:  n/a
 * Returns:    n/a
 *--------------------------------------------------------------------
 */
void ConsoleIO::ClearScreen()
{
	if (isendwin() || NULL == g_pWin) {
		system("clear");
	} else {
		clear();
		refresh();
	}
}

/*
 *--------------------------------------------------------------------
 * Method:     ScrHome()
 * Purpose:    Bring the console cursor to home position.
 * Arguments:  n/a
 * Returns:    n/a
 *--------------------------------------------------------------------
 */
void ConsoleIO::ScrHome()
{
	if (!isendwin() && NULL != g_pWin) {
		move(0, 0);
		refresh();
	} else {
		cout << "\033[1;1H";
	}
}

#endif // #define LINUX

/*
 *--------------------------------------------------------------------
 * Method:     GetChar()
 * Purpose:    Get character from console.
 * Arguments:  n/a
 * Returns:    int - character code.
 *--------------------------------------------------------------------
 */
int ConsoleIO::GetChar()
{
#if defined(LINUX)
  if (!isendwin() && NULL != g_pWin)
    return getch();
  else
    return getchar();
#else
  return getch();
#endif
}
/*
 *--------------------------------------------------------------------
 * Method:     KbHit()
 * Purpose:    Check if key has been pressed.
 * Arguments:  n/a
 * Returns:    bool - true if key pressed
 *--------------------------------------------------------------------
 */
bool ConsoleIO::KbHit()
{
#if defined(LINUX)
  if (!isendwin() && NULL != g_pWin) {
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch);
        return true;
    } else {
        return false;
    }
  } else {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return (bytesWaiting > 0);

  }
#else
  return (kbhit() != 0);
#endif
}


/*
 *--------------------------------------------------------------------
 * Method:     InitCursesScr()
 * Purpose:    Initialize nsurses screen.
 * Arguments:  n/a
 * Returns:    n/a
 *--------------------------------------------------------------------
 */
void ConsoleIO::InitCursesScr()
{
  fflush(stdin);
#if defined(LINUX)
	if (NULL == g_pWin) {
		g_pWin = initscr();
		cbreak();
    keypad(stdscr, TRUE);
		noecho();
		nodelay(stdscr, TRUE);
		scrollok(stdscr, TRUE);
	} else if (isendwin()) {
    refresh();
  }
#endif
}

/*
 *--------------------------------------------------------------------
 * Method:     closeCursesScr()
 * Purpose:    Close nsurses screen.
 * Arguments:  n/a
 * Returns:    n/a
 *--------------------------------------------------------------------
 */
void ConsoleIO::CloseCursesScr()
{
#if defined(LINUX)
	if (!isendwin() && NULL != g_pWin) {
		endwin();
	}
#endif
}

/*
 *--------------------------------------------------------------------
 * Method:     PrintChar()
 * Purpose:    Print character on the screen.
 * Arguments:  char - character.
 * Returns:    n/a
 *--------------------------------------------------------------------
 */
void ConsoleIO::PrintChar(char c)
{
#if defined(LINUX)
	if (!isendwin() && NULL != g_pWin) {
    echochar(c);
	} else {
    cout << c << flush;
	}
#else
  cout << c;
#endif
}

/*
 *--------------------------------------------------------------------
 * Method:     PrintString()
 * Purpose:    Print string on the screen.
 * Arguments:  char * - pointer to char array.
 * Returns:    n/a
 *--------------------------------------------------------------------
 */
void ConsoleIO::PrintString(string s)
{
#if defined(LINUX)
	if (!isendwin() && NULL != g_pWin) {
		addstr(s.c_str());
		refresh();
	} else {
		cout << s;
	}
#else
	cout << s;
#endif
}

/*
 *--------------------------------------------------------------------
 * Method:     Beep()
 * Purpose:    
 * Arguments:  
 * Returns:    
 *--------------------------------------------------------------------
 */
void ConsoleIO::Beep()
{
#if defined(LINUX)
	if (!isendwin() && NULL != g_pWin) {
		beep();
	} else {
		cout << "\a";
	}
#else
	cout << "\a";
#endif
}

}  // END namespace MKBasic 

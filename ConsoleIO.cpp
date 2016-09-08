/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:        ConsoleIO.cpp
 *
 * Purpose:     Implementation of ConsoleIO class.
 *              The ConsoleIO class has methods helpful when
 *              UI is utilizing STDIO (DOS) console.
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

#endif

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
   system("clear");
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
   cout << "\033[1;1H";
}

#endif // #devine LINUX

}  // END namespace MKBasic 
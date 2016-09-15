/*
 *--------------------------------------------------------------------
 * Project:     VM65 - Virtual Machine/CPU emulator programming
 *                     framework.  
 *
 * File:        ConsoleIO.h
 *
 * Purpose:     Prototype of ConsoleIO class.
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
#ifndef CONSOLEIO_H
#define CONSOLEIO_H

#include <string>
#include <queue>
#include <chrono>
#include <string.h>
#include "system.h"

//#define WINDOWS 1
#if defined (WINDOWS)
#include <windows.h>
#endif

using namespace std;

namespace MKBasic {

   class ConsoleIO
   {
      public:

        ConsoleIO();
        ~ConsoleIO();

        void ClearScreen();
        void ScrHome();
				void InitCursesScr();				
				void CloseCursesScr();
        void PrintChar(char c);
        void PrintString(string s);
        bool KbHit();
        int  GetChar();
				void Beep();

   };

}  // namespace MKBasic

#endif   // #ifndef CONSOLEIO_H 

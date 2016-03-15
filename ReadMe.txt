
Project: MKBasic (VM6502).

Author: Copyright (C) Marek Karcz 2016. All rights reserved.
        Free for personal and non-commercial use.
        Code can be distributed and included in derivative work under
        condition that the original copyright notice is preserved.
        For use in commercial product, please contact me to obtain
        permission and discuss possible fees, at: makarcz@yahoo.com
        This software is provided with no warranty.

Purpose:

MOS 6502 emulator, Virtual CPU/Machine and potentially retro-style 8-bit
computer emulator.
MOS-6502-compatible virtual computer featuring BASIC interpreter, machine code
monitor, input/output device emulation etc.
Program works in DOS/shell console (text mode) only.
Makefile are included to build under Windows 32/64 (mingw compiler required)
and under Linux Ubuntu or Ubuntu based.

To build under Windows 32/64:

* Install MINGW64 under C:\mingw-w64\x86_64-5.3.0 folder.
* Run mingw terminal.
* Change current directory to that of this project.
* Run: makeming.bat

To build under Linux:

* Make sure C++11 compliant version of GCC compiler is installed.
* Change current directory to that of this project.
* Run: make clean all

Program passed following tests:

* 6502 functional test by Klaus Dormann
* AllSuiteA.asm from project hmc-6502

1. Credits/attributions:

Parts of this project is based on or contains 3-rd party work:

- Tiny Basic.
- Enhanced Basic by Lee Davison.
- Microchess by Peter Jennings (http://www.benlo.com/microchess/index.html).
- 6502 functional test by Klaus Dormann.
- All Suite test from project hmc-6502.

2. Format of the memory image definition file.

Program can load raw binary image of MOS 6502 opcodes.
Binary image is always loaded from address $0000 and can be up to 64 kB long, 
so the code must be properly located inside that image. 
Depending on your favorite 6502 assembler, you may need to use proper command
line arguments or configuration to achieve properly formatted binary file.
E.g.: if using CL65 from CC65 package, create configuration file that defines
memory segments that your 6502 code uses, then all of the segments (except the
last one) must have attribute 'fill' set to 'yes' so the unsused areas are
filled with 0-s.
Two CFG files, one for microchess and one for All Suite from hmc-6502 project
are supplied with this project and assembler source code adapted to be
compiled with CL65.
Other assemblers may need a different approach or may not be able to generate
binary images that are required for this emulator.

Program can also load memory image definition file (plain text), which is
a format developed especially for this project.

The format of the plain text memory image definition file is described below:

; comments
ADDR
address
data
ORG
address
data
IOADDR
address
ROMBEGIN
address
ROMEND
address
ENROM
ENIO
EXEC
address

Where:
ADDR 			- label indicating that starting and run address will follow in 
						the next line
ORG 			- label indicating that the address counter will change to the
      			value provided in next line
IOADDR 		- label indicating that character I/O emulation trap address will
						follow in the next line
ROMBEGIN	- label indicating that the emulated read-only memory start address
						will follow in the next line
ROMEND		- label indicating that the emulated read-only memory end address
						will follow in the next line
ENROM			- enable read-only memory emulation
ENIO 			- enable character I/O emulation
EXEC      - label indicating that the auto-execute address will follow
						in the next line						


address - decimal or hexadecimal (prefix $) address in memory

E.g:
ADDR
$0200

or

ADDR
512
 
changes the default start address (256) to 512.

ORG
49152

moves address counter to address 49152, following data will be
loaded from that address forward

data - the multi-line stream of decimal of hexadecimal ($xx) values
       of size unsigned char (byte: 0-255) separated with spaces
       or commas. 

E.g.: 
$00 $00 $00 $00
$00 $00 $00 $00

or

$00,$00,$00,$00

or

0 0 0 0

or

0,0,0,0
0 0 0 0 

Each described above element of the memory image definition file is optional.

3. Character I/O emulation.

Emulator has ability to simulate a 80x25 text output display device and 
rudimentary character I/O functions. The emulation is implemented by the means
of trapping memory locations defined to be designated I/O emulation addresses.
The default memory location is $E000 and also by default, the character I/O
is disabled. It can be enabled from the debug console with 'I' command:

I hexaddr

E.g.:

I E000

or

I FE00

or by putting optional statements in the memory image dedinition file:

ENIO

or

IOADDR
address
ENIO

Where:

address - decimal or hexadecimal (with prefix '$') address in memory
          $0000 - $FFFF.

The same address is used for both, input and output operations.          

Reading from IOADDR inside the 6502 code invokes a blocking character
input function from user's DOS/shell session.
After user enters the character, the memory location contains the character
code and also emulated CPU Acc register contains the same code.

Reading from IOADDR+1 inside 6502 code invokes a non-blocking character
input function from user's DOS/shell session.
This function is different than blocking one in one respect. 
This function will return value 0 in the memory location and Acc register 
if there was no key pressed by the user (no character waiting in buffer).
If there was a key typed, the function will act as the blocking counterpart.

Note that there is no clearly distinguished prompt generated by emulator
when there is character input operation performed. It is designed like that
to avoid interfering with the character I/O performed by native 6502 code.
Therefore if user performs multi-step debugging in the debug console and
program suddenly stops, it is likely waiting for character input.
This is more clear when running the native 6502 code in non-debug execute
mode. In this case the I/O operations are represented on the screen instantly
and 6502 code may also produce prompts so user is aware when to enter data
to the program.

Writing to IOADDR inside the 6502 code will result in character code
being put in the IOADDR memory location and also written to the character
output buffer of the emulated display device. 

When VM is running in one of the debug modes, like step-by-step mode 
(S - step, N - go number of steps) or one of the debug code execution modes
(C- continue or G - go/cont. from new address), that character is not
immediately transferred to the user's DOS/shell session. 
It is only written to the emulated display's text memory.

When VM is running in non-debug code execution mode (X - execute from new 
address), the character is also output to the native DOS/shell console
(user's screen).
The character output history is therefore always kept in the memory of the
emulated text display device and can be recalled to the screen in debug
console with command 'T'.

There are 2 reasons for this:

* Performance. 
In previous version only the emulated text display device approach was used. 
That meant that each time there was a new character in the emulated display
buffer, the entire emulated text output device screen had to be refreshed on
the DOS/shell console. That was slow and caused screen flicker when characters
were output at high rate of speed.

* Record of character I/O operation.
During step-by-step debugging or multiple-step animated registers mode, any 
characters output is immediately replaced by the registers and stack status 
on the screen and is not visible on the screen. However user must be able to 
debug applications that perform character I/O operations and recall the 
history of the characters output to the emulated text display device. This is
when shadow copy of character I/O comes handy.

4. ROM (read-only memory) emulation.

This facility provides very basic means for memory mapping of the read-only
area. This may be required by some 6502 programs that check for non-writable
memory to establish the bounds of memory that can be used for data and code.
One good example is Tiny Basic.
By default the ROM emulation is disabled and the memory range of ROM is
defined as $D000 - $DFFF.
ROM emulation can be enabled (and the memory range defined) using debug
console's command 'K':

K [rombegin] [romend] - to enable

or

K - to disable

The ROM emulation can also be defined and enabled in the memory image
definition file with following statements:

ROMBEGIN
address
ROMEND
address
ENROM

5. Additional comments and remarks.

IOADDR is permitted to be located in the emulated ROM memory range.
The writing to IOADDR is trapped first before checking ROM range and writing
to it is permitted when character I/O emulation and ROM are enabled at the
same time. It is a good idea in fact to put the IOADDR inside ROM range,
otherwise memory scanning routines like the one in Tiny Basic may trigger
unexpected character input because of the reading from IOADDR during the scan.
If you experience unexpected character input prompt while emulating
6502 code, this may be the case. Reconfigure your IOADDR to be inside ROM in
such case and try again.

6. Warranty and License Agreement.

This software is provided with No Warranty.
I (The Author) will not be held responsible for any damage to computer
systems, data or user's health resulting from using this software.
Please use responsibly.
This software is provided in hope that it will be be useful and free of 
charge for non-commercial and educational use.
Distribution of this software in non-commercial and educational derivative
work is permitted under condition that original copyright notices and
comments are preserved. Some 3-rd party work included with this project
may require separate application for permission from their respective
authors/copyright owners.




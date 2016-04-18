
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

Emulator recognizes 4 formats of memory image:

- raw binary, no header, up to 64 kB of raw data,
- binary image with a configuration/snapshot header,
- Intel HEX format,
- plain text memory image definition file.

Please see detailed description of each format below:

Program can load raw binary image of MOS 6502 opcodes.
Binary image is always loaded from address $0000 and can be up to 64 kB long, 
so the code must be properly located inside that image. Image can be shorter
than 64 kB, user will receive warning in such case, but it will be loaded.
Binary image may have header attached at the top.
It consists of magic keyword 'SNAPSHOT' followed by 15 bytes of data
(subject to change in future versions).
The header data saves the status of CPU and emulation facilities like
character I/O address and enable flag, ROM boundaries and enable flag etc.
This header is added when user saves snapshot of the VM from debug console
menu with command: Y [file_name].
Header is not mandatory, so the binary image created outside application can 
also be used. User will receive warning at startup during image load if
header is missing, but image will be loaded. In this case, user may need
to configure necessary emulation facilities manually in debug console
before executing native 6502 code.
When binary image with a header is loaded, user can continue executing the
program from the place where it was saved by executing command from debug
console of the emulator:
   X pc_value
Where:
   pc_value - the address currently showing in CPU's PC register.
If the reset vector is set right, execute the 6502 code right from command
line:
   mkbasic -r image_name
Above will execute the code set in reset vector without having to start it
from debug console. If 6502 code requires character I/O and/or ROM facilities
then image should include header with proper setup.
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
binary images in format required for this emulator. In such case you may need
to design your own custom conversion tools to generate such images.

NOTE:
Simple conversion utility is supplied with this project (bin2hex), which is
described later in this file.

Emulator recognizes Intel HEX format file. It recognizes properly data
records and end-of-file record only at this time. Similar to binary image
with no header, when Intel HEX file is loaded, user may need to configure
necessary emulation facilities manually in debug console before executing
native 6502 code.

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
RESET

Where:
ADDR 		- label indicating that starting and run address will follow in 
			     the next line
ORG 		- label indicating that the address counter will change to the
      			value provided in next line
IOADDR 		- label indicating that character I/O emulation trap address will
				follow in the next line
ROMBEGIN	- label indicating that the emulated read-only memory start
                address	will follow in the next line
ROMEND		- label indicating that the emulated read-only memory end address
				will follow in the next line
ENROM		- enable read-only memory emulation
ENIO 		- enable character I/O emulation
EXEC        - label indicating that the auto-execute address will follow
				in the next line, 6502 program will auto-execute from that
                address after memory definition file is done loading
RESET       - initiate CPU reset sequence after loading memory definition file


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

Emulator is "cycle accurate" but not time or speed accurate. 
This means that each call to MKCpu::ExecOpcode() method is considered a single
CPU cycle, so depending on the executed opcode, multiple calls (# varies per
opcode and other conditions) are needed to complete the opcode execution and
proceed to the next one. Method returns pointer to the the virtual CPU 
registers. One of the members of this structure is named CyclesLeft. When this
variable reaches 0, the opcode execution is considered complete.

The VMachine class calls the ExecOpcode() method as fast as possible, so it is
not real-time accurate, as already mentioned. To implement real-time accurate
emulation, the MKCpu::ExecOpcode() method calls would have to be synchronized
to some fairly accurate time scale (some kind of timer thread or a different
solution).

6. Debugger Console Command Reference.
  
S - step
    Executes single opcode at current address.
C - continue
    Continues code execution from current address until BRK.
M - dump memory
    Usage: M [startaddr] [endaddr]
    Where: startaddr,endaddr - memory addr. in hexadecimal format [0000..FFFF].
    Dumps contents of memory, hexadecimal and ASCII formats."
G - go/continue from new address until BRK
    Usage: G [address]
    Where: address - memory addr. in hexadecimal format [0000.FFFF].
    Executes code at provided address, interrupted by BRK opcode.
X - execute code from new address until RTS
    Usage: X [address]
    Where: address - memory addr. in hexadecimal format [0000.FFFF].
    Executes code at provided address, until RTS (last one).
Q - quit
    Exits from the emulator/debugger.
A - set address for next step
    Usage: A [address]
    Where: address - memory addr. in hexadecimal format [0000.FFFF].
    Sets current address to a new value.
N - go number of steps
    Usage: N [steps]
    Where: steps - number of steps in decimal format
    Execute number of opcodes provided in steps argument starting
    from current address.
P - IRQ
    Send maskable interrupt request to CPU (set the IRQ line LOW).    
W - write to memory
    Usage: W [address] [hexval] [hexval] ... 100
    Where: address - memory addr. in hexadecimal format [0000.FFFF],
           hexval - byte value in hexadecimal format [00.FF].
    Writes provided values to memory starting at specified address.
I - toggle char I/O emulation
    Usage: I [address]
    Where: address - memory addr. in hexadecimal format [0000.FFFF],
    Toggles basic character I/O emulation. When enabled, all writes
    to the specified memory address also writes a character code to
    to a virtual console. All reads from specified memory address
    are interpreted as console character input.
R - show registers
    Displays CPU registers, flags and stack.
Y - snapshot
    Usage: Y [file_name]
    Where: file_name - the name of the output file.
    Save snapshot of current CPU and memory in a binary file.
T - show I/O console
    Displays/prints the contents of the virtual console screen.
    Note that in run mode (commands X, G or C), virtual screen is
    displayed automatically in real-time if I/O emulation is enabled.
E - toggle I/O local echo
    Toggles local echo on/off when I/O emulation is enabled.
B - blank (clear) screen
    Clears the screen, useful when after exiting I/O emulation or
    registers animation (long stack) your screen is messed up.
F - toggle registers animation mode
    When in multi-step debug mode (command: N), displaying registers
    can be suppressed or, when animation mode is enabled - they will
    be continuously displayed after each executed step.
J - set registers status animation delay
    Usage: J [delay]
    Where: delay - time of delay in milliseconds,
    Sets the time added at the end of each execution step in multi
    step mode (command: N). The default value is 250 ms.
K - toggle ROM emulation
    Usage: K [rombegin] [romend] - to enable,
           K - to disable,
           (OR just use 'K' in both cases and be prompted for arguments.)
    Where:
       rombegin - hexadecimal address [0200..FFFF],
       romend   - hexadecimal address [rombegin+1..FFFF].
    Enable/disable ROM emulation and define address range to which the ROM
    (read-only memory) will be mapped. Default range: $D000-$DFFF.
L - load memory image
    Usage: L [image_type] [image_name]
    Where: 
       image_type - B (binary), H (Intel HEX) OR D (definition),
       image_name - name of the image file.
    This function allows to load new memory image from either binary
    image file, Intel HEX format file or the ASCII definition file.
    The binary image is always loaded from address 0x0000 and can be up to
    64kB long. The definition file format is a plain text file that can
    contain following keywords and data:

      ADDR      This keyword defines the run address of the executable code.
                It is optional, but if exists, it must be the 1-st keyword
                in the definition file.
                Address in decimal or hexadecimal ($xxxx) format must follow
                in the next line.
                
      ORG       Changes the current address counter. The line that follows
                sets the new address in decimal or hexadecimal format.
                Data that follows will be put in memory starting from that
                address. This keyword is optional and can be used multiple
                times in the definition file.
                
      IOADDR    Defines the address of the character I/O emulation. The
                next line sets the address of I/O emulation in decimal or
                hexadecimal format. If the I/O emulation is enabled
                (see ENIO keyword), then any character written to this
                address will be sent to the virtual console. The reading
                from that address will invoke character input from the
                emulated console. That input procedure is of blocking
                type. To invoke non-blocking character procedure, reading
                should be performed from IOADDR+1.
                
      ROMBEGIN  Defines the address in memory where the beginning of the
                Read Only memory is mapped. The next line that follows this
                keyword sets the address in decimal or hexadecimal format.
                
      ROMEND    Defines the address in memory where the end of the Read
                Only Memory is mapped. The next line that follows this
                keyword sets the address in decimal or hexadecimal format.
                
      ENIO      Putting this keyword in memory definition file enables
                rudimentary character I/O emulation and virtual console
                emulation.
                
      ENROM     Putting this keyword in memory definition file enables
                emulation of Read Only Memory, in range of addresses
                defined by ROMBEGIN and ROMEND keywords.
                
      EXEC      Define starting address of code which will be automatically
                executed after the memory image is loaded.
                The next line that follows this keyword sets the address
                in decimal or hexadecimal format.

      RESET     Enables auto-reset of the CPU. After loading the memory
                definition file, the CPU reset sequence will be initiated.                      
                
O - display op-codes history
    Show the history of last executed op-codes/instructions, full with
    disassembled mnemonic and argument.
D - diassemble code in memory
    Usage: D [startaddr] [endaddr]
    Where: startaddr,endaddr - hexadecimal address [0000..FFFF].
    Attempt to disassemble code in specified address range and display
    the results (print) on the screen in symbolic form.
0 - reset
    Run the processor initialization sequence, just like the real CPU
    when its RTS signal is set to LOW and HIGH again. CPU will disable
    interrupts, copy address from vector $FFFC to processors PC and will
    start executing code. Programmer must put initialization routine
    under address pointed by $FFFC vector, which will set the arithmetic
    mode, initialize stack, I/O devices and enable IRQ if needed before
    jumping to main loop. The reset routine disables trapping last RTS
    opcode if stack is empty, so the VM will never return from opcodes
    execution loop unless user interrupts with CTRL-C or CTRL-Break.
                    
NOTE:
    1. If no arguments provided, each command will prompt user to enter
       missing data.
    2. It is possible to exit from running program to debugger console
       by pressing CTRL-C or CTRL-Pause/Break, which will generate
       a "Operator Interrupt". However in the character input mode
       use CTRL-Y combination or CTRL-Break (DOS), CTRL-C (Linux).
       You may need to press ENTER after that in input mode (DOS)       

7. Command line usage.

D:\src\wrk\mkbasic>mkbasic -h
Virtual Machine/CPU Emulator (MOS 6502) and Debugger.
Copyright (C) by Marek Karcz 2016. All rights reserved.


Usage:

        mkbasic [-h] | [ramdeffile] | [-b ramimage] | [-r ramimage]
        OR
        mkbasic [-x intelheximage]

Where:

        ramdeffile    - RAM definition file name
        intelheximage - Intel HEX format file
        ramimage      - RAM binary image file name


When ran with no arguments, program will load default memory
definition files: default.rom, default.ram and will enter the debug
console menu.
When ramdeffile argument is provided, program will load the memory
definition from the file, set the flags and parameters depending on the
contents of the memory definition file and enter the corresponding mode
of operation as defined in that file.
If used with flag -b or -x, program will load memory from the provided image
file and enter the debug console menu.
If used with flag -r, program will load memory from the provided image
file and execute CPU reset sequence.

8. Utilities.

Utility bin2hex is supplied with the project to aid in conversion from raw
binary memory image to one of the plain text formats recognized by emulator
NOTE: In current version, emulator can load raw binary format directly, so
      usefulness of this utility is somewhat deprecated.

D:\src\wrk\mkbasic>bin2hex -h

Program: bin2hex
  Convert binary file to Intel HEX format.
OR
  Convert binary file to memory image definition for MKBASIC (VM65) emulator.

Copyright: Marek Karcz 2016. All rights reserved.
Free for personal and educational use.

Usage:

  bin2hex -f input -o output [-w addr] [-x exec] [[-s] [-z] | -i]

Where:

  input  - binary file name
  output - output file name
  addr   - starting address to load data (default: 2048)
  exec   - address to auto-execute code from (default: 2048)
  -s     - suppress auto-execute statement in output
  -z     - suppress data blocks with 0-s only
  -i     - convert to Intel HEX format
           NOTE: When this switch is used, addr, exec, -s, -z are ignored,
                 addr = 0, exec is not set and data blocks with 0-s only
                 are always suppressed.

9. Warranty and License Agreement.

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





Project: (Working codename:) VM6502Q (but I'm open to good suggestions).

Author: Derivative work by Daniel Strano, (c) 2018

Purpose:

Quantum computational superset of Marek Karcz's MOS 6502 emulator. (Thank you, Marek!)
The accumulator, X register, and sign, zero, carry, and overflow flags become qubit-based.
Generalized quantum register behavior is provided by the "Qrack" project, Copyright (C) Daniel Strano 2018.
More documentation on quantum functionality is to follow. (Quantum opcodes are available in MKCpu.cpp and MKCpu.h.)

See end of this file for a brief overview of quantum functionality.
 

--Derived from:

/////////////////////////////BEGIN ORIGINAL README//////////////////////////////

Project: MKBasic (a.k.a. VM6502, a.k.a. VM65, I just can't decide
                  how to name it :-)).

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
Main UI of the program works in DOS/shell console.
Graphics display emulation requires SDL2.
Makefile are included to build under Windows 32/64 (mingw compiler required)
and under Linux Ubuntu or Ubuntu based distro.
SDL2 library must be on your execution path in order to run program.
E.g.:
set PATH=C:\src\SDL\lib\x64;%PATH%

To build under Windows 32/64:

* Install MINGW64 under C:\mingw-w64\x86_64-5.3.0 folder.
* Run mingw terminal.
* Change current directory to that of this project.
* Set environment variable SDLDIR. (E.g.: set SDLDIR=C:\src\SDL)
* Run: makeming.bat

To build under Linux:

* Make sure C++11 compliant version of GCC compiler is installed.
* Change current directory to that of this project.
* Set environment variable SDLDIR.
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
Older version of header consists of magic keyword 'SNAPSHOT' followed by 15 
bytes of data - this format had no space for expansion and will be removed
in future version. All new snapshots are saved in newest format.
Current version of header consists of magic keyword 'SNAPSHOT2' followed by
128 bytes of data. Not all of the 128 bytes are used, so there is a space
for expansion without the need of changing the file format.
The header data saves the status of CPU and emulation facilities like
character I/O address and enable flag, ROM boundaries and enable flag etc.
This header is added when user saves snapshot of the VM from debug console
menu with command: Y [file_name].
Below is the full detailed description of the header format:

 * MAGIC_KEYWORD
 * aabbccddefghijklmm[remaining unused bytes]
 *
 * Where:
 *    MAGIC_KEYWORD - text string indicating header, may vary between
 *                    versions thus rendering headers from previous
 *                    versions incompatible - currently: "SNAPSHOT2"
 *
 *    Data:
 *
 *    aa - low and hi bytes of execute address (PC)
 *    bb - low and hi bytes of char IO address
 *    cc - low and hi bytes of ROM begin address
 *    dd - low and hi bytes of ROM end address
 *    e - 0 if char IO is disabled, 1 if enabled
 *    f - 0 if ROM is disabled, 1 if enabled
 *    g - value in CPU Acc (accumulator) register
 *    h - value in CPU X (X index) register
 *    i - value in CPU Y (Y index) register
 *    j - value in CPU PS (processor status/flags)
 *    k - value in CPU SP (stack pointer) register
 *    l - 0 if generic graphics display device is disabled,
 *        1 if graphics display is enabled
 *    mm - low and hi bytes of graphics display base address
 *    [remaining unused bytes are filled with 0-s]

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
ENGRAPH
GRAPHADDR
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
              address will follow in the next line
ROMEND		- label indicating that the emulated read-only memory end address
			  will follow in the next line
ENROM		- enable read-only memory emulation
ENIO 		- enable character I/O emulation
EXEC        - label indicating that the auto-execute address will follow
			  in the next line, 6502 program will auto-execute from that
              address after memory definition file is done loading
ENGRAPH     - enable generic graphics display device emulation with default 
              base address
GRAPHADDR   - label indicating that base address for generic graphics display
              device will follow in next line, also enables generic graphics
              device emulation, but with the customized base address
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

The difference between blocking and non-blocking character input from 6502
code perspective:

 - blocking

   Just read from the address. Emulated device will wait for the character
   input. The 6502 code does not need to be polling the address for
   a keystroke.

 - non-blocking

   If there is no character/keystroke in the keyboard buffer, the function
   will move on returning the value of 0. If implementing waiting character
   input in 6502 code with this function, the 6502 code needs to be polling
   the value returned from this address in a loop until it is different
   than 0.

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
(user's screen, a.k.a. standard output).
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
on the screen and is not visible on the screen. However user, while debugging 
must be able to recall the history of the characters output to the emulated
text display device if user's program uses character I/O, for normal operation
or diagnostic purposes. This is when shadow copy of character I/O comes handy.
In real life, text output devices like text terminals, serial terminals,
especially the software emulated ones also offer the facilities to buffer the
output to memory for later review (history).

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

For more sophisticated memory mapping schemes including multiple ROM ranges
and/or MMU-s that allow to read from ROM, write to RAM in the same range etc.
I suggest to write your ROM device emulating code and use it to expand the
functionality of this emulator using memory Mapped Devices framework,
explained later in this document and in the programmer's reference manual
document included with this project.

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
proceed to the next one. Method returns pointer to the virtual CPU registers.
One of the members of this structure is named CyclesLeft. When this variable
reaches 0, the opcode execution is considered complete.

The VMachine class calls the ExecOpcode() method as fast as possible, so it is
not real-time accurate, as already mentioned. To implement real-time accurate
emulation, the MKCpu::ExecOpcode() method calls would have to be synchronized
to some fairly accurate time scale (some kind of timer thread or a different
solution) to emulate the timing on the bus signals level. This emulator does
not implement such level of accuracy.

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
V - toggle graphics display (video) emulation
    Usage: V [address]
    Where: address - memory addr. in hexadecimal format [0000.FFFF],
    Toggles basic raster (pixel) based RGB graphics display emulation.
    When enabled, window with graphics screen will open and several
    registers are available to control the device starting at provided
    base address. Read programmers reference for detailed documentation
    regarding the available registers and their functions.
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
       image_type - A - (auto), B (binary), H (Intel HEX) OR D (definition),
       image_name - name of the image file.
    This function allows to load new memory image from either binary
    image file, Intel HEX format file or the ASCII definition file.
    With option 'A' selected, automatic input format detection will be
    attempted.
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

      ENGRAPH   Enables raster graphics device emulation.

      GRAPHADDR Defines the base address of raster graphics device. The next
                line that follows sets the address in decimal or hexadecimal
                format.

     NOTE: The binary image file can contain a header which contains
           definitions corresponding to the above parameters at fixed
           positions. This header is created when user saves the snapshot of
           current emulator memory image and status. Example use scenario:
           * User loads the image definition file.
           * User adjusts various parameters of the emulator
             (enables/disables devices, sets addresses, changes memory
             contents).
           * User saves the snapshot with 'Y' command.
           * Next time user loads the snapshot image, all the parameters
             and memory contents stick. This way game status can be saved
             or a BASIC interpreter with BASIC program in it.
           See command 'Y' for details.
                
O - display op-codes history
    Show the history of last executed op-codes/instructions, full with
    disassembled mnemonic, argument and CPU registers and status.
    NOTE: op-codes execute history must be enabled, see command 'U'.
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
? - display commands menu
    Display the menu of all available in Debug Console commands.
U - enable/disable exec. history
    Toggle enable/disable of op-codes execute history.
    Disabling this feature improves performance.
Z - enable/disable debug traces
    Toggle enable/disable of debug traces.
2 - display debug traces
    Display recent debug traces.
1 - enable/disable performance stats
    Toggle enable/disable emulation speed measurement.
                    
NOTE:
    1. If no arguments provided, each command will prompt user to enter
       missing data.
    2. It is possible to exit from running program to debugger console
       by pressing CTRL-C or CTRL-Pause/Break, which will generate
       a "Operator Interrupt". However in the character input mode
       use CTRL-Y combination or CTRL-Break (DOS), CTRL-C (Linux).
       You may need to press ENTER after that in input mode (DOS).

7. Command line usage.

D:\src\wrk\mkbasic>vm65 -h
Virtual Machine/CPU Emulator (MOS 6502) and Debugger.
Copyright (C) by Marek Karcz 2016. All rights reserved.


Usage:

        vm65 [-h] | [ramdeffile] [-b | -x] [-r]


Where:

        ramdeffile    - RAM definition file name
        -b            - specify input format as binary
        -x            - specify input format as Intel HEX
        -r            - after loading, perform CPU RESET
        -h            - print this help screen


When ran with no arguments, program will load default memory
definition files: default.rom, default.ram and will enter the debug
console menu.
When ramdeffile argument is provided with no input format specified,
program will attempt to automatically detect input format and load the
memory definition from the file, set the flags and parameters depending
on the contents of the memory definition file and enter the corresponding
mode of operation as defined in that file.
If input format is specified (-b|-x), program will load memory from the
provided image file and enter the debug console menu.

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

9. Memory Mapped Device abstraction layer.

In microprocessor based systems in majority of cases communication with
peripheral devices is done via registers which in turn are located under
specific memory addresses.
Programming API responsible for modeling this functionality is implemented
in Memory and MemMapDev classes. The Memory class implements access to
specific memory locations and maintains the memory image.
The MemMapDev class implements specific device address spaces and handling
methods that are triggered when addresses that belong to the device are
accessed by the microprocessor.
Programmers can expand the functionality of this emulator by adding necessary
code emulating specific devices in MemMapDev and Memory classes implementation
and header files. In current version, two basic devices are implemented:
character I/O and raster (pixel based) graphics display. Both can be activated
or inactivated at will and provide simple register based interface that
requires no extra memory space use for data.
E.g.: 
Character I/O device uses just 2 memory locations, one for non-blocking I/O
and one for blocking I/O. Writing to location causes character output, while
reading from location waits for character input (blocking mode) or reads the
character from keyboard buffer if available (non-blocking mode) or returns 0.
The graphics display can be accessed by writing to multiple memory locations.

If we assume that GRDEVBASE is the base address of the Graphics Device, there
are following registers:

Offset   Register               Description
----------------------------------------------------------------------------
 0       GRAPHDEVREG_X_LO       Least significant part of pixel's X (column)
                                coordinate or begin of line coord. (0-255)
 1       GRAPHDEVREG_X_HI       Most significant part of pixel's X (column)
                                coordinate or begin of line coord. (0-1)
 2       GRAPHDEVREG_Y          Pixel's Y (row) coordinate (0-199)
 3       GRAPHDEVREG_PXCOL_R    Pixel's RGB color component - Red (0-255)
 4       GRAPHDEVREG_PXCOL_G    Pixel's RGB color component - Green (0-255)
 5       GRAPHDEVREG_PXCOL_B    Pixel's RGB color component - Blue (0-255)
 6       GRAPHDEVREG_BGCOL_R    Backgr. RGB color component - Red (0-255)
 7       GRAPHDEVREG_BGCOL_G    Backgr. RGB color component - Green (0-255)
 8       GRAPHDEVREG_BGCOL_B    Backgr. RGB color component - Blue (0-255)
 9       GRAPHDEVREG_CMD        Command code
10       GRAPHDEVREG_X2_LO      Least significant part of end of line's X
                                coordinate
11       GRAPHDEVREG_X2_HI      Most significant part of end of line's X
                                coordinate                                
12       GRAPHDEVREG_Y2         End of line's Y (row) coordinate (0-199)
13       GRAPHDEVREG_CHRTBL     Set the 2 kB bank where char. table resides
14       GRAPHDEVREG_TXTCURX    Set text cursor position (column)
15       GRAPHDEVREG_TXTCURY    Set text cursor position (row)
16       GRAPHDEVREG_PUTC       Output char. to current pos. and move cursor
17       GRAPHDEVREG_CRSMODE    Set cursor mode : 0 - not visible, 1 - block
18       GRAPHDEVREG_TXTMODE    Set text mode : 0 - normal, 1 - reverse

NOTE: Functionality maintaining text cursor is not yet implemented.

Writing values to above memory locations when Graphics Device is enabled
allows to set the corresponding parameters of the device, while writing to
command register executes corresponding command (performs action) per codes
listed below:

Command code                    Command description
------------------------------------------------------------------------------
GRAPHDEVCMD_CLRSCR = 0          Clear screen
GRAPHDEVCMD_SETPXL = 1          Set the pixel location to pixel color
GRAPHDEVCMD_CLRPXL = 2          Clear the pixel location (set to bg color)
GRAPHDEVCMD_SETBGC = 3          Set the background color
GRAPHDEVCMD_SETFGC = 4          Set the foreground (pixel) color
GRAPHDEVCMD_DRAWLN = 5          Draw line
GRAPHDEVCMD_ERASLN = 6          Erase line

Reading from registers has no effect (returns 0).

Above method of interfacing GD requires no dedicated graphics memory space
in VM's RAM. It is also simple to implement.
The downside - slower performance (multiple memory writes to select/unselect
a pixel or set color).
I plan to add graphics frame buffer in the VM's RAM address space in future
release.

Simple demo program written in EhBasic that shows how to drive the graphics
screen is included: grdevdemo.bas.

10. Performance considerations.

Program measures the emulation performance and displays it in the Debug
Console. It uses a 1 MHz CPU as a reference to return % of speed compared to
assumed 1,000,000 CPU cycles or clock ticks per second - which is considered
a 100 % speed.
Performance is measured during the whole execution cycle and calculated at the
end of the run. Captured speed is summed with previous result and divided by 2
to produce average emulation speed during single session.

This emulator has been optimized for performance. I had issues with
emulation speed in previous version, mostly because it was a prototype with
many debugging aids enabled by default and not yet optimized for speed.
I took a good look at all the critical parts of code and fixed the problems.
I am sure there is still space for improvement, but now the emulation speed
leaves good margin for expansion with new emulated peripherals and still
should compare well to the model 1 MHz CPU.
Emulating of pure 6502 machine code with all peripherals (memory mapped
devices, I/O etc.) emulation disabled and time critical debugging facility,
the op-codes execute history also disabled, returns performance in range of
646 % (PC1) and 468 % (PC2) (* see annotation below).
Enabling the op-code execute history drops the performance.
With all peripherals disabled and op-code history enabled we are down to
411 % on PC1 and 312 % on PC2.

Enabling and adding the emulated memory mapped devices to the pool may cause
the emulation speed to drop as well. However even with currently implemented
peripherals (char I/O, graphics raster device) enabled and actively used and
op-codes execute history enabled the performance is still well above 300 %
on both PC1 and on PC2 (* see annotations for PC configurations/specs).
The same case but with op-code execute history disabled - performance exceeds
400 % on both PC configurations.

Currently the main emulation loop is not synchronized to an accurate
clock tick or raster synchronization signal but just runs as fast as it can.
Therefore emulation speed may vary per PC and per current load on the system.

If this code is to be used as a template to implement emulator of a real-world
machine, like C-64 or Apple I, it may need some work to improve performance,
but I think is leaves a pretty good margin for expansion as it is.
On a fast PC (* see annotation) the emulation speed above 600 % with
basically nothing but CPU emulated and op-codes execute history disabled
(which should be disabled by default as it is needed for debugging purposes
only) is IMO decent if we don't want to emulate MOS 6502 machine with clock
much faster than 1 MHz.

Annotations to 'Performance considerations':
*)

PC1 stats:
Type:           Desktop
CPU:            2.49 GHz (64-bit Quad-core Q8300)
RAM:            4,060 MB
OS:             Windows 10 Pro (no SP) [6.2.9200]

PC2 stats:
Type:           Laptop
CPU:            2.3 GHz (64-bit Quad-core i5-6300HQ)
RAM:            15.9 GB
OS:             Win 10 Home.

11. Problems, issues, bugs.

* Regaining focus of the graphics window when it is not being written to by the
  6502 code is somewhat flakey. Since the window has no title bar, user can
  only switch to it by ALT-TAB (windows) or clicking on the corresponding icon
  on the task bar. However it doesn't always work. Switching to the DOS console
  of emulator while in emulation mode should bring the graphics window back
  to front.

12. Warranty and License Agreement.

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


//////////////////////////////END ORIGINAL README///////////////////////////////

13. Quantum functionality overview

This version of VM6502 (or MKBasic, or VM65) mirrors the state of the accumulator,
X register, and sign, zero, carry, and overflow flags in qubits and qubytes.

By a result well known to physicists called "Ehrenfest's theorem," the expectation
for the quantum trajectory of the processor should match that of the classical
equivalent of the QCPU. Hence, qubits are mirrored here with classical bits for
debugging. Qubit measurements take precedence, in the case that quantum uncertainty
and superposition lead to a deviation from the classical trajectory of the QCPU.

New opcodes have been added to leverage quantum functionality. (These can be looked
up in the reference table in MKCpu.cpp.) Two opcodes are quantum Fourier transforms
of the accumulator and X register. ("FTA," "FTX.") The rest of the new opcodes are
quarter rotations along various qubit axes. These are quarter turns, in that four
applications of the same new opcode should return any register to its original state.

The quarter rotation opcodes have the effects of putting |0> and |1> bits into 50/50
superposition along various axes of rotation. (The exceptions to this are "R1A" and
"R1X," which rotate each bit in the register one quarter turn around the |1> axis.
This is purely an effect on phase and not on probability, for |0> and |1> bits.)
Hence, with the exception of R1 rotations, these gates have the classical effect of
performing an operation like (reg = (reg + 127) & 0xFF) on the accumulator or X
register.

If the x register is in superposition, in can also address memory in superposition!
All memory besides the accumulator, X register, and four flags is assumed to be
classical. If the X register addresses classical memory from a state of superposition,
it superposes the values in classical memory by the probability and phases of the
superposition of the X register. This might be a realistic access model for true
quantum hardware, since superconducting quantum interference devices ("SQUIDs") have
already been observed to pass currents in superposition. With a super-cooled small
cache of classical RAM, a QCPU could be capable of addressing classical memory in
superposition, this way.  

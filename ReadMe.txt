
Project: MKBasic
Author: Copyright (C) Marek Karcz 2016. All rights reserved.
Purpose:
MOS 6502 emulator, Virtual CPU/Machine and potentially retro-style 8-bit computer emulator,
MOS-6502-compatible virtual computer featuring BASIC interpreter, machine code monitor etc.

Memory images extensions: .RAM, .ROM

Format of the memory definition file:

; comment
ADDR
address
data
ORG
address

Where:
ADDR - label indicating that starting address will follow in next
       line
ORG - label indicating that the address counter will change to the
      value provided in next line
address - decimal or hexadecimal (prefix $) address in memory

E.g:
ADDR
$200

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
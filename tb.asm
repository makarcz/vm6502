;----------------------------------------------------------------------------------
; Tiny Basic port to MKHBC-8-R2 6502 computer
; by Marek Karcz 2012.
;
; Based on the work by:
;
; ================= ORIGINAL HEADER ======================================
;
; OMS-02 Firmware
;
; Bill O'Neill - Last update: 2008/12/06
;
; Tiny Basic and minimal Monitor (BIOS)
;
; This version is for the assembler that comes with
; Michal Kowalski's 6502 emulator.  To run it, load it
; into the emulator, assemle it, begin debug mode then,
; set the PC to $1CF0 then run.
;
; The emulator was used to help figure out Tiny Basic
; and this code is only documented to that extent.
;
; It should be easy enough to configure this to run any-
; where in memory or convert it to assemble with any 6502
; assembler.  The current location is being used as this is
; to be placed into a controller I designed (OMS-02) around
; a 6507 CPU (a 6502 variant) that has only 8k off memory.
; The memory map for the OMS-02 is;
;
; $0000-$0FFF     RAM
; $1000-$13FF     I/O space (ACIA is at $1200)
; $1400-$1FFF     Tiny Basic and simple monitor
;
;
; Next steps:
;        Write a BREAK routine that will enable a break if any charater is typed at the console
;        More comments to document this code
;        Investigate using assembler variables and directives to make it easy to re-locate this code
;
; ============ END OF ORIGINAL HEADER =========================================
;
; Revision history (MKHBC-8-R1 port):
; 
; 1/4/2012:
;    TB port created.
;
; 1/5/2012:
;    Version 1.0.1.
;    Added 2-byte peek routine.
;
; 1/11/2012:
;	 Figured out jump addresses for routines and used symbolic labels
;	 instead of hardcoded addresses. Relocated TB to $0400 (from $1400).
;
; 2/15/2016
;    Ported to my own 6502 emulator.
;
; 2/20/2016
;    Increased stack size. Changed prompt.
;
;
;--------------------------------------------------------------------------------------

;.segment "BASIC"

;
; Tiny Basic starts here
;
    .org      $0400                 ; Start of Basic.  First 1K of ROM is shaded by I/O on the OMS-02

SOBAS:

;CLRSC	=	ClearScreen
C_00BC   =     $00BC             	; These are needed because my assembler
C_20     =     $20               	; does not hanle standard 6502 notation
C_22     =     $22               	; properly
C_24     =     $24               	;
C_2A     =     $2A               	; I may just fix this someday - HA!
C_2C     =     $2C               	;
C_2E     =     $2E               	;
C_B8     =     $B8               	;
C_BC     =     $BC               	;
C_C1     =     $C1               	;
C_C2     =     $C2               	;
C_C6     =     $C6               	;

SR_V_L   =     SOBAS + $1E         	; Base address for subroutine vector lo byte
SR_V_H   =     SOBAS + $1F         	; Base address for subroutine vector hi byte

CV:       jmp      COLD_S            ; Cold start vector
WV:       jmp      WARM_S            ; Warm start vector
IN_V:     jmp      RCCHR             ; Input routine address. 
OUT_V:    jmp      SNDCHR            ; Output routine address.
BREAK:    nop                        ; Begin dummy break routine
          clc                        ; Clear the carry flag
          rts                        ; End break routine
;
; Some codes
;
BSC:      .byte $5f                   	; Backspace code
LSC:      .byte $18                   	; Line cancel code
PCC:      .byte $80                   	; Pad character control
TMC:      .byte $00                   	; Tape mode control
;SSS:      .byte $04                   	; Spare Stack size. (was $04 but documentation suggests $20 ?)
SSS:      .byte $20                   	; Spare Stack size. (was $04 but documentation suggests $20 ?)

;
; Code fragment
; Seems to store or retreive a byte to/from memory and the accumulator
; ---
; M.K.: I think this code it is used from Tiny Basic programs by calling USR to store/retrieve
;       data (since TB does not support arrays, this is one way to emulate them).
; This location is at exactly 20 bytes distance from the start of TB code.
; TB programs typically make USR calls to locations: 
; SOBAS+20
; SOBAS+24
; E.g:
; LET S=_SOBAS_ REM (_SOBAS_ is TB start)
; LET B=_ARRAY_SIZE_+2 (_ARRAY_SIZE_ is the size of 2-dim array)
; Get byte from location V(I,J), store in Z.
; LET Z=USR(S+20,V+B*I+J,0)
; Store Z in location V(I,J)
; LET Z=USR(S+24,V+B*I+J,Z)
; From TB documentation:
;   For your convenience two subroutines have been included in the TINY BASIC interpreter to access memory.  
;   If S contains the address of the beginning of the TINY BASIC interpreter (256 for standard 6800, 512 
;   for standard 6502, etc.), then location S+20 (hex 0114) is the entry point of a subroutine to read one 
;   byte from the memory address in the index register, and location S+24 (hex 0118) is the entry point of 
;   a subroutine to store one byte into memory. 
;-------------------------------------------------
;      USR (address)
;      USR (address,Xreg)
;      USR (address,Xreg,Areg)
;
;   This function is actually a machine-language subroutine call to the address in the first argument.  
;   If the second argument is included the index registers contain that value on entry to the subroutine, 
;   with the most significant part in X.  If the third argument is included, the accumulators contain that 
;   value on entry to the subroutine, with the least significant part in A.  On exit, the value in the 
;   Accumulators (for the 6800; A and Y for the 6502) becomes the value of the function, with the least 
;   significant part in A.  All three arguments are evaluated as normal expressions.
;   It should be noted that machine language subroutine addresses are 16-bit Binary numbers.  TINY BASIC 
;   evaluates all expressions to 16-bit binary numbers, so any valid expression may be used to define a 
;   subroutine address.  However, most addresses are expressed in hexadecimal whereas TINY BASIC only accepts 
;   numerical constants in decimal.  Thus to jump to a subroutine at hex address 40AF, you must code USR(16559). 
;   Hex address FFB5 is similarly 65461 in decimal, though the equivalent (-75) may be easier to use. 
; 

OneBytePeek:

         stx $C3		; read one byte from memory address in the index register
         bcc LBL008

OneBytePoke:

         stx $C3		; store one byte into memory
         sta (C_C2),Y
         rts
LBL008:  lda (C_C2),Y
         ldy #$00
         rts

	
	 ; These seem to be addresses associated with the IL branch instructions
     ;.byte $62,$15, $64,$15, $D8,$15, $05,$16, $33,$16, $FD,$15    	
	 .byte <JUMP01,>JUMP01
	 .byte <LBL027,>LBL027
	 .byte <JUMP02,>JUMP02
	 .byte <JUMP03,>JUMP03
	 .byte <JUMP04,>JUMP04
	 .byte <JUMP05,>JUMP05

	; NOTE: The original comment below is obsolete now, since I found the jump
	;       addresses locations and put them in this table. Now assembler will
	;		take care of the reallocation.
	 ; START OF ORIGINAL COMMENT
	 ; This appears to be a table of
	 ; execution addresses to jump to ML routines
	 ; that handle the the IL instructions.
	 ; You cannot add code to or relocate this program
	 ; without updating these
	 ; END OF ORIGINAL COMMENT

     ;.byte $9F,$17, $42,$1B, $3F,$1B, $7A,$17, $FC,$18, $95,$17, $9F,$17, $9F,$17 
	 .byte <LBL067,>LBL067
	 .byte <LBL132,>LBL132
	 .byte <JUMP06,>JUMP06
	 .byte <JUMP07,>JUMP07
	 .byte <LBL035,>LBL035
	 .byte <LBL097,>LBL097
	 .byte <LBL067,>LBL067
	 .byte <LBL067,>LBL067
	 
     ;.byte $BD,$1A, $C1,$1A, $8A,$1A, $9B,$1A, $E9,$1A, $61,$17, $51,$17, $41,$1A 
	 .byte <JUMP08,>JUMP08
	 .byte <JUMP09,>JUMP09
	 .byte <JUMP10,>JUMP10
	 .byte <JUMP11,>JUMP11
	 .byte <JUMP12,>JUMP12
	 .byte <JUMP13,>JUMP13
	 .byte <JUMP14,>JUMP14
	 .byte <LBL071,>LBL071
	 
     ;.byte $52,$1A, $4F,$1A, $62,$1A, $E7,$19, $CD,$16, $06,$17, $9F,$17, $15,$18 
	 .byte <JUMP15,>JUMP15
	 .byte <JUMP16,>JUMP16
	 .byte <JUMP17,>JUMP17
	 .byte <JUMP18,>JUMP18
	 .byte <JUMP19,>JUMP19
	 .byte <JUMP20,>JUMP20
	 .byte <LBL067,>LBL067
	 .byte <JUMP21,>JUMP21
	 
     ;.byte $A7,$17, $B7,$16, $BF,$16, $83,$18, $A1,$16, $9F,$17, $9F,$17, $A8,$18 
	 .byte <JUMP22,>JUMP22
	 .byte <JUMP23,>JUMP23
	 .byte <LBL047,>LBL047
	 .byte <LBL081,>LBL081
	 .byte <LBL013,>LBL013
	 .byte <LBL067,>LBL067
	 .byte <LBL067,>LBL067
	 .byte <JUMP24,>JUMP24
	 
     ;.byte $4F,$1B, $4D,$1B, $07,$19, $AA,$14, $37,$17, $BD,$14, $1B,$1B, $B1,$1A 
     .byte <JUMP25,>JUMP25
	 .byte <JUMP26,>JUMP26
	 .byte <JUMP27,>JUMP27
	 .byte <MEM_T2,>MEM_T2
	 .byte <JUMP28,>JUMP28
	 .byte <WARM_S,>WARM_S
	 .byte <JUMP29,>JUMP29
	 .byte <JUMP30,>JUMP30

     .byte $20,$41, $54,$20     ; No idea ????

     .byte $80                  ; No idea
         
LBL002:  .byte <ILTBL	;$70                  ; $70 - lo byte of IL address
LBL003:  .byte >ILTBL	;$1B                  ; $1B - hi byte of IL address

;
;Begin Cold Start
;
;
;
; Load start of free ram ($1C00) into locations $0020 and $0022
; The memory between $1000 and $1Bff (3 kB) is free for assembler programs
; that can be loaded and used simultaneously with TB.
; $1C00 to the end of writable memory is the BASIC memory space.
;
COLD_S:  lda #$00                   ; Load accumulator with lo byte of lower and upper prg memory limits
         sta $20                    ; Store in $20
         sta $22                    ; Store in $22
         lda #$1C                   ; Load accumulator with hi byte of lower and upper prg memory limits
         sta $21                    ; Store in $21
         sta $23                    ; Store in $23
		 ; NOTE: $22,$23 vector will be updated by routine below to be the upper RAM limit for TB.
;
;
; Begin test for free ram
; As a result, memory locations $20,$21 will contain lo,hi byte order address of lower RAM boundary
; and $22,$23 upper RAM boundary respectively.
;
         
         ldy #$01                   ; Load register Y with $01
MEM_T:   lda (C_22),Y               ; Load accumulator With the contents of a byte of memory
         tax                        ; Save it to X
         eor #$FF                   ; Next 4 instuctions test to see if this memeory location
         sta (C_22),Y               ; is ram by trying to write something new to it - new value
         cmp (C_22),Y               ; gets created by XORing the old value with $FF - store the
         php                        ; result of the test on the stack to look at later
         txa                        ; Retrieve the old memory value
         sta (C_22),Y               ; Put it back where it came from
         inc $22                    ; Increment $22 (for next memory location)
         bne SKP_PI                 ; Goto $14A6 if we don't need to increment page
         inc $23                    ; Increment $23 (for next memory page)
SKP_PI:  plp                        ; Now look at the result of the memory test
         beq MEM_T                  ; Go test the next mempry location if the last one was ram
         dey                        ; If last memory location did not test as ram, decrement Y (should be $00 now)
MEM_T2:  cld                        ; Make sure we're not in decimal mode
         lda $20                    ; Load up the low-order by of the start of free ram
         adc SSS                    ; Add to the spare stack size
         sta $24                    ; Store the result in $0024
         tya                        ; Retrieve Y
         adc $21                    ; And add it to the high order byte of the start of free ram (this does not look right)
         sta $25                    ; Store the result in $0025
         tya                        ; Retrieve Y again
         sta (C_20),Y               ; Store A in the first byte of program memory
         iny                        ; Increment Y
         sta (C_20),Y               ; Store A in the second byte of program memory
;
;Begin Warm Start;
;
WARM_S:  lda $22
         sta $C6
         sta $26
         lda $23
         sta $C7
         sta $27
         jsr LBL001                 ; Go print CR, LF and pad charachters
LBL014:  lda LBL002
         sta $2A
         lda LBL003
         sta $2B
         lda #$80
         sta $C1
         lda #$30
         sta $C0
         ldx #$00
         stx $BE
         stx $C2
         dex
         txs
LBL006:  cld
         jsr LBL004                 ; Go read a byte from the TBIL table
         jsr LBL005
         jmp LBL006
;
;
;
         .byte $83                  ; No idea about this
         .byte $65                  ; No idea about this
;
;
; Routine to service the TBIL Instructions
;
LBL005:  cmp #$30                   ;
         bcs LBL011                 ; If it's $30 or higher, it's a Branch or Jump - go handle it
         cmp #$08                   ; 
         bcc LBL007                 ; If it's less than $08 it's a stack exchange - go handle it
         asl                        ; Multiply the OP code by 2 
         tax                        ; Transfer it to X
LBL022:  lda SR_V_H,X               ; Get the hi byte of the OP Code handling routine
         pha                        ; and save it on the stack
         lda SR_V_L,X               ; Get the lo byte
         pha                        ; and save it on the stack
         php                        ; save the processor status too
         rti                        ; now go execute the OP Code handling routine
;
;
; Routine to handle the stack exchange 
;
LBL007:  adc $C1
         tax
         lda (C_C1),Y
         pha
         lda $00,X
         sta (C_C1),Y
         pla
         sta $00,X
         rts
;
;
;
LBL015:  jsr LBL001                 ; Go print CR, LF and pad charachters
         lda #$21                   ; Load an ASCII DC2
         jsr OUT_V                  ; Go print it
         lda $2A                    ; Load the current TBIL pointer (lo) 
         sec                        ; Set the carry flag
         sbc LBL002                 ; Subtract the TBIL table origin (lo)
         tax                        ; Move the difference to X
         lda $2B                    ; Load the current TBIL pointer (hi)
         sbc LBL003                 ; Subtract the TBIL table origin (hi)
         jsr LBL010
         lda $BE
         beq LBL012
         lda #$7E
         sta $2A
         lda #$20
         sta $2B
         jsr LBL013
         ldx $28
         lda $29
         jsr LBL010
LBL012:  lda #$07                   ; ASCII Bell
         jsr OUT_V                  ; Go ring Bell
         jsr LBL001                 ; Go print CR, LF and pad charachters
LBL060:  lda $26
         sta $C6
         lda $27
         sta $C7
         jmp LBL014
;
;
;
LBL115:  ldx #$7C
LBL048:  cpx $C1
LBL019:  bcc LBL015
         ldx $C1
         inc $C1
         inc $C1
         clc
         rts
;
;
;
JUMP01:  dec $BD
LBL027:  lda $BD
         beq LBL015
LBL017:  lda $BC
         sta $2A
         lda $BD
         sta $2B
         rts
;
; Branch handling routine
;
LBL011:  cmp #$40	                  ;
         bcs LBL016                 ; If it's not a Jump, go to branch handler
         pha
         jsr LBL004                 ; Go read a byte from the TBIL table
         adc LBL002
         sta $BC
         pla
         pha
         and #$07
         adc LBL003
         sta $BD
         pla
         and #$08
         bne LBL017
         lda $BC
         ldx $2A
         sta $2A
         stx $BC
         lda $BD
         ldx $2B
         sta $2B
         stx $BD
LBL126:  lda $C6
         sbc #$01
         sta $C6
         bcs LBL018
         dec $C7
LBL018:  cmp $24
         lda $C7
         sbc $25
         bcc LBL019
         lda $BC
         sta (C_C6),Y
         iny
         lda $BD
         sta (C_C6),Y
         rts
;
;Branch handler
;
LBL016:  pha
         lsr
         lsr
         lsr
         lsr
         and #$0E
         tax
         pla
         cmp #$60
         and #$1F
         bcs LBL020
         ora #$E0
LBL020:  clc
         beq LBL021
         adc $2A
         sta $BC
         tya
         adc $2B
LBL021:  sta $BD
         JMP LBL022
;
;
;
JUMP02:  lda $2C
         sta $B8
         lda $2D
         sta $B9
LBL025:  jsr LBL023
         jsr LBL024
         eor (C_2A),Y
         tax
         jsr LBL004                 ; Go read a byte from the TBIL table
         txa
         beq LBL025
         asl
         beq LBL026
         lda $B8
         sta $2C
         lda $B9
         sta $2D
LBL028:  jmp LBL027
JUMP05:  jsr LBL023
         cmp #$0D
         bne LBL028
LBL026:  rts
;
;
;
JUMP03:  jsr LBL023
         cmp #$5B
         bcs LBL028
         cmp #$41
         bcc LBL028
         asl
         jsr LBL029
LBL024:  ldy #$00
         lda (C_2C),Y
         inc $2C
         bne LBL030
         inc $2D
LBL030:  cmp #$0D
         clc
         rts
;
;

LBL031:  jsr LBL024
LBL023:  lda (C_2C),Y
         cmp #$20
         beq LBL031
         cmp #$3A
         clc
         bpl LBL032
         cmp #$30
LBL032:  rts
;
;
;
JUMP04:  jsr LBL023
         bcc LBL028
         sty $BC
         sty $BD
LBL033:  lda $BC
         ldx $BD
         asl $BC
         rol $BD
         asl $BC
         rol $BD
         clc
         adc $BC
         sta $BC
         txa
         adc $BD
         asl $BC
         rol
         sta $BD
         jsr LBL024
         and #$0F
         adc $BC
         sta $BC
         tya
         adc $BD
         sta $BD
         jsr LBL023
         bcs LBL033
         jmp LBL034
LBL061:  jsr LBL035
         lda $BC
         ora $BD
         beq LBL036
LBL065:  lda $20
         sta $2C
         lda $21
         sta $2D
LBL040:  jsr LBL037
         beq LBL038
         lda $28
         cmp $BC
         lda $29
         sbc $BD
         bcs LBL038
LBL039:  jsr LBL024
         bne LBL039
         jmp LBL040
LBL038:  lda $28
         eor $BC
         bne LBL041
         lda $29
         eor $BD
LBL041:  rts
;
;
;
LBL043:  jsr LBL042
LBL013:  jsr LBL004                 ; Entry point for TBIL PC (print literal) - Go read a byte from the TBIL table
         bpl LBL043                 ; 
LBL042:  inc $BF
         bmi LBL044
         jmp OUT_V                  ; Go print it
LBL044:  dec $BF
LBL045:  rts
;
;
;
LBL046:  cmp #$22
         beq LBL045
         jsr LBL042
JUMP23:  jsr LBL024
         bne LBL046
LBL036:  jmp LBL015
LBL047:  lda #$20
         jsr LBL042
         lda $BF
         and #$87
         bmi LBL045
         bne LBL047
         rts
;
;
;
JUMP19:  ldx #$7B
         jsr LBL048
         inc $C1
         inc $C1
         inc $C1
         sec
         lda $03,X
         sbc $00,X
         sta $00,X
         lda $04,X
         sbc $01,X
         bvc LBL052
         eor #$80
         ora #$01
LBL052:  bmi LBL053
         bne LBL054
         ora $00,X
         beq LBL049
LBL054:  lsr $02,X
LBL049:  lsr $02,X
LBL053:  lsr $02,X
         bcc LBL050
LBL004:  ldy #$00                   ; Read a byte from the TBIL Table
         lda (C_2A),Y               ;
         inc $2A                    ; Increment TBIL Table pointer as required
         bne LBL051                 ;
         inc $2B                    ;
LBL051:  ora #$00                   ; Check for $00 and set the 'Z' flag acordingly
LBL050:  rts                        ; Return
;
;
;
JUMP20:  lda $BE
         beq LBL055
LBL056:  jsr LBL024
         bne LBL056
         jsr LBL037
         beq LBL057
LBL062:  jsr LBL058
         jsr BREAK
         bcs LBL059
         lda $C4
         sta $2A
         lda $C5
         sta $2B
         rts
;
;
;
LBL059:  lda LBL002
         sta $2A
         lda LBL003
         sta $2B
LBL057:  jmp LBL015
LBL055:  sta $BF
         jmp LBL060
JUMP28:  lda $20
         sta $2C
         lda $21
         sta $2D
         jsr LBL037
         beq LBL057
         lda $2A
         sta $C4
         lda $2B
         sta $C5
LBL058:  lda #$01
         sta $BE
         rts
;
;
;
JUMP14:  jsr LBL061
         beq LBL062
LBL066:  lda $BC
         sta $28
         lda $BD
         sta $29
         jmp LBL015
JUMP13:  jsr LBL063
         jsr LBL064
         jsr LBL065
         bne LBL066
         rts
;
;
;
LBL037:  jsr LBL024
         sta $28
         jsr LBL024
         sta $29
         ora $28
         rts
;
;
;
JUMP07:  jsr LBL035
         jsr LBL034
LBL034:  lda $BD
LBL131:  jsr LBL029
         lda $BC
LBL029:  ldx $C1
         dex
         sta $00,X
         stx $C1
         cpx $C0
         bne LBL067
LBL068:  jmp LBL015
LBL097:  ldx $C1
         cpx #$80
         bpl LBL068
         lda $00,X
         inc $C1
LBL067:  rts
;
;
;
LBL010:  sta $BD
         stx $BC
         jmp LBL069
JUMP22:  ldx $C1	; entry point to TBIL PN (print number)
         lda $01,X
         bpl LBL070
         jsr LBL071
         lda #$2D
         jsr LBL042
LBL070:  jsr LBL035
LBL069:  lda #$1F
         sta $B8
         sta $BA
         lda #$2A
         sta $B9
         sta $BB
         ldx $BC
         ldy $BD
         sec
LBL072:  inc $B8
         txa
         sbc #$10
         tax
         tya
         sbc #$27
         tay
         bcs LBL072
LBL073:  dec $B9
         txa
         adc #$E8
         tax
         tya
         adc #$03
         tay
         bcc LBL073
         txa
LBL074:  sec
         inc $BA
         sbc #$64
         bcs LBL074
         dey
         bpl LBL074
LBL075:  dec $BB
         adc #$0A
         bcc LBL075
         ora #$30
         sta $BC
         lda #$20
         sta $BD
         ldx #$FB
LBL199:  stx $C3
         lda $BD,X
         ora $BD
         cmp #$20
         beq LBL076
         ldy #$30
         sty $BD
         ora $BD
         jsr LBL042
LBL076:  ldx $C3
         inx
         bne LBL199
         rts
;
;
;
JUMP21:  lda $2D
         pha
         lda $2C
         pha
         lda $20
         sta $2C
         lda $21
         sta $2D
         lda $24
         ldx $25
         jsr LBL077
         beq LBL078
         jsr LBL077
LBL078:  lda $2C
         sec
         sbc $B6
         lda $2D
         sbc $B7
         bcs LBL079
         jsr LBL037
         beq LBL079
         ldx $28
         lda $29
         jsr LBL010
         lda #$20
LBL080:  jsr LBL042
         jsr BREAK
         bcs LBL079
         jsr LBL024
         bne LBL080
         jsr LBL081
         jmp LBL078
LBL077:  sta $B6
         inc $B6
         bne LBL082
         inx
LBL082:  stx $B7
         ldy $C1
         cpy #$80
         beq LBL083
         jsr LBL061
LBL099:  lda $2C
         ldx $2D
         sec
         sbc #$02
         bcs LBL084
         dex
LBL084:  sta $2C
         jmp LBL085
LBL079:  pla
         sta $2C
         pla
         sta $2D
LBL083:  rts
LBL081:  lda $BF
         bmi LBL083
;
;
; Routine to handle CR, LF and pad characters in the ouput
;
LBL001:  lda #$0D                   ; Load up a CR
         jsr OUT_V                  ; Go print it
         lda PCC                    ; Load the pad character code
         and #$7F                   ; Test to see - 
         sta $BF                    ; how many pad charachters to print
         beq LBL086                 ; Skip if 0
LBL088:  jsr LBL087                 ; Go print pad charcter
         dec $BF                    ; One less
         bne LBL088                 ; Loop until 0
LBL086:  lda #$0A                   ; Load up a LF
         jmp LBL089                 ; Go print it
;
;
;
LBL092:  ldy TMC
LBL091:  sty $BF
         bcs LBL090
JUMP24:  lda #$30                   ; Entry pont for TBIL GL (get input line)
         sta $2C
         sta $C0
         sty $2D
         jsr LBL034
LBL090:  eor $80
         sta $80
         jsr IN_V
         ldy #$00
         ldx $C0
         and #$7F
         beq LBL090
         cmp #$7F
         beq LBL090
         cmp #$13
         beq LBL091
         cmp #$0A
         beq LBL092
         cmp LSC
         beq LBL093
         cmp BSC
         bne LBL094
         cpx #$30
         bne LBL095
LBL093:  ldx $2C
         sty $BF
         lda #$0D
LBL094:  cpx $C1
         bmi LBL096
         lda #$07
         jsr LBL042
         jmp LBL090
LBL096:  sta $00,X
         inx
         inx
LBL095:  dex
         stx $C0
         cmp #$0D
         bne LBL090
         jsr LBL081
LBL035:  jsr LBL097
         sta $BC
         jsr LBL097
         sta $BD
         rts
;
;
;
JUMP27:  jsr LBL098
         jsr LBL061
         php
         jsr LBL099
         sta $B8
         stx $B9
         lda $BC
         sta $B6
         lda $BD
         sta $B7
         ldx #$00
         plp
         bne LBL100
         jsr LBL037
         dex
         dex
LBL101:  dex
         jsr LBL024
         bne LBL101
LBL100:  sty $28
         sty $29
         jsr LBL098
         lda #$0D
         cmp (C_2C),Y
         beq LBL102
         inx
         inx
         inx
LBL103:  inx
         iny
         cmp (C_2C),Y
         bne LBL103
         lda $B6
         sta $28
         lda $B7
         sta $29
LBL102:  lda $B8
         sta $BC
         lda $B9
         sta $BD
         clc
         ldy #$00
         txa
         beq LBL104
         bpl LBL105
         adc $2E
         sta $B8
         lda $2F
         sbc #$00
         sta $B9
LBL109:  lda (C_2E),Y
         sta (C_B8),Y
         ldx $2E
         cpx $24
         bne LBL106
         lda $2F
         cmp $25
         beq LBL107
LBL106:  inx
         stx $2E
         bne LBL108
         inc $2F
LBL108:  inc $B8
         bne LBL109
         inc $B9
         bne LBL109
LBL105:  adc $24
         sta $B8
         sta $2E
         tya
         adc $25
         sta $B9
         sta $2F
         lda $2E
         sbc $C6
         lda $2F
         sbc $C7
         bcc LBL110
         dec $2A
         jmp LBL015
LBL110:  lda (C_24),Y
         sta (C_2E),Y
         ldx $24
         bne LBL111
         dec $25
LBL111:  dec $24
         ldx $2E
         bne LBL112
         dec $2F
LBL112:  dex
         stx $2E
         cpx $BC
         bne LBL110
         ldx $2F
         cpx $BD
         bne LBL110
LBL107:  lda $B8
         sta $24
         lda $B9
         sta $25
LBL104:  lda $28
         ora $29
         beq LBL113
         lda $28
         sta (C_BC),Y
         iny
         lda $29
         sta (C_BC),Y
LBL114:  iny
         sty $B6
         jsr LBL024
         php
         ldy $B6
         sta (C_BC),Y
         plp
         bne LBL114
LBL113:  jmp LBL014
JUMP18:  jsr LBL115
         lda $03,X
         and #$80
         beq LBL116
         lda #$FF
LBL116:  sta $BC
         sta $BD
         pha
         adc $02,X
         sta $02,X
         pla
         pha
         adc $03,X
         sta $03,X
         pla
         eor $01,X
         sta $BB
         bpl LBL117
         jsr LBL118
LBL117:  ldy #$11
         lda $00,X
         ora $01,X
         bne LBL119
         jmp LBL015
LBL119:  sec
         lda $BC
         sbc $00,X
         pha
         lda $BD
         sbc $01,X
         pha
         eor $BD
         bmi LBL120
         pla
         sta $BD
         pla
         sta $BC
         sec
         jmp LBL121
LBL120:  pla
         pla
         clc
LBL121:  rol $02,X
         rol $03,X
         rol $BC
         rol $BD
         dey
         bne LBL119
         lda $BB
         bpl LBL122
LBL071:  ldx $C1
LBL118:  sec
         tya
         sbc $00,X
         sta $00,X
         tya
         sbc $01,X
         sta $01,X
LBL122:  rts
;
;
;
JUMP16:  jsr LBL071
JUMP15:  jsr LBL115
         lda $00,X
         adc $02,X
         sta $02,X
         lda $01,X
         adc $03,X
         sta $03,X
         rts
;
;
;
JUMP17:  jsr LBL115
         ldy #$10
         lda $02,X
         sta $BC
         lda $03,X
         sta $BD
LBL124:  asl $02,X
         rol $03,X
         rol $BC
         rol $BD
         bcc LBL123
         clc
         lda $02,X
         adc $00,X
         sta $02,X
         lda $03,X
         adc $01,X
         sta $03,X
LBL123:  dey
         bne LBL124
         rts
;
;
;
JUMP10:  jsr LBL097
         tax
         lda $00,X
         ldy $01,X
         dec $C1
         ldx $C1
         sty $00,X
         jmp LBL029
JUMP11:  ldx #$7D
         jsr LBL048
         lda $01,X
         pha
         lda $00,X
         pha
         jsr LBL097
         tax
         pla
         sta $00,X
         pla
         sta $01,X
         rts
JUMP30:  jsr LBL063
         lda $BC
         sta $2A
         lda $BD
         sta $2B
         rts
;
;
;
JUMP08:  ldx #$2C                   ; Entry point to Save Basic Pointer SB
         bne LBL125
JUMP09:  ldx #$2E
LBL125:  lda $00,X
         cmp #$80
         bcs LBL098
         lda $01,X
         bne LBL098
         lda $2C
         sta $2E
         lda $2D
         sta $2F
         rts
;
;
;
LBL098:  lda $2C
         ldy $2E
         sty $2C
         sta $2E
         lda $2D
         ldy $2F
         sty $2D
         sta $2F
         ldy #$00
         rts
;
;
;
JUMP12:  lda $28
         sta $BC
         lda $29
         sta $BD
         jsr LBL126
         lda $C6
         sta $26
         lda $C7
LBL064:  sta $27
LBL129:  rts
;
;
;
LBL063:  lda (C_C6),Y
         sta $BC
         jsr LBL127
         lda (C_C6),Y
         sta $BD
LBL127:  inc $C6
         bne LBL128
         inc $C7
LBL128:  lda $22
         cmp $C6
         lda $23
         sbc $C7
         bcs LBL129
         jmp LBL015
JUMP29:  jsr LBL130
         sta $BC
         tya
         jmp LBL131
LBL130:  jsr LBL035
         lda $BC
         sta $B6
         jsr LBL035
         lda $BD
         sta $B7
         ldy $BC
         jsr LBL035
         ldx $B7
         lda $B6
         clc
         jmp (C_00BC)
JUMP06:  jsr LBL132
LBL132:  jsr LBL004                 ; Go read a byte from the TBIL Table
         jmp LBL029
LBL085:  stx $2D
         cpx #$00
         rts
;
;
;
JUMP26:  ldy #$02
JUMP25:  sty $BC
         ldy #$29
         sty $BD
         ldy #$00
         lda (C_BC),Y
         cmp #$08
         bne LBL133
         jmp LBL117
LBL133:  rts
;
;
; Subroutine to decide which pad character to print
;
LBL089:  jsr OUT_V                  ; Entry point with a charater to print first
LBL087:  lda #$FF                   ; Normal entry point - Set pad to $FF
         bit PCC                    ; Check if the pad flag is on
         bmi LBL134                 ; Skip it if not
         lda #$00                   ; set pad to $00
LBL134:  jmp OUT_V                  ; Go print it


;
; TBIL Tables
;
ILTBL:   .byte $24, $3A, $91, $27, $10, $E1, $59, $C5, $2A, $56, $10, $11, $2C, $8B, $4C
         .byte $45, $D4, $A0, $80, $BD, $30, $BC, $E0, $13, $1D, $94, $47, $CF, $88, $54
         .byte $CF, $30, $BC, $E0, $10, $11, $16, $80, $53, $55, $C2, $30, $BC, $E0, $14
         .byte $16, $90, $50, $D2, $83, $49, $4E, $D4, $E5, $71, $88, $BB, $E1, $1D, $8F
         .byte $A2, $21, $58, $6F, $83, $AC, $22, $55, $83, $BA, $24, $93, $E0, $23, $1D
         .byte $30, $BC, $20, $48, $91, $49, $C6, $30, $BC, $31, $34, $30, $BC, $84, $54
         .byte $48, $45, $CE, $1C, $1D, $38, $0D, $9A, $49, $4E, $50, $55, $D4, $A0, $10
         .byte $E7, $24, $3F, $20, $91, $27, $E1, $59, $81, $AC, $30, $BC, $13, $11, $82
         .byte $AC, $4D, $E0, $1D, $89, $52, $45, $54, $55, $52, $CE, $E0, $15, $1D, $85
         .byte $45, $4E, $C4, $E0, $2D, $98, $4C, $49, $53, $D4, $EC, $24, $00, $00, $00
         .byte $00, $0A, $80, $1F, $24, $93, $23, $1D, $30, $BC, $E1, $50, $80, $AC, $59
         .byte $85, $52, $55, $CE, $38, $0A, $86, $43, $4C, $45, $41, $D2, $2B, $84, $52
         .byte $45, $CD, $1D, $A0, $80, $BD, $38, $14, $85, $AD, $30, $D3, $17, $64, $81
         .byte $AB, $30, $D3, $85, $AB, $30, $D3, $18, $5A, $85, $AD, $30, $D3, $19, $54
         .byte $2F, $30, $E2, $85, $AA, $30, $E2, $1A, $5A, $85, $AF, $30, $E2, $1B, $54
         .byte $2F, $98, $52, $4E, $C4, $0A, $80, $80, $12, $0A, $09, $29, $1A, $0A, $1A
         .byte $85, $18, $13, $09, $80, $12, $01, $0B, $31, $30, $61, $72, $0B, $04, $02
         .byte $03, $05, $03, $1B, $1A, $19, $0B, $09, $06, $0A, $00, $00, $1C, $17, $2F
         .byte $8F, $55, $53, $D2, $80, $A8, $30, $BC, $31, $2A, $31, $2A, $80, $A9, $2E
         .byte $2F, $A2, $12, $2F, $C1, $2F, $80, $A8, $30, $BC, $80, $A9, $2F, $83, $AC
         .byte $38, $BC, $0B, $2F, $80, $A8, $52, $2F, $84, $BD, $09, $02, $2F, $8E, $BC
         .byte $84, $BD, $09, $93, $2F, $84, $BE, $09, $05, $2F, $09, $91, $2F, $80, $BE
         .byte $84, $BD, $09, $06, $2F, $84, $BC, $09, $95, $2F, $09, $04, $2F, $00, $00
         .byte $00
;
; End of Tiny Basic


;.segment "MAIN"

         .org $0CF0    ; Address of main program

; Code needs work below here, BIOS must be changed for MKHBC-8-R2

;FBLK:
;
; Set some symbols
;
;ACIARW   = $1200                    ; Base address of ACIA
;ACIAST   = ACIARW+$01               ; ACIA status register 
;ACIACM   = ACIARW+$02               ; ACIA commnad register
;ACIACN   = ACIARW+$03               ; ACIA control register


;
; Begin base system initialization
;
;		 jmp main		; no 6522 on MKHBC-8-R1
;--------------------		 
;         sta ACIAST                 ; Do a soft reset on the ACIA
;         lda #$0B                   ; Set it up for :
;         sta ACIACM                 ;        no echo, no parity, RTS low, No IRQ, DTR low
;         lda #$1A                   ; and :
;         sta ACIACN                 ;        2400, 8 bits, 1 stop bit, external Rx clock
;--------------------

main:		 
         jsr CLRSC                  ; Go clear the screen
         ldy #$00                   ; Offset for welcome message and prompt
         jsr SNDMSG                 ; Go print it
ST_LP:   JSR RCCHR                  ; Go get a character from the console
         cmp #$43                   ; Check for 'C'
         BNE IS_WRM                 ; If not branch to next check
         jmp COLD_S                 ; Otherwise cold-start Tiny Basic
IS_WRM:  cmp #$57                   ; Check for 'W'
         BNE PRMPT                  ; If not, branch to re-prompt them
         jmp WARM_S                 ; Otherwise warm-start Tiny Basic
PRMPT:   ldx #$2F                   ; Offset of prompt
         jsr SNDMSG                 ; Go print the prompt	 
         jmp ST_LP                  ; Go get the response

;.segment "MESG"

         .org $0E00    ; Address of message area
MBLK:

;
; The message block begins at $1E00 and is at most 256 bytes long.
; Messages terminate with an FF.
;
         .byte "MKHBC-8-R2 TINY BASIC 6502 PORT"
         .byte  $0D, $0A ;, $0A
         .byte "Version: 1.0.3, 2/20/2016"
         .byte  $0D, $0A ;, $0A
         .byte  "(NOTE: Use UPPER CASE.)"
         .byte  $0D, $0A ;, $0A
         .byte "Boot ([C]old/[W]arm)? "
         .byte  $07, $FF
	
;.segment "SUBR"

     .org $0F00    ;address of subroutine area
	 
SBLK:
;
; Begin BIOS subroutines
;

; M.O.S. API defines.

StrPtr	=	$E0

; Kernel jump table

GetCh	=	$FFED
PutCh	=	$FFF0
Gets	=	$FFF3
Puts	=	$FFF6


;SNDCHR	=	PutCh
;RCCHR	=	GetCh


;
; Clear the screen
;
ESC	=	$1b

; 2-BYTE PEEK USR FUNCTION
; For TINY BASIC IL ASSEMBLER VERSION 0

TwoBytePeek:

   STX $C3          ;($C2=00)
   LDA ($C2),Y      ;GET MSB
   PHA              ;SAVE IT
   INY
   LDA ($C2),Y      ;GET LSB
   TAX
   PLA
   TAY              ;Y=MSB
   TXA
   RTS


;ClrScrCode:

;	.BYTE ESC,"[2J",0	;clear screen sequence (ANSI).
	
;ClearScreen:

;	lda #<ClrScrCode
;	sta StrPtr
;	lda #>ClrScrCode
;	sta StrPtr+1
;	jsr Puts
;	rts

CLRSC:   ldx #$19                   ; Load X - we're going tp print 25 lines
         lda #$0D                   ; CR
         jsr SNDCHR                 ; Send a carriage retuen
         lda #$0A                   ; LF
CSLP:    jsr SNDCHR                 ; Send the line feed
         dex                        ; One less to do
         bne CSLP                   ; Go send another untill we're done
         rts                        ; Return

;
; Print a message.
; This sub expects the messsage offset from MBLK in X.
;
SNDMSG:  lda MBLK,y                 ; Get a character from teh message block
         cmp #$FF                   ; Look for end of message marker
         beq EXSM                   ; Finish up if it is
         jsr SNDCHR                 ; Otherwise send the character
         iny                        ; Increment the pointer
         jmp SNDMSG                 ; Go get next character
EXSM:    rts                        ; Return

;
; Get a character from the ACIA
; Runs into SNDCHR to provide echo
;
RCCHR:   ;lda ACIAST                ; Read the ACAI status to (for OMS-02)
         ;and #$08                  ; Check if there is character in the receiver (for OMS-02)
         ;beq RCCHR                 ; Loop util we get one (for OMS-02)
         ;lda ACIARW                ; Load it into the accumulator (for OMS-02)
         LDA $E000                  ; Check if a character typed (for emulator)
         BEQ RCCHR                  ; Loop until we get one (for emulator)

;RCCHR:	 jsr GetCh
;		 jsr SNDCHR					; echo character to the console
;		 rts

;
;Send a character to the ACIA
;
SNDCHR:  sta $FE                    ; Save the character to be printed
         cmp #$FF                   ; Check for a bunch of characters
         BEQ EXSC                   ; that we don't want to print to
         cmp #$00	                  ; the terminal and discard them to
         BEQ EXSC	                  ; clean up the output
         cmp #$91	                  ;
         BEQ EXSC	                  ;
         cmp #$93	                  ;
         BEQ EXSC	                  ;
         cmp #$80	                  ;
         BEQ EXSC	                  ;
         jmp SCLP			  ; VM6502 emulator, comment for MKHBC-8-R2
	 jsr PutCh			  ; MKHBC-8-R2
	 lda $fe
	 rts
SCLP:    ;lda ACIAST                ; Check ACIA Status  (don't need for emulator)
         ;and #$10                  ; See if transmiter it busy (don't need for emulator)
         ;beq SCLP                  ; Wait for it (don't need for emulator)
         lda $FE                    ; Restore the character
         ;sta ACIARW                ; Transmit it (for OMS-02)
         STA $E000                  ; Transmit it (for emulator)
EXSC:    rts                        ; Return

;         .org $1FFC                 ; Address of reset vector (for 6507 not required for emulator)
;RSVEC
;         .byte $F0, $1C               ; Reset vector

;        .org $3000                  ; Address of last byte of EPROM
;EOROM:

;--------------------------- END ----------------------------------------------------------------------

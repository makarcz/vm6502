;***********************************************************************
;
;  MicroChess (c) 1996-2002 Peter Jennings, peterj@benlo.com 
;
;***********************************************************************
; Daryl Rictor:
; I have been given permission to distribute this program by the
; author and copyright holder, Peter Jennings.  Please get his 
; permission if you wish to re-distribute a modified copy of 
; this file to others.  He specifically requested that his 
; copyright notice be included in the source and binary images.
; Thanks! 
;
; Marek Karcz:
; I have been given permission to distribute this program by the
; original author Peter Jennings under condition that I include
; link to his website in attribution.
; Here it is: http://www.benlo.com/microchess/index.html
; I did not bother to contact Daryl Rictor to ask permission to
; distribute his modifications to this software because according to
; the copyright notice on the web page of his project, permission is
; already given for personal non-commercial use:
; http://sbc.rictor.org/avr65c02.html
; http://sbc.rictor.org/download/AVR65C02.zip, readme.txt
; If this is incorrect or I misunderstood the author's intention,
; please note that I acted with no malicious intent and will remove 
; this file from my project if I receive such request.
;
; To build this program with CL65:
;
; cl65 -C microchess.cfg -l --start-addr 1024 -t none -o microchess.bin
; microchess.asm
;
; Binary image microchess.bin can be loaded to emulator with 'L'
; command in debug console. Start emulator: mkbasic, then issue
; command in debug console:
; L B MICROCHESS.BIN
;
; Memory image definition file can be generated which can be loaded
; to emulator via command line argument and automatically executed.
; To create that file, build microchess.bin image, then execute:
; 
; bin2hex -f microchess.bin -o microchess.dat -w 0 -x 1024 -z
;
; and add following lines at the end of microchess.dat file:
;
; IOADDR
; $E000
; ENIO
;
; Instructions to play:
;
; Load the game to emulator and auto-execute with command:
;    mkbasic microchess.dat
; then perform following steps:
; 1. Press 'C' to setup board.
; 2. Enter your move: 4 digits - BBEE, BB - piece coordinates,
;    EE - destination coordinates and press ENTER
; 3. After board is updated, press 'P' to make program make the move.
; 4. Repeat steps 2 and 3 until the game is finished.
; 
;
; 1/14/2012
; 	Modified Daryl Rictor's port to run on MKHBC-8-R1 homebrew
;       computer under MKHBCOS (derivative of M.O.S. by Scott 
;       Chidester).
;
; 3/11/2016
;       Adapted to run in MKBASIC (V65) emulator.
;
; 3/12/2016
;       Modified UI behavior:
;       - chess board is only printed after move, not after each
;         keystroke
;       - copyright banner is only printed once at the start
;         of the program
;
; 6551 I/O Port Addresses
;
;ACIADat    =    $7F70
;ACIAsta    =    $7F71
;ACIACmd    =    $7F72
;ACIACtl    =    $7F73

; M.O.S. API defines (kernal) - OK for emulator, no changes

.define 	mos_StrPtr	$E0
.define		tmp_zpgPt	$F6

; jumps, originally for M.O.S., now modified for emulator

.define		mos_CallGetCh	$FFED
.define		mos_CallPutCh	$FFF0
.define		mos_CallPuts	$FFF3


;
; page zero variables
;
BOARD   =	$50 
BK      =	$60 
PIECE   =	$B0 
SQUARE  =	$B1 
SP2     =	$B2 
SP1     =	$B3 
incHEK  =	$B4 
STATE   =	$B5 
MOVEN   =	$B6 
REV	=       $B7
OMOVE   =	$2C 
WCAP0   =	$2D 
COUNT   =	$2E 
BCAP2   =	$2E 
WCAP2   =	$2F 
BCAP1   =	$20 
WCAP1   =	$21 
BCAP0   =	$22 
MOB     =	$23 
MAXC    =	$24 
CC      =	$25 
PCAP    =	$26 
BMOB    =	$23 
BMAXC   =	$24 
BMCC    =	$25 ; was bcc (TASS doesn't like it as a label)
BMAXP   =	$26 
XMAXC   =	$28 
WMOB    =	$2B 
WMAXC   =	$3C 
WCC     =	$3D 
WMAXP   =	$3E 
PMOB    =	$3F 
PMAXC   =	$40 
PCC     =	$41 
PCP     =	$42 
OLDKY   =	$43 
BESTP   =	$4B 
BESTV   =	$4A 
BESTM   =	$49 
DIS1    =	$4B 
DIS2    =	$4A 
DIS3    =	$49 
temp    =       $4C

;
;
;

.segment "BEGN"

        .ORG $0000

.segment "CODE"

        .ORG    $0400            ; load into RAM @ $1000-$15FF

        lda     #$00        ; REVERSE TOGGLE
        sta     REV
        jmp     CHESS

PAINT:  .byte   $FF         ; set this flag if board needs painting
                            ; unset otherwise
PRNBANN:
        .byte   $FF         ; set this flag to print copyright banner                            

        ;jsr     Init_6551
CHESS:  cld    			; INITIALIZE
        ldx    #$FF		; TWO STACKS
        txs    
        ldx    #$C8
        stx    SP2
;        
;       ROUTINES TO LIGHT LED
;       DISPLAY and GET KEY
;       FROM KEYBOARD
;        
OUT:    jsr    POUT       ; DISPLAY and
        jsr    KIN        ; GET INPUT   *** my routine waits for a keypress
        cmp    #$43        ; [C]
        bne    NOSET       ; SET UP
        lda    #$FF        ; set PAINT flag
        sta    PAINT       ; board needs to be diplayed
        ldx    #$1F        ; BOARD
WHSET:  lda    SETW,X      ; FROM
        sta    BOARD,X     ; SETW
        dex    
        bpl    WHSET
        ldx    #$1B        ; *ADDED
        stx    OMOVE       ; INITS TO $FF
        lda    #$CC        ; DISPLAY CCC
        bne    CLDSP
;        
NOSET:  cmp    #$45        ; [E]
        bne    NOREV       ; REVERSE
        lda    #$FF
        sta    PAINT
        jsr    REVERSE     ; BOARD IS
        sec
        lda    #$01
        sbc    REV
        sta    REV          ; TOGGLE REV FLAG
        lda    #$EE         ; IS
        bne    CLDSP
;        
NOREV:  cmp    #$40         ; [P]
        bne    NOGO         ; PLAY CHESS
        lda    #$FF
        sta    PAINT
        jsr    GO
CLDSP:  sta    DIS1         ; DISPLAY
        sta    DIS2         ; ACROSS
        sta    DIS3         ; DISPLAY
        bne    CHESS
;        
NOGO:   cmp    #$0D         ; [Enter]
        bne    NOMV         ; MOVE MAN
        pha
        lda    #$FF
        sta    PAINT
        pla
        jsr    MOVE         ; AS ENTERED
        jmp    DISP
NOMV:   cmp    #$41         ; [Q] ***Added to allow game exit***
        beq    DONE         ; quit the game, exit back to system.  
        pha
        lda    #$00
        sta    PAINT
        pla
        jmp    INPUT        ; process move
DONE:   rts
;jmp     $FF00        ; *** MUST set this to YOUR OS starting address
;        
;       THE ROUTINE JANUS DIRECTS THE
;       ANALYSIS BY DETERMINING WHAT
;       SHOULD OCCUR AFTER EACH MOVE
;       GENERATED BY GNM
;        
;        
;
JANUS:  ldx    STATE
        bmi    NOCOUNT
;        
;       THIS ROUTINE COUNTS OCCURRENCES
;       IT DEPENDS UPON STATE TO INdex
;       THE CORRECT COUNTERS
;        
COUNTS: lda    PIECE
        beq    OVER             ; IF STATE=8
        cpx    #$08             ; DO NOT COUNT
        bne    OVER             ; BLK MAX CAP
        cmp    BMAXP            ; MOVES FOR
        beq    XRT              ; WHITE
;         
OVER:   inc    MOB,X            ; MOBILITY
        cmp    #$01             ;  + QUEEN
        bne    NOQ              ; FOR TWO
        inc    MOB,X
;        
NOQ:    bvc    NOCAP
        ldy    #$0F             ; CALCULATE
        lda    SQUARE           ; POINTS
ELOOP:  cmp    BK,Y             ; CAPTURED
        beq    FOUN             ; BY THIS
        dey            ; MOVE
        bpl    ELOOP
FOUN:   lda    POINTS,Y
        cmp    MAXC,X
        bcc    LESS             ; SAVE IF
        sty    PCAP,X           ; BEST THIS
        sta    MAXC,X           ; STATE
;        
LESS:   clc    
        php            ; ADD TO
        adc    CC,X             ; CAPTURE
        sta    CC,X             ; COUNTS
        plp    
;        
NOCAP:  cpx    #$04
        beq    ON4
        bmi    TREE             ;(=00 ONLY)
XRT:    rts    
;        
;      GENERATE FURTHER MOVES FOR COUNT
;      and ANALYSIS    
;        
ON4:    lda    XMAXC            ; SAVE ACTUAL 
        sta    WCAP0             ; CAPTURE
        lda    #$00               ; STATE=0
        sta    STATE
        jsr    MOVE              ; GENERATE
        jsr    REVERSE           ; IMMEDIATE
        jsr    GNMZ             ; REPLY MOVES  
        jsr    REVERSE
;        
        lda    #$08           ; STATE=8
        sta    STATE            ; GENERATE
;        jsr    OHM              ; CONTINUATION
        jsr    UMOVE             ; MOVES
;        
        jmp    STRATGY           ; FINAL EVALUATION
NOCOUNT:cpx    #$F9
        bne    TREE
;        
;      DETERMINE IF THE KING CAN BE
;      TAKEN, USED BY CHKCHK
;        
        lda    BK               ; IS KING
        cmp    SQUARE           ; IN CHECK?
        bne    RETJ             ; SET incHEK=0
        lda    #$00             ; IF IT IS
        sta    incHEK
RETJ:   rts    
;        
;      IF A PIECE HAS BEEN CAPTURED BY
;      A TRIAL MOVE, GENERATE REPLIES &
;      EVALUATE THE EXCHANGE GAIN/LOSS
;        
TREE:   bvc    RETJ              ; NO CAP
        ldy    #$07               ; (PIECES)
        lda    SQUARE
LOOPX:  cmp    BK,Y
        beq    FOUNX
        dey    
        beq    RETJ              ; (KING)
        bpl    LOOPX             ; SAVE
FOUNX:  lda    POINTS,Y           ; BEST CAP
        cmp    BCAP0,X            ; AT THIS
        bcc    NOMAX             ; LEVEL
        sta    BCAP0,X
NOMAX:  dec    STATE
        lda    #$FB               ; IF STATE=FB
        cmp    STATE            ; TIME TO TURN
        beq    UPTREE            ; AROUND
        jsr    GENRM             ; GENERATE FURTHER
UPTREE: inc    STATE            ; CAPTURES
        rts    
;        
;      THE PLAYER'S MOVE IS INPUT
;        
INPUT:  cmp    #$08           ; NOT A LEGAL
        bcs    ERROR          ; SQUARE #
        jsr    DISMV
DISP:   ldx    #$1F
SEARCH: lda    BOARD,X
        cmp    DIS2
        beq    HERE           ; DISPLAY
        dex                   ; PIECE AT    
        bpl    SEARCH         ; FROM
HERE:   stx    DIS1           ; SQUARE
        stx    PIECE
ERROR:  jmp    CHESS
;        
;      GENERATE ALL MOVES FOR ONE
;      SIDE, CALL JANUS AFTER EACH
;      ONE FOR NEXT STE?
;        
;
GNMZ:   ldx    #$10            ; CLEAR
GNMX:   lda    #$00            ; COUNTERS
CLEAR:  sta    COUNT,X
        dex    
        bpl    CLEAR
;        
GNM:    lda    #$10            ; SET UP
        sta    PIECE            ; PIECE
NEWP:   dec    PIECE            ; NEW PIECE
        bpl    NEX               ; ALL DONE?
        rts            ; #NAME?
;        
NEX:    jsr    RESET            ; READY
        ldy    PIECE            ; GET PIECE
        ldx    #$08
        stx    MOVEN            ; COMMON staRT
        cpy    #$08            ; WHAT IS IT?
        bpl    PAWN              ; PAWN
        cpy    #$06
        bpl    KNIGHT            ; KNIGHT
        cpy    #$04
        bpl    BISHOP           ; BISHOP
        cpy    #$01
        beq    QUEEN             ; QUEEN
        bpl    ROOK              ; ROOK
;        
KING:   jsr    SNGMV             ; MUST BE KING!
        bne    KING              ; MOVES
        beq    NEWP              ; 8 TO 1
QUEEN:  jsr    LINE
        bne    QUEEN             ; MOVES
        beq    NEWP              ; 8 TO 1
;        
ROOK:   ldx    #$04
        stx    MOVEN            ; MOVES
AGNR:   jsr    LINE              ; 4 TO 1
        bne    AGNR
        beq    NEWP
;        
BISHOP: jsr    LINE
        lda    MOVEN            ; MOVES
        cmp    #$04               ; 8 TO 5
        bne    BISHOP
        beq    NEWP
;        
KNIGHT: ldx    #$10
        stx    MOVEN            ; MOVES
AGNN:   jsr    SNGMV             ; 16 TO 9
        lda    MOVEN
        cmp    #$08
        bne    AGNN
        beq    NEWP
;        
PAWN:   ldx    #$06
        stx    MOVEN
P1:     jsr    CMOVE             ; RIGHT CAP?
        bvc    P2
        bmi    P2
        jsr    JANUS             ; YES
P2:     jsr    RESET
        dec    MOVEN            ; LEFT CAP?
        lda    MOVEN
        cmp    #$05
        beq    P1
P3:     jsr    CMOVE             ; AHEAD
        bvs    NEWP              ; ILLEGAL
        bmi    NEWP
        jsr    JANUS
        lda    SQUARE           ; GETS TO
        and    #$F0               ; 3RD RANK?
        cmp    #$20
        beq    P3                ; DO DOUBLE
        jmp    NEWP
;        
;      CALCULATE SINGLE STEP MOVES
;      FOR K,N    
;        
SNGMV:  jsr    CMOVE            ; CALC MOVE
        bmi    ILL1               ; -IF LEGAL
        jsr    JANUS           ; -EVALUATE
ILL1:   jsr    RESET
        dec    MOVEN
        rts    
;        
;     CALCULATE ALL MOVES DOWN A
;     STRAIGHT LINE FOR Q,B,R
;        
LINE:   jsr    CMOVE             ; CALC MOVE
        bcc    OVL                ; NO CHK
        bvc    LINE        ; NOCAP       
OVL:    bmi    ILL             ; RETURN
        php    
        jsr    JANUS             ; EVALUATE POSN
        plp    
        bvc    LINE              ; NOT A CAP
ILL:    jsr    RESET             ; LINE STOPPED
        dec    MOVEN             ; NEXT DIR
        rts    
;        
;      EXCHANGE SIDES FOR REPLY
;      ANALYSIS    
;        
REVERSE:ldx    #$0F
ETC:    sec    
        ldy    BK,X               ; SUBTRACT
        lda    #$77               ; POSITION
        sbc    BOARD,X            ; FROM 77
        sta    BK,X
        sty    BOARD,X         ; and
        sec    
        lda    #$77               ; EXCHANGE
        sbc    BOARD,X            ; PIECES
        sta    BOARD,X
        dex    
        bpl    ETC
        rts    
;        
;        CMOVE CALCULATES THE TO SQUARE
;        USING SQUARE and THE MOVE
;       TABLE  FLAGS SET AS FOLLOWS:
;       N#NAME?    MOVE
;       V#NAME?    (LEGAL UNLESS IN CR)
;       C#NAME?    BECAUSE OF CHECK
;       [MY &THANKS TO JIM BUTTERFIELD
;        WHO WROTE THIS MORE EFFICIENT
;        VERSION OF CMOVE)
;        
CMOVE:  lda    SQUARE           ; GET SQUARE
        ldx    MOVEN           ; MOVE POINTER
        clc    
        adc    MOVEX,X            ; MOVE LIST
        sta    SQUARE           ; NEW POS'N
        and    #$88
        bne    ILLEGAL           ; OFF BOARD
        lda    SQUARE
;            
        ldx    #$20
LOOP:   dex            ; IS TO
        bmi    NO                ; SQUARE
        cmp    BOARD,X            ; OCCUPIED?
        bne    LOOP
;            
        cpx    #$10            ; BY SELF?
        bmi    ILLEGAL
;            
        lda    #$7F        ; MUST BE CAP!
        adc    #$01            ; SET V FLAG
        bvs    SPX             ; (jmp)
;            
NO:     clv            ; NO CAPTURE
;            
SPX:    lda    STATE             ; SHOULD WE
        bmi    RETL               ; DO THE
        cmp    #$08             ; CHECK CHECK?
        bpl    RETL
;            
;        CHKCHK REVERSES SIDES
;       and LOOKS FOR A KING
;       CAPTURE TO INDICATE
;       ILLEGAL MOVE BECAUSE OF
;       CHECK  SincE THIS IS
;       TIME CONSUMING, IT IS NOT
;       ALWAYS DONE    
;        
CHKCHK: pha                ; STATE  #392
        php    
        lda    #$F9
        sta    STATE             ; GENERATE
        sta    incHEK            ; ALL REPLY
        jsr    MOVE              ; MOVES TO
        jsr    REVERSE           ; SEE IF KING
        jsr    GNM               ; IS IN
        jsr    RUM               ; CHECK
        plp    
        pla    
        sta    STATE
        lda    incHEK
        bmi    RETL               ; NO - SAFE
        sec            ; YES - IN CHK
        lda    #$FF
        rts    
;        
RETL:   clc            ; LEGAL
        lda    #$00            ; RETURN
        rts    
;        
ILLEGAL:lda    #$FF
        clc            ; ILLEGAL
        clv            ; RETURN
        rts    
;        
;       REPLACE PIECE ON CORRECT SQUARE
;        
RESET:  ldx    PIECE          ; GET LOGAT
        lda    BOARD,X        ; FOR PIECE
        sta    SQUARE         ; FROM BOARD
        rts    
;        
;        
;        
GENRM:  jsr    MOVE             ; MAKE MOVE
GENR2:  jsr    REVERSE          ; REVERSE BOARD
        jsr    GNM              ; GENERATE MOVES
RUM:    jsr    REVERSE          ; REVERSE BACK
;        
;       ROUTINE TO UNMAKE A MOVE MADE BY
;      MOVE
;        
UMOVE:  tsx            ; UNMAKE MOVE
        stx    SP1
        ldx    SP2     ; EXCHANGE
        txs            ; STACKS
        pla            ; MOVEN
        sta    MOVEN
        pla            ; CAPTURED
        sta    PIECE   ; PIECE
        tax    
        pla            ; FROM SQUARE
        sta    BOARD,X
        pla            ; PIECE
        tax    
        pla            ; TO SOUARE
        sta    SQUARE
        sta    BOARD,X
        jmp    STRV
;        
;       THIS ROUTINE MOVES PIECE
;       TO SQUARE, PARAMETERS
;       ARE SAVED IN A staCK TO UNMAKE
;       THE MOVE LATER
;        
MOVE:   tsx    
        stx    SP1     ; SWITCH
        ldx    SP2     ; STACKS
        txs    
        lda    SQUARE
        pha            ; TO SQUARE
        tay    
        ldx    #$1F
CHECK:  cmp    BOARD,X ; CHECK FOR
        beq    TAKE    ; CAPTURE
        dex    
        bpl    CHECK
TAKE:   lda    #$CC
        sta    BOARD,X
        txa            ; CAPTURED
        pha            ; PIECE
        ldx    PIECE
        lda    BOARD,X
        sty    BOARD,X ; FROM
        pha            ; SQUARE
        txa    
        pha            ; PIECE
        lda    MOVEN
        pha            ; MOVEN
STRV:   tsx    
        stx    SP2     ; SWITCH
        ldx    SP1     ; STACKS
        txs            ; BACK
        rts    
;            
;       CONTINUATION OF SUB STRATGY
;       -CHECKS FOR CHECK OR CHECKMATE
;       and ASSIGNS VALUE TO MOVE
;        
CKMATE: ldy    BMAXC   ; CAN BLK CAP
        cpx    POINTS  ; MY KING?
        bne    NOCHEK    
        lda    #$00    ; GULP!
        beq    RETV    ; DUMB MOVE!
;        
NOCHEK: ldx    BMOB    ; IS BLACK
        bne    RETV    ; UNABLE TO
        ldx    WMAXP   ; MOVE and
        bne    RETV    ; KING IN CH?
        lda    #$FF    ; YES! MATE
;        
RETV:   ldx    #$04    ; RESTORE
        stx    STATE   ; STATE=4
;        
;       THE VALUE OF THE MOVE (IN ACCU)
;       IS COMPARED TO THE BEST MOVE and
;       REPLACES IT IF IT IS BETTER
;        
PUSH:   cmp    BESTV       ; IS THIS BEST
        bcc    RETP        ; MOVE SO FAR?
        beq    RETP
        sta    BESTV       ; YES!
        lda    PIECE       ; SAVE IT
        sta    BESTP
        lda    SQUARE
        sta    BESTM       ; FLASH DISPLAY
RETP:   lda    #'.'        ; print ... instead of flashing disp
        jmp    syschout    ; print . and return
;        
;       MAIN PROGRAM TO PLAY CHESS
;       PLAY FROM OPENING OR THINK
;        
GO:     ldx    OMOVE           ; OPENING?
        bmi    NOOPEN          ; -NO   *ADD CHANGE FROM bpl
        lda    DIS3            ; -YES WAS
        cmp    OPNING,X        ; OPPONENT'S
        bne    END             ; MOVE OK?
        dex    
        lda    OPNING,X         ; GET NEXT
        sta    DIS1             ; CANNED
        dex            ; OPENING MOVE
        lda    OPNING,X
        sta    DIS3             ; DISPLAY IT
        dex    
        stx    OMOVE            ; MOVE IT
        bne    MV2              ; (jmp)
;            
END:    lda     #$FF        ; *ADD - STOP CANNED MOVES
        sta    OMOVE        ; FLAG OPENING
NOOPEN: ldx    #$0C         ; FINISHED
        stx    STATE        ; STATE=C
        stx    BESTV        ; CLEAR BESTV
        ldx    #$14         ; GENERATE P
        jsr    GNMX         ; MOVES
;        
        ldx    #$04         ; STATE=4
        stx    STATE        ; GENERATE and
        jsr    GNMZ         ; TEST AVAILABLE
;
;    MOVES
;        
        ldx    BESTV        ; GET BEST MOVE
        cpx    #$0F         ; IF NONE
        bcc    MATE         ; OH OH!
;        
MV2:    ldx    BESTP        ; MOVE
        lda    BOARD,X      ; THE
        sta    BESTV        ; BEST
        stx    PIECE        ; MOVE
        lda    BESTM
        sta    SQUARE       ; and DISPLAY
        jsr    MOVE         ; IT
        jmp    CHESS
;        
MATE:   lda    #$FF         ; RESIGN
        rts                 ; OR staLEMATE
;        
;       SUBROUTINE TO ENTER THE
;       PLAYER'S MOVE
;        
DISMV:  ldx    #$04    ; ROTATE
Drol:   asl    DIS3    ; KEY
        rol    DIS2    ; INTO
        dex            ; DISPLAY
        bne    Drol    ;
        ora    DIS3
        sta    DIS3
        sta    SQUARE
        rts    
;        
;       THE FOLLOWING SUBROUTINE ASSIGNS
;       A VALUE TO THE MOVE UNDER
;       CONSIDERATION and RETURNS IT IN
;    THE ACCUMULATOR
;        

STRATGY:clc    
        lda    #$80
        adc    WMOB            ; PARAMETERS
        adc    WMAXC           ; WITH WHEIGHT
        adc    WCC             ; OF O25
        adc    WCAP1
        adc    WCAP2
        sec    
        sbc    PMAXC
        sbc    PCC
        sbc    BCAP0
        sbc    BCAP1
        sbc    BCAP2
        sbc    PMOB
        sbc    BMOB
        bcs    POS             ; UNDERFLOW
        lda    #$00            ; PREVENTION
POS:    lsr    
        clc                    ; **************
        adc    #$40
        adc    WMAXC           ; PARAMETERS
        adc    WCC             ; WITH WEIGHT
        sec                    ; OF 05
        sbc    BMAXC
        lsr            ; **************
        clc    
        adc    #$90
        adc    WCAP0           ; PARAMETERS
        adc    WCAP0           ; WITH WEIGHT
        adc    WCAP0           ; OF 10
        adc    WCAP0
        adc    WCAP1
        sec                    ; [UNDER OR OVER-
        sbc    BMAXC           ; FLOW MAY OCCUR
        sbc    BMAXC           ; FROM THIS
        sbc    BMCC            ; secTION]
        sbc    BMCC
        sbc    BCAP1
        ldx    SQUARE          ; ***************
        cpx    #$33
        beq    POSN            ; POSITION
        cpx    #$34            ; BONUS FOR
        beq    POSN            ; MOVE TO
        cpx    #$22            ; CENTRE
        beq    POSN            ; OR
        cpx    #$25            ; OUT OF
        beq    POSN            ; BACK RANK
        ldx    PIECE
        beq    NOPOSN
        ldy    BOARD,X
        cpy    #$10
        bpl    NOPOSN
POSN:   clc
        adc    #$02
NOPOSN: jmp    CKMATE          ; CONTINUE


;-----------------------------------------------------------------
; The following routines were added to allow text-based board
; DISPLAY over a standard RS-232 port.
;
POUT:   lda    PAINT
        bne    POUT0
        rts                ; return if PAINT flag = 0
POUT0:  jsr    POUT9       ; print CRLF
        jsr    POUT13      ; print copyright
        jsr    POUT10      ; print column labels
        ldy    #$00        ; init board location
        jsr    POUT5       ; print board horz edge
POUT1:  lda    #'|'        ; print vert edge
        jsr    syschout    ; PRINT ONE ASCII CHR - SPACE
        ldx    #$1F
POUT2:  tya                ; scan the pieces for a location match
        cmp    BOARD,X     ; match found?
        beq    POUT4       ; yes; print the piece's color and type
        dex                ; no
        bpl    POUT2       ; if not the last piece, try again
        tya                ; empty square    
        and    #$01        ; odd or even column?                
        sta    temp        ; save it
        tya                ; is the row odd or even
        lsr                ; shift column right 4 spaces 
        lsr                ;
        lsr                ;
        lsr                ;
        and    #$01        ; strip LSB  
        clc                ; 
        adc    temp        ; combine row & col to determine square color  
        and    #$01        ; is board square white or blk?
        bne    POUT25      ; white, print space
        lda    #'*'        ; black, print *
		
        .byte  $2c         ; used to skip over lda #$20
		;jmp    POUT25A

POUT25: lda    #$20        ; ASCII space
POUT25A:jsr    syschout    ; PRINT ONE ASCII CHR - SPACE
        jsr    syschout    ; PRINT ONE ASCII CHR - SPACE
POUT3:  iny            ; 
        tya            ; get row number
        and    #$08        ; have we completed the row?     
        beq    POUT1       ; no, do next column
        lda    #'|'        ; yes, put the right edge on
        jsr    syschout    ; PRINT ONE ASCII CHR - |             
        jsr    POUT12      ; print row number
        jsr    POUT9       ; print CRLF
        jsr    POUT5       ; print bottom edge of board
        clc                ; 
        tya                ; 
        adc    #$08        ; point y to beginning of next row
        tay                ;
        cpy    #$80        ; was that the last row?
        beq    POUT8       ; yes, print the LED values
        bne    POUT1       ; no, do new row

POUT4:  lda    REV        ; print piece's color & type
        beq    POUT41     ;
        lda    cpl+16,X   ;
        bne    POUT42     ;
POUT41: lda    cpl,x      ;
POUT42: jsr    syschout   ;
        lda    cph,x      ;
        jsr    syschout   ; 
        bne    POUT3      ; branch always

POUT5:  txa            ; print "-----...-----<crlf>"
        pha
        ldx    #$19
        lda    #'-'
POUT6:  jsr    syschout    ; PRINT ONE ASCII CHR - "-"
        dex
        bne    POUT6
        pla
        tax
        jsr    POUT9
        rts             

POUT8:  jsr    POUT10        ; 
        lda    BESTP
        jsr    syshexout    ; PRINT 1 BYTE AS 2 HEX CHRS    
        lda    #$20
        jsr    syschout    ; PRINT ONE ASCII CHR - SPACE
        lda    BESTV
        jsr    syshexout    ; PRINT 1 BYTE AS 2 HEX CHRS    
        lda    #$20
        jsr    syschout    ; PRINT ONE ASCII CHR - SPACE
        lda    DIS3
        jsr    syshexout    ; PRINT 1 BYTE AS 2 HEX CHRS    

POUT9:  lda    #$0D
        jsr    syschout    ; PRINT ONE ASCII CHR - CR
        lda    #$0A
        jsr    syschout    ; PRINT ONE ASCII CHR - LF
        rts 

POUT10: ldx    #$00        ; print the column labels
POUT11: lda    #$20        ; 00 01 02 03 ... 07 <CRLF>
        jsr    syschout
        txa
        jsr    syshexout
        inx
        cpx    #$08
        bne    POUT11
        beq    POUT9
POUT12: tya
        and    #$70
        jsr    syshexout
        rts

; print banner only once, preserve registers A, X, Y
POUT13: stx    tmp_zpgPt
	sta    tmp_zpgPt+1
	sty    tmp_zpgPt+2
        lda    PRNBANN
        beq    NOPRNBANN
	lda    #<banner
        sta    mos_StrPtr
	lda    #>banner
	sta    mos_StrPtr+1
	jsr    mos_CallPuts
NOPRNBANN:
        lda    #$00
        sta    PRNBANN        
	ldx    tmp_zpgPt
	lda    tmp_zpgPt+1
	ldy    tmp_zpgPt+2
	rts

;        ldx    #$00        ; Print the copyright banner
;POUT14: lda    banner,x
;        beq    POUT15
;        jsr    syschout
;        inx
;        bne    POUT14
;POUT15: rts         

KIN:    lda    #'?'
        jsr    syschout    ; PRINT ONE ASCII CHR - ?
        jsr    syskin      ; GET A KEYSTROKE FROM SYSTEM
        jsr    syschout    ; echo entered character
        and    #$4F        ; MASK 0-7, and ALpha'S
        rts
;
; 6551 I/O Support Routines
;
;
;Init_6551      lda   #$1F               ; 19.2K/8/1
;               sta   ACIActl            ; control reg 
;               lda   #$0B               ; N parity/echo off/rx int off/ dtr active low
;               sta   ACIAcmd            ; command reg 
;               rts                      ; done
;
; input chr from ACIA1 (waiting)
;
syskin:
		jsr mos_CallGetCh
		rts

               ;lda   ACIAsta            ; Serial port status             
               ;and   #$08               ; is recvr full
               ;beq   syskin             ; no char to get
               ;lda   ACIAdat            ; get chr
               ;rts                      ;
;
; output to OutPut Port
;
syschout:		; MKHBCOS: must preserve X, Y and A
		stx tmp_zpgPt
		sta tmp_zpgPt+1
		;sty tmp_zpgPt+2
		jsr mos_CallPutCh
		ldx tmp_zpgPt
		lda tmp_zpgPt+1
		;ldy tmp_zpgPt+2
		rts
;	        pha                      ; save registers
;ACIA_Out1      lda   ACIAsta            ; serial port status
;               and   #$10               ; is tx buffer empty
;               beq   ACIA_Out1          ; no
;               pla                      ; get chr
;               sta   ACIAdat            ; put character to Port
;               rts                      ; done

syshexout:     pha                     ;  prints AA hex digits
               lsr                     ;  MOVE UPPER NIBBLE TO LOWER
               lsr                     ;
               lsr                     ;
               lsr                     ;
               jsr   PrintDig          ;
               pla                     ;
PrintDig:      
	       sty	 tmp_zpgPt+2
               and   #$0F              ;
               tay                     ;
               lda   Hexdigdata,Y      ;
               ldy   tmp_zpgPt+2       ;
               jmp   syschout          ;

Hexdigdata:   .byte    "0123456789ABCDEF"
banner:       .byte    "MicroChess (c) 1996-2002 Peter Jennings, peterj@benlo.com"
              .byte    $0d, $0a, $00
cpl:          .byte    "WWWWWWWWWWWWWWWWBBBBBBBBBBBBBBBBWWWWWWWWWWWWWWWW"
cph:          .byte    "KQCCBBRRPPPPPPPPKQCCBBRRPPPPPPPP"
              .byte    $00
;
; end of added code
;
; BLOCK DATA

.segment "DATA"

	.ORG $0A20

SETW:       .byte     $03, $04, $00, $07, $02, $05, $01, $06
            .byte     $10, $17, $11, $16, $12, $15, $14, $13
            .byte     $73, $74, $70, $77, $72, $75, $71, $76
            .byte     $60, $67, $61, $66, $62, $65, $64, $63

MOVEX:      .byte     $00, $F0, $FF, $01, $10, $11, $0F, $EF, $F1
            .byte     $DF, $E1, $EE, $F2, $12, $0E, $1F, $21

POINTS:     .byte     $0B, $0A, $06, $06, $04, $04, $04, $04
            .byte     $02, $02, $02, $02, $02, $02, $02, $02

OPNING:     .byte     $99, $25, $0B, $25, $01, $00, $33, $25
            .byte     $07, $36, $34, $0D, $34, $34, $0E, $52
            .byte     $25, $0D, $45, $35, $04, $55, $22, $06
            .byte     $43, $33, $0F, $CC

.segment "KERN"

        .ORG $FE00

CHRIN:  lda $E000
        rts

CHROUT: sta $E000
        rts          

; this function was shamelessly ripped :-) from M.O.S. code (c) by Scott Chidester

STROUT:
        ldy #0                  ; Non-indexed variant starts at zero, of course
        lda mos_StrPtr+1        ; Save StrPtr so it isn't modified
        pha
PutsLoop:
        lda (mos_StrPtr),y      ; Get the next char in the string
        beq PutsDone            ; Zero means end of string
        jsr CHROUT              ; Otherwise put the char

    ; Update string pointer
        iny                     ; increment StrPtr-lo
        bne PutsLoop            ; No rollover? Loop back for next character
        inc mos_StrPtr+1        ; StrPtr-lo rolled over--carry hi byte
        jmp PutsLoop            ; Now loop back

PutsDone:
        pla
        sta mos_StrPtr+1            ; Restore StrPtr
        rts

.segment "VECT"

        .ORG $FFED

        jmp CHRIN
        jmp CHROUT
        jmp STROUT

;
;
; end of file
;

# Project: MKBasic

QRACKVER = qrack.cpp #qrack_ocl, qrack.cpp, or qrack_serial.cpp
SDLBASE  = $(SDLDIR)
SDLINCS   = -I"/usr/include/SDL2"
CPP      = g++ -D__DEBUG__ -DLINUX
CC       = gcc -D__DEBUG__
OBJ      = main.o VMachine.o MKCpu.o Memory.o Display.o GraphDisp.o MemMapDev.o MKGenException.o ConsoleIO.o qrack.o complex16simd.o
LINKOBJ  = main.o VMachine.o MKCpu.o Memory.o Display.o GraphDisp.o MemMapDev.o MKGenException.o ConsoleIO.o qrack.o complex16simd.o
BIN      = vm65
SDLLIBS  = -L/usr/lib -lSDL2main -lSDL2
LIBS     = -static-libgcc -m64 -g3 -ltermcap -lncurses -lpthread -lm -lOpenCL
CLIBS    = -static-libgcc -m64 -g3
INCS     =
CXXINCS  = 
CXXFLAGS = $(CXXINCS) -m64 -std=c++11 -Wall -pedantic -g3 -fpermissive
#CFLAGS   = $(INCS) -m32 -std=c++0x -Wall -pedantic -g3
CFLAGS   = $(INCS) -m64 -Wall -pedantic -g3
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) bin2hex all-after

clean: clean-custom
	${RM} $(OBJ) testall.o $(BIN) bin2hex

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS) $(SDLLIBS)

main.o: main.cpp
	$(CPP) -c main.cpp -o main.o $(CXXFLAGS) $(SDLINCS)

VMachine.o: VMachine.cpp
	$(CPP) -c VMachine.cpp -o VMachine.o $(CXXFLAGS) $(SDLINCS)

MKBasic.o: MKBasic.cpp
	$(CPP) -c MKBasic.cpp -o MKBasic.o $(CXXFLAGS)

MKCpu.o: MKCpu.cpp
	$(CPP) -c MKCpu.cpp -o MKCpu.o $(CXXFLAGS) $(SDLINCS)

Memory.o: Memory.cpp
	$(CPP) -c Memory.cpp -o Memory.o $(CXXFLAGS) $(SDLINCS)

Display.o: Display.cpp
	$(CPP) -c Display.cpp -o Display.o $(CXXFLAGS)

bin2hex: bin2hex.c
	$(CC) bin2hex.c -o bin2hex $(CFLAGS) $(CLIBS)

MKGenException.o: MKGenException.cpp
	$(CPP) -c MKGenException.cpp -o MKGenException.o $(CXXFLAGS)

GraphDisp.o: GraphDisp.cpp GraphDisp.h
	$(CPP) -c GraphDisp.cpp -o GraphDisp.o $(CXXFLAGS) $(SDLINCS)

MemMapDev.o: MemMapDev.cpp MemMapDev.h
	$(CPP) -c MemMapDev.cpp -o MemMapDev.o $(CXXFLAGS) $(SDLINCS)

ConsoleIO.o: ConsoleIO.cpp ConsoleIO.h
	$(CPP) -c ConsoleIO.cpp -o ConsoleIO.o $(CXXFLAGS)	

complex16simd.o: complex16simd.cpp
	$(CPP) -c complex16simd.cpp -o complex16simd.o $(CXXFLAGS)	

qrack.o: $(QRACKVER)
	$(CPP) -c $(QRACKVER) -o qrack.o $(CXXFLAGS)	

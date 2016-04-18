# Project: MKBasic

CPP      = g++ -D__DEBUG__ -DLINUX
CC       = gcc -D__DEBUG__
OBJ      = main.o VMachine.o MKBasic.o MKCpu.o Memory.o Display.o MKGenException.o
LINKOBJ  = main.o VMachine.o MKBasic.o MKCpu.o Memory.o Display.o MKGenException.o
BIN      = mkbasic
LIBS     = -static-libgcc -m32 -g3 -ltermcap
CLIBS    = -static-libgcc -m32 -g3
INCS     =
CXXINCS  = 
CXXFLAGS = $(CXXINCS) -m32 -std=c++0x -Wall -pedantic -g3 -fpermissive
#CFLAGS   = $(INCS) -m32 -std=c++0x -Wall -pedantic -g3
CFLAGS   = $(INCS) -m32 -Wall -pedantic -g3
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) bin2hex all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN) bin2hex

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

main.o: main.cpp
	$(CPP) -c main.cpp -o main.o $(CXXFLAGS)

VMachine.o: VMachine.cpp
	$(CPP) -c VMachine.cpp -o VMachine.o $(CXXFLAGS)

MKBasic.o: MKBasic.cpp
	$(CPP) -c MKBasic.cpp -o MKBasic.o $(CXXFLAGS)

MKCpu.o: MKCpu.cpp
	$(CPP) -c MKCpu.cpp -o MKCpu.o $(CXXFLAGS)

Memory.o: Memory.cpp
	$(CPP) -c Memory.cpp -o Memory.o $(CXXFLAGS)

Display.o: Display.cpp
	$(CPP) -c Display.cpp -o Display.o $(CXXFLAGS)

bin2hex: bin2hex.c
	$(CC) bin2hex.c -o bin2hex $(CFLAGS) $(CLIBS)

MKGenException.o: MKGenException.cpp
	$(CPP) -c MKGenException.cpp -o MKGenException.o $(CXXFLAGS)

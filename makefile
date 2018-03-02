# Project: MKBasic

SDLBASE  = $(SDLDIR)
SDLINCS   = -I"/usr/include/SDL2"
CPP      = g++ -D__DEBUG__ -DLINUX
CC       = gcc -D__DEBUG__
OBJ      = main.o VMachine.o MKCpu.o Memory.o Display.o GraphDisp.o MemMapDev.o MKGenException.o ConsoleIO.o
LINKOBJ  = main.o VMachine.o MKCpu.o Memory.o Display.o GraphDisp.o MemMapDev.o MKGenException.o ConsoleIO.o
BIN      = vm65
SDLLIBS  = -L/usr/lib -lSDL2main -lSDL2
LIBS     = -static-libgcc -m64 -g3 -ltermcap -lncurses -lpthread -lm
CLIBS    = -static-libgcc -m64 -g3
INCS     =

QRACKLIBS= qrack/libqrack.a
QRACKINCS= -Iqrack

CXXINCS  = $(SDLINCS) $(QRACKINCS)
CXXFLAGS = $(CXXINCS) -m64 -std=c++11 -Wall -pedantic -g3 -fpermissive
#CFLAGS   = $(INCS) -m32 -std=c++0x -Wall -pedantic -g3
CFLAGS   = $(INCS) -m64 -Wall -pedantic -g3
RM       = rm -f

ENABLE_OPENCL ?= 1
OPENCL_AMDSDK = /opt/AMDAPPSDK-3.0

ifeq (${ENABLE_OPENCL},1)
  LIBS += -lOpenCL
  CXXFLAGS += -DENABLE_OPENCL=1
  # Support the AMD SDK OpenCL stack
  ifneq ($(wildcard $(OPENCL_AMDSDK)/.),)
    LDFLAGS += -L$(OPENCL_AMDSDK)/lib/x86_64
  endif
else
  CXXFLAGS += -DENABLE_OPENCL=0
endif


.PHONY: all all-before all-after clean clean-custom qrack

all: all-before $(BIN) bin2hex all-after

$(QRACKLIBS):
	ENABLE_OPENCL=$(ENABLE_OPENCL) make -C qrack

clean: clean-custom
	${RM} $(OBJ) testall.o $(BIN) bin2hex
	ENABLE_OPENCL=$(ENABLE_OPENCL) make -C qrack clean

$(BIN): $(OBJ) ${QRACKLIBS}
	$(CPP) $(LINKOBJ) $(QRACKLIBS) -o $(BIN) $(LDFLAGS) $(LIBS) $(SDLLIBS)

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

GraphDisp.o: GraphDisp.cpp GraphDisp.h
	$(CPP) -c GraphDisp.cpp -o GraphDisp.o $(CXXFLAGS)

MemMapDev.o: MemMapDev.cpp MemMapDev.h
	$(CPP) -c MemMapDev.cpp -o MemMapDev.o $(CXXFLAGS)

ConsoleIO.o: ConsoleIO.cpp ConsoleIO.h
	$(CPP) -c ConsoleIO.cpp -o ConsoleIO.o $(CXXFLAGS)	

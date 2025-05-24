CXX = g++
CXXFLAGS = -O -std=c++20 -Wall -I ../lexertl17/include -I ../parsertl17/include

LDFLAGS = -O

LIBS = 

all: z80_disassem

z80_disassem: data.o disassem.o main.o
	$(CXX) $(LDFLAGS) -o z80_disassem data.o disassem.o main.o $(LIBS)

data.o: ../z80_assembler/data.cpp
	$(CXX) $(CXXFLAGS) -o data.o -c ../z80_assembler/data.cpp

disassem.o: ../z80_assembler/disassem.cpp
	$(CXX) $(CXXFLAGS) -o disassem.o -c ../z80_assembler/disassem.cpp

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -o main.o -c main.cpp

library:

binary:

clean:
	- rm *.o
	- rm z80_disassem

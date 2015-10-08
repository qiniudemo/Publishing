CC=g++
LD=g++

CXX_OBJ=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
SOURCES=$(shell find . -name "*.o") 
BIN=pub_mon
LIB_A=

CXXFLAGS=-Wall -Wno-unused -std=c++11 -g
LDFLAGS=

all: $(CXX_OBJ) link

build: clean all

clean:
	-rm *.o *.a $(BIN) *~
link:
	@echo "LD " $(BIN) 
	@$(LD) $(SOURCES) $(LIB_A) -o $(BIN) $(LDFLAGS)

.cpp.o:
	@echo "CC " $<
	@$(CC) $(CXXFLAGS) -c $<


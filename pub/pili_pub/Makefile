CC=g++
LD=g++

CXX_OBJ=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
#SOURCES=$(CXX_OBJ)
# $(shell) command will get called when $(SOURCES) is used
SOURCES=$(shell find . -name "*.o") 
BIN=pili_pub
LIB_A=../../pili/libpili_push.a

CXXFLAGS=-I../ -I../../pili/ -I../../rtmpdump/librtmp/ -Wall -Wno-unused -std=c++11 -g
LDFLAGS=-lcrypto -lssl -lm -lpthread -lz

all: $(CXX_OBJ) dep link

build: clean all

clean:
	-rm *.o *.a $(BIN) *~
dep:
	@echo "CP ../pub.o"
	@cp ../pub.o .
	@echo "CP ../x264.o"
	@cp ../x264.o .

link:
	@echo "LD " $(BIN) 
	@$(LD) $(SOURCES) $(LIB_A) -o $(BIN) $(LDFLAGS)

.cpp.o:
	@echo "CC " $<
	@$(CC) $(CXXFLAGS) -c $<


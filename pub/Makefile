CC=g++
LD=g++

CXX_OBJ=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
SOURCES=$(CXX_OBJ)
BIN=pub

CXXFLAGS=-I../rtmpdump/librtmp/ -I../pili/ -Wall -Wno-unused -std=c++11 -g
LDFLAGS=-lcrypto -lssl -lm -lpthread -lz

CH := $(shell find . -name "*.cpp" -o -name "*.hpp")
LIB_A=$(shell find . -name "*.a")
LIB_SO=

LIBRTMP=../rtmpdump/librtmp

all: $(SOURCES) pili librtmp link

build: clean all

clean:
	-rm *.o *.a $(BIN) *~
	cd ../rtmpdump/librtmp/ && make clean
pili:
	cp ../pili/libpili_push.a .
librtmp:
	cd $(LIBRTMP) && make
	cp $(LIBRTMP)/*.a .
link:
	@echo "LD " $(BIN) 
	@$(LD) $(SOURCES) $(LIB_A) -o $(BIN) $(LDFLAGS)

.cpp.o:
	@echo "CC " $<
	@$(CC) $(CXXFLAGS) -c $<

indent:
	for f in $(CH);  \
		do \
			echo $$f; \
	done


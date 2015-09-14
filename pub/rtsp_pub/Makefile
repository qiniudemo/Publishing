CC=g++
LD=g++

CXX_OBJ=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
SOURCES=$(shell find . -name "*.o") 
BIN=rtsp_pub
LIB_A=-L../../live555/liveMedia/ -L../../live555/groupsock/ -L../../live555/UsageEnvironment/ -L../../live555/BasicUsageEnvironment/ ../../rtmpdump/librtmp/librtmp.a

CXXFLAGS=-I../ -I../../pili/ -I../../rtmpdump/librtmp/ -I../../live555/liveMedia/include/ -I../../live555/groupsock/include/ -I../../live555/UsageEnvironment/include/ -I../../live555/BasicUsageEnvironment/include/ -Wall -Wno-unused -std=c++11 -g
LDFLAGS=-lcrypto -lssl -lm -lpthread -lz -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment

all: $(CXX_OBJ) live555 dep link

build: clean all

clean:
	-rm *.o *.a $(BIN) *~
clean555:
	cd ../../live555 && make clean
live555:
	if [ ! -e ../../live555/liveMedia/libliveMedia.a ]; then \
		cd ../../live555 && make; \
	fi
dep:
	if [ ! -e ../pub.o -o ! -e ../x264.o ]; then \
		cd .. && make; \
	fi
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


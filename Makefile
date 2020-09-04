PLUGIN=begone_inheritance64

IDASDK=$(HOME)/idasdk75

CC=clang++
IDADIR=/Applications/IDA\ Pro\ 7.5/ida.app/Contents/MacOS
CFLAGS=-D__MAC__ -L$(IDADIR) -I$(IDADIR)/plugins/hexrays_sdk/include
CFLAGS+=-I$(IDASDK)/include -std=c++11 -g
LDFLAGS=-dynamiclib -lida64

begone_inheritance : begone_inheritance.cpp
	$(CC) $(CFLAGS) begone_inheritance.cpp $(LDFLAGS) -o $(PLUGIN).dylib
	cp ./$(PLUGIN).dylib $(IDADIR)/plugins

sample : sample.c
	clang -arch arm64 -isysroot `xcrun --sdk iphoneos --show-sdk-path` sample.c -o sample

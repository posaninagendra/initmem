GCCDIR = /home/nagendra/gcc-plugins/gcc-install/bin
CXX = $(GCCDIR)/g++
CXXFLAGS = -std=c++11 -Wall -fno-rtti
CXXFLAGS += -Wno-literal-suffix
PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include

all: find_uninitialised.so

find_uninitialised.so: find_uninitialised.o
	$(CXX) $(LDFLAGS) -shared -o $@ $<

find_uninitialised.o: find_uninitialised.cc
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

clean:
	rm -f find_uninitialised.o find_uninitialised.so

check: find_uninitialised.so test.cc
	$(CXX) -fplugin=./find_uninitialised.so  test.cc 
	
	
.PHONY: all clean check

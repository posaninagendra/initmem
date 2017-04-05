GCCDIR = /home/nagendra/gcc-plugins/gcc-install/bin
CXX = $(GCCDIR)/g++
CXXFLAGS = -std=c++11 -Wall -fno-rtti
CXXFLAGS += -Wno-literal-suffix
PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include

all: warn_unused.so

warn_unused.so: warn_unused.o
	$(CXX) $(LDFLAGS) -shared -o $@ $<

warn_unused.o: warn_unused.cc
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

clean:
	rm -f warn_unused.o warn_unused.so

check: warn_unused.so test.cc
	$(CXX) -c -fplugin=./warn_unused.so -c test.cc 
	
	
.PHONY: all clean check

GCCDIR = /home/nagendra/gcc-plugins/gcc-install/bin
TESTFILE = /home/nagendra/gcc-plugins/gcc-plugins/initmem/testbed/data_structures.c 
#TESTFILE = test.c 
CXX = $(GCCDIR)/g++
CXXFLAGS = -std=c++11 -Wall -fno-rtti
CXXFLAGS += -Wno-literal-suffix
PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include

all: initmem.so

initmem.so: initmem.o
	$(CXX) $(LDFLAGS) -shared -o $@ $<

initmem.o: initmem.cc
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

clean:
	rm -f initmem.o initmem.so

check: initmem.so 
	$(CXX) -fplugin=./initmem.so -lm $(TESTFILE)
	
fcheck: initmem.so 
	$(CXX) -fplugin=./initmem.so -lm -fdump-tree-all $(TESTFILE)
	
.PHONY: all clean check fcheck

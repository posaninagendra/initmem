GCCDIR = /home/nagendra/gcc-plugins/gcc-install/bin
CXX = $(GCCDIR)/g++
CXXFLAGS = -std=c++11 -Wall -fno-rtti
CXXFLAGS += -Wno-literal-suffix
PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include

all: draw_cfg.so

draw_cfg.so: draw_cfg.o
	$(CXX) $(LDFLAGS) -shared -o $@ $<

draw_cfg.o: draw_cfg.cc
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

clean:
	rm -f draw_cfg.o draw_cfg.so

check: draw_cfg.so
	$(CXX) -fplugin=./draw_cfg.so -c test.cc -o /dev/null 2> test.dot
	dot -Tpdf test.dot > test.pdf
	xdg-open test.pdf
	
.PHONY: all clean check

#I hate error messages, and NULL is a necessary thing for C++!
FLAGS = -Wno-null-conversion -Wno-null-arithmetic -std=c++1y
#It's best to keep this high, as this program does take a while on large files
OPTIMIZATION 	= -O3
CXX		= clang++
LIBS_PREFIX	= /usr

all: build install libs-install

build: RS274.o
	$(CXX) $(FLAGS) $(OPTIMIZATION) ooptest.cpp RS274.o

install: build
	cp gcmanip /usr/bin/gcode-manipulator
	ln -s /usr/bin/gcode-manipulator /usr/bin/gcmanip

%.o: %.cpp
	$(CXX) -c $(FLAGS) $(OPTIMIZATION) $<

shared:
	$(CXX) -c $(FLAGS) $(OPTIMIZATION) RS274.cpp -o RS274.so

install-libs: build shared
	mkdir -p $(LIBS_PREFIX)/include/RS274
#	mkdir -p $(LIBS_PREFIX)/lib/RS274
	cp RS274.hpp $(LIBS_PREFIX)/include/RS274/RS274.hpp
	cp RS274.so $(LIBS_PREFIX)/lib/libRS274.so

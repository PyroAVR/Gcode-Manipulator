FLAGS = -std=c++1y
LIBS = -lpthread
#It's best to keep this high, as this program does take a while on large files
DEBUG = -O0 -g -Wall -fno-inline
OPTIMIZATION 	= -O3
CXX		= clang++
LIBS_PREFIX	= /usr
OUTPUT_NAME	= gcode-manipulator
SHORT_NAME	= gcmanip
all: build

build: libs
	$(CXX) $(FLAGS) $(LIBS) $(OPTIMIZATION) gcode-manipulator.cpp RS274.o -o $(OUTPUT_NAME)

install: build
	cp $(OUTPUT_NAME) /usr/bin/$(OUTPUT_NAME)
	ln -s /usr/bin/$(OUTPUT_NAME) /usr/bin/$(SHORT_NAME)

%.o: %.cpp
	$(CXX) -c $(FLAGS) $(LIBS) $(OPTIMIZATION) $<

libs: RS274.o

shared:
	$(CXX) -c $(FLAGS) $(LIBS) $(OPTIMIZATION) RS274.cpp -o RS274.so

install-libs: shared
	mkdir -p $(LIBS_PREFIX)/include/RS274
	cp RS274.hpp $(LIBS_PREFIX)/include/RS274/RS274.hpp
	cp RS274.so $(LIBS_PREFIX)/lib/libRS274.so

#failing
tests: libs
	$(CXX) $(FLAGS) $(LIBS) $(OPTIMIZATION) RS274.o tests/read_range.cpp -o tests/read_range
	$(CXX) $(FLAGS) $(LIBS) $(OPTIMIZATION) RS274.o tests/debug_info.cpp -o tests/debug_info
clean:
	rm -f *.o *.so tests/read_range tests/debug_info gcode-manipulator
help:
	@echo "Targets:"
	@echo "build		build everything, do not install to system"
	@echo "install		build everything and install binaries"
	@echo "install-libs	build lib and install libraries"
	@echo "tests		build tests"
	@echo "clean		remove all built files"

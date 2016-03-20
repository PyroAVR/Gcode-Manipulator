#I hate error messages, and NULL is a necessary thing for C++!
FLAGS = -Wno-null-conversion -Wno-null-arithmetic -std=c++1y
#It's best to keep this high, as this program does take a while on large files
OPTIMIZATION 	= -O3
CXX		= clang++
LIBS_PREFIX	= /usr
OUTPUT_NAME	= gcode-manipulator
SHORT_NAME	= gcmanip
all:
	make build
	clear
	@echo "Enter password to install, or control-C to stop."
	sudo make install
	sudo make libs-install

build: RS274.o
	$(CXX) $(FLAGS) $(OPTIMIZATION) main.cpp RS274.o -o $(OUTPUT_NAME)

install: build
	cp $(OUTPUT_NAME) /usr/bin/$(OUTPUT_NAME)
	ln -s /usr/bin/$(OUTPUT_NAME) /usr/bin/$(SHORT_NAME)

%.o: %.cpp
	$(CXX) -c $(FLAGS) $(OPTIMIZATION) $<

shared:
	$(CXX) -c $(FLAGS) $(OPTIMIZATION) RS274.cpp -o RS274.so

install-libs: shared
	mkdir -p $(LIBS_PREFIX)/include/RS274
	cp RS274.hpp $(LIBS_PREFIX)/include/RS274/RS274.hpp
	cp RS274.so $(LIBS_PREFIX)/lib/libRS274.so

#failing
tests: install-libs
	$(CXX) -lRS274 $(FLAGS) tests/read_range.cpp -o tests/read_range

clean:
	rm -f *.o *.so
help:
	@echo "Targets:"
	@echo "build		build everything, do not install to system"
	@echo "install		build everything and install binaries"
	@echo "install-libs	build lib and install libraries"
	@echo "tests		build tests"
	@echo "clean		remove all built files"

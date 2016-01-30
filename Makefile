#I hate error messages, and NULL is a necessary thing for C++!
FLAGS = -Wno-null-conversion -Wno-null-arithmetic -std=c++1y
#It's best to keep this high, as this program does take a while on large files
OPTIMIZATION = -O3
CXX		= clang++

all:
	$(CXX) $(FLAGS) $(OPTIMIZATION) main.cpp -o gcmanip

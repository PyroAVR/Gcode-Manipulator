#I hate error messages, and NULL is a necessary thing for C++!
all:
	clang++ -Wno-null-conversion -Wno-null-arithmetic -std=c++1y main.cpp -o gcmanip

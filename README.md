**Gcode Manipulator**

*what an uncreative name...*

This program performs Cartesian translations on Gcode files.
It can handle the following codes:

G
M
F
P
;comments

It is unconcerned with spaces and other whitespace, and will print neatly formatted output.
It has low memory usage due to reading only one line at a time to avoid data duplication in memory.
For this reason, it can handly _very_ large files with almost no impact on system resources beyond
the size of the original file. Efficiency, *HAYO!*

To build for your platform, simply type _make_.
You may need to change the compiler.  I use LLVM/Clang++, but GCC and MSVC should work fine.
_-std=c++1y_ may be an issue, try replacing it with _-std=c++11_ if _make_ fails.

Good luck!

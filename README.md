A simple program with simple logic to list out all rootfiles (of a C source code) in a given directory.  
rootfile: a header file (*.h) or a C source file (*.c) which contains (#'include's) only the standard headers available in the system (i.e. files which do not have any user-defined header files included in file using the syntax: `#include "userdefined.h`).

warning: This code is not well-tested, so use carefully!!

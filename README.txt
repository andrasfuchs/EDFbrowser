
Requirements
============
Qt  http://www.qt.io/

Minimum version 4.7.1 or later, preferable 4.8.7

Do not use Qt 5.x.x to compile EDFbrowser, Qt5 is not yet ready for production use. It contains still bugs that affects EDFbrowser.


The GCC compiler on Linux or Mingw-w64 on windows. <http://mingw-w64.sourceforge.net/>

http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/4.8.2/threads-posix/dwarf/i686-4.8.2-release-posix-dwarf-rt_v3-rev3.7z/download

Please, do not use any microsoft tools or compilers. Don't waste your time, it's not going to work!



Introduction
============

EDFbrowser is a Qt4 application and uses qmake as part of the build
process.  qmake is part of your Qt4 installation.



Build and run without "installing"
==================================

You need to have Qt and GCC (Mingw-w64 on Windows) installed.

Extract the sourcefile and enter the following commands:

qmake

make

Now you can run the program.



Installing
==========

sudo make install



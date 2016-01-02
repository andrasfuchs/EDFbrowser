EDFBrowser Pro
==============

The original EDFBrowser was created by Teunis van Beelen.

This Pro version in an extended version of the original and maintained by [Andras Fuchs](linkedin.com/in/andrasfuchs).

New features in the Pro version
-----------

* Software architecture
  * Added .gitattributes to standardize line endings in files, and .gitignore to avoid personalized config file/temp file/generated file conflicts.
  * Added multi-target support, so the source can be compiled with Visual Studio 2013 Community Edition and the Visual C compiler.
  * The compilation with QtCreator is not effected.
  * All the target-specific definition are now in the multitargersupport.h file.
  * Moving towards to OOP design
  * Moving towards the Model-View-Controller design pattern's source code separation
  * C++ v11

* Power Spectrum
  * new looks
  * support for different mouse gestures (dragging (click+move), zoom (mousewheel), reset zoom (double-click))
  * multi-signal support on one histogram (but it's still backward compatible)
  * signals are displayed in three different time ranges for comparison
  * color bar text positioning
  * support for "blind spots" for unavailable data
  * values are formatted with a thousand separator char
  * cursor displays the unit as well
  * new controls to change the Y-axis, turn the colorbars on/off and for signal selection
  * calculation of the input signal's on 3 different time-intervals
  * performance improvements

* real-time playback (Ctrl+Space)

Screenshots
----------------

![Power Spectrum with multiple signals](https://cloud.githubusercontent.com/assets/910321/11691278/9c44339e-9e9a-11e5-8c9a-213c7ca832c1.png)

![Power Spectrum with multiple signals zoom](https://cloud.githubusercontent.com/assets/910321/11691682/68e58172-9e9c-11e5-95f6-eeeeb3ab4b79.png)

![Power Spectrum controls](https://cloud.githubusercontent.com/assets/910321/11691693/737814d8-9e9c-11e5-9606-afb2d9506cc3.png)

![One signal and the controls](https://cloud.githubusercontent.com/assets/910321/11691377/f5abc636-9e9a-11e5-8aff-a832d7025666.png)

![One signal, the color bars and the controls](https://cloud.githubusercontent.com/assets/910321/11691382/ffa71df2-9e9a-11e5-8b71-301520db2ee5.png)




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

EDFbrowser is a Qt4 application and uses qmake as part of the build process.  qmake is part of your Qt4 installation.



Build and run without "installing"
==================================

You need to have Qt and GCC (Mingw-w64 on Windows) installed.

Extract the sourcefile and enter the following commands:

```
qmake

make
```

Now you can run the program.



Installing on Linux
==========

```
sudo make install
```

Installing on Windows
==========

1. Download the current version from the releases tab.
2. Run and install


Version History
===============
* 1.70
  * Initial release of the Pro version
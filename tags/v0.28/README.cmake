    QDevelop - A Development Environment for Qt4

    (C) 2006 Jean-Luc Biord <jlbiord@qtfr.org>

Compiling using cmake
~~~~~~~~~~~~~~~~~~~~~

Since Version 0.20, it's possible to compile the application using cmake. Please
note that the official build tool for QDevelop is still qmake, and cmake it in 
early beta stage.

The IDE cannot handle cmake projects yet, but this may change in the future.

You need to use cmake version 2.4.3 (2.4.0 and above shuold work, but only 2.4.3
and above have been tested). The compilation was tested under Linux, and on Win32
the compilation does work, but QDevelop loads up with a console window.

To build QDevelop, we need to create a new subdirectory to build off source, we call
cmake, and the finally make:

	mkdir cbuild
	cd cbuild
	cmake -G "MinGW Makefiles" ../
	make
	
Why using cmake?
~~~~~~~~~~~~~~~~

Quite frankly, qmake is nice, but it's a toy. You cannot set dependencies,
you cannot make conditional compile time switches (or at lest not in an automated 
way, when packaging for example). 

Using cmake, you also have a percentage which displays how much the compilation 
is going on. The output is MUCH nicer: by default you do not see the 
"gcc -c ..." messages, instead you see nice messages saying that the file is being 
compiled (in color!).

- diego - diegoiast@gmail.com

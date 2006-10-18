    QIde - A Development Environment for Qt4

    (C) 2006 Jean-Luc Biord <jlbiord@qtfr.org>

Installation
~~~~~~~~~~~~

Download is available from http://qide.free.fr, the official website.

You need Qt (from Trolltech) at least version 4.0 to be able to compile
QIde. Most Linux distributions normally already package the latest
version of Qt, so it is likely that you need to worry about this. However,
you need the development package as well (sometimes named as qt-devel or
qt4-devel, it varies depends on the distributions).

The simplest way to compile QIde package is:

1. Extract the source zip.
2.`cd' to the directory containing the package's source code and type
     `qmake' (`qmake -recursive' with Qt 4.2.x) to generate the Makefile for your system.
3. Type `make' on Linux or `mingw32-make` on Windows to compile the package.
4. The executable file QIde (QIde.exe under Windows) is built on bin/ directory.

If you want to have code completion, install the package ctags. Under Linux,
ctags is normally installed with development packages. For Windows, ctags is accessible here : http://prdownloads.sourceforge.net/ctags/ec554w32.zip?download 
If you want to have debugging functionalities, you must install gdb. Like ctags, gdb is normally installed 
with development packages under Linux. gdb for Windows is available here : http://www.mingw.org/download.shtml/

Command line options
~~~~~~~~~~~~~~~~~~~~

QIde may be launched with some parameters:

QIde [-l translation] [file(s)] [project]

-l translation
	When QIde starts, it detect automatically the language of installed system. If you want to use another language, use this option.
	Available languages are English, French, German and Dutch.

[file(s)] Open the files in editors. 

[project] Open the (.pro) file in QIde. The other files are closed before open the project.

Documentation
~~~~~~~~~~~~~

Up to date documentation can be found at:
http://qide.free.fr

License
~~~~~~~~~~~~~

Copyright (C) 2006 Jean-Luc Biord <jlbiord@qtfr.org>
Code completion Copyright (C) 2006 Frédéric Julian <fred.julian@gmail.com> and IComplete Team.

QIde is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

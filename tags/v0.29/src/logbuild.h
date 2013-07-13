/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2007  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://biord-software.org/qdevelop/
*
*/

#ifndef LOGBUILD_H
#define LOGBUILD_H
//
#include <QTextEdit>
#include <QTextBlockUserData>
//
class MainImpl;
//
class BlockLogBuild : public QTextBlockUserData
{
public:
	BlockLogBuild(QString d) : QTextBlockUserData() { m_directory = d; }
	QString directory() { return m_directory; }
private:
	QString m_directory;
};
//
/*! \brief The LogBuild class is used as build logger displayed in the dock Outputs.
*           
*/
class LogBuild : public QTextEdit
{
Q_OBJECT
public:
	/**
	* The constructor
	*/
	LogBuild(QWidget * parent = 0);
	/**
	* Called by the mainimpl to set the pointer with the address of the main window.
	*/
	void setMainImpl( MainImpl *mainImpl ) { m_mainImpl = mainImpl; }
	static bool containsError(QString message, QString & file, uint & line);
	static bool containsWarning(QString message, QString & file, uint & line);
protected:	
	/**
	* When the user double-click on a error or warning line, the file is opened in a editor.
	*/
    void mouseDoubleClickEvent( QMouseEvent * event );
public slots:
	/**
	* In main window, the signal message(QString, QString) of the class Build is connected
	*			to the slot slotMessagesBuild in this class.
	*/
	void slotMessagesBuild(QString list, QString directory);
private:
	/** 
	* A pointer to the MainImpl class 
	*/
	MainImpl* m_mainImpl;
signals:
	void incErrors();
	void incWarnings();
};
#endif

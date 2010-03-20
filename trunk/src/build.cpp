/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006 - 2010 Jean-Luc Biord
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
#include "editor.h"
#include "build.h"
#include <QProcess>
#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <QDir>
#include <QLocale>
#include <QTextCodec>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
Build::Build(QObject * parent, QString qmakeName, QString makeName, QString makeOptions, QString absoluteProjectName, 
	bool qmake, bool n, bool g, QString compileFile, QString forceMode)

	: QThread(parent)
{
	connect(parent, SIGNAL(stopBuild()), this, SLOT(slotStopBuild()) );
	m_isStopped = false;
	m_qmakeName = qmakeName;
	m_qmake = qmake;
	m_makeName = makeName;
	m_makeOptions = makeOptions;
	m_projectDirectory = QFileInfo(absoluteProjectName).absolutePath();
	m_projectName = QFileInfo(absoluteProjectName).fileName();
	m_clean = n;
	m_build = g;
	m_compileFile = compileFile;
	m_errors = 0;
	m_warnings = 0;
	m_forceMode = forceMode;
}
//
void Build::slotIncErrors() 
{ 
	m_errors++; 
}
//
void Build::slotIncWarnings() 
{ 
	m_warnings++; 
}
//
void Build::run()
{
	QStringList list;
	m_buildProcess = new QProcess();
	connect(m_buildProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slotBuildMessages()) );
	connect(m_buildProcess, SIGNAL(readyReadStandardError()), this, SLOT(slotBuildMessages()) );
	m_buildProcess->setWorkingDirectory( m_projectDirectory );
	if( m_qmake || !m_forceMode.isEmpty() )
	{
		emit message( QString(tr("Update project"))+" (qmake "+m_projectName+")..." );
		QStringList forceDebugList;
		if (!m_forceMode.isEmpty())
		{
			forceDebugList << "-after" << "CONFIG+="+m_forceMode;
		}
#ifdef Q_WS_MAC
		m_buildProcess->start(m_qmakeName, QStringList() <<"-spec"<< "macx-g++" << m_projectName << forceDebugList);
#else
		m_buildProcess->start(m_qmakeName, QStringList() << m_projectName << forceDebugList);
#endif
    	if (!m_buildProcess->waitForFinished(800000))
		{
			m_buildProcess->deleteLater();
        	return;
		}
		emit message( QString(m_buildProcess->readAll()) );
	}
	if( m_clean )
	{

		emit message( QString("\n"+tr("Clean Project")+" (make clean)...\n") );
		m_buildProcess->start(m_makeName, QStringList("clean"));
    		if (!m_buildProcess->waitForFinished(800000))
		{
			m_buildProcess->deleteLater();
        		return;
		}
		emit message( QString(m_buildProcess->readAll()) );
	}	
	if( m_build )
	{
		QString compilation = buildOnly(m_compileFile);
		if( !compilation.isEmpty() )
		{
			m_buildProcess->start( compilation );
			emit message( tr("\nCompilation of %1...\n").arg(m_compileFile) );
		}
		else
		{
			emit message( QString("\n"+tr("Build")+" (make)...\n") );
			m_buildProcess->start(m_makeName, QStringList() << m_makeOptions.split(" ", QString::SkipEmptyParts));
		}
    		if (!m_buildProcess->waitForFinished(800000))
		{
			m_buildProcess->deleteLater();
        	return;
		}
	}
	emit message( QString(m_buildProcess->readAll()), m_projectDirectory);
	m_buildProcess->deleteLater();
}
//
void Build::slotBuildMessages()
{
	#ifdef Q_OS_WIN32
	// Divius: workaround for Windows: console use IBM 866 but system locale is CP1251
	static QTextCodec *codec = QTextCodec::codecForName("IBM 866");
	if (QLocale::system().language() == QLocale::Russian && codec)
	{
		emit message( codec->toUnicode(m_buildProcess->readAllStandardOutput()), m_projectDirectory );
		emit message( codec->toUnicode(m_buildProcess->readAllStandardError()), m_projectDirectory );
	}
	else
	{
		emit message( QString::fromLocal8Bit(m_buildProcess->readAllStandardOutput()), m_projectDirectory );
		emit message( QString::fromLocal8Bit(m_buildProcess->readAllStandardError()), m_projectDirectory );
	}
	#else
	emit message( QString::fromLocal8Bit(m_buildProcess->readAllStandardOutput()), m_projectDirectory );
	emit message( QString::fromLocal8Bit(m_buildProcess->readAllStandardError()), m_projectDirectory );
	#endif
}
//
void Build::slotStopBuild()
{
	m_isStopped = true;
	emit message( QString("\n---------------------- "+tr("Build stopped")+"  ----------------------\n") );
	m_buildProcess->kill();
	m_buildProcess->deleteLater();
}
//
QString Build::buildOnly( QString sourceFile )
{
	if( sourceFile.isEmpty() )
		return QString();
	bool automaticCompilation = false;
	if( sourceFile.endsWith("-qdeveloptmp.cpp") )
	{
		sourceFile = sourceFile.section("-qdeveloptmp.cpp",0 ,0)+".cpp";
		automaticCompilation = true;
	}
	QString objectFile = sourceFile.mid(0, sourceFile.lastIndexOf("."))+".o";
#ifndef WIN32
	//QString name = QDir( m_projectDirectory ).relativeFilePath( sourceFile ); 
	//return m_makeName+" "+ name.mid(0, name.lastIndexOf("."))+".o";
#endif
	QString shortObjectFile = objectFile;
	if( !objectFile.section("/", -1, -1).isEmpty() )
		shortObjectFile = shortObjectFile.section("/", -1, -1).section(".", 0, 0)+".o";
	QString directives;
	QFile makefile(m_projectDirectory+"/"+"Makefile");
	if (!makefile.open(QIODevice::ReadOnly | QIODevice::Text))
		return QString();
	QString target, makefileFile, CXX, DEFINES, CXXFLAGS, INCPATH;
	while (!makefile.atEnd()) 
	{
		QString line = QString( makefile.readLine() );
		// This block is for Windows where the Makefile include Makefile.Debug or Makefile.Release.
		// Not used under Linux
		if( line.section(" ", 0, 0).simplified() == "first:" && (line.section(" ", 1, 1).simplified()=="debug" || line.section(" ", 1, 1).simplified()=="release"))
			target = line.section(" ", 1, 1).simplified();
		if( line.section("=", 0, 0).simplified() == "MAKEFILE" )
			makefileFile = line.section("=", 1, 1).simplified();
		if( !target.isEmpty() && line.section(":", 0, 0) == target )
		{
			makefileFile = line.section(" ", 1, 1).simplified().replace("$(MAKEFILE)", makefileFile);
			makefile.close();
			makefile.setFileName( m_projectDirectory+"/"+makefileFile );
			if (!makefile.open(QIODevice::ReadOnly | QIODevice::Text))
				return QString();
			target = QString();
			continue;
		}
		// Common block for Windows and Linux 
		if( line.section("=", 0, 0).simplified() == "CXX" )
			CXX = line.section("=", 1, 1).simplified();
		if( line.section("=", 0, 0).simplified() == "DEFINES" )
			DEFINES = line.section("=", 1, 1).simplified();
		if( line.section("=", 0, 0).simplified() == "CXXFLAGS" )
			CXXFLAGS = line.section("=", 1, 1).simplified().replace("$(DEFINES)", DEFINES);
		if( line.section("=", 0, 0).simplified() == "INCPATH" )
			INCPATH = line.section("=", 1, 1).simplified();
		if( line.section("-o ", 1, 1).section(" ", 0, 0).replace("\\", "/").simplified().right(shortObjectFile.length()) == shortObjectFile )
		{
			line = line.simplified();
			line = line.replace("$(CXX)", CXX);
			line = line.replace("$(CXXFLAGS)", CXXFLAGS);
			line = line.replace("$(INCPATH)", INCPATH);
			directives = line;
			break;
		}

	}
	if( automaticCompilation )
	{
		QString s = sourceFile.section(".cpp", 0, 0).section("/", -1);
		directives.replace(s+".o", s+"-qdeveloptmp.o");
		directives.replace(s+".cpp", s+"-qdeveloptmp.cpp");
	}
	return directives;
}
//

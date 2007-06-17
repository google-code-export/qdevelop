/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006  Jean-Luc Biord
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
* Contact e-mail: Jean-Luc Biord <jl.biord@free.fr>
* Program URL   : http://qdevelop.org
*
*/
#include "editor.h"
#include "build.h"
#include <QProcess>
#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <QDir>
//
Build::Build(QObject * parent, QString qmakeName, QString makeName, QString rep, bool qmake, bool n, bool g, QString compileFile)

	: QThread(parent)
{
	connect(parent, SIGNAL(stopBuild()), this, SLOT(slotStopBuild()) );
	m_isStopped = false;
	m_qmakeName = qmakeName;
	m_qmake = qmake;
	m_makeName = makeName;
	projectDirectory = rep;
	m_clean = n;
	m_build = g;
	m_compileFile = compileFile;
	m_errors = 0;
	m_warnings = 0;
}
//
void Build::incErrors() 
{ 
	m_errors++; 
}
//
void Build::incWarnings() 
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
	m_buildProcess->setWorkingDirectory( projectDirectory );
	if( m_qmake )
	{
		emit message( QString(tr("Update project"))+" (qmake)..." );
		m_buildProcess->start(m_qmakeName);
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
			m_buildProcess->start(m_makeName);
		}
    		if (!m_buildProcess->waitForFinished(800000))
		{
			m_buildProcess->deleteLater();
        	return;
		}
	}
	emit message( QString(m_buildProcess->readAll()), projectDirectory);
	m_buildProcess->deleteLater();
}
//
void Build::slotBuildMessages()
{
	emit message( QString::fromLocal8Bit(m_buildProcess->readAllStandardOutput()), projectDirectory );
	emit message( QString::fromLocal8Bit(m_buildProcess->readAllStandardError()), projectDirectory );
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
	QString objectFile = sourceFile.mid(0, sourceFile.lastIndexOf("."))+".o";
#ifndef WIN32
	QString name = QDir( projectDirectory ).relativeFilePath( sourceFile ); 
	return m_makeName+" "+ name.mid(0, name.lastIndexOf("."))+".o";
#endif
	QString shortObjectFile = objectFile;
	if( !objectFile.section("/", -1, -1).isEmpty() )
		shortObjectFile = shortObjectFile.section("/", -1, -1).section(".", 0, 0)+".o";
	QString directives;
	QFile makefile(projectDirectory+"/"+"Makefile");
	if (!makefile.open(QIODevice::ReadOnly | QIODevice::Text))
		return QString();
	QString target, makefileFile, CXX, DEFINES, CXXFLAGS, INCPATH;
	while (!makefile.atEnd()) 
	{
		QString line = QString( makefile.readLine() );
		// Partie concernant le fichier Makefile appelant sous Windows Makefile.Debug ou Makefile.Release.
		// Sans objet sous Linux
		if( line.section(" ", 0, 0).simplified() == "first:" && (line.section(" ", 1, 1).simplified()=="debug" || line.section(" ", 1, 1).simplified()=="release"))
			target = line.section(" ", 1, 1).simplified();
		if( line.section("=", 0, 0).simplified() == "MAKEFILE" )
			makefileFile = line.section("=", 1, 1).simplified();
		if( !target.isEmpty() && line.section(":", 0, 0) == target )
		{
			makefileFile = line.section(" ", 1, 1).simplified().replace("$(MAKEFILE)", makefileFile);
			makefile.close();
			makefile.setFileName( projectDirectory+"/"+makefileFile );
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
	return directives;
}
//

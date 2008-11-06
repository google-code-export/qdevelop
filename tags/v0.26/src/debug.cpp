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
#include "debug.h"
#include "editor.h"
#include "registersimpl.h"
#include <QProcess>
#include <QDebug>
#include <QRegExp>
#include <QDir>

//
#ifdef  Q_OS_LINUX
#include <signal.h>
#include <sys/types.h>
#endif
//
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
Debug::Debug(QObject * parent, RegistersImpl *registersImpl, QString gdbName, Parameters p, QString exe, bool exeOnly)
	: QThread(parent)
{
	m_parent = parent;
	m_registersImpl = registersImpl;
	m_gdbName = gdbName;
	executableName = exe;
	m_executeWithoutDebug = exeOnly;
	m_parameters = p;
}
//
void Debug::run()
{
	m_pid = 0;
	m_request = None;
	connect(m_parent, SIGNAL(debugCommand(QString)), this, SLOT(slotDebugCommand(QString)) );
	connect(m_registersImpl, SIGNAL(debugCommand(QString)), this, SLOT(slotDebugCommand(QString)) );
	connect(m_parent, SIGNAL(stopDebug()), this, SLOT(slotStopDebug()) );
	connect(m_parent, SIGNAL(pauseDebug()), this, SLOT(slotPauseDebug()) );
	connect(m_parent, SIGNAL(otherVariables(QStringList)), this, SLOT(slotOtherVariables(QStringList)) );
	if( m_executeWithoutDebug )
		executeWithoutDebug();
	else
		launchDebug();
}
//
void Debug::executeWithoutDebug()
{
	emit message( tr("Running...") );
	processDebug = new QProcess();
	connect(processDebug, SIGNAL(readyReadStandardOutput()), this, SLOT(slotMessagesDebug()) );
	connect(processDebug, SIGNAL(readyReadStandardError()), this, SLOT(slotMessagesDebug()) );
    
	setEnvironment( processDebug );

	//processDebug->start("\""+executableName+"\"",  QStringList() << m_parameters.arguments);
	processDebug->start(executableName,  QStringList() << m_parameters.arguments);
    processDebug->waitForFinished(500); // On attend un peu pour passer en �at Running
	while( processDebug->state() == QProcess::Running )
	{
     	processDebug->waitForFinished(5);
	}
	emit message( "---------------------- "+tr("Exited normally")+" ----------------------" );
	emit endDebug();
}
//
void Debug::launchDebug()
{
	emit message( tr("Debug...") );
	processDebug = new QProcess();
	connect(processDebug, SIGNAL(readyReadStandardOutput()), this, SLOT(slotMessagesDebug()) );
	connect(processDebug, SIGNAL(readyReadStandardError()), this, SLOT(slotMessagesDebug()) );
	setEnvironment( processDebug );
	processDebug->start(m_gdbName, QStringList() << "--silent" << "--fullname" << executableName );
	configureGdb();
	writeMessagesToDebugger();
	processDebug->write("run "+m_parameters.arguments.toLatin1()+"\n");
    processDebug->waitForFinished(500); // On attend un peu pour passer en �at Running
	while( processDebug->state() == QProcess::Running )
	{
     	processDebug->waitForFinished(5);
		writeMessagesToDebugger();
	}
	emit message( "---------------------- "+tr("Debug exited")+" ----------------------" );
}
//
void Debug::setEnvironment(QProcess *process)
{
	process->setWorkingDirectory( m_parameters.workingDirectory );
	process->setEnvironment( m_parameters.env );
}
//
void Debug::writeMessagesToDebugger()
{
	QStringListIterator it(messagesToDebugger);
	while( it.hasNext() )
	{
		QString msg = it.next();
		processDebug->write( msg.toLatin1() );
	}
	messagesToDebugger.clear();
}
//
void Debug::slotMessagesDebug()
{
	char breakpoint[] = { 26, 26, 0x0 };
	QString messages = processDebug->readAllStandardOutput();
	if( m_executeWithoutDebug )
	{
		slotMessagesError();
		return;	
	}
	static QString listMessagesDebug;
	listMessagesDebug += QString( messages );
	if( !listMessagesDebug.endsWith("(gdb) ") )
	{
		return;
	}
	while( listMessagesDebug.length() )
	{
		QString s = listMessagesDebug.section("\n", 0, 0);
		listMessagesDebug = listMessagesDebug.section("\n", 1);
		if( !s.isEmpty() )
		{
			if( s.indexOf( breakpoint )==0 )
			{
				emit message( QString::fromUtf8(s.toLocal8Bit()) );
				listVariables.clear();
				listVariablesToSend.clear();
				m_request = InfoScope;
				QString numLine = s.section(":", -4, -4);
				messagesToDebugger << "info scope "+numLine+"\n";
				continue;
			}
			//
			if( s.indexOf("Program exited normally.") == 0 )
			{
				emit message( QString::fromUtf8(s.toLocal8Bit()) ); 
				slotStopDebug();
			}
			else if( s.indexOf("[New Thread") == 0 && m_pid==0)
			{
				m_pid = s.section("(", 1, 1).section(" ", 1, 1).section(")", 0, 0).toInt();
			}
			else if( m_request == Registers )
			{
				s = "Registers:"+listMessagesDebug;
				emit message( QString::fromUtf8(s.toLocal8Bit()) ); 
				listMessagesDebug.clear();
				messagesToDebugger <<  "info sources\n";
				m_request = InfoSources;
			}
			else if( m_request == InfoSources )
			{
				if( s.indexOf("(gdb)") == 0)
				{
					m_request = None;
					s = "InfoSourcesEnd";
					messagesToDebugger <<  "bt\n";
				}
				else
					s = "InfoSources:"+s;
				emit message( QString::fromUtf8(s.toLocal8Bit()) ); 
			}
			else if( m_request == InfoScope  )
			{
				if( s.indexOf("(gdb)") == 0 && listVariables.count() != 0 )
				{
					foreach(QString s, m_otherVariables)
					{
						Variable v;
						v.name = s;
						v.kind = Debug::OtherArgs;
						listVariables.append( v );
					}
					m_request = Whatis;
					Variable variable = listVariables.at(0);
					messagesToDebugger << "whatis "+variable.name+"\n";
				}
				else if( s.contains( "No arguments" ) )
				{
					if( listVariables.count() )
					{
						foreach(QString s, m_otherVariables)
						{
							Variable v;
							v.name = s;
							v.kind = Debug::OtherArgs;
							listVariables.append( v );
						}
						m_request = Whatis;
						Variable variable = listVariables.at(0);
						messagesToDebugger << "whatis "+variable.name+"\n";
					}
					else
					{
						m_request = None;
					}
				}
				else if( s.indexOf("Symbol ") == 0 )
				{
					QString name = s.section("Symbol ", 1).section(" ", 0, 0).simplified();
					if( !name.isEmpty() )
					{
						Variable variable;
						variable.name = name;
						variable.kind = Local;
						listVariables.append( variable );
					}
				}
			}
			else if ( m_request ==  Whatis )
			{
				QString type = s.section("type = ", 1).simplified();
				if( !type.isEmpty() )
				{
					Variable variable = listVariables.at(0);
					listVariables.removeAt( 0 );
					variable.type = type;
					listVariables.prepend( variable );
					m_request = Address;
					if( type.contains( "*" ) )
					{
						messagesToDebugger << "p "+variable.name+"\n";
					}
					else
					{
						messagesToDebugger << "p &"+variable.name+"\n";
					}
				}
			}
			else if ( m_request ==  Address )
			{
				QString address = s.section(" = ", 1).simplified();
				if( !address.isEmpty() )
				{
					Variable variable = listVariables.at(0);
					listVariables.removeAt( 0 );
					variable.address = address;
					listVariables.prepend( variable );
					if( variable.type.section(" ", 0, 0) == "QString" )
					{
						m_request = Length;
						messagesToDebugger << "p "+variable.name+"->d.size\n";
					}
					else
					{
						m_request = Value;
						if( variable.type.section(" ", 0, 0) == "string" )
							messagesToDebugger << "p "+variable.name+".c_str()\n";
						else
							messagesToDebugger << "p "+variable.name+"\n";
					}
				}
			}
			else if ( m_request ==  Length )
			{
				if( s.length() && s.at(0) == '$' )
				{
					int length = s.section(" = ", 1).simplified().toInt();
					Variable variable = listVariables.at(0);
					listVariables.removeAt( 0 );
					if( length > 0 )
					{
						variable.length = qMin(length, 100);
						listVariables.prepend( variable );
						m_request = Value;
						messagesToDebugger << "p "+variable.name+"->d.data[0]\n";
					}
					else
					{
						listVariablesToSend.append( variable );
						if( listVariables.count() )
						{
							Variable variable = listVariables.at(0);
							messagesToDebugger << "whatis "+variable.name+"\n";
							m_request = Whatis;
						}
						else 
						{
							emit debugVariables( listVariablesToSend );
							listVariablesToSend.clear();
							m_request = None;
						}
					}
				}
			}
			else if ( m_request ==  Value )
			{
				s.remove( "(gdb) ");
				if( s.length() && s.at(0) == '$' )
				{
					Variable variable = listVariables.at(0);
					listVariables.removeAt( 0 );
					QString content = s.section("= ", 1);
					if( variable.type.section(" ", 0, 0) == "QString" )
					{
						if( variable.length && variable.content.length() < variable.length )
						{
							variable.content += QChar( content.toInt() );	
							m_request = Value;
							listVariables.prepend( variable );
							messagesToDebugger << "p "+variable.name+"->d.data["+QString::number(variable.content.length())+"]\n";
							continue;
						}
						else
						{
							listVariablesToSend.append( variable );
						}
					}
					else
					{
						variable.content = QString::fromUtf8(content.toLocal8Bit());
						listVariablesToSend.append( variable );
					}
					if( listVariables.count() )
					{
						variable = listVariables.at(0);
						messagesToDebugger << "whatis "+variable.name+"\n";
						m_request = Whatis;
					}
					else
					{
						m_request = None;
						emit debugVariables( listVariablesToSend );
						listVariablesToSend.clear();
						listMessagesDebug.clear();
						messagesToDebugger <<  "info register\n";
						m_request = Registers;
					}
				}
			}
			else
			{
				emit message( QString::fromUtf8(s.toLocal8Bit()) );
			}
		}
	}
	slotMessagesError();
}
void Debug::slotMessagesError()
{
	QStringList list = QString(processDebug->readAllStandardError()).split("\n");
	if( list.count() )
	foreach(QString s, list)
	{
		if( !s.isEmpty() )
		{
			if( m_request != None  )
			{
				if( listVariables.count() )
				{
					Variable variable = listVariables.at(0);
					variable.content = tr("Error: Unable to evaluate value");
					listVariablesToSend.append( variable );
					listVariables.removeAt( 0 );
				}
				if( listVariables.count() )
				{
					Variable variable = listVariables.at(0);
					messagesToDebugger << "whatis "+variable.name+"\n";
					m_request = Whatis;
				}
				else 
				{
					emit debugVariables( listVariablesToSend );
					listVariablesToSend.clear();
						m_request = None;
				}
			}
			else
				emit message( s );
		}
	}
	list.clear();
}
//
void Debug::slotOtherVariables(QStringList list)
{
	m_otherVariables = list;
}
//
void Debug::slotDebugCommand(QString text)
{
	if( text == "info sources\n" )
		m_request = InfoSources;
	messagesToDebugger << text;
}
//
void Debug::slotBreakpoint(QString filename, unsigned int line, BlockUserData *blockUserData)
{
	QString point;
	if( blockUserData->breakpoint )
	{
		if( blockUserData->isTrue )
		{
		    if( blockUserData->breakpointCondition.isEmpty() )
			    point = "break "+filename+":"+QString::number(line)+"\n";
		    else
			    point = "break "+filename+":"+QString::number(line)+" if "+blockUserData->breakpointCondition+"\n";
		}
		else
		{
			point = "watch "+blockUserData->breakpointCondition+"\n";
		}
	}
	else
		point = "clear "+filename+":"+QString::number(line)+"\n";	
	messagesToDebugger << point;
}
//

void Debug::slotStopDebug()
{
	emit endDebug();
#ifdef Q_OS_LINUX
	if( m_pid )
	{
		kill(m_pid, SIGKILL);
		m_pid = 0;
	}
#endif 
	processDebug->kill();
	processDebug->deleteLater();
}
//
void Debug::slotPauseDebug()
{
#ifdef Q_OS_LINUX
	if( m_pid )
	{
		kill(m_pid, SIGINT);
		emit onPause();
	}
#endif
}
//
void Debug::configureGdb()
{
	slotDebugCommand("set pagination off\n");
	slotDebugCommand("set width 0\n");
	slotDebugCommand("set height 0\n");	
	slotDebugCommand("set complaints 0\n");

	//
}


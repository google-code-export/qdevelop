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
#include "assistant.h"
#include "mainimpl.h"
#include <QProcess>
#include <QMessageBox>
#include <QTextStream>
#include <QtNetwork/QTcpSocket>
#include <QLibraryInfo>
#include <QDir>
#include <QUrl>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
Assistant::Assistant()
{
	process = new QProcess( this );
	socket = new QTcpSocket( this );
}
//
Assistant::~Assistant()
{
	if ( process->state() == QProcess::Running )
	{
		process->terminate();
	}
}
//
void Assistant::showQtWord(QString className, QString word)
{
	QString lu;
    QString doc = QDir::cleanPath( m_documentationDirectory ) + "/html/";
	if ( process->state() == QProcess::NotRunning )
	{
		if( m_qVersion.left(5).remove(".").toInt() >= 440 )
			process->start(m_assistantName, QStringList() << "-enableRemoteControl" );
		else
		{
			process->start(m_assistantName, QStringList() << "-server" );
			process->waitForFinished(3000);
			lu = process->readAll();
			if ( lu.isEmpty() )
			{
				QMessageBox::information(0, "QDevelop", QObject::tr("Unable to start Assistant !") );
				return;
			}
			else
			{
				m_port = lu.toUShort();
				socket->connectToHost( "localhost", m_port );
			}
				
		}
	}
	QTextStream os( socket );
    if( !word.isEmpty() )
    {
    	if( className == word )
    	{
			if( m_qVersion.left(5).remove(".").toInt() >= 440 )
	    	{
	    		QTextStream str(process);
	    		str << QString("activateIdentifier "+className) << endl;
    		}
	    	else
	    		os << doc+className.toLower()+".html" << "\n";
    	}
    	else
    	{
			if( m_qVersion.left(5).remove(".").toInt() >= 440 )
	    	{
 	    		if (!className.isEmpty())
	    		{
	    			QTextStream str(process);
	    			str << QString("activateIdentifier "+className+"::"+word) << endl;
    			}
	    		else
	    		{
	    			QTextStream str(process);
	    			str << QString("activateKeyword "+word) << endl;
    			}
    		}
	    	else
	    		os << doc+className.toLower()+".html#"+word << "\n";
    	}
    }
	if( m_qVersion.left(5).remove(".").toInt() < 440 )
		os.flush();
}

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
#include "designer.h"
#include "mainimpl.h"
#include <QProcess>
#include <QMessageBox>
#include <QTextStream>
#include <QtNetwork/QTcpSocket>
#include <QLibraryInfo>
#include <QDir>
#include <QDebug>
//
Designer::Designer()
{
	if( QString(qVersion()).remove(".").toInt() >= 430 )
	{
		process = new QProcess( this );
		socket = new QTcpSocket( this );
	}
}
//
Designer::~Designer()
{
	if( QString(qVersion()).remove(".").toInt() >= 430 )
	{
		if ( process->state() == QProcess::Running )
		{
			process->terminate();
		}
	}
}
//
void Designer::openUI(QString file)
{
	if( QString(qVersion()).remove(".").toInt() >= 430 )
	{
		QString lu;
		if ( process->state() == QProcess::NotRunning )
		{
			process->start(m_designerName, QStringList() << "-server" );
			process->waitForFinished(3000);
			lu = process->readAll();
			if ( lu.isEmpty() )
			{
				QMessageBox::information(0, "QDevelop", QObject::tr("Unable to start Designer !") );
				return;
			}
			else
			{
				m_port = lu.toUShort();
				socket->connectToHost( "localhost", m_port );
			}
		}
		QTextStream os( socket );
	    os << file << "\n";
		os.flush();
	}
	else
	{
        QProcess::startDetached (m_designerName, QStringList(file));
	}
}

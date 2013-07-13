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
#ifndef DESIGNER_H
#define DESIGNER_H
//
#include <QObject>
//
class MainImpl;
class QProcess;
class QTcpSocket;
//
class Designer : public QObject
{
public:
	Designer();
	~Designer();
	void openUI(QString file);
    void setName( QString s ) { m_designerName = s; };
	//
protected:
	//
private:
	QProcess *process;
	QTcpSocket *socket;
	QString m_location;
	quint16 m_port;
    QString m_designerName;
signals:	
protected slots:
public slots:
};
//
#endif 


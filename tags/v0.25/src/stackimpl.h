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
* Contact e-mail: Jean-Luc Biord <jl.biord@free.fr>
* Program URL   : http://qdevelop.org
*
*/

#ifndef STACKIMPL_H
#define STACKIMPL_H
//
#include <QDebug>
#include <QStringList>
//
class MainImpl;
class QListWidget;
class QListWidgetItem;
//
class StackImpl : public QObject
{
Q_OBJECT
public:
	void clear();
	StackImpl( MainImpl * parent, QListWidget *list);
	void addLine( const QString line );
	void setDirectory( QString directory ) { m_directory = directory; };
	void infoSources(const QString s );
private slots:
	void slotCurrentItemChanged ( QListWidgetItem * item, QListWidgetItem * );
private:
	QString m_directory;
	QStringList m_infoSources;
	MainImpl *m_mainImpl;
	QListWidget *m_list;
};
#endif



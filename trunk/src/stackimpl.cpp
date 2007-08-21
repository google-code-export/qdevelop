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

#include "stackimpl.h"
#include "mainimpl.h"
//
#include <QDir>
//
StackImpl::StackImpl( MainImpl * parent, Qt::WFlags f) 
	: QDialog(parent, f), m_mainImpl(parent)
{
	setupUi(this);
}
//
void StackImpl::closeEvent( QCloseEvent * event )
{
	// TODO remove gcc warnings
	event = NULL;
}
//
void StackImpl::addLine( const QString line )
{
	QString s = line;
	s = s.mid( s.indexOf(" ") );
	if( s.contains(" in ") )
		s = line.section(" in ", -1);
	s = s.simplified();
	list->addItem( s );
}
//
void StackImpl::on_list_currentItemChanged ( QListWidgetItem * item, QListWidgetItem * )
{
	if( !item )
		return;
	QString s = item->text().section(" at ", -1);
	QString filename = s.section(":", 0, 0);
	int numLine = s.section(":", -1, -1).toInt();
	QStringList filter = m_infoSources.filter( filename );
	QStringList files;
	foreach(QString f, filter)
	{
		if( !QFile::exists( f ) )
			f = m_directory + "/" + f;
		files << f;
	}
	if( filter.count() )
		m_mainImpl->openFile( files, numLine);
}
//
void StackImpl::infoSources(const QString s )
{
	if( s.indexOf("InfoSources:Source files") == -1)
	{
		foreach(QString text, s.split(",") )
		{
			m_infoSources << text.remove(QString("InfoSources:")).simplified();
			
		}
	}
}

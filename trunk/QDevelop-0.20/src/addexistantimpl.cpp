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
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Jean-Luc Biord <jlbiord@qtfr.org>
* Program URL   : http://qdevelop.org
*
*/
#include "addexistantimpl.h"
#include "projectmanager.h"
#include "misc.h"
#include <QLineEdit>
#include <QFileDialog>
#include <QComboBox>
//
AddExistantImpl::AddExistantImpl(ProjectManager * parent) 
	: QDialog(0), m_projectManager(parent)
{
	setupUi(this); 
	connect(fileButton, SIGNAL(clicked()), this, SLOT(slotFilesChoice()) );
	connect(comboProjects, SIGNAL(activated(QString)), this, SLOT(slotComboProjects(QString)) );
}
//
void AddExistantImpl::slotFilesChoice()
{
	QStringList s = QFileDialog::getOpenFileNames(
		this,
		tr("Choose the file to add"),
		m_projectDirectory,
		tr("Files (*.cpp *.h *.ui *.qrc *.ts)") );
	if( s.isEmpty() )
	{
		// Cancel is clicked
		return;
	}
	QString line;
	foreach(QString file, s)
		line += "\"" + file + "\", ";
	if( line.length() )
		line = line.mid(0, line.length() - 2);
	filename->setText( line );	
}
//
void AddExistantImpl::slotComboProjects(QString projectName)
{
	QVariant variant = comboProjects->itemData( comboProjects->currentIndex() );
	//QTreeWidgetItem *item = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());
	QTreeWidgetItem *item = (QTreeWidgetItem*)variantToItem(variant);
	m_projectDirectory = m_projectManager->projectDirectory( item );
}
//


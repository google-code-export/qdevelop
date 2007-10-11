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
#include "addnewimpl.h"
#include "projectmanager.h"
#include "misc.h"
#include <QLineEdit>
#include <QFileDialog>
#include <QComboBox>
#include <QMessageBox>
//
AddNewImpl::AddNewImpl(ProjectManager * parent) 
	: QDialog(0), m_projectManager(parent)
{
	setupUi(this); 
	suffixe = "cpp";
	connect(okButton, SIGNAL(clicked()), this, SLOT(slotAccept()) );
	connect(directoryButton, SIGNAL(clicked()), this, SLOT(slotDirectoryChoice()) );
	connect(comboProjects, SIGNAL(activated(QString)), this, SLOT(slotComboProjects(QString)) );
	connect(source, SIGNAL(clicked()), this, SLOT(slotFileType()) );
	connect(header, SIGNAL(clicked()), this, SLOT(slotFileType()) );
	connect(dialog, SIGNAL(clicked()), this, SLOT(slotFileType()) );
	connect(resource, SIGNAL(clicked()), this, SLOT(slotFileType()) );
	connect(translation, SIGNAL(clicked()), this, SLOT(slotFileType()) );
}
//
void AddNewImpl::slotDirectoryChoice()
{
	QString s = QFileDialog::getExistingDirectory(
		this,
		tr("Choose the file location"),
		location->text(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	if( s.isEmpty() )
	{
        // Cancel choosen
		return;
	}
	location->setText( s );	
}
//
void AddNewImpl::slotComboProjects(QString /*projectName*/)
{
	QVariant variant = comboProjects->itemData( comboProjects->currentIndex() );
	QTreeWidgetItem *item = (QTreeWidgetItem*)(variantToItem( variant ) );
	m_projectDirectory = m_projectManager->projectDirectory( item );
	location->setText( m_projectDirectory );
}
//
void AddNewImpl::slotFileType()
{
	QVariant variant = comboProjects->itemData( comboProjects->currentIndex() );
	QTreeWidgetItem *item = (QTreeWidgetItem*)(variantToItem( variant ) );
	if( source->isChecked() )
    {
		suffixe = "cpp";
        location->setText( m_projectManager->srcDirectory( item ) );
	}
	else if( header->isChecked() )
    {
		suffixe = "h";
        location->setText( m_projectManager->srcDirectory( item ) );
	}
	else if( dialog->isChecked() )
    {
		suffixe = "ui";
        location->setText( m_projectManager->uiDirectory( item ) );
	}
	else if( resource->isChecked() )
    {
		suffixe = "qrc";
        location->setText( m_projectManager->projectDirectory( item ) );
	}
	else if( translation->isChecked() )
    {
		suffixe = "ts";
        location->setText( m_projectManager->projectDirectory( item ) );
	}
	//
	if( filename->text().lastIndexOf(".") > 0 )
	{
		QString prefixe = filename->text().left( filename->text().lastIndexOf(".") );
		filename->setText( prefixe + "." + suffixe); 
	}
	else if( filename->text().length() )
		filename->setText( filename->text() + "." + suffixe); 
}
//
void AddNewImpl::slotAccept()
{
	QString suffixeSaisi;
	if( filename->text().lastIndexOf(".") > 0 )
	{
		suffixeSaisi = filename->text().mid( filename->text().lastIndexOf(".")+1 );
	}
	if( suffixeSaisi != suffixe )
	{
		QMessageBox::warning(this, 
			"QDevelop", tr("The suffix of the file is not correct."),
			tr("Cancel") );
		return;
	}
	if( suffixe == "cpp" || suffixe == "c" || suffixe == "h" || suffixe == "hpp" )
	{
		QString type = tr("header");
		QString autreSuffixe = "h";
		if( suffixe == "h" || suffixe == "hpp")
		{
			type = "source";
			autreSuffixe = "cpp";
		}
		int rep = QMessageBox::question(0, "QDevelop", 
			tr("Also create the file %1 in same location ?").arg(type), tr("Yes"), tr("No"), tr("Cancel"), 0, 2 );
		if( rep == 2 )
			return;
		if( rep == 0 )
		{
			QString nomFic = filename->text();
			QString nomAutre = nomFic.left( nomFic.lastIndexOf(".") ) + "." + autreSuffixe;
			filename->setText( "\"" + nomFic + "\", " + "\"" + nomAutre + "\"");	
		}
	}
	else
		filename->setText( "\"" + filename->text() + "\"");	
	accept();
}
//

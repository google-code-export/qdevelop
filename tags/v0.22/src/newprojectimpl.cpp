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
* Contact e-mail: Jean-Luc Biord <jlbiord@qtfr.org>
* Program URL   : http://qdevelop.org
*
*/
#include "newprojectimpl.h"
#include "projectmanager.h"
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
//
NewProjectImpl::NewProjectImpl(QWidget * parent, QString s) 
	: QDialog(0), m_projectLocation(s)
{
	setupUi(this); 
	connect(locationButton, SIGNAL(clicked()), this, SLOT(slotChooseDirectory()) );
	connect(projectName, SIGNAL(textChanged(QString)), this, SLOT(slotLabel()) );
	connect(location, SIGNAL(textChanged(QString)), this, SLOT(slotLabel()) );
	location->setText( s );
	
	// TODO remove gcc warnings
	parent = NULL;
}
//
void NewProjectImpl::slotChooseDirectory()
{
	QString s = QFileDialog::getExistingDirectory(
		this,
		tr("Choose the project directory"),
		m_projectLocation,
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	if( s.isEmpty() )
	{
		// Cancel clicked
		return;
	}
	location->setText( s );
}
//
void NewProjectImpl::slotLabel()
{
	label->setText( label->text().section(":", 0, 0) + ": "	+ location->text() + "/" + projectName->text() );
	if( projectName->text().isEmpty() || location->text().isEmpty() )
	{
		okButton->setEnabled( false );
		label->setText( label->text().section(":", 0, 0) + ": "	);
	}
	else
	{
		okButton->setEnabled( true );
		label->setText( label->text().section(":", 0, 0) + ": "	+ location->text() + "/" + projectName->text() );
	}
}

void NewProjectImpl::on_dialog_clicked(bool checked)
{
	if (!checked)
		return;
	
	uiFilename->setText( "dialog" );
	subclassFilename->setText( "dialogimpl" );
	uiObjectName->setText( "Dialog" );
	subclassObjectName->setText( "DialogImpl" );
}

void NewProjectImpl::on_mainwindow_clicked(bool checked)
{
	if (!checked)
		return;

	uiFilename->setText( "mainwindow" );
	subclassFilename->setText( "mainwindowimpl" );
	uiObjectName->setText( "MainWindow" );
	subclassObjectName->setText( "MainWindowImpl" );
}

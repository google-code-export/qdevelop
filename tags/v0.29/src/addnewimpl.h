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
#ifndef ADDNEW_H
#define ADDNEW_H

#include "ui_addnew.h"
#include <QDialog>
//
class ProjectManager;
//
class AddNewImpl : public QDialog, public Ui::AddNew
{
Q_OBJECT
public:
	AddNewImpl(ProjectManager * parent = 0);
public slots:
	void slotComboProjects(QString projectName);
	void slotFileType();
private:
	ProjectManager *m_projectManager;
	QString m_projectDirectory;
	QString suffixe;
private slots:
	void slotAccept();
	void slotDirectoryChoice();
};

#endif

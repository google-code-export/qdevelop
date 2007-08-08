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
#ifndef NEWPROJECTIMPL_H
#define NEWPROJECTIMPL_H

#include "ui_newproject.h"
#include <QDialog>
//
class ProjectManager;
class MainImpl;
//
class NewProjectImpl : public QDialog, public Ui::NewProject
{
Q_OBJECT
public:
	NewProjectImpl(QWidget * parent, QString s);
	QString absoluteProjectName() {
		return m_absoluteProjectName;
	}
	QString filename() {
		return m_filename;
	}
public slots:
private:
    QString m_projectLocation;
    QString m_absoluteProjectName;
    QString m_filename;
	MainImpl *m_mainImpl;
private slots:
	void on_okButton_clicked();
	void on_dialog_clicked(bool checked);
	void on_mainwindow_clicked(bool checked);
	void slotChooseDirectory();
	void slotLabel();
};

#endif


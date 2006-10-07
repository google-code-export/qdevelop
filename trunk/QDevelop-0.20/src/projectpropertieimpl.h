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
* Program URL   : http://qtfr.org
*
*/
#ifndef PROJECTPROPERTIE_IMPL
#define PROJECTPROPERTIE_IMPL

#include "ui_projectpropertie.h"
#include "ui_newvariable.h"
#include <QDialog>
#include <QTreeWidgetItem>
//
class ProjectManager;
//
class ProjectPropertieImpl : public QDialog, public Ui::ProjectPropertie
{
Q_OBJECT
public:
	ProjectPropertieImpl(ProjectManager * parent, QTreeWidget *tree,QTreeWidgetItem *itProject);
public slots:
private:
	ProjectManager *m_projectManager;
	QTreeWidgetItem *m_itProject;
	QTreeWidgetItem *m_copyItProject;
	QTreeWidget *m_treeFiles;
	QTreeWidget *m_copyTreeFiles;
	//
	void parse(QTreeWidgetItem *it);
	void parseTemplate(QTreeWidgetItem *it);
	void parseConfig(QTreeWidgetItem *it);
	void parseQT(QTreeWidgetItem *it);
	void populateComboScope();
	void connections();
	void unconnections();
	void clearFields();
	QTreeWidgetItem *subItTemplate(QTreeWidgetItem *it);
	QTreeWidgetItem *subItQT(QTreeWidgetItem *it);
	QTreeWidgetItem *subItConfig(QTreeWidgetItem *it);
	void makeComboVariables( QComboBox *combo );
private slots:
	void slotSubdirs(bool);
	void slotCheck(bool);
	void slotAccept();
	void slotComboScope(int index);
	void slotCurrentItemChanged ( QListWidgetItem * current, QListWidgetItem * previous );
	void slotAddVariable();
	void slotDeleteVariable();
	void slotAddValue();
	void slotDeleteValue();
	void slotModifyValue();
	void copyTreeWidget(QTreeWidgetItem *source, QTreeWidgetItem *dest);
};

#endif

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

#ifndef ADDNEWCLASSVARIABLEIMPL_H
#define ADDNEWCLASSVARIABLEIMPL_H
//
#include "ui_addnewclassvariable.h"
//
class MainImpl;
class TreeClasses;
class QTreeWidgetItem;
class Editor;
//
class AddNewClassVariableImpl : public QDialog, public Ui::AddNewClassVariable
{
Q_OBJECT
public:
	AddNewClassVariableImpl( MainImpl * parent, TreeClasses *treeClasses, QTreeWidgetItem *treeWidget, QString declaration, QString implementation, QString classname);
	bool addGet() { return get->isChecked(); }
	QString addGetName() { return getName->text(); }
	bool addGetInline() { return getInline->isChecked(); }
	bool addSet() { return set->isChecked(); }
	QString addSetName() { return setName->text(); }
	bool addSetInline() { return setInline->isChecked(); }
private slots:
	void on_variableName_textChanged(QString );
	void on_okButton_clicked();
private:
	MainImpl *m_mainImpl;
	TreeClasses *m_treeClasses;
	QTreeWidgetItem *m_treeWidget;
	QString m_declaration;
	QString m_implementation;
	QString m_classname;
	void insertInDeclaration(QString scope, QString insertedText);
	void insertInImplementation(QString insertedText);
};
#endif








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
#ifndef SUBCLASSINGIMPL_H
#define SUBCLASSINGIMPL_H

#include "ui_subclassing.h"
#include "ui_newimplementation.h"

class ProjectManager;
class QSortFilterProxyModel;
class QTreeWidget;
class MainImpl;

class SubclassingImpl : public QDialog, public Ui::Subclassing
{
Q_OBJECT
public:
	SubclassingImpl(ProjectManager * parent, MainImpl *mainImpl, QString srcDirectory, QString uiName, QStringList headers);
	QString newFile();
	inline bool isValid() {
		return !objectName().isEmpty();
	}
private:	
	void		implementations(QStringList);
	static		QStringList signatures(QString header);
	QString		objectName();
	QString		className();
	QStringList	templateHeaderImpl();
	QStringList	templateSourceImpl();
	
private slots:
	void		on_okButton_clicked();
	void		on_clearButton_clicked();
	void		on_newImplementation_clicked();
	void		on_comboClassName_activated(int i);
	void		on_filterEdit_textChanged(const QString &text);
	
	void		slotParseForm();
	void		slotLocation();
	void		slotEnableokButton(QString);
	
private:
	ProjectManager		*m_parent;
	MainImpl 			*m_mainImpl;
	QSortFilterProxyModel	*proxyModel;
	QString			m_uiName;
	QString			m_srcDirectory;
	QTreeWidget		*treeSlots;
	Ui::NewImplementation	uiNewImplementation;	
	
};

#endif

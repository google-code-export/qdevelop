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
#ifndef TREECLASSES_H
#define TREECLASSES_H

#include <QTreeWidget>
#include <QPointer>
#include <QFile>
#include <QMap>
#include <QSqlDatabase>

//
class QProcess;
//
typedef struct  
{
	QString key;
	QStringList parents;
	QString name;
	QString implementation;
	QString declaration;
	QString ex_cmd;
	QString language;
	QString classname;
	QString structname;
	QString enumname;
	QString access;
	QString signature;
	QString kind;
	bool markedForDelete;
	QString icon;
} ParsedItem;
Q_DECLARE_METATYPE(ParsedItem)
//
typedef struct 
{
	QStringList parents;
	QString filename;
	int numTempFile;
	QString ext;
} TempProcess;
//
class ProjectManager;
class MainImpl;
//
class TreeClasses : public QTreeWidget
{
Q_OBJECT
public:
	TreeClasses(QWidget * parent);
	~TreeClasses();
	void setProjectManager( QPointer<ProjectManager> g ) { m_projectManager = g; };
	void setCtagsIsPresent( bool b ) { m_ctagsPresent = b; };
	void setMainImpl(MainImpl *m) { m_mainImpl=m; };
	void clear();
	void toDB(QString projectDirectory);
	void fromDB(QString projectDirectory);
	void setCtagsName(QString s) { m_ctagsName=s;};
	const QList<ParsedItem> *treeClassesItems() { return m_treeClassesItems; };
	//const QList<ParsedItem> *treeClassesItems(QString classname);
	QString signature(QString line);
    QStringList methods(QString filename, QString classname);
protected:
	void mousePressEvent( QMouseEvent * event );
	void mouseDoubleClickEvent ( QMouseEvent * event );
private:
	QList<ParsedItem> *m_treeClassesItems;
	QTreeWidgetItem *m_itemClicked;
	QList<QTreeWidgetItem *> m_listDeletion;
	QTreeWidgetItem *findItem(const QTreeWidgetItem *begin, const QString text, const QString key, const bool recursive);
	QTreeWidgetItem *findAndCreate(QTreeWidgetItem *begin, QString pixname, QString text, QString key, bool recursive, bool update, ParsedItem parsedItem);
	bool m_ctagsPresent;
	QMap<QProcess *,TempProcess> tempProcessMap;
	MainImpl *m_mainImpl;
	QPointer<ProjectManager> m_projectManager;
	void markForDeletion(QTreeWidgetItem *current, QString filename, QString ext, QStringList parents);
	void deleteMarked(QTreeWidgetItem *current);
	QString markForSorting(QString kind, QString text);
	void setSortingSymbols( QTreeWidgetItem *it, bool active, QString filename, QString ext, QStringList parents);
	void setTooltip(QTreeWidgetItem *item, ParsedItem parsedItem);
	void writeItemsInDB(const QTreeWidgetItem *it, QString parents, QSqlQuery query, QString projectDirectory);
	void createItemFromDB(QTreeWidgetItem *parent, QString text, QString tooltip, QString parents, ParsedItem parsedItem);
	QString getPathHash(QString const& pathName);
	QString m_ctagsName;
private slots:
	void slotParseCtags();
	void slotOpenImplementation();
	void slotOpenDeclaration();
	void slotRefresh();
	void parse(ParsedItem parsedItem);
    void slotAddNewClassMethod();
    void slotAddNewClassVariable();
    void slotAddGetSetMethod();
public slots:
	void updateClasses( QString filename, QString buffer, QStringList parents, QString ext);
signals:
};

#endif

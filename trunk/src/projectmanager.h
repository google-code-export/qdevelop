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
#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QMap>
#include <QPair>
#include <QMultiMap>
#include <QStringList>
#include <QVariantList>
#include <QTextStream>
#include <QListWidget>

//
typedef struct Parameters
{
	QString arguments;
	QString workingDirectory;
	QStringList env;
	bool isEmpty;
};
//
class TreeProject;
class TreeClasses;
class QTreeWidget;
class QTreeWidgetItem;
class MainImpl;
//
//
class ProjectManager : public QObject
{
Q_OBJECT
public:
	ProjectManager(MainImpl * parent, TreeProject *treeFiles, TreeClasses *treeClasses, QString name);
	~ProjectManager();
	QString absoluteNameProjectFile(QTreeWidgetItem *it);
	QString projectFilename(QTreeWidgetItem *it);
	QString projectDirectory(QTreeWidgetItem *it);
	QString projectDirectory(QString projectName);
	QString fileDirectory(QString absoluteFilename);
	QString projectName(QString directoryName);
    void setUiDirectory(QTreeWidgetItem *it, QString s);
    QString uiDirectory(QTreeWidgetItem *it);
    void setSrcDirectory(QTreeWidgetItem *it, QString s);
    QString srcDirectory(QTreeWidgetItem *it);
	QStringList buildableProjectsDirectories();
	QStringList dependpath(QTreeWidgetItem *it);
	void headers(QTreeWidgetItem *it, QStringList &headerFiles);
	void sources(QTreeWidgetItem *it, QStringList &sourcesFiles);
	bool qmake(QString projectName=QString());
	bool isModifiedProject() { return m_isModifiedProject; };
	QString executableName(QString preferedVersion);
	bool isReleaseVersion() { return m_releaseVersion; };
	void saveProjectSettings();
	bool close();
	void childsList(QTreeWidgetItem *begin, QString key, QList<QTreeWidgetItem *> &list);
	Parameters parameters();
	void setParameters(Parameters p);
	QString projectDirectoryOfExecutable() { return m_projectDirectoryOfExecutable;};
	QString findData(QString projectName, QString key);
	QTreeWidgetItem *itemProject(QString projectName);
	QStringList parents(QTreeWidgetItem *it);
	void parseTreeClasses(bool force=false );
	void setQmake(QString projectName);
	void insertFile(QTreeWidgetItem *it, QString filename, bool silentMode=false);
private:
	enum findMode { Key, Data };
	// Methods
	void loadProject(QString s, QTreeWidgetItem *newProjectItem);
	QString findExecutable( QString projectDirectory, QString preferedVersion );
	void parseFile(QString file);
	QString projectVersion(QTreeWidgetItem *it);
	QTreeWidgetItem *item(QTreeWidgetItem *begin, QString name, findMode type);
	bool saveDataOfProject(QTreeWidgetItem *item, QTextStream *s, int nbSpace=-1, bool aSuivre=false);
	QTreeWidgetItem * insertItem(QTreeWidgetItem *parent, QString key, QString data);
	bool listContains(QList<QTreeWidgetItem *>, QString name, findMode type);
	void loadProjectSettings();
	bool m_isModifiedProject;
	// Variables
	MainImpl *m_parent;
	TreeProject *m_treeFiles;
	TreeClasses *m_treeClasses;
	QString m_absoluteNameProjectFile;
	QString m_projectDirectoryOfExecutable;
	QStringList m_classes;
	QStringList m_executablesList;
	QStringList m_projectDirectoryOfExecutableList;
	QWidget *m_previewForm;
    Parameters m_parameters;
    bool m_releaseVersion;
protected:
	bool eventFilter( QObject *obj, QEvent *ev );
signals:
public slots:
	void slotResetExecutablesList() { m_executablesList.clear(); };
	bool slotSaveProject();
	void slotAddExistingFiles(QTreeWidgetItem *it=0);
	void slotAddNewItem(QTreeWidgetItem *it=0);
	void slotAddNewClass(QTreeWidgetItem *it=0);
	void slotAddScope(QTreeWidgetItem *it=0);
	void slotAddSubProject(QTreeWidgetItem *it=0);
	void slotlupdate(QTreeWidgetItem *);
	void slotlrelease(QTreeWidgetItem *);
	void slotDeleteItem(QTreeWidgetItem *it=0, bool silentMode=false);
	void slotRenameItem(QTreeWidgetItem *it=0);
	void slotProjectPropertie(QTreeWidgetItem *it=0);
	void slotSubclassing(QTreeWidgetItem *it=0);	
	void slotPreviewForm(QTreeWidgetItem *it=0);
	void slotSort();
};

#endif

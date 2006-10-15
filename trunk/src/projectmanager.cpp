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
#include "mainimpl.h"
#include "projectmanager.h"
#include "ui_exechoice.h"
#include "addexistantimpl.h"
#include "addnewimpl.h"
#include "addscopeimpl.h"
#include "misc.h"
#include "newprojectimpl.h"
#include "projectpropertieimpl.h"
#include "subclassingimpl.h"
#include "parametersimpl.h"
#include "tabwidget.h"
#include "editor.h"
//
#include <QTreeWidget>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QMessageBox>
#include <QProcess>
#include <QInputDialog>
#include <QUiLoader>
#include <QScrollArea>
#include <QDesktopWidget>
#include <QMetaMethod>
#include <QSqlQuery>
#include <QSqlError>
//
ProjectManager::ProjectManager(MainImpl * parent, TreeProject *treeFiles, TreeClasses *treeClasses, QString name)
	: m_parent(parent), m_treeFiles(treeFiles), m_treeClasses(treeClasses)
{
	m_treeFiles->clear();
	m_treeClasses->clear();
	//
	QTreeWidgetItem *newProjectItem = new QTreeWidgetItem(m_treeFiles);
	//
	//
	connect(m_treeFiles, SIGNAL(addNewItem(QTreeWidgetItem *)), this, SLOT(slotAddNewItem(QTreeWidgetItem *)));
	connect(m_treeFiles, SIGNAL(addExistingsFiles(QTreeWidgetItem *)), this, SLOT(slotAddExistingFiles(QTreeWidgetItem *)));	
	connect(m_treeFiles, SIGNAL(addScope(QTreeWidgetItem *)), this, SLOT(slotAddScope(QTreeWidgetItem *)));	
	connect(m_treeFiles, SIGNAL(lupdate(QTreeWidgetItem *)), this, SLOT(slotlupdate(QTreeWidgetItem *)));	
	connect(m_treeFiles, SIGNAL(lrelease(QTreeWidgetItem *)), this, SLOT(slotlrelease(QTreeWidgetItem *)));	
	connect(m_treeFiles, SIGNAL(addSubProject(QTreeWidgetItem *)), this, SLOT(slotAddSubProject(QTreeWidgetItem *)));
	connect(m_treeFiles, SIGNAL(projectPropertie(QTreeWidgetItem *)), this, SLOT(slotProjectPropertie(QTreeWidgetItem *)));
	connect(m_treeFiles, SIGNAL(open(QTreeWidgetItem *, int)), m_parent, SLOT(slotDoubleClickTreeFiles(QTreeWidgetItem *, int)));
	connect(m_treeFiles, SIGNAL(deleteItem(QTreeWidgetItem *)), this, SLOT(slotDeleteItem(QTreeWidgetItem *)));
	connect(m_treeFiles, SIGNAL(subclassing(QTreeWidgetItem *)), this, SLOT(slotSubclassing(QTreeWidgetItem *)));
	connect(m_treeFiles, SIGNAL(previewForm(QTreeWidgetItem *)), this, SLOT(slotPreviewForm(QTreeWidgetItem *)));
	connect(m_treeFiles, SIGNAL(sort()), this, SLOT(slotSort()));
	m_isModifiedProject = false;
	m_previewForm = 0;
	m_parameters.isEmpty = true;
	loadProject(name, newProjectItem);
	//QString projectDirectory = absoluteNameProjectFile(m_treeFiles->topLevelItem(0));
	//TreeClasses::connectDB( projectDirectory + "/project.db" );
	parseTreeClasses();
	loadProjectSettings();
}
//
ProjectManager::~ProjectManager()
{
	QString projectDirectory = absoluteNameProjectFile(m_treeFiles->topLevelItem(0));
	if( m_treeFiles->topLevelItem ( 0 ) )
		m_treeClasses->toDB( projectDirectory );
	m_treeFiles->clear();
	m_treeClasses->clear();
	if( m_previewForm )
		delete m_previewForm;
	QSqlDatabase::removeDatabase ( "QSQLITE" );
}
//
QStringList ProjectManager::parents(QTreeWidgetItem *it)
{
	QStringList parentsList;
	if( !it )
		return QStringList();
	QTreeWidgetItem *parent = it;
	if( it->data(0, Qt::UserRole).toString() == "PROJECT" || it->data(0, Qt::UserRole).toString() == "SCOPE" )
		parentsList.prepend( it->text( 0 ) );
	//	
	while( (parent = parent->parent()) )
	{
		if( parent->data(0, Qt::UserRole).toString() == "PROJECT" || parent->data(0, Qt::UserRole).toString() == "SCOPE" )
			parentsList.prepend( parent->text( 0 ) );
	}
	return parentsList;
}
//
void ProjectManager::parseTreeClasses(bool force)
{
	if( !m_parent->showTreeClasses() )
		return;
	m_treeClasses->clear();
	QString directory = projectDirectory( m_treeFiles->topLevelItem( 0 ) );
	if( QFile::exists( directory+"/project.db" ) && !force)
	{
		m_treeClasses->fromDB( directory );
	}
	else
	{
	 	//
		QList<QTreeWidgetItem *> projectsList;
		childsList(0, "PROJECT", projectsList);
		for(int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
		{
			QString projectName = projectsList.at(nbProjects)->text(0);
			QString projectDir = findData(projectName, "projectDirectory");
			QStringList files;
			sources(projectsList.at(nbProjects), files );
			headers(projectsList.at(nbProjects), files );
			files.sort();
			foreach(QString s, files)
			{
				QStringList parentsList = parents(projectsList.at(nbProjects));
				QFile file(s);
			    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			        continue;
				QString buffer = file.readAll();
				file.close();
				m_treeClasses->updateClasses(s, buffer, parentsList, "."+s.section(".", -1, -1));
			}
		}
	}
}
//
Parameters ProjectManager::parameters() 
{ 
	if( m_parameters.isEmpty )
	{
		ParametersImpl *parametersimpl = new ParametersImpl(0);
		m_parameters = parametersimpl->parameters();
		delete parametersimpl;		
	}
	m_parameters.isEmpty = false;
	return m_parameters; 
}
//
void ProjectManager::setParameters(Parameters p) 
{ 
	m_parameters = p;
	m_parameters.isEmpty = false;
}
//
QStringList ProjectManager::dependpath(QTreeWidgetItem *it)
{
	if( !it )
		return QStringList();
	while( it->data(0, Qt::UserRole).toString() != "PROJECT" )
		it = it->parent();
	QTreeWidgetItem *itDepend = 0;
	for(int i=0; i<it->childCount(); i++)
	{
		if( it->child(i)->data(0, Qt::UserRole).toString() == "DEPENDPATH" )
		{
			itDepend = it->child(i);
			break;
		}
	}
	if( !itDepend )
		return QStringList();
	QStringList path;
	for(int i=0; i<itDepend->childCount(); i++)
	{
		path << itDepend->child( i )->text(0);
	}
	return path;
}
//
bool ProjectManager::close()
{
	QTreeWidgetItem *it = item(m_treeFiles->topLevelItem ( 0 ), "PROJECT", Key);
	if( !it )
		return true;
	QString projectName = it->text(0);
	if( isModifiedProject() )
	{
		// Save?
		int rep = QMessageBox::question(0, "QDevelop", 
			tr("Save project changes ?"), tr("Yes"), tr("No"), tr("Cancel"), 0, 2 );
		if( rep == 2 )
			return false;
		if( rep == 0 )
		{
			slotSaveProject();
		}
	}
	return true;
}
//
void ProjectManager::saveProjectSettings()
{
	// Save opened files
	QString directory = projectDirectory(m_treeFiles->topLevelItem(0));
	if( !connectDB( directory + "/project.db" ) )
		return;
	QSqlQuery query;
	QString queryString = "delete from editors where 1";
    if (!query.exec(queryString))
    {
		qDebug() << "Failed to execute" << queryString;
    	return;
   	}
	for(int i=0; i<m_parent->tabEditors()->count(); i++)
	{
		Editor *editor = ((Editor *)m_parent->tabEditors()->widget( i ));
		if( editor )
		{
			QSqlQuery query;
			query.prepare("INSERT INTO editors (filename, scrollbar, numline) "
			           "VALUES (:filename, :scrollbar, :numline)");
			query.bindValue(":filename", editor->filename());
			query.bindValue(":scrollbar", editor->verticalScrollBar());
			query.bindValue(":numline", editor->currentLineNumber());
			if( !query.exec() )
				qDebug() << query.lastError();
		}
	}
	if( m_parent->tabEditors()->count() )
	{
		//settings.setValue("currentIndex", m_tabEditors->currentIndex());
	    QSqlQuery query;
	    query.prepare("update config set currentEditor = ? where 1");
	    query.addBindValue(m_parent->tabEditors()->currentIndex());
		query.exec();
		if( !query.exec() )
			qDebug() << query.lastError();
		
	}
	//db.close();
}
//
void ProjectManager::loadProjectSettings()
{
	// Save opened files
	QString directory = projectDirectory(m_treeFiles->topLevelItem(0));
	if( !connectDB( directory + "/project.db" ) )
		return;
	QSqlQuery query;
	query.prepare("select * from editors where 1");
    query.exec();
    while (query.next())
    {
    	QString filename = query.value(0).toString();
    	int scrollbar = query.value(1).toInt();
    	int numline = query.value(2).toInt();
		m_parent->openFile( QStringList( filename ) );
		if( m_parent->tabEditors()->count() && numline )
		{
			((Editor *)m_parent->tabEditors()->widget( m_parent->tabEditors()->count()-1 ))->setVerticalScrollBar( scrollbar );
			((Editor *)m_parent->tabEditors()->widget( m_parent->tabEditors()->count()-1 ))->gotoLine( numline,false );
		}
    }
	query.prepare("select * from config where 1");
    query.exec();
    while (query.next())
    {
    	int currentEditor = query.value( 0 ).toInt();
qDebug()<<"currentEditor"<<currentEditor;
		m_parent->tabEditors()->setCurrentIndex( currentEditor );
    }
    //db.close();
}
//
void ProjectManager::slotAddExistingFiles(QTreeWidgetItem *it)
{
	if( !it )
		it = m_treeFiles->currentItem();
	if( !it )
		it = m_treeFiles->topLevelItem( 0 );
	QString projectName;
	QString projectDir;
	QString plateforme;
	QStringList filesList;
	AddExistantImpl *window = new AddExistantImpl(this);
	
	QList<QTreeWidgetItem *> projectsList;
	childsList(0, "PROJECT", projectsList);
	for(int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
	{
		projectName = projectsList.at(nbProjects)->text(0);
		projectDir = findData(projectName, "projectDirectory");
		QList<QTreeWidgetItem *> listeTemplate;
		bool ajouter = true;
		for(int enfant=0; enfant<projectsList.at(nbProjects)->childCount(); enfant++)
		{
			ajouter = true;
			if( projectsList.at(nbProjects)->child(enfant)->data(0,Qt::UserRole).toString() == "TEMPLATE")
			{
				QTreeWidgetItem *itTemplate = projectsList.at(nbProjects)->child(enfant);
				for(int nbTemplate=0; nbTemplate < itTemplate->childCount(); nbTemplate++)
				{
					if( itTemplate->child(nbTemplate)->text(0) == "subdirs" )
					{
						ajouter = false;
						break;
					}
				}
			}
		}
		if( ajouter )
		{
			//window->comboProjects->addItem( projectName,  QVariant(reinterpret_cast<uint>(projectsList.at(nbProjects))));
			window->comboProjects->addItem( projectName,  addressToVariant( projectsList.at(nbProjects) ) );
			if( projectsList.at(nbProjects) == it )
				window->comboProjects->setCurrentIndex( window->comboProjects->count()-1);
			QList<QTreeWidgetItem *> listeScope;
			childsList(projectsList.at(nbProjects), "SCOPE", listeScope);
			for(int nbScope=0; nbScope < listeScope.count(); nbScope++)
			{
				QString nomScope;
				QTreeWidgetItem *tmp = listeScope.at(nbScope);
				int nbSpace = 0;
				while( tmp )
				{
					QString cleTmp = tmp->data(0,Qt::UserRole).toString();
					QString indent;
					for(int i=0; i<nbSpace; i++)
						indent += "  ";
					if( cleTmp == "SCOPE" || cleTmp == "PROJECT" )
						nomScope = indent + tmp->text(0) + ":" + nomScope.simplified();
					if( cleTmp == "PROJECT" )
						break;
					tmp = tmp->parent();
					nbSpace++;
				}
				//window->comboProjects->addItem( nomScope, QVariant(reinterpret_cast<uint>(listeScope.at(nbScope))));
				window->comboProjects->addItem( nomScope, addressToVariant(listeScope.at(nbScope)));
				if( listeScope.at(nbScope) == it )
					window->comboProjects->setCurrentIndex( window->comboProjects->count()-1);
			}
		}
	}
	//
	if( window->comboProjects->count() == 1 )
		window->comboProjects->setEnabled( false );
	window->slotComboProjects( window->comboProjects->currentText() );
	QVariant variant;
	if( window->exec() == QDialog::Accepted )
	{
		projectName = window->comboProjects->currentText();
		QString line = window->filename->text();
		filesList = line.split(",");
		variant = window->comboProjects->itemData( window->comboProjects->currentIndex() );
		delete window;
	}
	else
	{
		delete window;
		return;
	}
	//QTreeWidgetItem *item = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());
	QTreeWidgetItem *item = (QTreeWidgetItem*)variantToItem(variant);
	projectDir = projectDirectory( item );
	setQmake( projectFilename(item) );
	foreach(QString filename, filesList)
	{
		filename = filename.remove("\"").simplified();
		filename = QDir(projectDir).relativeFilePath(filename).replace("\\", "/");
		insertFile(item, filename);
	}
}
//
void ProjectManager::slotAddNewItem(QTreeWidgetItem *it)
{
	if( !it )
		it = m_treeFiles->currentItem();
	if( !it )
		it = m_treeFiles->topLevelItem( 0 );
	QString projectName;
	QString projectDir;
	QString absoluteFilename;
	QString plateforme;
	QStringList filesList;
	AddNewImpl *window = new AddNewImpl(this);
	
	QList<QTreeWidgetItem *> projectsList;
	childsList(0, "PROJECT", projectsList);
	for(int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
	{
		projectName = projectsList.at(nbProjects)->text(0);
		projectDir = findData(projectName, "projectDirectory");
		QList<QTreeWidgetItem *> listeTemplate;
		bool ajouter = true;
		for(int enfant=0; enfant<projectsList.at(nbProjects)->childCount(); enfant++)
		{
			ajouter = true;
			if( projectsList.at(nbProjects)->child(enfant)->data(0,Qt::UserRole).toString() == "TEMPLATE")
			{
				QTreeWidgetItem *itTemplate = projectsList.at(nbProjects)->child(enfant);
				for(int nbTemplate=0; nbTemplate < itTemplate->childCount(); nbTemplate++)
				{
					if( itTemplate->child(nbTemplate)->text(0) == "subdirs" )
					{
						ajouter = false;
						break;
					}
				}
			}
		}
		if( ajouter )
		{
			window->comboProjects->addItem( projectName,  addressToVariant(projectsList.at(nbProjects)));
			if( projectsList.at(nbProjects) == it )
				window->comboProjects->setCurrentIndex( window->comboProjects->count()-1);
			QList<QTreeWidgetItem *> listeScope;
			childsList(projectsList.at(nbProjects), "SCOPE", listeScope);
			for(int nbScope=0; nbScope < listeScope.count(); nbScope++)
			{
				QString nomScope;
				QTreeWidgetItem *tmp = listeScope.at(nbScope);
				int nbSpace = 0;
				while( tmp )
				{
					QString cleTmp = tmp->data(0,Qt::UserRole).toString();
					QString indent;
					for(int i=0; i<nbSpace; i++)
						indent += "  ";
					if( cleTmp == "SCOPE" || cleTmp == "PROJECT" )
						nomScope = indent + tmp->text(0) + ":" + nomScope.simplified();
					if( cleTmp == "PROJECT" )
						break;
					tmp = tmp->parent();
					nbSpace++;
				}
				//window->comboProjects->addItem( nomScope, QVariant(reinterpret_cast<uint>(listeScope.at(nbScope))));
				window->comboProjects->addItem( nomScope, addressToVariant(listeScope.at(nbScope)));
				if( listeScope.at(nbScope) == it )
					window->comboProjects->setCurrentIndex( window->comboProjects->count()-1);
			}
		}
	}
	//
	if( window->comboProjects->count() == 1 )
		window->comboProjects->setEnabled( false );
	window->slotComboProjects( window->comboProjects->currentText() );
	QVariant variant;
	QString filename;
	QString repCreation;
	if( window->exec() == QDialog::Accepted )
	{
		QString line = window->filename->text();
		filesList = line.split(",");
		repCreation = window->location->text();
		variant = window->comboProjects->itemData( window->comboProjects->currentIndex() );
		delete window;
	}
	else
	{
		delete window;
		return;
	}
	//QTreeWidgetItem *item = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());	
	QTreeWidgetItem *item = (QTreeWidgetItem*)variantToItem(variant);
	projectDir = projectDirectory( item );
	setQmake( projectFilename( item ) );
	foreach(QString filename, filesList)
	{
		absoluteFilename = repCreation+"/"+filename.remove("\"").simplified();
		filename = QDir(projectDir).relativeFilePath(absoluteFilename).replace("\\", "/");
		QFile file ( absoluteFilename );
		if( file.exists() )
		{
			QMessageBox::warning(0, 
				"QDevelop", tr("The file already exist on directory."),
				tr("Cancel") );
			return;
		}
		QByteArray templateData;;
		if( absoluteFilename.section(".", -1, -1) == "ui" )
		{
			QStringList items;
			items << "QDialog" << "QMainWindow" << "QWidget";
			bool ok;
			QString item = QInputDialog::getItem(0, "QDevelop", tr("Forms:"), items, 0, false, &ok);
			if (ok && !item.isEmpty())
			{
				QString templateName;
				if( item == "QDialog" )
					templateName = ":/templates/templates/dialog.ui";
				else if( item == "QMainWindow" )
					templateName = ":/templates/templates/mainwindow.ui";
				else
					templateName = ":/templates/templates/widget.ui";
				QFile templateFile(templateName);
				templateFile.open(QIODevice::ReadOnly);
				templateData = templateFile.readAll();
				templateFile.close();
			}
			else
				return;
		}
		if( !file.open(QIODevice::WriteOnly | QIODevice::Text) )
		{
			QMessageBox::warning(0, 
				"QDevelop", tr("Unable to create file."),
				tr("Cancel") );
			return;
		}
		if( absoluteFilename.section(".", -1, -1) == "ui" )
		{
			file.write( templateData );
		}
		file.close();
		insertFile(item, filename);
	}
	m_isModifiedProject = true;
}
//
bool ProjectManager::listContains(QList<QTreeWidgetItem *>list, QString name, findMode type)
{
	for(int i=0; i < list.count(); i++)
	{
		if( (type==Data && list.at( i )->text(0)==name) )
			return true;
		else if (type==Key && list.at( i )->data(0, Qt::UserRole).toString()==name)
			return true;
	}
	return false;
}
//
void ProjectManager::insertFile(QTreeWidgetItem *it, QString filename, bool silentMode)
{
	filename = filename.remove("\"").simplified();
	if( filename.isEmpty() )
		return;
	QString kindFile = filename.toLower().section(".", -1, -1);
	QTreeWidgetItem *newItem;
	if( ( newItem = item(it, filename, Data) ) )
	{
		if( !silentMode)
			QMessageBox::warning(0, 
				"QDevelop", tr("The file already exist."),
				tr("Cancel") 
			);
		//return;
	}
	else
	{
		if( !QString("ui|cpp|h|qrc|ts").contains( kindFile ) )
		{
			if( !silentMode)
				QMessageBox::warning(0, 
					"QDevelop", tr("This file is not permit."),
					tr("Ok") 
				);
			return;
		}
		QString parentKey;
		if( kindFile == "ui" )
			parentKey = "FORMS";
		else if( kindFile == "cpp" )
		{
			parentKey = "SOURCES";
		}
		else if( kindFile == "h" )
		{
			parentKey = "HEADERS";
			
		}
		else if( kindFile == "qrc" )
			parentKey = "RESOURCES";
		else if( kindFile == "ts" )
			parentKey = "TRANSLATIONS";
		// Le file de type "DATA" doit être contenu dans une clé correspondant au type de file
		QTreeWidgetItem *parent = 0;
		for(int i=0; i<it->childCount(); i++)
		{
			if( it->child( i )->data(0, Qt::UserRole).toString() == parentKey )
			{
				parent =  it->child( i );
				break;
			}
		}
		if( !parent )
			parent = insertItem(it, parentKey, parentKey);
		newItem = insertItem(parent, "DATA", filename);
		//
		do {
			m_treeFiles->setItemExpanded(parent, true );
		} while( (parent = parent->parent()) );
	}
	//
	if( kindFile == "cpp" || kindFile == "h")
	{
		QString projectDir = projectDirectory(newItem);
		QString absoluteName = QDir(projectDir+"/"+filename).absolutePath();
		QFile file(absoluteName);
	    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	    {
			QString buffer = file.readAll();
			file.close();
			QStringList parentsList = parents(newItem);
			m_treeClasses->updateClasses(absoluteName, buffer, parentsList, "."+kindFile);
    	}
	}
}
void ProjectManager::slotAddSubProject(QTreeWidgetItem *it)
{
	if( !it )
		it = m_treeFiles->currentItem();
	if( !it )
		it = m_treeFiles->topLevelItem( 0 );
	NewProjectImpl *window = new NewProjectImpl(m_parent, projectDirectory(it->text(0)));
	window->setWindowTitle( tr("Sub-project creation") );
	window->parentProjectName->setText( it->text(0) );
	QString nomAbsoluProjet;
	if( window->exec() == QDialog::Accepted )
	{
		setQmake( projectFilename( it ) );
		QString filename = window->projectName->text();
		if( !filename.toLower().contains( ".pro" ) )
			filename += ".pro";
		QString location = window->location->text();
		location += "/" + filename.left( filename.lastIndexOf(".") );
		nomAbsoluProjet = location + "/" + filename ;
		delete window;
		QDir dir;
		if( !dir.mkdir(location) )
		{
			QMessageBox::warning(0, 
				"QDevelop", tr("Unable to create directory")+" \""+location+"\"",
				tr("Cancel") );
			return;
		}
		QFile file ( nomAbsoluProjet );
		if( file.exists() )
		{
			QMessageBox::warning(0, 
				"QDevelop", tr("The project")+" \"")+nomAbsoluProjet+("\"\n "+tr("already exist on directory."),
				tr("Cancel") );
			return;
		}
		if( !file.open(QIODevice::WriteOnly | QIODevice::Text) )
		{
			QMessageBox::warning(0, 
				"QDevelop", tr("Unable to create project."),
				tr("Cancel") );
			return;
		}
		else
			file.write( "TEMPLATE = app\nQT = gui \\\n  core\nCONFIG += qt \\\n  debug \\\n  warn_on\n");
		file.close();
		bool continuer = true;
		QList<QTreeWidgetItem *> listeTemplate;
		childsList(it, "TEMPLATE", listeTemplate);
		if( listeTemplate.count() )
		{
			for(int nbTemplate=0; nbTemplate < listeTemplate.first()->childCount(); nbTemplate++)
			{
				if( listeTemplate.first()->child(nbTemplate)->text(0) == "subdirs" )
				{
					continuer = false;
				}
			}
		}
		while( continuer )
		{
			continuer = false;
			for(int i=0; i<it->childCount(); i++)
			{
				if( !QString("absoluteNameProjectFile:projectDirectory").contains( it->child( i )->data(0,Qt::UserRole).toString() ) )
				{
					delete it->child( i );
					continuer = true;
				}
			}
		}
		if( !item(it, "TEMPLATE", Key ) )
			insertItem( insertItem(it, "TEMPLATE", "TEMPLATE"), "DATA", "subdirs" );
		QTreeWidgetItem *itSubdir = item(it, "SUBDIRS", Key);
		if( !itSubdir )
			itSubdir = insertItem(it, "SUBDIRS", "SUBDIRS");
		itSubdir->setText(0, tr("Sub-Projects"));
		QTreeWidgetItem *itSousProjet = insertItem(itSubdir, "PROJECT", filename);
    	insertItem(itSousProjet, "qmake", "1");
		insertItem(itSousProjet, "absoluteNameProjectFile", nomAbsoluProjet);
		insertItem(itSousProjet, "projectDirectory", location);
		insertItem(itSousProjet, "subProjectName", filename.left(filename.lastIndexOf(".")));
		QTreeWidgetItem *itTemplate  = insertItem(itSousProjet, "TEMPLATE", "TEMPLATE");
		insertItem(itTemplate, "DATA", "app" );
		QTreeWidgetItem *itQT = insertItem(itSousProjet, "QT", "QT");
		insertItem(itQT, "DATA", "gui" );
		insertItem(itQT, "DATA", "core" );
		QTreeWidgetItem *itConfig = insertItem(itSousProjet, "CONFIG", "CONFIG");
		insertItem(itConfig, "DATA", "qt" );
		insertItem(itConfig, "DATA", "debug" );
		insertItem(itConfig, "DATA", "warn_on" );
	}
	else
		delete window;
}
//
void ProjectManager::slotProjectPropertie(QTreeWidgetItem *it)
{
	if( !it )
		it = m_treeFiles->currentItem();
	if( !it )
		it = m_treeFiles->topLevelItem( 0 );
	QString projectName = projectFilename(it);;
	ProjectPropertieImpl *window = new ProjectPropertieImpl(this, m_treeFiles, it);
	if( window->exec() == QDialog::Accepted )
	{
		m_isModifiedProject = true;
		setQmake(projectName);
	}
	delete window;
}
//
void ProjectManager::slotAddScope(QTreeWidgetItem *it)
{
	if( !it )
		it = m_treeFiles->currentItem();
	if( !it )
		it = m_treeFiles->topLevelItem( 0 );
	QString projectName;
	AddScopeImpl *window = new AddScopeImpl(this);
	
	QList<QTreeWidgetItem *> projectsList;
	childsList(0, "PROJECT", projectsList);
	for(int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
	{
		projectName = projectsList.at(nbProjects)->text(0);
		//projectDir = data(projectName, "projectDirectory");
		QList<QTreeWidgetItem *> listeTemplate;
		bool ajouter = true;
		for(int enfant=0; enfant<projectsList.at(nbProjects)->childCount(); enfant++)
		{
			ajouter = true;
			if( projectsList.at(nbProjects)->child(enfant)->data(0,Qt::UserRole).toString() == "TEMPLATE")
			{
				QTreeWidgetItem *itTemplate = projectsList.at(nbProjects)->child(enfant);
				for(int nbTemplate=0; nbTemplate < itTemplate->childCount(); nbTemplate++)
				{
					if( itTemplate->child(nbTemplate)->text(0) == "subdirs" )
					{
						ajouter = false;
						break;
					}
				}
			}
		}
		if( ajouter )
		{
			//window->comboProjects->addItem( projectName,  QVariant(reinterpret_cast<uint>(projectsList.at(nbProjects))));
			window->comboProjects->addItem( projectName,  addressToVariant(projectsList.at(nbProjects)));
			if( projectsList.at(nbProjects) == it )
				window->comboProjects->setCurrentIndex( window->comboProjects->count()-1);
			QList<QTreeWidgetItem *> listeScope;
			childsList(projectsList.at(nbProjects), "SCOPE", listeScope);
			for(int nbScope=0; nbScope < listeScope.count(); nbScope++)
			{
				QString nomScope;
				QTreeWidgetItem *tmp = listeScope.at(nbScope);
				int nbSpace = 0;
				while( tmp )
				{
					QString cleTmp = tmp->data(0,Qt::UserRole).toString();
					QString indent;
					for(int i=0; i<nbSpace; i++)
						indent += "  ";
					if( cleTmp == "SCOPE" || cleTmp == "PROJECT" )
						nomScope = indent + tmp->text(0) + ":" + nomScope.simplified();
					if( cleTmp == "PROJECT" )
						break;
					tmp = tmp->parent();
					nbSpace++;
				}
				//window->comboProjects->addItem( nomScope, QVariant(reinterpret_cast<uint>(listeScope.at(nbScope))));				
				window->comboProjects->addItem( nomScope, addressToVariant(listeScope.at(nbScope)));
				if( listeScope.at(nbScope) == it )
					window->comboProjects->setCurrentIndex( window->comboProjects->count()-1);
			}
		}
	}
	//
	if( window->comboProjects->count() == 1 )
		window->comboProjects->setEnabled( false );
	QVariant variant;
	QString scopeName;
	if( window->exec() == QDialog::Accepted )
	{
		if( window->win32->isChecked() )
			scopeName = "win32";
		else if( window->Unix->isChecked() )
			scopeName = "unix";
		else if( window->mac->isChecked() )
			scopeName = "mac";
		else if( window->debug->isChecked() )
			scopeName = "debug";
		else if( window->release->isChecked() )
			scopeName = "release";
		else
			scopeName = window->scopeName->text();
		variant = window->comboProjects->itemData( window->comboProjects->currentIndex() );
		delete window;
	}
	else
	{
		delete window;
		return;
	}
	//QTreeWidgetItem *item = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());
	QTreeWidgetItem *item = (QTreeWidgetItem*)variantToItem(variant);
	insertItem(item, "SCOPE", scopeName);
	setQmake( projectFilename(item) );
}
//
void ProjectManager::setQmake(QString projectName)
{ 
	QTreeWidgetItem *itProjet = item(m_treeFiles->topLevelItem ( 0 ), projectName, Data);
	QTreeWidgetItem *itModifie = item(itProjet, "qmake", Key);
	//itModifie->setData(0, Qt::UserRole, QVariant(QString("1")));
	itModifie->setText(0, "1");
	m_isModifiedProject = true;
}
//
bool ProjectManager::qmake(QString projectName) 
{ 
	bool retour = true;
	if( !projectName.isEmpty() )
	{
		retour = findData(projectName, "qmake") == "1";
		QTreeWidgetItem *itProjet = item(m_treeFiles->topLevelItem ( 0 ), projectName, Data);
		//if( itProjet )
		QTreeWidgetItem *itModifie = item(itProjet, "qmake", Key);
		if( itModifie )
			itModifie->setText(0, "0");
	}
	else
	{
		QList<QTreeWidgetItem *> list;
		childsList(m_treeFiles->topLevelItem ( 0 ), "PROJECT", list);
		for(int i=0; i<list.count(); i++)
		{
			QTreeWidgetItem *itModifie = item(list.at( i ), "qmake", Key);
			if( itModifie )
			{
				retour = itModifie->text(0) == "1";
				itModifie->setText(0, "0");
			}
		}
	}
	return retour;
}
//
void ProjectManager::slotlupdate(QTreeWidgetItem *it)
{
	if( m_parent->saveBeforeBuild() )
			slotSaveProject();
	if( isModifiedProject() )
	{
		// Proposer sauvegarde
		int rep = QMessageBox::question(0, "QDevelop", 
			tr("Save project changes ?"), tr("Yes"), tr("No"), tr("Cancel"), 0, 2 );
		if( rep == 2 )
			return;
		if( rep == 0 )
		{
			slotSaveProject();
		}
	}
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QString projectDir = projectDirectory( it );
	QString name = projectName( projectDir );
	QProcess process;
	process.setWorkingDirectory( projectDir );
	process.start("lupdate", QStringList() << "-noobsolete" << name); 
	process.waitForFinished();
    QApplication::restoreOverrideCursor();
}
//
void ProjectManager::slotlrelease(QTreeWidgetItem *it)
{
	if( m_parent->saveBeforeBuild() )
			slotSaveProject();
	if( isModifiedProject() )
	{
		// Proposer sauvegarde
		int rep = QMessageBox::question(0, "QDevelop", 
			tr("Save project changes ?"), tr("Yes"), tr("No"), tr("Cancel"), 0, 2 );
		if( rep == 2 )
			return;
		if( rep == 0 )
		{
			slotSaveProject();
		}
	}
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QString projectDir = projectDirectory( it );
	QString name = projectName( projectDir );
	QProcess process;
	process.setWorkingDirectory( projectDir );
	process.start("lrelease", QStringList(name)); 
	process.waitForFinished();
	setQmake( projectFilename(it) );
    QApplication::restoreOverrideCursor();
}
//
void ProjectManager::slotPreviewForm(QTreeWidgetItem *it)
{
	if( m_previewForm )
	{
		delete m_previewForm;
		m_previewForm = 0;
	}
	QString projectDir = projectDirectory(it);
	QString filename = it->text(0);
	QString uiName = QDir(projectDir+"/"+filename).absolutePath();
    QUiLoader builder;
	QFile file(uiName);
	file.open(QFile::ReadOnly);
	m_previewForm = builder.load(&file, 0);
	m_previewForm->setWhatsThis( filename );
	file.close();
	//
	if( m_previewForm )
	{
		// Set tooltip for all widgets in form
	  	QString name = m_previewForm->objectName();
	  	QString className = m_previewForm->metaObject()->className();        	
	  	m_previewForm->setToolTip( name+" ("+className+")" );
	  	QRect rect = QDesktopWidget().screenGeometry();
	  	m_previewForm->setGeometry((rect.width()-m_previewForm->width())/2, (rect.height()-m_previewForm->height())/2, m_previewForm->width(), m_previewForm->height());
		QList<QWidget *> widgets;
		widgets += m_previewForm->findChildren<QWidget *>();
		foreach(QWidget *w, widgets)
		{
			w->installEventFilter( this );
	       	name = w->objectName();
	       	className = w->metaObject()->className();        	
	       	w->setToolTip( name+" ("+className+")" );
			w->setWhatsThis( filename );
		}
		//
		m_previewForm->installEventFilter( this );
		QDialog *dlg =  dynamic_cast<QDialog*>(m_previewForm);
		QMainWindow *win = dynamic_cast<QMainWindow*>(m_previewForm);
		if( win )
		{
			win->setAttribute(Qt::WA_QuitOnClose, false);
			win->show();
		}
		else if( dlg )
		{
			dlg->show();
		}
		else
		{
			m_previewForm->show();
		}
	}
	else
	{
		QString error = tr("The file")+" \""+uiName+"\" "+tr("");
		QMessageBox::warning(0, 
			"QDevelop", error,
			tr("Cancel") );
	}
}
//
void ProjectManager::slotSubclassing(QTreeWidgetItem *it)
{
	QString projectDir = projectDirectory(it);
	QString projectName = projectFilename( it );
	QString filename = it->text(0);
	QString uiName = QDir(projectDir+"/"+filename).absolutePath();
	QStringList listeHeaders;
	headers( itemProject(projectName), listeHeaders);
	SubclassingImpl *dialog = new SubclassingImpl(0, projectDir, uiName, listeHeaders);
	if( dialog->exec() == QDialog::Accepted )
	{
		while( it->data(0, Qt::UserRole).toString() != "PROJECT" )
			it = it->parent();
		projectDir = projectDirectory( it );
		setQmake( projectFilename(it) );
		QString filename = QDir(projectDir).relativeFilePath(dialog->newFile()+".h").replace("\\", "/");
		insertFile(it, filename, true);
		filename = QDir(projectDir).relativeFilePath(dialog->newFile()+".cpp").replace("\\", "/");
		insertFile(it, filename, true);
	}
	delete dialog;
}
//
void ProjectManager::slotDeleteItem(QTreeWidgetItem *it)
{
	if( !it )
		return;
	if( it == m_treeFiles->topLevelItem ( 0 ) )
	{
		QMessageBox::warning(0, 
			"QDevelop", tr("Unable to delete the main project."),
			tr("Ok") );
		return;
	}
	int rep = QMessageBox::question(0, "QDevelop", 
		tr("Do you want to delete")+" \""+it->text(0)+"\" "+tr("on project ?"), 
		tr("Yes"), tr("No") );
	if( rep == 1 )
		return;
	//
	/*QString filename = it->text(0);
	QString projectName = projectFilename( it );
	QString projectDir = projectDirectory(it);
	QString absoluteName = QDir(projectDir+"/"+filename).absolutePath();
	if( filename.section(".", -1, -1).toLower() == "h" || filename.section(".", -1, -1).toLower() == "cpp")
		m_treeClasses->clearFile( absoluteName );*/
	setQmake( projectFilename( it ) );
	QTreeWidgetItem *parent = it->parent();
	delete it;
	while( !parent->childCount() )
	{
		it = parent->parent();
		delete parent;
		parent = it;
	}
	//
	m_treeClasses->clear();
	parseTreeClasses();
	//
	// Le parent est de type PROJECT. S'il a un TEMPLATE subdirs mais qu'il n'a plus de sous-projets,
	// il doit devenir un projet normal de TEMPLATE app.
	if( parent->data(0, Qt::UserRole).toString() == "PROJECT" )
	{
		bool subdirs = false;
		QTreeWidgetItem *itTemplate = item(parent, "TEMPLATE", Key);
		if( itTemplate )
		{
			QList<QTreeWidgetItem *> list;
			childsList(itTemplate, "DATA", list);
			for(int i=0; i<list.count(); i++)
			{
				//qDebug() << list.at( i )->text(0);
				if( list.at( i )->text(0) == "subdirs" )
					subdirs = true;
			}
		}
		if( subdirs )
		{
			QTreeWidgetItem *itTemplate = item(parent, "TEMPLATE", Key);
			if( itTemplate )
				delete itTemplate;
			itTemplate  = insertItem(parent, "TEMPLATE", "TEMPLATE");
			insertItem(itTemplate, "DATA", "app" );
		}
	}
}
//
	
void ProjectManager::loadProject(QString s, QTreeWidgetItem *newProjectItem)
{
//qDebug() << s;
	QTreeWidgetItem *itemProject = newProjectItem;
	QFile file(s);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
	QString projectName = s.section("/",-1,-1);
	newProjectItem->setText(0, projectName);
	newProjectItem->setData(0, Qt::UserRole, QVariant("PROJECT"));	
	m_treeFiles->setItemExpanded(newProjectItem, true );
	//
	//
	insertItem(newProjectItem, "absoluteNameProjectFile", s);
	QTreeWidgetItem *it; 

	QString projectDirectory = QDir().absoluteFilePath(s).left( QDir().absoluteFilePath(s).lastIndexOf("/") );
	insertItem(newProjectItem, "projectDirectory", projectDirectory);
	insertItem(newProjectItem, "qmake", "1");

	QString key;
	QString data;
	bool toFollow = false;
	bool scope = false;
	while (!file.atEnd()) 
	{
        QString line = QString( file.readLine() );
		if( scope && !toFollow)
		{
			scope = false;
			newProjectItem = newProjectItem->parent();
		}
		//while ( !line.contains(QRegExp("\"")) && !line.contains(QRegExp("/")) && !line.contains(QRegExp("^#")) && line.contains(":") )
		while ( !line.contains(QRegExp("\"")) && !line.contains(QRegExp("/")) && !line.contains(QRegExp("^#")) 
			&& line.indexOf(":")!=-1 && ( line.indexOf(":")<line.indexOf("=") || line.indexOf("=")==-1 ) )
		{
			it = item(newProjectItem, line.section(":", 0, 0).simplified(), Data);
			if( it && it->data(0, Qt::UserRole).toString() == "SCOPE" )
				newProjectItem = it;
			else
				newProjectItem = insertItem(newProjectItem, "SCOPE", line.section(":", 0, 0).simplified());
			scope = true;
//qDebug()<<line;
			line = line.section(":", 1);
//qDebug()<<line;
		}
		if( !line.contains(QRegExp("^#")) && !line.contains(QRegExp("\"")) && !line.contains(QRegExp("/")) && line.contains("{") )
		{
			it = item(newProjectItem, line.section("{", 0, 0).simplified(), Data);
			if( it && it->data(0, Qt::UserRole).toString() == "SCOPE" )
				newProjectItem = it;
			else
				newProjectItem = insertItem(newProjectItem, "SCOPE", line.section("{", 0, 0).simplified());
			toFollow = true;
			continue;
		}
		if( !line.contains(QRegExp("^#")) && !line.contains(QRegExp("\"")) && !line.contains(QRegExp("/")) && line.contains("}") )
		{
			newProjectItem = newProjectItem->parent();
			continue;
		}
		//
		if( !line.contains(QRegExp("^#")) /*&& !line.contains(QRegExp("\""))*/  && line.contains("=") )
		{
			QString operateur;
			if( line.contains("+=") )
				operateur = "+=";
			else 
				operateur = "=";
			key = line.simplified().section(operateur, 0, 0).simplified();
			data =  line.simplified().section(operateur, 1).simplified();
			toFollow = line.section(" ",-1,-1).simplified() == "\\";
			if( data.simplified().right(1) == "\\" )
				data = data.simplified().left( data.simplified().length()-1 );
			QTreeWidgetItem *it = 0;
			for(int i=0; i<newProjectItem->childCount(); i++)
			{
				if( newProjectItem->child( i )->data(0, Qt::UserRole).toString() == key )
				{
					it = newProjectItem->child( i );
					break;
				}
			}
			if( !it )
				it = insertItem(newProjectItem, key, key);
			if( key == "SUBDIRS" )
				it->setText(0, tr("Sub-projects"));
			if( toFollow )
					newProjectItem = it;
			QString name;
			bool guillemet = false;
			data = data.simplified();
			for(int i=0; i<data.length(); i++)
			{
				name += data[i];
				if( data[i] == '\"' )
				{
					if( !guillemet )
						guillemet = true;
					else
					{
						insertItem(it, "DATA", name);
						name = "";
						guillemet = false;
					}
				}
				if(	data[i] == ' ' && !guillemet )
				{
					if( name.simplified().length() )
					{
						insertItem(it, "DATA", name.simplified());
					}
					name = "";
				}
			}
			if( name.simplified().length() )
			{
				insertItem(it, "DATA", name.simplified());
			}
		}
		else if( !line.contains(QRegExp("^#")) && !line.contains(QRegExp("\"")) && toFollow )
		{
			data =  line.simplified().section("\\", 0, 0).simplified();
			foreach(QString name, data.simplified().split(" "))
			{
				insertItem(newProjectItem, "DATA", name);
			}
			toFollow = line.section(" ",-1,-1).simplified() == "\\";
			if( !toFollow )
				newProjectItem = newProjectItem->parent();
		}
		/*else //if ( line.contains(QRegExp("^#")) )
		{
			insertItem(newProjectItem, "TEXTELIBRE", line);
			toFollow = false;
		}*/
    }
    //
    if( findData(projectName, QString("TEMPLATE")).isEmpty() )
    {
		QTreeWidgetItem *tmp = insertItem(itemProject, "TEMPLATE", "TEMPLATE");
		insertItem(tmp, "DATA", "app");
    }
    // On traite les SUBDIRS
	for(int i=0; i<itemProject->childCount(); i++)
	{
		if( itemProject->child( i )->data(0, Qt::UserRole) == "SUBDIRS" )
		{
			QTreeWidgetItem *sub = itemProject->child( i );
			m_treeFiles->setItemExpanded(sub, true );
			for(int n=0; n<sub->childCount(); n++)
			{
				QString name = sub->child(n)->text(0);
				QString subDirName = QDir(projectDirectory+"/"+name).absolutePath();
				QStringList filesList = QDir(subDirName).entryList();
				foreach(QString newProjectName, filesList )
				{
					if( newProjectName.toLower().right(4) == ".pro" )
					{
						loadProject(subDirName+"/"+newProjectName, sub->child( n ));
						insertItem(sub->child( n ), "subProjectName", name);

					}
				}
			}
		}
	}
	return;
}
//
QTreeWidgetItem * ProjectManager::insertItem(QTreeWidgetItem *parent, QString key, QString data)
{
	QTreeWidgetItem *it = new QTreeWidgetItem(parent);
	it->setText(0, data);
	it->setData(0, Qt::UserRole, QVariant(key));
	//it->setToolTip(0, key);
	key = "|"+key+"|";
	QString parentKey = "|"+it->parent()->data(0, Qt::UserRole).toString()+"|";
	if( !QString("|PROJECT|SUBDIRS|SOURCES|SCOPE|HEADERS|FORMS|TRANSLATIONS|RESOURCES|").contains(key) )
		if (key!="|DATA|" || !QString("|PROJECT|SUBDIRS|SOURCES|SCOPE|HEADERS|FORMS|TRANSLATIONS|RESOURCES|").contains(parentKey) )
			m_treeFiles->setItemHidden(it, true);

	if( key == "|HEADERS|" )
	{
		it->setText(0, tr("Headers"));
		it->setIcon(0, QIcon(":/treeview/images/h.png"));
	}
	else if( key == "|SOURCES|" )
	{
		it->setText(0, tr("Sources"));
		it->setIcon(0, QIcon(":/treeview/images/cpp.png"));
	}
	else if( key == "|FORMS|" )
	{
		it->setText(0, tr("Dialogs"));
		it->setIcon(0, QIcon(":/treeview/images/designer.png"));
	}
	else if( key == "|TRANSLATIONS|" )
	{
		it->setText(0, tr("Translations"));
		it->setIcon(0, QIcon(":/treeview/images/linguist.png"));
	}
	else if( key == "|RESOURCES|" )
	{
		it->setText(0, tr("Resources"));
		it->setIcon(0, QIcon(":/treeview/images/qrc.png"));
	}
	QTreeWidgetItem *itemParent = it->parent();
	bool hide = true;
	for(int i = 0; i < itemParent->childCount(); i++ )
		if( !m_treeFiles->isItemHidden(itemParent->child(i) ) )
			hide = false;
	if( hide && itemParent->data(0, Qt::UserRole).toString() != "SCOPE" && itemParent->data(0, Qt::UserRole).toString() != "PROJECT")
		m_treeFiles->setItemHidden(itemParent, true);
	else
	{
		do
		{
			m_treeFiles->setItemHidden(itemParent, false);
		} while( (itemParent = itemParent->parent()) );
			
	}
	return it;
}
//
bool ProjectManager::slotSaveProject()
{
	if( !isModifiedProject() )
		return true;
	QTreeWidgetItem *itemProject = m_treeFiles->topLevelItem ( 0 );
	bool ret = saveDataOfProject(itemProject, new QTextStream());
	if( ret )
	{
		QTreeWidgetItem *it = item(m_treeFiles->topLevelItem ( 0 ), "PROJECT", Key);
		if( !it )
			return true;
		it = item(it, "absoluteNameProjectFile", Key);
		if( !it )
			return true;
	}
	m_isModifiedProject = false;
	return ret;
}
//
bool ProjectManager::saveDataOfProject(QTreeWidgetItem *it, QTextStream *s, int nbSpace, bool toFollow)
{
	QTextStream *output = s;
	QString key = it->data(0, Qt::UserRole).toString();
	QString data = it->text(0);
	QString indent;
	for(int x=0; x<nbSpace; x++)
		indent += "  ";
	QFile file;
	if( (key == "PROJECT" && it != m_treeFiles->topLevelItem ( 0 ) ) )
	{
		QString subProjectName = item(it, "subProjectName", Key)->text(0);
		*output << subProjectName;
		if( toFollow )
			*output << " \\\n";
		else
			*output << "\n";
	}
	if( key == "PROJECT" )
	{
		nbSpace = -1;
		QString absoluteName = item(it, "absoluteNameProjectFile", Key)->text(0);
		file.setFileName(absoluteName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return false;
		output = new QTextStream(&file);
	}
	else if ( key == "SCOPE" )
	{
		*output << indent+data << " {" << "\n";
		if( !it->childCount() )
		{
			nbSpace--;
			for(int x=0; x<nbSpace; x++)
				indent += "  ";
			*output << indent + "}" << "\n";
		}
	}
	else if ( key == "DATA" )
	{
		*output << indent+data;
		if( toFollow )
			*output << " \\\n";
		else
			*output << "\n";
	}
	else if( !QString("absoluteNameProjectFile:projectDirectory:subProjectName:qmake").contains(key) )
	{
		if( key == "CONFIG" || key == "QT" )
			*output << indent+key << " += ";
		else
			*output << indent+key << " = ";
        if( !it->childCount() )
            *output << "\n";
	}
   	for( int i=0; i<it->childCount(); i++)
   	{
   		saveDataOfProject(it->child(i), output, nbSpace+1, !(i+1==it->childCount()));
   	}
	if( it->childCount() && key == "SCOPE" )
	{
		*output << indent+"}" << "\n";
	}
	return true;
}
//
QString ProjectManager::projectDirectory(QTreeWidgetItem *it)
{
	QTreeWidgetItem *tmp = it;
	do
	{
		if( tmp->text(0).toLower().right(4) == ".pro" )
		{
			for(int i=0; i<tmp->childCount(); i++ )
				if( tmp->child( i )->data(0, Qt::UserRole).toString() == "projectDirectory" )
					return tmp->child( i )->text(0);
		}
	} while( (tmp = tmp->parent()) );
	return QString();
}
//
QString ProjectManager::projectDirectory(QString projectName)
{
	QString rep = findData(projectName, "projectDirectory");
//qDebug()<< rep;
	return rep;
}
//
QString ProjectManager::fileDirectory(QString absoluteFilename)
{
	// Renvoie le repertoire du projet auquel appartient le file nommé absoluteFilename
	QList<QTreeWidgetItem *> projectsList;
	childsList(0, "PROJECT", projectsList);
	for(int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
	{
		QString projectDir = findData(projectsList.at(nbProjects)->text(0), "projectDirectory");
		QList<QTreeWidgetItem *> dirSourcesList;
		childsList(projectsList.at(nbProjects), "SOURCES", dirSourcesList);
		for(int nbSrc=0; nbSrc < dirSourcesList.count(); nbSrc++)
		{
			QList<QTreeWidgetItem *> filesList;
			childsList(dirSourcesList.at(nbSrc), "DATA", filesList);
			for(int nbFic=0; nbFic < filesList.count(); nbFic++)
			{
				QString name = filesList.at(nbFic)->text(0);
				if( QDir().absoluteFilePath(projectDir + "/" + name) == absoluteFilename )
					return projectDir;
			}
		}
	}
	return QString();
}
//
QString ProjectManager::projectName(QString location)
{
	// Renvoie le name du projet présent dans location
	QList<QTreeWidgetItem *> projectsList;
	childsList(0, "PROJECT", projectsList);
	for(int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
	{
		QString name = projectsList.at(nbProjects)->text(0);
		if( findData(name, "projectDirectory") == location )
			return name;
	}
	return QString();
}
//
QString ProjectManager::absoluteNameProjectFile(QTreeWidgetItem *it)
{
	// Renvoie le name absolu du file projet auquel appartient le file sélectionné dans le treeview
	// Nom au format /home/jl/monprojet/monprojet.pro ou C:/jl/monprojet/monprojet.pro
	QTreeWidgetItem *tmp = item(it, "PROJECT", Key);
	if( !tmp )
		return "";
	QString projectName = tmp->text(0);
	return findData(projectName, "projectDirectory")+"/"+projectName;
}
//
QString ProjectManager::findData(QString projectName, QString key)
{
//qDebug()<<projectName<<key;
	QTreeWidgetItem *it = item(0, projectName, Data);
	if( !it )
		return QString();
	it = item(it, key, Key);
	if( it )
		return it->text(0);
	return QString();
}
//
QString ProjectManager::projectFilename(QTreeWidgetItem *it)
{
	// Renvoie le name court du file projet auquel appartient le file sélectionné dans le treeview
	// Nom au format monprojet.pro
	QTreeWidgetItem *tmp = it;
	do
	{
		if( tmp->text(0).toLower().right(4) == ".pro" )
		{
			return tmp->text(0);
		}
	} while( (tmp = tmp->parent()) );
	return QString();
}
//
QTreeWidgetItem *ProjectManager::item(QTreeWidgetItem *begin, QString name, findMode type)
{
	// Renvoie le premier item correspondant à la recherche
	if( begin == 0 )
		begin = m_treeFiles->topLevelItem( 0 );
	if( (type==Data && begin->text(0)==name) )
		return begin;
	else if (type==Key && begin->data(0, Qt::UserRole).toString()==name)
		return begin;
	else
	{
		// Les enfants
		for(int i=0; i<begin->childCount(); i++)
		{
			if( (type==Data && begin->child( i )->text(0)==name) )
				return begin->child( i );
			else if (type==Key && begin->child( i )->data(0, Qt::UserRole).toString()==name)
				return begin->child( i );
		}
		// Les petits-enfants
		for(int i=0; i<begin->childCount(); i++)
		{
			QTreeWidgetItem *retour = item(begin->child( i ), name, type);
			if( retour )
				return retour;
		}
	}
	return 0;
}
//
QTreeWidgetItem *ProjectManager::itemProject(QString projectName)
{
	QList<QTreeWidgetItem *> projectsList;
	childsList(0, "PROJECT", projectsList);
	for(int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
	{
		if( projectName == projectsList.at(nbProjects)->text(0) )
			return projectsList.at(nbProjects);
	}
	return 0;
}
//
void ProjectManager::childsList(QTreeWidgetItem *begin, QString key, QList<QTreeWidgetItem *> &list)
{
	// Renvoie la list enfants correspondants à la clé
	if( begin == 0 )
		begin = m_treeFiles->topLevelItem( 0 );
	if( begin->data(0, Qt::UserRole) == key )
		list.append(begin);
	for(int i=0; i<begin->childCount(); i++)
	{
		childsList(begin->child(i), key, list);
	}
	return;
}
//
QStringList ProjectManager::buildableProjectsDirectories()
{
	QStringList list;
	QList<QTreeWidgetItem *> projectsList;
	childsList(0, "PROJECT", projectsList);
	for(int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
	{
		bool ajouter = false;
		QString projectName = projectsList.at(nbProjects)->text(0);
		QString projectDir = findData(projectName, "projectDirectory");
		//QList<QTreeWidgetItem *> listeTemplate;
		//childsList(projectsList.at(nbProjects), "TEMPLATE", listeTemplate);
		QTreeWidgetItem *itemTemplate = item(projectsList.at(nbProjects), "TEMPLATE", Key);
		ajouter = true;
		if( itemTemplate )
		{
			for(int nbTemplate=0; nbTemplate < itemTemplate->childCount(); nbTemplate++)
			{
				if( itemTemplate->child(nbTemplate)->text(0) == "subdirs" )
				{
					ajouter = false;
				}
			}
		}
		if( ajouter )
		{
			list << projectDir;
		}
	}
	return list;
}
//
void ProjectManager::parseFile(QString file)
{
}
//
QString ProjectManager::projectVersion(QTreeWidgetItem *it)
{
	QTreeWidgetItem *itConfig = 0;
	QTreeWidgetItem *itTemplate = 0;
	for(int i=0; i<it->childCount(); i++)
	{
		QTreeWidgetItem *item = it->child( i );
		QString key = item->data(0, Qt::UserRole).toString();
		if( key == "CONFIG" )
		{
			itConfig = item;
		}
		if( key == "TEMPLATE" )
		{
			itTemplate = item;
		}
	}
	if( itTemplate )
	{
		for(int i=0; i<itTemplate->childCount(); i++)
		{
			QTreeWidgetItem *item = itTemplate->child( i );
			QString data = item->text(0);
			if( data == "lib" || data == "subdirs" )
			{
				return QString();
			}
		}
	}
	if( itConfig )
	{
		for(int i=0; i<itConfig->childCount(); i++)
		{
			QTreeWidgetItem *item = itConfig->child( i );
			QString data = item->text(0);
			if( data == "debug" || data == "release" || data == "debug_and_release" )
			{
				return data;
			}
		}
	}
	return "debug";
}
//
QString ProjectManager::executableName(QString preferedVersion)
{
	QStringList m_projectDirectoryOfExecutableList;
	if( !m_executablesList.count() )
	{
		QList<QTreeWidgetItem *> projectsList;
		childsList(0, "PROJECT", projectsList);
		for(int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
		{
			QTreeWidgetItem *it = projectsList.at(nbProjects);
			QString projectName = it->text(0);
			QString projectDir = projectDirectory( it );
			QString realVersion = projectVersion( it );
			if( realVersion.toLower() == "debug_and_release" )
				realVersion = preferedVersion;
			else if( realVersion != preferedVersion && realVersion.length() )
			{
				QMessageBox::warning(0, 
					"QDevelop", tr("The only available version for")+" \""+projectName+"\" "+tr("is") + " " +realVersion,
					tr("Run on")+" "+realVersion );
			}
			if( realVersion.toLower() == "release" )
				m_releaseVersion = true;
			else
				m_releaseVersion = false;
			QString name = findExecutable(projectDir, realVersion);
			if( !name.isNull() )
			{
				m_projectDirectoryOfExecutableList << projectDir;
				m_executablesList << name;
			}
		}
	}
	if( m_executablesList.count() == 1 )
	{
		QString choice = m_executablesList.first();
		m_executablesList.clear();
		m_projectDirectoryOfExecutable = m_projectDirectoryOfExecutableList.first();
		return choice;
	}
	else if( m_executablesList.count() == 0 )
		return QString();
	else
	{	
		QString choice;
		QDialog *window = new QDialog;
		Ui::ExeChoice ui;
		ui.setupUi(window);
		foreach(QString name, m_executablesList )
		{
			if( name.length() < 50 )
				ui.list->addItem( name );
			else
				ui.list->addItem( "... "+name.right(50) );
		}
		ui.list->setCurrentRow( 0 );
		if( window->exec() == QDialog::Accepted )
		{
			int line = ui.list->currentRow();
			choice = m_executablesList.at(line);
			if( ui.dontask->isChecked() )
			{
				m_executablesList.clear();
				m_executablesList << choice;
				m_projectDirectoryOfExecutable = m_projectDirectoryOfExecutableList.at(line);
			}
		}
		delete window;
		return choice;
	}
	return QString();
}
//
void ProjectManager::headers(QTreeWidgetItem *it, QStringList &headerFiles)
{
	if( !it )
		return;
	QString projectDir = projectDirectory( it );
	for(int i=0; i<it->childCount(); i++)
	{
		if( it->child(i)->data(0, Qt::UserRole).toString() == "HEADERS" )
		{
			for(int j=0; j<it->child(i)->childCount(); j++)
			{
				headerFiles += projectDir+"/"+it->child(i)->child(j)->text(0);
			}
		}
		else if( it->child(i)->data(0, Qt::UserRole).toString() == "SCOPE" )
		{
			headers(it->child(i), headerFiles);
		}
	}
	return;
}
//
void ProjectManager::sources(QTreeWidgetItem *it, QStringList &sourcesFiles)
{
	if( !it )
		return;
	QString projectDir = projectDirectory( it );
	for(int i=0; i<it->childCount(); i++)
	{
		QTreeWidgetItem *itChild = it->child(i);
		if( it->child(i)->data(0, Qt::UserRole).toString() == "SOURCES" )
		{
			for(int j=0; j<itChild->childCount(); j++)
			{
				sourcesFiles += projectDir+"/"+itChild->child(j)->text(0);
			}
		}
		else if( it->child(i)->data(0, Qt::UserRole).toString() == "SCOPE" )
		{
			sources(it->child(i), sourcesFiles);
		}
	}
	return;
}
//
QString ProjectManager::findExecutable( QString projectDirectory, QString preferedVersion )
{
	// Find on Makefile, Makefile.Debug or Makefile.Release the name of executable
	// then return it if the suffixe is not .so, .dll or .a
	QString fichierMakefile, cible, repertoireDest, nomCompletDest;
	QString exeName;
	QString line;
	QFile makefile(projectDirectory+"/"+"Makefile");
	if (!makefile.open(QIODevice::ReadOnly | QIODevice::Text))
		return QString();
	while (!makefile.atEnd()) 
	{
		line = QString( makefile.readLine() );
		// Partie concernant le file Makefile appelant sous Windows Makefile.Debug ou Makefile.Release.
		// Sans objet sous Linux
		if( line.contains(" ") && line.section(" ", 0, 0).simplified() == "first:" && (line.section(" ", 1, 1).simplified()=="all" ))
			cible = preferedVersion+"-all";
		else if( line.contains(" ") && line.section(" ", 0, 0).simplified() == "first:" && (line.section(" ", 1, 1).simplified()=="debug" || line.section(" ", 1, 1).simplified()=="release"))
			cible = line.section(" ", 1, 1).simplified();
		if( line.contains("=") && line.section("=", 0, 0).simplified() == "MAKEFILE" )
			fichierMakefile = line.section("=", 1, 1).simplified();
		if( !cible.isNull() && line.section(":", 0, 0) == cible )
		{
			fichierMakefile = line.section(" ", 1, 1).simplified().replace("$(MAKEFILE)", fichierMakefile);
			makefile.close();
			makefile.setFileName( projectDirectory+"/"+fichierMakefile );
			if (!makefile.open(QIODevice::ReadOnly | QIODevice::Text))
				return QString();
			cible = QString();
			continue;
		}
		// Partie commune à Win et Linux 
		// TARGET is the good variable to find exe on Linux 
		if( line.contains("=") && line.section("=", 0, 0).simplified() == "TARGET" )
		{
			QString exe = line.section("=", 1, 1).simplified();
			int pos = exe.indexOf("#");
			if( pos > -1 )
				exe = exe.left(pos);
			exe = QDir::cleanPath(projectDirectory + "/" + exe );
			if( !QString("so:dll:a").contains(exe.section(".", -1, -1).toLower() ) && QDir().exists(exe) )
				exeName = exe;
		}
		// DESTDIR_TARGET is only present on MinGW Makefile
		if( line.contains("=") && line.section("=", 0, 0).simplified() == "DESTDIR_TARGET" )
		{
			QString exe = line.section("=", 1, 1).simplified();
			int pos = exe.indexOf("#");
			if( pos > -1 )
				exe = exe.left(pos);
			exe = QDir::cleanPath(projectDirectory + "/" + exe );
			if( !QString("so:dll:a").contains(exe.section(".", -1, -1).toLower() ) && QDir().exists(exe) )
				exeName = exe;
		}
	}
	makefile.close();
	return exeName;
}
//
void ProjectManager::slotSort()
{
	m_treeFiles->sortItems(0, Qt::AscendingOrder);
	m_isModifiedProject = true;
	return;
}
//
bool ProjectManager::eventFilter( QObject *obj, QEvent *ev )
{
	if( obj == m_previewForm )
	{
		if ( ev->type() == QEvent::Close ) 
	    {
	    	if( m_previewForm )
	    	{
	    		//qDebug()<<"DeleteLater "+obj->objectName();
				m_previewForm->deleteLater();		
				m_previewForm = 0;
    		}
			return true;
        }
	    else 
	    {
			return false;
        }
	}
	else
		return QObject::eventFilter(obj,ev);	
}
//
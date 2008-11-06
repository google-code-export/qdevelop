/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2007  Jean-Luc Biord
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

#include "misc.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QDir>

#ifdef Q_OS_WIN32
#include <shlobj.h>
#endif

//
QVariant addressToVariant(void *it ) 
{
#if QT_POINTER_SIZE == 4
	return QVariant( reinterpret_cast<uint>(it)); 
#else
	return QVariant( reinterpret_cast<qulonglong>(it)); 
#endif
}
//
QTreeWidgetItem* variantToItem( QVariant variant )
{
#if QT_POINTER_SIZE == 4
		return (QTreeWidgetItem*)variant.toUInt();
#else
	return (QTreeWidgetItem*)variant.toULongLong();
#endif
}
//
QAction* variantToAction( QVariant variant )
{
#if QT_POINTER_SIZE == 4
	return (QAction*)variant.toUInt();
#else
	return (QAction*)variant.toULongLong();
#endif
}
//
bool connectDB(QString const& dbName)
{
	QSqlDatabase database;
	if( QSqlDatabase::database(dbName).databaseName() != dbName )
	{
		database = QSqlDatabase::addDatabase("QSQLITE", dbName);
		database.setDatabaseName(dbName);
	}
	else
	{
		database = QSqlDatabase::database(dbName);
		if ( database.isOpen() )
			return true;
	}
	//
    if (!database.open()) {
        QMessageBox::critical(0, "QDevelop",
            QObject::tr("Unable to establish a database connection.")+"\n"+
                     QObject::tr("QDevelop needs SQLite support. Please read "
                     "the Qt SQL driver documentation for information how "
                     "to build it."), QMessageBox::Cancel,
                     QMessageBox::NoButton);
        return false;
    }
	else
	{
		QString queryString = "create table classesbrowser ("
		    "text string,"
		    "tooltip string,"
		    "icon string,"
		    "key string,"
		    "parents string,"
		    "name string,"
		    "implementation string,"
		    "declaration string,"
		    "ex_cmd string,"
		    "language string,"
		    "classname string,"
		    "structname string,"
		    "enumname string,"
		    "access string,"
		    "signature string,"
		    "kind string"
		    ")";
		
		QSqlQuery query(database);
		query.exec(queryString);
		queryString = "create table editors ("
		    "filename string,"
		    "scrollbar int,"
		    "numline int"
		    ")";
		query.exec(queryString);
		//
		queryString = "create table bookmarks ("
		    "filename string,"
		    "numline int"
		    ")";
		query.exec(queryString);
		//
		queryString = "create table breakpoints ("
		    "filename string,"
		    "numline int,"
		    "breakpointCondition string,"
		    "isTrue int"
		    ")";
		query.exec(queryString);
		//
		queryString = "create table projectsDirectories ("
		    "projectName string,"
		    "srcDirectory string,"
		    "uiDirectory string"
		    ")";
		query.exec(queryString);
		//
		queryString = "create table config ("
		    "key string,"
		    "value string"
		    ")";
		query.exec(queryString);
		//
		queryString = "create table checksums ("
		    "filename string,"
		    "checksum int"
		    ")";
		query.exec(queryString);
		//
    }
    return true;
}
//

//static function to determine the QDevelop directory (used for settings and global ctags database)
// the directory is returnded WITH a trailing slash
QString getQDevelopPath(void)
{
	static QString path;
	if (!path.isEmpty()) return path;
	
	// if we havn't yet done so, determine the full db file name and make sure the directory exists
	// determine path for application data dirs
#ifdef Q_OS_WIN32
	wchar_t buf[MAX_PATH];
	if (!SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, buf))
		path = QString::fromUtf16((ushort *)buf)+"/";
	else
		path = QDir::homePath()+"/Application Data/"; // this shouldn't happen
#else
	path = QDir::homePath()+"/";
#endif

	// create subdir
	QDir dir(path);
#ifdef Q_OS_WIN32
	dir.mkdir("QDevelop");
	path += "QDevelop/";
#else
	dir.mkdir(".qdevelop");
	path += ".qdevelop/";
#endif
	return path;
}
//
QString simplifiedText(const QString string)
{
    QString m_fileString = string;
    enum CodeState
    {
        NORMAL,
        NORMAL_GOT_SLASH,
        COMMENT_SINGLE_LINE,
        COMMENT_MULTI_LINE,
        COMMENT_GOT_STAR,
        COMMENT_GOT_ESCAPE,
        STRING,
        STRING_GOT_ESCAPE,
        CHAR,
        CHAR_GOT_ESCAPE
    } state = NORMAL;
    
    QChar c;
    int i;
    bool isAfterSharp = false;
    for (i = 0; i < m_fileString.count(); i++)
    {
        c = m_fileString.at(i);
        switch (state)
        {
            case NORMAL:
                if (c == '/')
                {
                    state = NORMAL_GOT_SLASH;
                    break;
                }
                else if (c == '"' )
                {
                    state = STRING;
                }
                else if (c == '\'' )
                {
                    state = CHAR;
                }
                else if (c == '#' )
                {
                    isAfterSharp = true;
                }
                else if (c == QChar('\n') )
                {
                    isAfterSharp = false;
                }
                break;
            case NORMAL_GOT_SLASH:
                if (c == '/')
                {
                    state = COMMENT_SINGLE_LINE;
	                m_fileString[i-1] = m_fileString[i] = ' ';
                }
                else if (c == '*')
                {
                    state = COMMENT_MULTI_LINE;
	                m_fileString[i-1] = m_fileString[i] = ' ';
                }
                else
                {
                    state = NORMAL;
                }
                isAfterSharp = false;
                break;
            case COMMENT_SINGLE_LINE:
                if (c == '\n')
                {
                    state = NORMAL;
                }
                else if (c == '\\')
                {
                    state = COMMENT_GOT_ESCAPE;
                	m_fileString[i] = ' ';
                }
                else
                	m_fileString[i] = ' ';
                isAfterSharp = false;
                break;
            case COMMENT_MULTI_LINE:
                if (c == '*')
                {
                    state = COMMENT_GOT_STAR;
                }
                m_fileString[i] = ' ';
                isAfterSharp = false;
                break;
            case COMMENT_GOT_STAR:
                if (c == '*')
                    state = COMMENT_GOT_STAR;
                else if (c == '/')
                    state = NORMAL;
                else
                    state = COMMENT_MULTI_LINE;
                m_fileString[i] = ' ';
                isAfterSharp = false;
                break;
            case COMMENT_GOT_ESCAPE:
                state = COMMENT_SINGLE_LINE;
                m_fileString[i] = ' ';
                break;
            case STRING:
                if (c == '\\')
                {
                    state = STRING_GOT_ESCAPE;
	                m_fileString[i] = ' ';
               	}
                else if (c == '"')
                {
                    state = NORMAL;
                }
                else if( isAfterSharp  )
	                m_fileString[i] = c;
                else
	                m_fileString[i] = ' ';
                break;
            case CHAR:
                if (c == '\\')
                {
                    state = CHAR_GOT_ESCAPE;
	                m_fileString[i] = ' ';
                	
               	}
                else if (c == '\'')
                {
                    state = NORMAL;
                }
                else
	                m_fileString[i] = ' ';
                break;
            case STRING_GOT_ESCAPE:
                state = STRING;
                m_fileString[i] = ' ';
                break;
            case CHAR_GOT_ESCAPE:
                state = CHAR;
                m_fileString[i] = ' ';
                break;
        }
    }
    return m_fileString;
}

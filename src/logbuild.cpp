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
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://biord-software.org/qdevelop/
*
*/

#include "logbuild.h"
#include "mainimpl.h"
#include <QDir>
#include <QDebug>
//
LogBuild::LogBuild( QWidget * parent )
        : QTextEdit(parent)
{}
//
void LogBuild::mouseDoubleClickEvent( QMouseEvent * /*event*/ )
{
    // First highlight the line double-clicked
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    setTextCursor( cursor );
    //
    BlockLogBuild *blockUserData = (BlockLogBuild*)cursor.block().userData();
    // If blockUserData is null, the line doesn't contains "error" or "warning", we quit this function.
    if ( !blockUserData )
        return;
    QString projectDirectory = blockUserData->directory();
    QString text = cursor.block().text();
    QString filename;
    uint numLine;
    if ( !containsError(text, filename, numLine) && !containsWarning(text, filename, numLine) )
        return;
    if ( numLine == 0 )
        return;
    QString absoluteName = QDir(projectDirectory).absoluteFilePath(filename);
    m_mainImpl->openFile( QStringList( absoluteName ), numLine);
}
//
void LogBuild::slotMessagesBuild(QString list, QString directory)
{
    foreach(QString message, list.split("\n"))
    {
        if ( !message.isEmpty() )
        {
            message.remove( "\r" );
            setTextColor( Qt::black );
            QString fileName;
            uint line;
            if ( containsError(message,fileName,line) )
            {
                setTextColor( Qt::red );
                m_mainImpl->resetProjectsDirectoriesList();
                m_mainImpl->resetDebugAfterBuild();
                emit incErrors();
            }
            else if ( containsWarning(message,fileName,line) )
            {
                setTextColor( Qt::blue );
                emit incWarnings();
            }
            append( message );
            if ( !directory.isEmpty() )
            {
                QTextCursor cursor = textCursor();
                BlockLogBuild *blockUserData = new BlockLogBuild(directory);
                cursor.block().setUserData( blockUserData );
                setTextCursor( cursor );
            }
        }
    }
    // Move the cursor to the bottom. Ensure it is visible.
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    setTextCursor( cursor );
}
//
#define ERR_EXP "^(.+):\\s*(\\d+)\\s*:\\s*(error|%1|undefined reference to|%2)"
#define WARN_EXP "^(.+):\\s*(\\d+)\\s*:\\s*(warning|%1)"
/*  If your language is not translated in QDevelop and if g++ display the errors and warnings in your language, 
modify the two strings below "error" and "warning" to adapt in your language. Also have a look at editor.cpp*/
bool LogBuild::containsError(QString message, QString & file, uint & line)
{
	QRegExp exp( QString(ERR_EXP).arg(tr("error", "Compiler message").toLower()).arg(tr("undefined reference to", "Linker message").toLower()) );
    bool result = !message.startsWith("make") && !message.contains("------") && exp.indexIn(message) > -1;
    if (result)
    {
    	Q_ASSERT(exp.capturedTexts().size() > 3);
    	file = exp.cap(1);
    	line = exp.cap(2).toUInt();
   	}
   	return result;
}

bool LogBuild::containsWarning(QString message, QString & file, uint & line)
{
	QRegExp exp( QString(WARN_EXP).arg(tr("warning", "Compiler message").toLower()) );
    bool result = !message.startsWith("make") && !message.contains("------") && exp.indexIn(message) > -1;
    if (result)
    {
    	Q_ASSERT(exp.capturedTexts().size() > 3);
    	file = exp.cap(1);
    	line = exp.cap(2).toUInt();
   	}
   	return result;
}
//
